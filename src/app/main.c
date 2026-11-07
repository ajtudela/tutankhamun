// tutankhamun: the game as a program of its own, started from the user's game directory alone (its data compiled
// in, its icon and cursors its own): the play mode, the export and check of the gameplay configs and the game
// directory search (--find-game-dir).

#include "app/app_files.h"
#include "app/game_dir.h"
#include "app/image.h"
#include "app/play.h"
#include "app/play_cursors.h"
#include "core/config.h"
#include "game/run.h"
#include "platform/discord_rpc.h"
#include "platform/log.h"
#include "platform/platform_file.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char PLAY_LOG_NAME[] = "tutankhamun_play.log";

static int32_t names_exe(const char *path)
{
    size_t length = strlen(path);
    return length >= 4 && platform_text_compare_nocase(path + length - 4, ".exe") == 0;
}

static int32_t is_directory(const char *path)
{
    return platform_dir_exists(path);
}

static int32_t is_game_argument(const char *path)
{
    return names_exe(path) || is_directory(path);
}

static int32_t has_game_data(const char *dir)
{
    char data[PLATFORM_PATH_MAX];
    if (strlen(dir) + sizeof("\\Data") > sizeof(data)) {
        return 0;
    }
    snprintf(data, sizeof(data), "%s\\Data", dir);
    return is_directory(data);
}

static int32_t enter_game_dir(const char *game)
{
    char dir[PLATFORM_PATH_MAX];
    snprintf(dir, sizeof(dir), "%s", game);
    if (names_exe(dir)) {
        char *slash = strrchr(dir, '\\'), *other = strrchr(dir, '/');
        slash = other > slash ? other : slash;
        if (!slash) {
            snprintf(dir, sizeof(dir), ".");
        } else {
            slash[slash == dir + 2 && dir[1] == ':' ? 1 : 0] = 0;
        }
    }
    if (!platform_dir_enter(dir)) {
        platform_log("port: cannot enter the game directory %s", dir);
        fprintf(stderr, "cannot enter the game directory %s\n", dir);
        return 0;
    }
    if (!has_game_data(".")) {
        platform_log("port: %s has no Data folder", dir);
        fprintf(stderr, "%s is not a game directory: it has no Data folder\n", dir);
        return 0;
    }
    platform_log("port: game directory %s", dir);
    return 1;
}

static int32_t find_game_dir(game_dir_sources *sources, game_dir_choice *choice)
{
    static char tried[4096];
    game_dir_sources_of_system(sources, 1);
    if (game_dir_find(sources, choice, tried, sizeof(tried))) {
        return 1;
    }
    game_dir_report_not_found(tried, 1);
    return 0;
}

static int32_t prepare(const char *game_dir)
{
    if (!image_init()) {
        fprintf(stderr, "cannot set up the game's data, see the log\n");
        return 0;
    }
    if (!enter_game_dir(game_dir)) {
        return 0;
    }
    if (!app_startup()) {
        fprintf(stderr, "the game's start-up failed, see the log\n");
        return 0;
    }
    return 1;
}

static void default_log_path(char *path, size_t size)
{
#if defined(_WIN32)
    platform_path_absolute(PLAY_LOG_NAME, path, (uint32_t) size);
#else
    char ini[PLATFORM_PATH_MAX];
    char *slash = app_ini_path(ini, sizeof(ini)) ? strrchr(ini, '/') : NULL;
    if (!slash) {
        platform_path_absolute(PLAY_LOG_NAME, path, (uint32_t) size);
        return;
    }
    *slash = 0;
    snprintf(path, size, "%s/%s", ini, PLAY_LOG_NAME);
#endif
}

static int32_t parse_size(const char *text, int32_t *width, int32_t *height)
{
    char rest;
    return sscanf(text, "%dx%d%c", width, height, &rest) == 2 && *width > 0 && *height > 0;
}

static int32_t parse_cursor_scale(const char *text, int32_t *percent)
{
    char rest;
    double scale;
    if (sscanf(text, "%lf%c", &scale, &rest) != 1 || scale < 1.0 || scale > 2.0) {
        return 0;
    }
    *percent = (int32_t) (scale * 100.0 + 0.5);
    return 1;
}

