// Finding the user's game data when the game starts without a game directory (game_dir.h): every candidate is checked
// for the files the start-up reads, the first complete one wins and is logged with its source, every refused one is
// listed with the file it lacks. `tutankhamun --find-game-dir` runs the lookup alone on given sources, read only.
// On Windows the Steam and GOG installs come from the registry and the picker is the shell's folder dialog; elsewhere
// Steam is looked for in its usual folders (native, Flatpak, Snap), GOG in the usual folders of its Linux installers,
// Heroic and Wine, and the picker is zenity or kdialog when one of them is installed.

#include "app/game_dir.h"

#include "app/app_files.h"
#include "platform/log.h"
#include "platform/platform_dialog.h"
#include "platform/platform_file.h"

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GAME_DIR_MAX_TRIED 64
#define GAME_DIR_MAX_LIBRARIES 32
#define GAME_DIR_VDF_MAX_BYTES (1024 * 1024)
#define GAME_DIR_INI_SECTION "game"
#if defined(_WIN32)
#define STEAM_NOT_FOUND "no SteamPath in HKCU\\SOFTWARE\\Valve\\Steam"
#define GOG_NOT_FOUND "no game with a path in HKLM\\SOFTWARE\\GOG.com\\Games"
#else
#define STEAM_NOT_FOUND "no steamapps in ~/.steam/steam, ~/.local/share/Steam, the Flatpak or the Snap Steam"
#define GOG_NOT_FOUND "no game in ~/GOG Games, ~/Games/Heroic or the Wine prefix ~/.wine"
#endif
#define GAME_DIR_INI_KEY "directory"
#if defined(_WIN32)
#define STEAM_GAME_FOLDER "steamapps\\common\\Pharaoh + Cleopatra"
#define STEAM_LIBRARY_FILE "steamapps\\libraryfolders.vdf"
#define PATH_SEPARATOR "\\"
#else
#define STEAM_GAME_FOLDER "steamapps/common/Pharaoh + Cleopatra"
#define STEAM_LIBRARY_FILE "steamapps/libraryfolders.vdf"
#define PATH_SEPARATOR "/"
#endif

static const char *const required_files[] = { "campaign.txt", "Pharaoh_Text.eng", "Pharaoh_MM.eng" };
static const char *const cleopatra_files[] = { "Data\\Expansion.sg3", "Data\\SprMain2.sg3" };

typedef struct {
    game_dir_choice *choice;
    char *tried;
    size_t tried_size;
    size_t tried_used;
    char seen[GAME_DIR_MAX_TRIED][GAME_DIR_PATH_SIZE];
    int32_t seen_count;
} game_dir_search;

typedef struct {
    char paths[GAME_DIR_MAX_LIBRARIES][GAME_DIR_PATH_SIZE];
    int32_t count;
} steam_libraries;

static int32_t join_path(char *out, const char *dir, const char *name)
{
    size_t dir_length = strlen(dir);
    int32_t needs_slash = dir_length > 0 && dir[dir_length - 1] != '\\' && dir[dir_length - 1] != '/';
    int32_t written = snprintf(out, GAME_DIR_PATH_SIZE, "%s%s%s", dir, needs_slash ? PATH_SEPARATOR : "", name);
    return written > 0 && written < GAME_DIR_PATH_SIZE;
}

static void copy_text(char *out, const char *text)
{
    snprintf(out, GAME_DIR_PATH_SIZE, "%s", text);
}

static int32_t path_is_directory(const char *path)
{
    return platform_dir_exists(path);
}

static int32_t path_is_file(const char *path)
{
    return platform_file_exists(path) && !platform_dir_exists(path);
}

static int32_t dir_has_file(const char *dir, const char *name)
{
    char path[GAME_DIR_PATH_SIZE];
    return join_path(path, dir, name) && path_is_file(path);
}

