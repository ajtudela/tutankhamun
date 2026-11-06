// Finding the user's game data when the game starts without a game directory, as Akhenaten does: the directory
// remembered in the ini beside the program, the current folder and the program's folder, the Steam and GOG installs,
// then a folder picker when asked for. The sources are passed in, so --find-game-dir can be given other folders than
// the registry's.

#ifndef APP_GAME_DIR_H
#define APP_GAME_DIR_H

#include <stddef.h>
#include <stdint.h>

#define GAME_DIR_MAX_GOG 32
#define GAME_DIR_PATH_SIZE 260

typedef struct {
    const char *ini;
    const char *here;
    const char *program_dir;
    const char *steam_root;
    const char *gog_paths[GAME_DIR_MAX_GOG];
    int32_t gog_count;
    int32_t picker;
} game_dir_sources;

typedef struct {
    char dir[GAME_DIR_PATH_SIZE];
    const char *reason;
    int32_t remember;
} game_dir_choice;

const char *game_dir_missing_file(const char *dir);
void game_dir_sources_of_system(game_dir_sources *sources, int32_t picker);
int32_t game_dir_find(const game_dir_sources *sources, game_dir_choice *choice, char *tried, size_t tried_size);
int32_t game_dir_remember(const char *ini, const char *dir);
void game_dir_report_not_found(const char *tried, int32_t box);
int32_t game_dir_probe_main(int32_t argc, char **argv);

#endif