static void print_usage(void)
{
    fprintf(stderr,
        "usage: tutankhamun [game dir] [save.sav] [--log file] [--size WxH] [--window | --fullscreen] [--cursor-scale N] [--discord-log]\n"
        "       tutankhamun --export-configs <dir> | --check-configs <dir>\n"
        "       tutankhamun --find-game-dir [--system] [--ini file] [--here dir] [--steam root] [--gog dir]... [...]\n"
        "(the game directory holds the game's Data folder; a path ending in .exe stands for its folder)\n");
}

static int32_t play_first_argument(int32_t argc, char **argv)
{
    return argc > 1 && strcmp(argv[1], "--play") == 0 ? 2 : 1;
}

static int32_t play_main(int32_t argc, char **argv, int32_t first)
{
    const char *game_dir = 0, *save = 0, *log = 0;
    int32_t size_width = 0, size_height = 0, start_fullscreen = -1, cursor_scale_percent = 100;
    for (int32_t i = first; i < argc; i++) {
        if ((strcmp(argv[i], "--size") == 0 || strcmp(argv[i], "--resolution") == 0) && i + 1 < argc) {
            if (!parse_size(argv[++i], &size_width, &size_height)) {
                fprintf(stderr, "%s must be followed by a size such as 1024x768, not %s\n", argv[i - 1], argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "--window") == 0 || strcmp(argv[i], "--windowed") == 0) {
            start_fullscreen = 0;
        } else if (strcmp(argv[i], "--fullscreen") == 0) {
            start_fullscreen = 1;
        } else if (strcmp(argv[i], "--cursor-scale") == 0 && i + 1 < argc) {
            if (!parse_cursor_scale(argv[++i], &cursor_scale_percent)) {
                fprintf(stderr, "--cursor-scale must be followed by 1, 1.5 or 2, not %s\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            log = argv[++i];
        } else if (strcmp(argv[i], "--discord-log") == 0) {
            discord_rpc_set_verbose(true);
        } else if (strncmp(argv[i], "--", 2) == 0) {
            fprintf(stderr, "unknown option %s\n", argv[i]);
            print_usage();
            return 1;
        } else if (!game_dir && !save && is_game_argument(argv[i])) {
            game_dir = argv[i];
        } else if (!save) {
            save = argv[i];
        } else {
            fprintf(stderr, "too many arguments for the game\n");
            print_usage();
            return 1;
        }
    }
    static char save_path[PLATFORM_PATH_MAX], log_path[PLATFORM_PATH_MAX];
    if (save) {
        platform_path_absolute(save, save_path, sizeof(save_path));
        save = save_path;
    }
    if (!log) {
        default_log_path(log_path, sizeof(log_path));
    } else {
        platform_path_absolute(log, log_path, sizeof(log_path));
    }
    platform_log_open(log_path);
    static const play_host host = { platform_log, game_run_tick, game_run_frame };
    play_bind(&host);
    play_cursors_set_scale(cursor_scale_percent);
    app_report_no_configs_in_box();
    static game_dir_sources sources;
    static game_dir_choice found;
    if (!game_dir) {
        if (!find_game_dir(&sources, &found)) {
            return 2;
        }
        game_dir = found.dir;
    }
    play_display_request request = { size_width, size_height, start_fullscreen };
    app_request_display_mode(play_display_prepare(&request));
    if (!prepare(game_dir)) {
        return 2;
    }
    if (found.remember) {
        game_dir_remember(sources.ini, found.dir);
    }
    return play_run(save);
}

static int32_t configs_main(int32_t argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: tutankhamun --export-configs <dir> | --check-configs <dir>\n");
        return 1;
    }
    if (strcmp(argv[1], "--export-configs") == 0) {
        platform_dir_create(argv[2]);
        return config_export_all(argv[2]) ? 0 : 1;
    }
    return config_check_all(argv[2]) ? 0 : 1;
}

int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "--export-configs") == 0 || strcmp(argv[1], "--check-configs") == 0)) {
        return configs_main(argc, argv);
    }
    if (argc > 1 && strcmp(argv[1], "--find-game-dir") == 0) {
        return game_dir_probe_main(argc, argv);
    }
    return play_main(argc, argv, play_first_argument(argc, argv));
}