const char *game_dir_missing_file(const char *dir)
{
    char data[GAME_DIR_PATH_SIZE];
    if (!dir || !*dir || !path_is_directory(dir)) {
        return "such folder";
    }
    if (!join_path(data, dir, "Data") || !path_is_directory(data)) {
        return "Data folder";
    }
    for (size_t i = 0; i < sizeof(required_files) / sizeof(required_files[0]); i++) {
        if (!dir_has_file(dir, required_files[i])) {
            return required_files[i];
        }
    }
    for (size_t i = 0; i < sizeof(cleopatra_files) / sizeof(cleopatra_files[0]); i++) {
        if (dir_has_file(dir, cleopatra_files[i])) {
            return 0;
        }
    }
    return "Data\\Expansion.sg3 or Data\\SprMain2.sg3 (Cleopatra)";
}

#if defined(_WIN32)
static void normalize_path(char *out, const char *path)
{
    char slashed[GAME_DIR_PATH_SIZE];
    copy_text(slashed, path);
    for (char *c = slashed; *c; c++) {
        if (*c == '/') {
            *c = '\\';
        }
    }
    DWORD length = GetFullPathNameA(slashed, GAME_DIR_PATH_SIZE, out, 0);
    if (length == 0 || length >= GAME_DIR_PATH_SIZE) {
        copy_text(out, slashed);
        length = (DWORD) strlen(out);
    }
    int32_t is_drive_root = length == 3 && out[1] == ':';
    if (length > 1 && out[length - 1] == '\\' && !is_drive_root) {
        out[length - 1] = 0;
    }
}
#else
static void normalize_path(char *out, const char *path)
{
    platform_path_absolute(path, out, GAME_DIR_PATH_SIZE);
    size_t length = strlen(out);
    while (length > 2 && out[length - 1] == '.' && out[length - 2] == '/') {
        out[length - 2] = 0;
        length -= 2;
    }
    if (length > 1 && out[length - 1] == '/') {
        out[length - 1] = 0;
    }
}
#endif

static void search_note(game_dir_search *search, const char *format, ...)
{
    if (search->tried_used + 1 >= search->tried_size) {
        return;
    }
    va_list args;
    va_start(args, format);
    int32_t written = vsnprintf(search->tried + search->tried_used, search->tried_size - search->tried_used, format, args);
    va_end(args);
    if (written > 0) {
        size_t room = search->tried_size - search->tried_used - 1;
        search->tried_used += (size_t) written < room ? (size_t) written : room;
    }
}

static int32_t search_seen(game_dir_search *search, const char *dir)
{
    for (int32_t i = 0; i < search->seen_count; i++) {
        if (platform_text_compare_nocase(search->seen[i], dir) == 0) {
            return 1;
        }
    }
    if (search->seen_count < GAME_DIR_MAX_TRIED) {
        copy_text(search->seen[search->seen_count++], dir);
    }
    return 0;
}

static int32_t try_candidate(game_dir_search *search, const char *source, const char *dir, int32_t remember)
{
    char full[GAME_DIR_PATH_SIZE];
    normalize_path(full, dir);
    if (search_seen(search, full)) {
        return 0;
    }
    const char *missing = game_dir_missing_file(full);
    if (missing) {
        search_note(search, "  %s: %s (no %s)\n", source, full, missing);
        platform_log("port: game directory candidate %s (%s) refused: no %s", full, source, missing);
        return 0;
    }
    copy_text(search->choice->dir, full);
    search->choice->reason = source;
    search->choice->remember = remember;
    platform_log("port: game directory %s chosen: %s", full, source);
    return 1;
}

static char *read_whole_file(const char *path)
{
    FILE *file = platform_fopen(path, "rb");
    if (!file) {
        return 0;
    }
    char *text = malloc(GAME_DIR_VDF_MAX_BYTES + 1);
    size_t length = text ? fread(text, 1, GAME_DIR_VDF_MAX_BYTES, file) : 0;
    fclose(file);
    if (text) {
        text[length] = 0;
    }
    return text;
}

static const char *vdf_next_token(const char *at, char *token, int32_t *is_string)
{
    while (*at && *at != '"' && *at != '{' && *at != '}') {
        at++;
    }
    if (!*at) {
        return 0;
    }
    if (*at != '"') {
        token[0] = *at;
        token[1] = 0;
        *is_string = 0;
        return at + 1;
    }
    size_t length = 0;
    for (at++; *at && *at != '"'; at++) {
        if (*at == '\\' && (at[1] == '\\' || at[1] == '"')) {
            at++;
        }
        if (length + 1 < GAME_DIR_PATH_SIZE) {
            token[length++] = *at;
        }
    }
    token[length] = 0;
    *is_string = 1;
    return *at ? at + 1 : at;
}

