// The tables of game/game_data.c.

#ifndef GAME_GAME_DATA_H
#define GAME_GAME_DATA_H

#include <stdint.h>

#include "building/building_types.h"
#include "game/cheats.h"
#include "game/tutorial.h"
#include "graphics/button.h"
#include "io/file_path.h"
#include "window/hotkeys.h"

typedef struct {
    int32_t x;
    int32_t y;
} briefing_goal_position;

typedef struct {
    image_button back_button;
    briefing_goal_position goal_positions[6];
} briefing_buttons;

typedef struct {
    tutorial_entry entries[TUTORIAL_COUNT];
    image_button briefing_accept_button;
} tutorial_records;

typedef struct {
    char mummys_revenge[16];
    char jail_break[12];
    char ancient_astronauts[20];
    char crimson_tide[16];
    char hail_to_the_chief[20];
    char amphibious_assault[20];
    char crop_busters[16];
    char kitty_litter[16];
    char cat_fight[12];
    char meow[8];
    char cat_nip[8];
    char seth_strikes[16];
    char fury_of_seth[16];
    char typhonian_relief[20];
    char spirit_of_typhon[20];
    char grenow[8];
    char big_dave[12];
    char noble_djed[12];
    char supreme_craftsman[20];
    char bird_of_prey[16];
    char mesektet[12];
    char sun_disk[12];
    char pharaohs_glory[16];
    char underworld[12];
    char mummys_curse[16];
    char bounty[8];
    char life_from_death[16];
    char frame_counter[16];
    char mockattack2[12];
    char mockattack1[12];
    char hippo_stomp[12];
    char side_show[12];
    char treasure_chest[16];
    char pharaohs_tomb[16];
} cheat_names_xor3;

typedef struct {
    generic_button close_button;
    cheat_names_xor3 names;
} cheat_buttons_records;

extern const tutorial_lists tutorial_building_lists;

extern const char maps_directory[8];

extern const char map_file_extension[4];

extern const char str_sphinx_extra[16];

typedef struct {
    char sphinx1d[12];
    char sphinx1c[12];
    char sphinx1b[12];
    char sphinx1a[12];
    char pharaoh_terrain[16];
    char pharaoh_general[16];
    char hyksos[8];
    char spr_main[8];
    char pharaoh_unloaded[20];
    char libmain_dummy[16];
} sphinx_script_texts;
extern sphinx_script_texts s_sphinx1d;

extern const char str_maps_folder[8];

extern const char str_save_folder[8];

extern const char save_folder_path[8];

extern const char str_pharaohmap_inf[16];

extern const char sierra_ini_key_cpu[4];

extern const char sierra_ini_key_checkcd[8];

extern const char sierra_ini_key_ram[4];

extern const char str_err_sierra_ini[32];

extern cheat_entry cheat_table[34];

extern cheat_buttons_records cheat_buttons;

extern const char screenshot_file_name_alt1[12];

extern const char screenshot_file_name_alt2[12];

extern const char str_hailstorm_mp3[28];

extern int32_t difficulty_treasury_percentage[5];

extern int32_t difficulty_tax_percentages[5];

extern int32_t difficulty_starting_favor[5];

extern int32_t difficulty_sentiment_base_table[11];

extern char autosave_file_name[68];

extern int32_t game_setup_started;

extern int32_t window_blocks_ticks[88];

extern int32_t game_millis_per_tick_per_speed[11];

extern char battle_music_path[24];

extern const char player_file_extension[8];

typedef struct {
    tutorial_records tutorial;
    uint32_t accept_button_padding[1];
    briefing_buttons briefing;
} tutorial_briefing_table_group;
extern tutorial_briefing_table_group tutorial_briefing_tables;

extern int32_t animation_timers[51];

extern uint8_t animation_should_advance[51];

extern uint8_t tutorial_flags_b[15];

extern uint8_t tutorial_legions_message_posted;

extern uint8_t tutorial_flags_a[26];

extern uint8_t cheat_input_enabled;

extern int32_t cheat_input_previous_window;

extern char cheat_input_text[FILE_PATH_SIZE];

extern int32_t city_hailstorm_countdown;

extern int32_t pyramid_speedup_countdown;

extern int32_t city_frogs_countdown;

extern int32_t game_time_year;

extern int32_t game_time_month;

extern int32_t game_time_day;

extern int32_t game_time_total_days;

extern int32_t game_time_tick;

extern int32_t saved_reserved_1;

extern uint8_t map_sprite_backup[51984];

extern int16_t undo_building_ids[525];

extern building undo_buildings[525];

extern int32_t undo_num_buildings;

extern int32_t undo_resources[36];

extern int32_t undo_timeout_ticks;

extern int32_t g_scenario_climate;

extern int32_t speed_times;

extern int32_t speed_measure_elapsed;

extern int32_t g_slide_speed_metric;

extern int32_t key_ctrl;

extern map_bookmark_table map_bookmarks;

extern uint8_t cheat_mummys_revenge_used;

extern uint8_t pyramid_speedup_active;

extern char player_record_path_buffer[FILE_PATH_SIZE];

extern int32_t minimap_invalidate_flag;

extern int32_t sound_music_fading;

extern int32_t sound_speech_fading;

extern int32_t sound_music_playing_special;

#endif
