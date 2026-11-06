// The built-in data of the game's start, files, cheats, tutorial, difficulty and time. game/game_data.h declares
// them; the cheat names are stored with every character XOR 3.

#include "game/cheats.h"
#include "game/game_data.h"
#include "window/intermezzo.h"

#include <assert.h>
#include <stddef.h>

const tutorial_lists tutorial_building_lists = { .fire_buildings = { 167 }, .population_buildings = { 115, 71, 70 }, .population_menus = { 7, 0 },
    .granary_food_buildings = { 180 }, .collapse_buildings = { 81 }, .gold_buildings = { 150, 96, 209, 0 },
    .gold_menus = { 11 }, .gods_pleased_buildings = { 31, 36 }, .gods_pleased_menus = { 10 },
    .disease_buildings = { 46, 206 }, .disease_menus = { 8 }, .food_stock_buildings = { 109, 3, 114, 72 },
    .food_stock_menus = { 1 }, .pottery_request_menus = { 4 },
    .pottery_stored_buildings = { 39, 41, 42, 43, 38, 91, 136, 52, 138 }, .beer_stored_buildings = { 86, 77 },
    .beer_stored_menus = { 12, 13 }, .houses_done_buildings = { 51, 203, 195 }, .houses_done_menus = { 9 },
    .scribe_menus = { 6 }, .bricks_stored_buildings = { 183, 178 }, .bricks_stored_menus = { 14 },
    .mission4_menus = { 5 }, .mission5_menus = { 2, 3 } };

const char maps_directory[8] = "maps\\";

const char map_file_extension[4] = "map";

const char str_sphinx_extra[16] = "SPHINX_EXTRA";

sphinx_script_texts s_sphinx1d = {
    "SPHINX1D", "SPHINX1C", "SPHINX1B", "SPHINX1A", "Pharaoh_Terrain", "Pharaoh_General", "Hyksos", "SprMain",
    "Pharaoh_Unloaded", "LibMainDummy",
};

const char str_maps_folder[8] = "Maps";

const char str_save_folder[8] = "Save";

const char save_folder_path[8] = "save\\";

const char str_pharaohmap_inf[16] = "Pharaohmap.inf";

const char sierra_ini_key_cpu[4] = "CPU";

const char sierra_ini_key_checkcd[8] = "CheckCD";

const char sierra_ini_key_ram[4] = "RAM";

const char str_err_sierra_ini[32] = "Err:Could not read sierra.ini";

cheat_entry cheat_table[CHEAT_COUNT] = {
    { cheat_buttons.names.pharaohs_tomb, cheat_pharaohs_tomb, 1, { 0, 0, 0 } },
    { cheat_buttons.names.treasure_chest, cheat_treasure_chest, 1, { 0, 0, 0 } },
    { cheat_buttons.names.side_show, cheat_side_show, 1, { 0, 0, 0 } },
    { cheat_buttons.names.hippo_stomp, cheat_hippo_stomp, 1, { 0, 0, 0 } },
    { cheat_buttons.names.mockattack1, cheat_mockattack1, 0, { 0, 0, 0 } },
    { cheat_buttons.names.mockattack2, cheat_mockattack2, 0, { 0, 0, 0 } },
    { cheat_buttons.names.frame_counter, button_none, 0, { 0, 0, 0 } },
    { cheat_buttons.names.life_from_death, cheat_life_from_death, 1, { 0, 0, 0 } },
    { cheat_buttons.names.bounty, cheat_bounty, 1, { 0, 0, 0 } },
    { cheat_buttons.names.mummys_curse, cheat_mummys_curse, 1, { 0, 0, 0 } },
    { cheat_buttons.names.underworld, cheat_underworld, 1, { 0, 0, 0 } },
    { cheat_buttons.names.pharaohs_glory, cheat_pharaohs_glory, 1, { 0, 0, 0 } },
    { cheat_buttons.names.sun_disk, cheat_sun_disk, 1, { 0, 0, 0 } },
    { cheat_buttons.names.mesektet, cheat_mesektet, 1, { 0, 0, 0 } },
    { cheat_buttons.names.bird_of_prey, cheat_bird_of_prey, 1, { 0, 0, 0 } },
    { cheat_buttons.names.supreme_craftsman, cheat_supreme_craftsman, 1, { 0, 0, 0 } },
    { cheat_buttons.names.noble_djed, cheat_noble_djed, 1, { 0, 0, 0 } },
    { cheat_buttons.names.big_dave, cheat_big_dave, 1, { 0, 0, 0 } },
    { cheat_buttons.names.grenow, cheat_grenow, 1, { 0, 0, 0 } },
    { cheat_buttons.names.spirit_of_typhon, cheat_spirit_of_typhon, 1, { 0, 0, 0 } },
    { cheat_buttons.names.typhonian_relief, cheat_typhonian_relief, 1, { 0, 0, 0 } },
    { cheat_buttons.names.fury_of_seth, cheat_fury_of_seth, 1, { 0, 0, 0 } },
    { cheat_buttons.names.seth_strikes, cheat_seth_strikes, 1, { 0, 0, 0 } },
    { cheat_buttons.names.cat_nip, cheat_cat_nip, 1, { 0, 0, 0 } },
    { cheat_buttons.names.meow, cheat_meow, 1, { 0, 0, 0 } },
    { cheat_buttons.names.cat_fight, cheat_cat_fight, 1, { 0, 0, 0 } },
    { cheat_buttons.names.kitty_litter, cheat_kitty_litter, 1, { 0, 0, 0 } },
    { cheat_buttons.names.crop_busters, cheat_crop_busters, 1, { 0, 0, 0 } },
    { cheat_buttons.names.amphibious_assault, cheat_amphibious_assault, 1, { 0, 0, 0 } },
    { cheat_buttons.names.hail_to_the_chief, cheat_hail_to_the_chief, 1, { 0, 0, 0 } },
    { cheat_buttons.names.crimson_tide, cheat_crimson_tide, 1, { 0, 0, 0 } },
    { cheat_buttons.names.ancient_astronauts, cheat_ancient_astronauts, 1, { 0, 0, 0 } },
    { cheat_buttons.names.jail_break, cheat_jail_break, 1, { 0, 0, 0 } },
    { cheat_buttons.names.mummys_revenge, cheat_mummys_revenge, 1, { 0, 0, 0 } } };

