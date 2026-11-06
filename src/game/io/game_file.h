// Saved games and scenarios as a plain sequence of chunks described by a chunk table, raw or compressed, without a
// header. Loading an older file converts the chunks whose layout changed, using the chunk table saved in the file and
// a list of converters by version per chunk.

#ifndef IO_GAME_FILE_H
#define IO_GAME_FILE_H

#include <stdint.h>

enum mission_saved_game_state {
    MISSION_SAVE_NONE = 0,
    MISSION_SAVE_WRITTEN = 1,
    MISSION_SAVE_DUE = 2,
};
#include "io/chunk_io.h"


#define SAVEGAME_CHUNKS 181
#define SCENARIO_CHUNKS 24

enum save_version {
    SAVE_VERSION_OLDEST = 115,
    SAVE_VERSION_ALLOWED_BUILDINGS = 117,
    SAVE_VERSION_EMPIRE_OBJECT_STATES = 118,
    SAVE_VERSION_FORMATION_FIELD = 120,
    SAVE_VERSION_MESSAGE_FIELDS = 121,
    SAVE_VERSION_PLAYER_BLOCK = 122,
    SAVE_VERSION_PLAYER_COUNTS = 123,
    SAVE_VERSION_RECORD_RELAYOUT = 124,
    SAVE_VERSION_FLOODPLAIN_SETTINGS = 125,
    SAVE_VERSION_FIGURE_FIELDS = 127,
    SAVE_VERSION_SERVICE_VISIT_GRID = 128,
    SAVE_VERSION_MESSAGE_TEXT_IDS = 129,
    SAVE_VERSION_EVENT_RELAYOUT = 130,
    SAVE_VERSION_FLOOD_NEXT_QUALITY = 132,
    SAVE_VERSION_REPLAY_FLAG = 133,
    SAVE_VERSION_PLAYER_DIFFICULTY = 134,
    SAVE_VERSION_MESSAGE_RELAYOUT = 136,
    SAVE_VERSION_FIGURE_TARGETS = 139,
    SAVE_VERSION_PYRAMID_REDRAW = 141,
    SAVE_VERSION_FARM_FIELDS = 142,
    SAVE_VERSION_BOOKMARKS = 143,
    SAVE_VERSION_EVENT_FIELDS = 144,
    SAVE_VERSION_SCENARIO_PYRAMID_REDRAW = 145,
    SAVE_VERSION_MONUMENT_FIELDS = 146,
    SAVE_VERSION_LAST_FLOOD_QUALITY = 147,
    SAVE_VERSION_FORMATION_FIGURES = 148,
    SAVE_VERSION_STORAGE_FLAGS = 149,
    SAVE_VERSION_PLAYER_FIELDS = 150,
    SAVE_VERSION_PREDATOR_VARIANT = 151,
    SAVE_VERSION_SCENARIO_EVENTS = 152,
    SAVE_VERSION_EMPIRE_OBJECT_FLAGS = 153,
    SAVE_VERSION_MONUMENT_SPOT_STATE = 154,
    SAVE_VERSION_ZOO = 155,
    SAVE_VERSION_PLAYER_COUNTS_2 = 156,
    SAVE_VERSION_COUNTDOWNS = 157,
    SAVE_VERSION_SWEEP_TOTAL = 158,
    SAVE_VERSION_SOLDIER_IMAGES = 159,
    SAVE_VERSION_BUILDING_ORIENTATION = 160,
    SAVEGAME_VERSION = SAVE_VERSION_BUILDING_ORIENTATION,
};

void game_file_init_save_chunk_table_ext(void);
void game_file_init_map_chunk_table_ext(void);
int32_t game_file_io_read_chunk(int32_t index, chunk_entry *table, int32_t fd);
void game_file_io_read_saved_game_state(int32_t fd);
void game_file_io_read_scenario_state(int32_t fd);
int32_t game_file_io_write_saved_game(const char *path);
int32_t game_file_io_write_scenario(const char *path);
int32_t game_file_io_delete_saved_game(const char *path);
int32_t game_file_load_saved_game(const char *path);
int32_t game_file_load_saved_game_keep_difficulty(const char *path);
void game_file_initialize_saved_game(void);
int32_t game_file_load_scenario_data(const char *path, int32_t skip_scripts);
int32_t game_file_load_campaign_mission(int32_t pak, int32_t skip_initialize);
void game_file_write_mission_saved_game(void);
void game_file_initialize_scenario(void);

#endif