static int32_t vdf_names_library(const char *key, const char *value)
{
    if (platform_text_compare_nocase(key, "path") == 0) {
        return 1;
    }
    int32_t numeric_key = *key != 0;
    for (const char *c = key; *c; c++) {
        numeric_key = numeric_key && *c >= '0' && *c <= '9';
    }
    return numeric_key && (strchr(value, ':') || (value[0] == '\\' && value[1] == '\\') || value[0] == '/');
}

static void steam_read_libraries(const char *steam_root, steam_libraries *libraries)
{
    char vdf_path[GAME_DIR_PATH_SIZE];
    libraries->count = 0;
    if (!join_path(vdf_path, steam_root, STEAM_LIBRARY_FILE)) {
        return;
    }
    char *text = read_whole_file(vdf_path);
    if (!text) {
        return;
    }
    char key[GAME_DIR_PATH_SIZE], token[GAME_DIR_PATH_SIZE];
    int32_t expecting_key = 1, is_string = 0;
    for (const char *at = vdf_next_token(text, token, &is_string); at; at = vdf_next_token(at, token, &is_string)) {
        if (!is_string) {
            expecting_key = 1;
        } else if (expecting_key) {
            copy_text(key, token);
            expecting_key = 0;
        } else {
            if (vdf_names_library(key, token) && libraries->count < GAME_DIR_MAX_LIBRARIES) {
                copy_text(libraries->paths[libraries->count++], token);
            }
            expecting_key = 1;
        }
    }
    free(text);
}

static int32_t try_steam(game_dir_search *search, const char *steam_root)
{
    if (!steam_root || !*steam_root) {
        search_note(search, "  Steam: not installed (%s)\n", STEAM_NOT_FOUND);
        return 0;
    }
    char root[GAME_DIR_PATH_SIZE], game[GAME_DIR_PATH_SIZE];
    normalize_path(root, steam_root);
    if (join_path(game, root, STEAM_GAME_FOLDER) && try_candidate(search, "Steam", game, 1)) {
        return 1;
    }
    static steam_libraries libraries;
    steam_read_libraries(root, &libraries);
    for (int32_t i = 0; i < libraries.count; i++) {
        char library[GAME_DIR_PATH_SIZE];
        normalize_path(library, libraries.paths[i]);
        if (join_path(game, library, STEAM_GAME_FOLDER) && try_candidate(search, "a Steam library", game, 1)) {
            return 1;
        }
    }
    return 0;
}

static int32_t try_gog(game_dir_search *search, const game_dir_sources *sources)
{
    if (sources->gog_count == 0) {
        search_note(search, "  GOG: %s\n", GOG_NOT_FOUND);
        return 0;
    }
    for (int32_t i = 0; i < sources->gog_count; i++) {
        if (try_candidate(search, "GOG", sources->gog_paths[i], 1)) {
            return 1;
        }
    }
    return 0;
}

#if defined(_WIN32)
static int32_t pick_folder(char *folder)
{
    BROWSEINFOA info;
    memset(&info, 0, sizeof(info));
    info.lpszTitle = "The game data was not found. Choose the folder of Pharaoh + Cleopatra (the one with Data and "
                     "campaign.txt):";
    info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    HRESULT com = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    LPITEMIDLIST item = SHBrowseForFolderA(&info);
    int32_t picked = item && SHGetPathFromIDListA(item, folder);
    if (item) {
        CoTaskMemFree(item);
    }
    if (SUCCEEDED(com)) {
        CoUninitialize();
    }
    return picked;
}
#else
static int32_t pick_folder(char *folder)
{
    static const char *const pickers[] = {
        "zenity --file-selection --directory --title='tutankhamun: choose the folder of Pharaoh + Cleopatra' 2>/dev/null",
        "kdialog --getexistingdirectory \"$HOME\" --title 'tutankhamun: choose the folder of Pharaoh + Cleopatra' 2>/dev/null",
    };
    for (size_t i = 0; i < sizeof(pickers) / sizeof(pickers[0]); i++) {
        FILE *pipe = popen(pickers[i], "r");
        if (!pipe) {
            continue;
        }
        char line[GAME_DIR_PATH_SIZE];
        int32_t got = fgets(line, sizeof(line), pipe) != NULL;
        int status = pclose(pipe);
        if (status == 127 * 256 || status == -1) {
            continue;
        }
        if (!got) {
            return 0;
        }
        line[strcspn(line, "\r\n")] = 0;
        copy_text(folder, line);
        return folder[0] != 0;
    }
    return 0;
}
#endif

