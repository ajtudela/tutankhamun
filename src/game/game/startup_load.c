// The start-up steps before the main loop: the settings (defaults, Pharaoh.ini, then Pharaoh.inf), the start of a
// campaign from its first link, the default names, the graphics set-up (Pharaohmap.inf, save and map folders, the
// SG files of the image collection), the game state before any game, the sidebar positions and the speed
// measurement.
//
// RAM= is read even when the key is missing (the '=' is then looked for from the start of the text), and game_setup
// returns 1 only when the spare screen cannot be allocated.

#include "game/startup_load.h"

#include "city/city_player.h"
#include "city/difficulty.h"
#include "city/message.h"
#include "core/random.h"
#include "core/text.h"
#include "editor/editor_data.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_present.h"
#include "graphics/image_collection.h"
#include "input/keyboard_capture.h"
#include "io/file.h"
#include "io/file_read_all.h"
#include "io/io_data.h"
#include "map/ring.h"
#include "platform/loading_text.h"
#include "platform/platform_file.h"
#include "platform/platform_time.h"
#include "platform/platform_window.h"
#include "platform/status_log.h"
#include "scenario/campaign.h"
#include "scenario/campaign_load.h"
#include "text/text_lookup.h"
#include "widget/city_view.h"
#include "widget/city_viewport.h"
#include "widget/screen_layout.h"
#include "widget/widget_data.h"
#include "window/advisors.h"
#include "window/loading_window.h"
#include "window/options_windows.h"
#include "window/text_input.h"
#include "window/top_menu_entries.h"
#include "window/window_image_group.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t settings_loaded;

enum startup_limits {
    INI_BUFFER_SIZE = 20000,
    SETTINGS_COLOR_DEPTH = 16,
    SETTINGS_LOW_RAM = 24,
};

enum startup_text {
    STARTUP_TEXT_DEFAULT_NAMES = 9,
    STARTUP_TEXT_PLAYER_NAME = 5,
    STARTUP_TEXT_FILE_NAME = 6,
    STARTUP_TEXT_MAP_NAME = 7,
    STARTUP_TEXT_LOADING = 11,
};

static const char *const sg_file_names[] = {
    "Pharaoh_Unloaded",
    "SprMain",
    "Hyksos",
    "Pharaoh_General",
    "Pharaoh_Terrain",
    "SPHINX1A",
    "SPHINX1B",
    "SPHINX1C",
    "SPHINX1D",
    "SPHINX_EXTRA",
    "TEMPLE_NILE",
    "empire",
    "SprAmbient",
    "Pharaoh_Fonts",
    "Egyptian",
    "SprMain2",
    "expansion",
    "PyramidDummy",
    "CaesareumDummy",
    "LtHouseDummy",
    "LibMainDummy",
    "LibMainDummy",
    "LibMainDummy",
    "LibExtra",
    "MausCarryoverDummy",
    "TombThutDummy",
    "AbuSimbelDummy",
    "TombTutDummy",
    "TombSetiDummy",
    "TombRamDummy",
};

static int32_t ini_value_offset(const char *text, const char *key)
{
    int32_t offset = string_find(text, key, INI_BUFFER_SIZE);
    if (offset <= 0) {
        return 0;
    }
    offset += string_index_of(text + offset, '=', INI_BUFFER_SIZE);
    return (offset > 0) ? offset : 0;
}

int32_t settings_read_ini(void)
{
    char *text = malloc(INI_BUFFER_SIZE);
    if (!text) {
        return 0;
    }
    memset(text, 0, INI_BUFFER_SIZE);
    FILE *file = fopen("Pharaoh.ini", file_mode_read_binary);
    if (!file) {
        log_info_text(str_err_sierra_ini, 0);
        free(text);
        return 0;
    }
    fread(text, INI_BUFFER_SIZE, 1, file);
    fclose(file);
    int32_t offset = string_find(text, sierra_ini_key_ram, INI_BUFFER_SIZE);
    offset += string_index_of(text + offset, '=', INI_BUFFER_SIZE);
    if (offset > 0) {
        settings_data.ram = atoi(text + offset);
    }
    offset = ini_value_offset(text, sierra_ini_key_checkcd);
    if (offset > 0) {
        settings_data.check_cd = (uint8_t) atoi(text + offset);
    }
    offset = ini_value_offset(text, sierra_ini_key_cpu);
    if (offset > 0) {
        settings_data.cpu = atoi(text + offset);
    }
    free(text);
    return 1;
}