cheat_buttons_records cheat_buttons = {
    .close_button = { 0, 0, 150, 30, game_cheat_input_close, button_none, 0, 0, 0, 1, 1, 0 },
    .names = {
        .mummys_revenge = "Nvnnzp#Qfufmdf",
        .jail_break = "Ibjo#Aqfbh",
        .ancient_astronauts = "Bm`jfmw#Bpwqlmbvwp",
        .crimson_tide = "@qjnplm#Wjgf",
        .hail_to_the_chief = "Kbjo#wl#wkf#@kjfe",
        .amphibious_assault = "Bnskjajlvp#Bppbvow",
        .crop_busters = "@qls#Avpwfqp",
        .kitty_litter = "Hjwwz#Ojwwfq",
        .cat_fight = "@bw#Ejdkw",
        .meow = "Nflt",
        .cat_nip = "@bw#Mjs",
        .seth_strikes = "Pfwk#Pwqjhfp",
        .fury_of_seth = "Evqz#le#Pfwk",
        .typhonian_relief = "Wzsklmjbm#Qfojfe",
        .spirit_of_typhon = "Psjqjw#le#Wzsklm",
        .grenow = "Dqfmlt",
        .big_dave = "Ajd#Gbuf",
        .noble_djed = "Mlaof#Gifg",
        .supreme_craftsman = "Pvsqfnf#@qbewpnbm",
        .bird_of_prey = "Ajqg#le#Sqfz",
        .mesektet = "Nfpfhwfw",
        .sun_disk = "Pvm#Gjph",
        .pharaohs_glory = "Skbqblkp#Dolqz",
        .underworld = "Vmgfqtlqog",
        .mummys_curse = "Nvnnzp#@vqpf",
        .bounty = "Alvmwz",
        .life_from_death = "Ojef#eqln#Gfbwk",
        .frame_counter = "eqbnf`lvmwfq",
        .mockattack2 = "nl`hbwwb`h1",
        .mockattack1 = "nl`hbwwb`h2",
        .hippo_stomp = "Kjssl#Pwlns",
        .side_show = "Pjgf#Pklt",
        .treasure_chest = "Wqfbpvqf#@kfpw",
        .pharaohs_tomb = "Skbqblkp#Wlna" }
};

const char screenshot_file_name_alt1[12] = "shot1.bmp";

const char screenshot_file_name_alt2[12] = "shot2.bmp";

const char str_hailstorm_mp3[28] = "audio\\ambient\\hailstorm.mp3";

int32_t difficulty_treasury_percentage[5] = { 200, 100, 100, 100, 75 };

int32_t difficulty_tax_percentages[5] = { 50, 50, 50, 50, 50 };

int32_t difficulty_starting_favor[5] = { 70, 60, 50, 50, 40 };

int32_t difficulty_sentiment_base_table[11] = { 80, 70, 60, 55, 50, 40, 60, 80, 100, 120 };

char autosave_file_name[68] = "last.sav\0\0\0\0"
                              "unable to find a repair queue for this shipyard!\n\0\0\0"
                              "\r\n";

int32_t game_setup_started = 1;