static int32_t try_picker(game_dir_search *search)
{
    char folder[GAME_DIR_PATH_SIZE];
    while (pick_folder(folder)) {
        search->seen_count = 0;
        if (try_candidate(search, "the folder picker", folder, 1)) {
            return 1;
        }
        char text[GAME_DIR_PATH_SIZE + 200];
        snprintf(text, sizeof(text), "%s is not the game folder: it has no %s.\nChoose the folder of Pharaoh + Cleopatra.",
            folder, game_dir_missing_file(folder));
        platform_message_box(0, text, "tutankhamun: not the game folder", PLATFORM_BOX_OK | PLATFORM_BOX_ICON_WARNING);
    }
    search_note(search, "  the folder picker: cancelled\n");
    return 0;
}

static int32_t try_remembered(game_dir_search *search, const char *ini)
{
    char remembered[GAME_DIR_PATH_SIZE];
    if (!ini) {
        return 0;
    }
    app_ini_read_text(ini, GAME_DIR_INI_SECTION, GAME_DIR_INI_KEY, remembered, sizeof(remembered));
    if (!remembered[0]) {
        search_note(search, "  remembered: none in %s\n", ini);
        return 0;
    }
    return try_candidate(search, "remembered in the ini", remembered, 0);
}

int32_t game_dir_find(const game_dir_sources *sources, game_dir_choice *choice, char *tried, size_t tried_size)
{
    static game_dir_search search;
    memset(&search, 0, sizeof(search));
    memset(choice, 0, sizeof(*choice));
    search.choice = choice;
    search.tried = tried;
    search.tried_size = tried_size;
    if (tried_size > 0) {
        tried[0] = 0;
    }
    if (try_remembered(&search, sources->ini)) {
        return 1;
    }
    if (sources->here && try_candidate(&search, "the current folder", sources->here, 0)) {
        return 1;
    }
    if (sources->program_dir && try_candidate(&search, "the program's folder", sources->program_dir, 0)) {
        return 1;
    }
    if (try_steam(&search, sources->steam_root) || try_gog(&search, sources)) {
        return 1;
    }
    if (sources->picker && try_picker(&search)) {
        return 1;
    }
    return 0;
}

int32_t game_dir_remember(const char *ini, const char *dir)
{
    int32_t written = ini && app_ini_write(ini, GAME_DIR_INI_SECTION, GAME_DIR_INI_KEY, dir);
    platform_log(written ? "port: game directory %s remembered in %s" : "port: cannot remember %s in %s", dir, ini ? ini : "?");
    return written;
}

#if defined(_WIN32)
static void read_steam_root(char *root)
{
    HKEY key;
    root[0] = 0;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Valve\\Steam", 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return;
    }
    DWORD type = 0, size = GAME_DIR_PATH_SIZE - 1;
    if (RegQueryValueExA(key, "SteamPath", 0, &type, (LPBYTE) root, &size) != ERROR_SUCCESS || type != REG_SZ) {
        size = 0;
    }
    root[size < GAME_DIR_PATH_SIZE ? size : GAME_DIR_PATH_SIZE - 1] = 0;
    RegCloseKey(key);
}

static int32_t gog_path_known(char paths[][GAME_DIR_PATH_SIZE], int32_t count, const char *path)
{
    for (int32_t i = 0; i < count; i++) {
        if (platform_text_compare_nocase(paths[i], path) == 0) {
            return 1;
        }
    }
    return 0;
}