void settings_start_campaign(int32_t campaign_id)
{
    int8_t campaign_index = (int8_t) campaign_id;
    settings_data.starting_favor = difficulty_starting_favor[settings_data.difficulty];
    settings_data.campaign = (uint8_t) campaign_index;
    settings_data.personal_savings_last_mission = 0;
    settings_data.current_mission_id = 0;
    city_player_current()->personal_savings = 0;
    settings_data.campaign_link = (int8_t) campaign_find_link(&campaign_object.campaign, campaign_index, -1, 0);
    int32_t link = settings_data.campaign_link;
    int32_t link_valid = (link >= 0) && (link < campaign_object.campaign.link_count);
    campaign_mission_choice.choice = 0;
    settings_data.mission_id = link_valid ? (int8_t) campaign_object.campaign.links[link].mission_id : 0;
    settings_data.previous_campaign_link = CAMPAIGN_LINK_NONE;
}

static void settings_set_defaults(void)
{
    settings_data.desktop_color_depth = SETTINGS_COLOR_DEPTH;
    settings_data.fullscreen = 1;
    settings_data.paused = 0;
    settings_data.unused_a = 0;
    settings_data.unused_b = 2;
    settings_data.ram = 15;
    settings_data.cpu = 66;
    settings_data.unused_c = 0;
    settings_data.cd_drive_letter = DEFAULT_CD_DRIVE_LETTER;
    settings_data.sound_effects_enabled = 1;
    settings_data.sound_music_enabled = 1;
    settings_data.sound_speech_enabled = 1;
    settings_data.sound_city_enabled = 1;
    settings_data.warnings = 1;
    settings_data.tooltips = TOOLTIPS_FULL;
    settings_data.game_speed = 90;
    settings_data.scroll_speed = 70;
    settings_data.check_cd = 1;
    settings_data.difficulty = DIFFICULTY_NORMAL;
    settings_data.gods_enabled = 1;
    settings_data.sound_effects_volume = 100;
    settings_data.sound_music_volume = 80;
    settings_data.sound_speech_volume = 100;
    settings_data.sound_city_volume = 100;
    settings_data.display_mode = DISPLAY_MODE_640X480;
}

