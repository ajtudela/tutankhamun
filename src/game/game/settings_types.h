// The game's settings record (settings_data), the 564 bytes of Pharaoh.inf in the file's order: display, sound,
// speeds, the player's name, campaign position, tooltips, warnings, autosaves, difficulty, gods and the other
// options.

#ifndef GAME_SETTINGS_TYPES_H
#define GAME_SETTINGS_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum settings_sizes {
    SETTINGS_PLAYER_NAME_SIZE = 32,
    SETTINGS_PERSONAL_SAVINGS_MISSIONS = 98,
};

enum settings_defaults {
    DEFAULT_CD_DRIVE_LETTER = 'D',
};

typedef struct settings_saved_with_game {
    int32_t personal_savings[SETTINGS_PERSONAL_SAVINGS_MISSIONS];
    int32_t victory_video_senate;
} settings_saved_with_game;

typedef struct settings_file {
    int32_t desktop_color_depth;
    int32_t fullscreen;
    uint8_t paused;
    uint8_t unused_a;
    uint8_t cd_drive_letter;
    uint8_t sound_effects_enabled;
    uint8_t sound_music_enabled;
    uint8_t sound_speech_enabled;
    uint8_t reserved_1[2];
    int32_t display_mode;
    int32_t game_speed;
    int32_t scroll_speed;
    char player_name[SETTINGS_PLAYER_NAME_SIZE];
    int32_t debug_grid_overlay;
    uint8_t reserved_2[12];
    int32_t last_advisor;
    int8_t mission_id;
    uint8_t campaign;
    uint8_t previous_campaign_link;
    int8_t campaign_link;
    int32_t tooltips;
    int32_t starting_favor;
    int32_t personal_savings_last_mission;
    int32_t current_mission_id;
    int32_t is_custom_scenario;
    uint8_t sound_city_enabled;
    uint8_t warnings;
    uint8_t monthly_autosave;
    uint8_t pyramid_speedup;
    int32_t sound_effects_volume;
    int32_t sound_music_volume;
    int32_t sound_speech_volume;
    int32_t sound_city_volume;
    int32_t unused_b;
    int32_t ram;
    int32_t cpu;
    int32_t unused_c;
    int32_t display_mode_start;
    uint8_t reserved_3[4];
    int32_t autosave;
    uint32_t message_filter;
    settings_saved_with_game saved_with_game;
    int32_t difficulty;
    int32_t gods_enabled;
    uint8_t classical_names;
    uint8_t check_cd;
    uint8_t reserved_4[2];
} settings_file;

static_assert(sizeof(settings_file) == 0x234, "settings_file is the settings file's record of 564 bytes");
static_assert(offsetof(settings_file, player_name) == 0x1C && offsetof(settings_file, last_advisor) == 0x4C &&
                  offsetof(settings_file, sound_city_enabled) == 0x68 && offsetof(settings_file, autosave) == 0x94 &&
                  offsetof(settings_file, saved_with_game) == 0x9C && offsetof(settings_file, saved_with_game.victory_video_senate) == 0x224 &&
                  offsetof(settings_file, difficulty) == 0x228 && offsetof(settings_file, classical_names) == 0x230,
              "settings_file is the settings file's layout");

#endif