int32_t window_blocks_ticks[88] = { 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

int32_t game_millis_per_tick_per_speed[11] = { 0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700 };

char battle_music_path[24] = "audio\\music\\battle.mp3";

const char player_file_extension[8] = ".dat";

tutorial_briefing_table_group tutorial_briefing_tables = {
    .tutorial = { .entries = { { 0, 1, tutorial_building_lists.fire_buildings, 0, NULL, 0xFC, 0 },
                                      { 0, 3, tutorial_building_lists.population_buildings, 2, tutorial_building_lists.population_menus,
                                          0xEF, 0 },
                                      { 0, 1, tutorial_building_lists.granary_food_buildings, 0, NULL, 0xFF, 0 },
                                      { 0, 1, tutorial_building_lists.collapse_buildings, 0, NULL, 0xFD, 0 },
                                      { 1, 4, tutorial_building_lists.gold_buildings, 1, tutorial_building_lists.gold_menus, 0x102, 0 },
                                      { 1, 2, tutorial_building_lists.gods_pleased_buildings, 1, tutorial_building_lists.gods_pleased_menus,
                                          0xF0, 0 },
                                      { 2, 2, tutorial_building_lists.disease_buildings, 1, tutorial_building_lists.disease_menus, 0xFA,
                                          0 },
                                      { 2, 4, tutorial_building_lists.food_stock_buildings, 1, tutorial_building_lists.food_stock_menus,
                                          0x103, 0 },
                                      { 2, 0, NULL, 1, tutorial_building_lists.pottery_request_menus, 0xFB, 0 },
                                      { 2, 9, tutorial_building_lists.pottery_stored_buildings, 0, NULL, 0x100, 0 },
                                      { 3, 2, tutorial_building_lists.beer_stored_buildings, 2, tutorial_building_lists.beer_stored_menus,
                                          0x136, 0 },
                                      { 4, 3, tutorial_building_lists.houses_done_buildings, 1, tutorial_building_lists.houses_done_menus,
                                          0xFE, 0 },
                                      { 4, 0, NULL, 1, tutorial_building_lists.scribe_menus, 0x104, 0 },
                                      { 4, 2, tutorial_building_lists.bricks_stored_buildings, 1,
                                          tutorial_building_lists.bricks_stored_menus, 0x11B, 0 },
                                      { 4, 0, NULL, 0, NULL, 0x11A, 0 }, { 0, 0, NULL, 0, NULL, 0xF9, 0 }, { 1, 0, NULL, 0, NULL, 0xF1, 0 },
                                      { 2, 0, NULL, 0, NULL, 0xF2, 0 }, { 3, 0, NULL, 0, NULL, 0xF3, 0 },
                                      { 4, 0, NULL, 1, tutorial_building_lists.mission4_menus, 0xF4, 0 },
                                      { 5, 0, NULL, 2, tutorial_building_lists.mission5_menus, 0xF5, 0 }, { 6, 0, NULL, 0, NULL, 0xF6, 0 },
                                      { 7, 0, NULL, 0, NULL, 0xF7, 0 }, { 8, 0, NULL, 0, NULL, 0xF8, 0 }, { 9, 0, NULL, 0, NULL, 0x11C, 0 },
                                      { 10, 0, NULL, 0, NULL, 0x11D, 0 } },
        .briefing_accept_button = { 0, 0, 27, 27, 4, 461, 0, 0, mission_button_accept, button_none, 1, 0, 0, 0, 1,
            0 } },
    .briefing = { .back_button = { 0, 0, 31, 20, 4, 359, 8, 0, mission_button_back, button_none,
                                           1, 0, 0, 0, 0, 0 },
        .goal_positions = { { 32, 90 }, { 288, 90 }, { 32, 112 }, { 288, 112 }, { 32, 134 }, { 288, 134 } } }
};

int32_t animation_timers[51];

uint8_t animation_should_advance[51];

uint8_t tutorial_flags_b[15];

uint8_t tutorial_legions_message_posted;

uint8_t tutorial_flags_a[26];

uint8_t cheat_input_enabled;

int32_t cheat_input_previous_window;

char cheat_input_text[FILE_PATH_SIZE];

int32_t city_hailstorm_countdown;

int32_t pyramid_speedup_countdown;

int32_t city_frogs_countdown;

int32_t game_time_year;

int32_t game_time_month;

int32_t game_time_day;

int32_t game_time_total_days;

int32_t game_time_tick;

int32_t saved_reserved_1;

uint8_t map_sprite_backup[51984];

int16_t undo_building_ids[525];

building undo_buildings[525];

int32_t undo_num_buildings;

int32_t undo_resources[36];

int32_t undo_timeout_ticks;

int32_t g_scenario_climate;

int32_t speed_times;

int32_t speed_measure_elapsed;

int32_t g_slide_speed_metric;

int32_t key_ctrl;

static_assert(sizeof(map_bookmark_table) == 32, "map_bookmarks is the savegame's map bookmarks of 32 bytes");
map_bookmark_table map_bookmarks;

uint8_t cheat_mummys_revenge_used;

uint8_t pyramid_speedup_active;

char player_record_path_buffer[FILE_PATH_SIZE];

int32_t minimap_invalidate_flag;

int32_t sound_music_fading;

int32_t sound_speech_fading;

int32_t sound_music_playing_special;