static void settings_set_menu_texts(void)
{
    if (settings_data.tooltips == TOOLTIPS_FULL) {
        g_menu_items[MENU_ITEM_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_FULL;
        g_menu_items[MENU_ITEM_EDITOR_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_FULL;
    } else if (settings_data.tooltips == TOOLTIPS_SOME) {
        g_menu_items[MENU_ITEM_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_SOME;
        g_menu_items[MENU_ITEM_EDITOR_HELP_TOOLTIPS].text_number = HELP_MENU_MOUSE_HELP_SOME;
    }
    if (settings_data.warnings == 1) {
        g_menu_items[MENU_ITEM_HELP_WARNINGS].text_number = HELP_MENU_WARNINGS_ON;
    }
    g_menu_items[MENU_ITEM_OPTIONS_AUTOSAVE].text_number = settings_data.autosave ? OPTIONS_MENU_AUTOSAVE_ON : OPTIONS_MENU_AUTOSAVE_OFF;
    g_menu_items[MENU_ITEM_OPTIONS_PYRAMID_SPEEDUP].text_number = settings_data.pyramid_speedup ? OPTIONS_MENU_PYRAMID_SPEEDUP_ON : OPTIONS_MENU_PYRAMID_SPEEDUP_OFF;
    g_menu_items[MENU_ITEM_OPTIONS_CITY_NAMES].text_number = settings_data.classical_names ? OPTIONS_MENU_CITIES_CLASSICAL : OPTIONS_MENU_CITIES_EGYPTIAN;
}

int32_t settings_load(void)
{
    settings_set_defaults();
    settings_read_ini();
    if (settings_data.ram < SETTINGS_LOW_RAM) {
        settings_data.game_speed = 100;
        settings_data.scroll_speed = 100;
        settings_data.display_mode = DISPLAY_MODE_640X480;
    }
    int32_t screen_width = platform_desktop_width();
    if (screen_width < 800) {
        settings_data.display_mode = DISPLAY_MODE_640X480;
    } else if (screen_width < 1024) {
        settings_data.display_mode = DISPLAY_MODE_800X600;
    }
    settings_data.display_mode_start = settings_data.display_mode;
    settings_start_campaign(0);
    settings_data.last_advisor = ADVISOR_LABOR;
    io_read_file_whole(str_pharaoh_inf, &settings_data);
    settings_data.check_cd = 1;
    settings_read_ini();
    settings_loaded = 1;
    settings_data.paused = 0;
    settings_set_menu_texts();
    return 1;
}

void settings_load_default_names(void)
{
    strcpy(settings_data.player_name, lang_get_string(STARTUP_TEXT_DEFAULT_NAMES, STARTUP_TEXT_PLAYER_NAME));
    const char *file_name = lang_get_string(STARTUP_TEXT_DEFAULT_NAMES, STARTUP_TEXT_FILE_NAME);
    strcpy(file_dialog_name, file_name);
    strcpy(file_dialog_last_name, file_name);
    strcpy(scenario_map_name, lang_get_string(STARTUP_TEXT_DEFAULT_NAMES, STARTUP_TEXT_MAP_NAME));
}

static void loading_stage(int32_t stage, int32_t text)
{
    window_loading_draw_stage(stage);
    const char *loading_text = lang_get_string(STARTUP_TEXT_LOADING, text);
    platform_draw_loading_text(loading_text);
    graphics_refresh_screen(1);
    platform_draw_loading_text(loading_text);
}

static void load_next_sg_file(void)
{
    image_collection_load_one((const char **) sg_file_names);
}

int32_t game_setup(void)
{
    io_read_file_into_buffer(str_pharaohmap_inf, &map_info, 0);
    settings_data.debug_grid_overlay = 16;
    game_setup_started = 1;
    platform_dir_create(str_save_folder);
    platform_dir_create(str_maps_folder);
    int32_t size = (g_screen_height * g_screen_width) * 2;
    if (size <= SPARE_SCREEN_MIN_BYTES) {
        size = SPARE_SCREEN_MIN_BYTES;
    }
    spare_screen = malloc((size_t) size);
    load_next_sg_file();
    loading_stage(0, 7);
    if (spare_screen == 0) {
        return 1;
    }
    load_next_sg_file();
    loading_stage(1, 8);
    load_next_sg_file();
    loading_stage(2, 9);
    for (int32_t i = 0; i < 5; i++) {
        load_next_sg_file();
    }
    loading_stage(3, 10);
    image_collection_load_all((const char **) sg_file_names);
    loading_stage(3, 11);
    window_loading_draw_background();
    graphics_refresh_screen(1);
    log_info_text("OK :Setup loaded DAT files.", 0);
    return 0;
}

static void game_state_init_text_inputs(void)
{
    keyboard_field_index = 0;
    keyboard_start_capture(KEYBOARD_FIELD_PLAYER_NAME, settings_data.player_name, 31, 104, 1, image_group(IMAGE_GROUP_FONT_SMALL));
    keyboard_start_capture(KEYBOARD_FIELD_FILE_NAME, file_dialog_name, 32, 280, 1, image_group(IMAGE_GROUP_FONT_WHITE));
    keyboard_start_capture(KEYBOARD_FIELD_MAP_NAME, scenario_map_name, 32, 280, 1, image_group(IMAGE_GROUP_FONT_WHITE));
    keyboard_start_capture(KEYBOARD_FIELD_SCENARIO_NAME, scenario_data.name, 24, 280, 0, image_group(IMAGE_GROUP_FONT_SMALL));
    keyboard_start_capture(KEYBOARD_FIELD_SCENARIO_BRIEFING, scenario_data.briefing, 500, 400, 0, image_group(IMAGE_GROUP_FONT_SMALL));
    keyboard_start_capture(KEYBOARD_FIELD_CHEAT, cheat_input_text, 256, 280, 0, image_group(IMAGE_GROUP_FONT_SMALL));
}

void game_state_init(void)
{
    city_victory_state = 0;
    g_scenario_climate = 0;
    debug_evolve_info_enabled = 0;
    debug_building_info_enabled = 0;
    debug_figure_info_enabled = 0;
    map_ring_init();
    city_view_orientation = 0;
    city_view_calculate_lookup(0);
    if (g_sidebar_collapsed == 1) {
        city_view_set_viewport_without_sidebar();
    } else {
        city_view_set_viewport_with_sidebar();
    }
    map_info.camera_x = 109;
    map_info.camera_y = 218;
    map_info.orientation = 0;
    city_view_calculate_map_bounds();
    city_view_check_camera_boundaries();
    random_generate_pool();
    game_state_init_text_inputs();
    city_warning_clear_all();
}

void sidebar_init_positions(void)
{
    int32_t width = g_screen_width;
    if (!screen_layout_is_standard_size()) {
        city_view_width_expanded = screen_layout_view_width(0);
        city_view_width_collapsed = screen_layout_view_width(1);
    } else if (width == 640) {
        city_view_width_expanded = 478;
        city_view_width_collapsed = 598;
    } else if (width == 800) {
        city_view_width_expanded = 598;
        city_view_width_collapsed = 718;
    } else if (width == 1024) {
        city_view_width_expanded = 838;
        city_view_width_collapsed = 958;
    }
}

void platform_measure_speed(void)
{
    uint32_t start = platform_time_ms();
    speed_measure_elapsed = (int32_t) (platform_time_ms() - start);
    start = platform_time_ms();
    for (int32_t i = 3; i > 0; i--) {
        graphics_refresh_screen(1);
    }
    int32_t refresh = (int32_t) (platform_time_ms() - start);
    speed_times = refresh;
    g_slide_speed_metric = refresh + speed_measure_elapsed;
}