static int32_t read_gog_paths(char paths[][GAME_DIR_PATH_SIZE], int32_t max_paths)
{
    static const REGSAM views[] = { KEY_WOW64_32KEY, KEY_WOW64_64KEY };
    int32_t count = 0;
    for (size_t view = 0; view < sizeof(views) / sizeof(views[0]); view++) {
        HKEY games;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\GOG.com\\Games", 0, KEY_READ | views[view], &games) != ERROR_SUCCESS) {
            continue;
        }
        char name[256];
        for (DWORD index = 0; count < max_paths; index++) {
            DWORD name_size = sizeof(name);
            if (RegEnumKeyExA(games, index, name, &name_size, 0, 0, 0, 0) != ERROR_SUCCESS) {
                break;
            }
            HKEY game;
            if (RegOpenKeyExA(games, name, 0, KEY_READ | views[view], &game) != ERROR_SUCCESS) {
                continue;
            }
            char path[GAME_DIR_PATH_SIZE];
            DWORD type = 0, size = GAME_DIR_PATH_SIZE - 1;
            if (RegQueryValueExA(game, "path", 0, &type, (LPBYTE) path, &size) == ERROR_SUCCESS && type == REG_SZ) {
                path[size < GAME_DIR_PATH_SIZE ? size : GAME_DIR_PATH_SIZE - 1] = 0;
                if (path[0] && !gog_path_known(paths, count, path)) {
                    copy_text(paths[count++], path);
                }
            }
            RegCloseKey(game);
        }
        RegCloseKey(games);
    }
    return count;
}

static int32_t current_dir(char *dir)
{
    DWORD length = GetCurrentDirectoryA(GAME_DIR_PATH_SIZE, dir);
    return length > 0 && length < GAME_DIR_PATH_SIZE;
}
#else
static void read_steam_root(char *root)
{
    static const char *const homes[] = {
        ".steam/steam",
        ".local/share/Steam",
        ".var/app/com.valvesoftware.Steam/.local/share/Steam",
        "snap/steam/common/.local/share/Steam",
    };
    const char *home = getenv("HOME");
    root[0] = 0;
    if (!home || !*home) {
        return;
    }
    for (size_t i = 0; i < sizeof(homes) / sizeof(homes[0]); i++) {
        char candidate[GAME_DIR_PATH_SIZE], steamapps[GAME_DIR_PATH_SIZE];
        snprintf(candidate, sizeof(candidate), "%s/%s", home, homes[i]);
        if (join_path(steamapps, candidate, "steamapps") && path_is_directory(steamapps)) {
            copy_text(root, candidate);
            return;
        }
    }
}

static int32_t read_gog_paths(char paths[][GAME_DIR_PATH_SIZE], int32_t max_paths)
{
    static const char *const homes[] = {
        "GOG Games/Pharaoh + Cleopatra",
        "GOG Games/Pharaoh Gold",
        "Games/Heroic/Pharaoh + Cleopatra",
        "Games/Heroic/Pharaoh Gold",
        "Games/pharaoh-cleopatra/drive_c/GOG Games/Pharaoh Gold",
        ".wine/drive_c/GOG Games/Pharaoh Gold",
        ".wine/drive_c/GOG Games/Pharaoh + Cleopatra",
        ".wine/drive_c/Program Files (x86)/GOG Galaxy/Games/Pharaoh + Cleopatra",
        ".wine/drive_c/Program Files (x86)/GOG Galaxy/Games/Pharaoh Gold",
    };
    const char *home = getenv("HOME");
    int32_t count = 0;
    if (!home || !*home) {
        return 0;
    }
    for (size_t i = 0; i < sizeof(homes) / sizeof(homes[0]) && count < max_paths; i++) {
        char candidate[GAME_DIR_PATH_SIZE];
        snprintf(candidate, sizeof(candidate), "%s/%s", home, homes[i]);
        if (path_is_directory(candidate)) {
            copy_text(paths[count++], candidate);
        }
    }
    return count;
}

static int32_t current_dir(char *dir)
{
    return getcwd(dir, GAME_DIR_PATH_SIZE) != NULL;
}
#endif

void game_dir_sources_of_system(game_dir_sources *sources, int32_t picker)
{
    static char ini[GAME_DIR_PATH_SIZE], here[GAME_DIR_PATH_SIZE], program_dir[GAME_DIR_PATH_SIZE];
    static char steam_root[GAME_DIR_PATH_SIZE], gog_paths[GAME_DIR_MAX_GOG][GAME_DIR_PATH_SIZE];
    memset(sources, 0, sizeof(*sources));
    if (app_ini_path(ini, sizeof(ini))) {
        sources->ini = ini;
    }
    if (app_program_dir(program_dir, sizeof(program_dir))) {
        sources->program_dir = program_dir;
    }
    sources->here = current_dir(here) ? here : 0;
    read_steam_root(steam_root);
    sources->steam_root = steam_root;
    sources->gog_count = read_gog_paths(gog_paths, GAME_DIR_MAX_GOG);
    for (int32_t i = 0; i < sources->gog_count; i++) {
        sources->gog_paths[i] = gog_paths[i];
    }
    sources->picker = picker;
}

void game_dir_report_not_found(const char *tried, int32_t box)
{
    static char text[8192];
    snprintf(text, sizeof(text),
        "The game data was not found. tutankhamun needs the folder of an installed Pharaoh + Cleopatra (the one with "
        "Data, campaign.txt and Pharaoh_Text.eng); start it with that folder:\n"
        "    tutankhamun <game dir>\n"
        "Tried:\n%s",
        tried);
    platform_log("port: %s", text);
    fprintf(stderr, "%s", text);
    if (box) {
        platform_message_box(0, text, "tutankhamun: no game data", PLATFORM_BOX_OK | PLATFORM_BOX_ICON_ERROR);
    }
}

int32_t game_dir_probe_main(int32_t argc, char **argv)
{
    static game_dir_sources sources;
    static char ini_path[GAME_DIR_PATH_SIZE];
    const char *own_ini = 0;
    int32_t remember = 0;
    memset(&sources, 0, sizeof(sources));
    for (int32_t i = 2; i < argc; i++) {
        int32_t has_value = i + 1 < argc;
        if (strcmp(argv[i], "--system") == 0) {
            game_dir_sources_of_system(&sources, sources.picker);
            sources.ini = own_ini;
        } else if (strcmp(argv[i], "--remember") == 0) {
            remember = 1;
        } else if (strcmp(argv[i], "--picker") == 0) {
            sources.picker = 1;
        } else if (strcmp(argv[i], "--ini") == 0 && has_value) {
            normalize_path(ini_path, argv[++i]);
            own_ini = sources.ini = ini_path;
        } else if (strcmp(argv[i], "--here") == 0 && has_value) {
            sources.here = argv[++i];
        } else if (strcmp(argv[i], "--program-dir") == 0 && has_value) {
            sources.program_dir = argv[++i];
        } else if (strcmp(argv[i], "--steam") == 0 && has_value) {
            sources.steam_root = argv[++i];
        } else if (strcmp(argv[i], "--gog") == 0 && has_value && sources.gog_count < GAME_DIR_MAX_GOG) {
            sources.gog_paths[sources.gog_count++] = argv[++i];
        } else {
            fprintf(stderr,
                "usage: tutankhamun --find-game-dir [--system] [--ini file] [--remember] [--here dir] [--program-dir dir]\n"
                "                    [--steam root] [--gog dir]... [--picker]\n"
                "(--system: the real current folder, program folder, Steam and GOG, read only; --remember needs --ini)\n");
            return 1;
        }
    }
    printf("steam root: %s\n", sources.steam_root ? sources.steam_root : "-");
    for (int32_t i = 0; i < sources.gog_count; i++) {
        printf("gog path: %s\n", sources.gog_paths[i]);
    }
    static char tried[4096];
    static game_dir_choice choice;
    if (!game_dir_find(&sources, &choice, tried, sizeof(tried))) {
        game_dir_report_not_found(tried, 0);
        return 2;
    }
    printf("game directory: %s\nfound by: %s\n", choice.dir, choice.reason);
    if (tried[0]) {
        printf("refused before it:\n%s", tried);
    }
    if (remember && own_ini && choice.remember) {
        printf("remembered in %s: %s\n", own_ini, game_dir_remember(own_ini, choice.dir) ? "yes" : "no");
    }
    return 0;
}
