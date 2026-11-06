// The scenario editor's menu bar and sidebar buttons: File (new, open and save map, with the map file dialog), the
// "clear" entries of the special points, Help > mission editor guide, the sidebar's tool, preview, enemy, climate and
// soldier image buttons, the empire button and the legion window's exit.
//
// Saving first checks that the special points lie on valid terrain and that at most four kinds of food are traded;
// a failed check shows its "Can't Save" message and stops.

#include "editor/editor_menu.h"
#include "building/building_type.h"
#include "city/message.h"
#include "core/text.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_new_map_tiles.h"
#include "editor/editor_select_list.h"
#include "editor/editor_terrain_tiles.h"
#include "editor/editor_terrain_tool.h"
#include "empire/empire_helpers.h"
#include "figure/animals.h"
#include "figure/animals_wild.h"
#include "figure/figure_phrase.h"
#include "figure/formation.h"
#include "figure/natives.h"
#include "flood/flood_internal.h"
#include "game/game_data.h"
#include "game/scenario_start.h"
#include "graphics/label_animation_slots.h"
#include "io/file.h"
#include "io/file_write.h"
#include "io/game_file.h"
#include "io/game_file_relayout.h"
#include "io/io_data.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/moisture.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "platform/platform_shell.h"
#include "platform/platform_window.h"
#include "scenario/invasion.h"
#include "scenario/scenario_climate.h"
#include "text/lang.h"
#include "window/build_menu.h"
#include "window/building_info.h"
#include "window/building_info_military.h"
#include "window/building_menu.h"
#include "window/empire_window.h"
#include "window/file_list.h"
#include "window/input_dialogs.h"
#include "window/main_menu.h"
#include "window/message_box.h"
#include "window/popup_dialog.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_input.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define EDITOR_POINT_BLOCKING_TERRAIN \
    (TERRAIN_ROCK | TERRAIN_GARDEN | TERRAIN_RUBBLE | TERRAIN_WALL | TERRAIN_GATEHOUSE | TERRAIN_DIKE | TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_MONUMENT | \
        TERRAIN_ROCK_NEAR_MONUMENT)

enum editor_menu_values {
    MAP_SIZE_LIST_X = 50,
    MAP_SIZE_LIST_Y = 50,
    MAP_SIZE_LIST_ENTRIES = 7,
    ENEMY_LIST_X = 20,
    ENEMY_LIST_Y = 40,
    ENEMY_LIST_ENTRIES = 12,
    LIST_ROWS_AUTO = -1,
    MAP_FILE_DIALOG_LOAD = 1,
    MAP_FILE_DIALOG_SAVE = 0,
    MAP_FILE_BUTTON_CANCEL = 0,
    MAP_FILE_BUTTON_OK = 1,
    MAP_FILE_PATH_SIZE = 100,
    MAP_FILE_EXTENSION_LENGTH = 3,
    MAP_FILE_ERROR_FRAMES = 100,
};

void help_menu_editor_guide(void)
{
    platform_window_minimize(platform_main_window);
    platform_shell_open(platform_main_window, "open", "mission editor guide.txt");
}

void editor_clear_prey_points(void)
{
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        scenario_data.prey_herd_x[i] = -1;
        scenario_data.prey_herd_y[i] = -1;
    }
}

void editor_clear_predator_points(void)
{
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        scenario_data.predator_herd_x[i] = -1;
        scenario_data.predator_herd_y[i] = -1;
    }
}

void editor_clear_fishing_points(void)
{
    for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
        scenario_data.fishing_point_x[i] = -1;
        scenario_data.fishing_point_y[i] = -1;
    }
}

void editor_clear_invasion_landings(void)
{
    for (int32_t i = 0; i < SCENARIO_INVASION_LANDINGS; i++) {
        scenario_data.invasion_landing_x[i] = -1;
        scenario_data.invasion_landing_y[i] = -1;
    }
}

void editor_clear_invasion_points(void)
{
    for (int32_t i = 0; i < SCENARIO_INVASION_POINTS; i++) {
        scenario_data.invasion_point_x[i] = -1;
        scenario_data.invasion_point_y[i] = -1;
    }
}

void editor_map_refresh(void)
{
    if (window_current_id != WINDOW_CITY || editor_active != 1) {
        return;
    }
    editor_fix_all();
    map_pyramid_redraw();
    map_tiles_update_all();
    floods_build(&floods_data, 0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    farm_update_map();
}

void editor_menu_new_map(void)
{
    editor_select_list_show(MAP_SIZE_LIST_X, MAP_SIZE_LIST_Y, MAP_SIZE_LIST_ENTRIES, LANG_GROUP_MAP_SIZES, 0, 0, 0, &editor_new_map_size,
        SELECT_LIST_USE_NEW_SCENARIO, INPUT_TARGET_INT32, 0, 0, 0, LIST_ROWS_AUTO, 0);
    window_redraw_request = 1;
}

static void editor_open_map_dialog(uint8_t type)
{
    dir_list_files(1);
    keyboard_accepted = 0;
    window_current_id = WINDOW_MAP_FILE_DIALOG;
    file_dialog_type = type;
    label_animation[LABEL_SLOT_FILE_DIALOG_SCROLL] = 0;
    window_redraw_request = 1;
}

void editor_menu_open_map(void)
{
    editor_open_map_dialog(MAP_FILE_DIALOG_LOAD);
}

static uint32_t terrain_at(int32_t x, int32_t y)
{
    return map_grids.terrain[map_grid_offset(x, y)];
}

static int32_t point_on_water(int32_t x, int32_t y)
{
    return (terrain_at(x, y) & TERRAIN_WATER) != 0;
}

bool editor_fishing_points_valid(void)
{
    for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
        int32_t x = scenario_data.fishing_point_x[i];
        int32_t y = scenario_data.fishing_point_y[i];
        if (x >= 0 && y >= 0 && !point_on_water(x, y)) {
            return false;
        }
    }
    return true;
}

bool editor_terrain_walkable(int32_t terrain)
{
    uint32_t bits = (uint32_t) terrain;
    return (bits & EDITOR_POINT_BLOCKING_TERRAIN) == 0 && (bits & (TERRAIN_WATER | TERRAIN_MARSH)) == 0;
}

static int32_t edge_point_set(int32_t x, int32_t y)
{
    return x > 0 || y > 0;
}

bool editor_entry_exit_valid(void)
{
    int32_t x = scenario_data.entry_point_x;
    int32_t y = scenario_data.entry_point_y;
    if (edge_point_set(x, y) && !editor_terrain_walkable(terrain_at(x, y))) {
        return false;
    }
    x = scenario_data.exit_point_x;
    y = scenario_data.exit_point_y;
    if (edge_point_set(x, y) && !editor_terrain_walkable(terrain_at(x, y))) {
        return false;
    }
    return true;
}

bool editor_river_points_valid(void)
{
    int32_t x = scenario_data.river_entry_x;
    int32_t y = scenario_data.river_entry_y;
    if (edge_point_set(x, y) && !point_on_water(x, y)) {
        return false;
    }
    x = scenario_data.river_exit_x;
    y = scenario_data.river_exit_y;
    if (edge_point_set(x, y) && !point_on_water(x, y)) {
        return false;
    }
    return true;
}

bool editor_invasion_points_valid(void)
{
    for (int32_t i = 0; i < SCENARIO_LAND_INVASION_POINTS; i++) {
        int32_t x = scenario_data.invasion_point_x[i];
        int32_t y = scenario_data.invasion_point_y[i];
        if (x >= 0 && y >= 0 && !editor_terrain_walkable(terrain_at(x, y))) {
            return false;
        }
    }
    return true;
}

bool editor_sea_invasion_points_valid(void)
{
    for (int32_t i = SCENARIO_LAND_INVASION_POINTS; i < SCENARIO_INVASION_POINTS; i++) {
        int32_t x = scenario_data.invasion_point_x[i];
        int32_t y = scenario_data.invasion_point_y[i];
        if (x >= 0 && y >= 0 && !point_on_water(x, y)) {
            return false;
        }
    }
    return true;
}

bool editor_food_kinds_valid(void)
{
    return empire_count_food_kinds() <= FOOD_KINDS_MAX;
}

bool map_editor_points_terrain_valid(void)
{
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        int32_t x = scenario_data.predator_herd_x[i];
        int32_t y = scenario_data.predator_herd_y[i];
        if (x >= 0 && y >= 0 && !animal_terrain_allowed_climate(map_grids.terrain[map_info.grid_start + x + y * GRID_SIZE], 0)) {
            return 0;
        }
    }
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        int32_t x = scenario_data.prey_herd_x[i];
        int32_t y = scenario_data.prey_herd_y[i];
        if (x >= 0 && y >= 0 && !animal_terrain_allowed_climate(map_grids.terrain[x + y * GRID_SIZE + map_info.grid_start], 1)) {
            return 0;
        }
    }
    return 1;
}

static void show_save_check_box(int32_t title_text)
{
    const char *title = lang_get_string(LANG_GROUP_POPUP, title_text);
    const char *text = lang_get_string(LANG_GROUP_POPUP, title_text + 1);
    window_message_box_show(title, text, 0);
}

static int32_t save_check_failed(void)
{
    if (!map_editor_points_terrain_valid()) {
        return POPUP_HERD_POINTS_INVALID;
    }
    if (!editor_fishing_points_valid()) {
        return POPUP_FISHING_POINTS_INVALID;
    }
    if (!editor_entry_exit_valid()) {
        return POPUP_ENTRY_EXIT_POINTS_INVALID;
    }
    if (!editor_river_points_valid()) {
        return POPUP_RIVER_POINTS_INVALID;
    }
    if (!editor_invasion_points_valid()) {
        return POPUP_LAND_INVASION_POINTS_INVALID;
    }
    if (!editor_sea_invasion_points_valid()) {
        return POPUP_SEA_INVASION_POINTS_INVALID;
    }
    if (!editor_food_kinds_valid()) {
        return POPUP_TOO_MANY_FOOD_KINDS;
    }
    return 0;
}

void editor_menu_save_map(void)
{
    int32_t failed = save_check_failed();
    if (failed) {
        show_save_check_box(failed);
        return;
    }
    editor_open_map_dialog(MAP_FILE_DIALOG_SAVE);
}

static void map_file_dialog_ok(void)
{
    char path[MAP_FILE_PATH_SIZE];
    file_remove_extension(scenario_map_name);
    memcpy(file_extension, map_file_extension, MAP_FILE_EXTENSION_LENGTH);
    file_append_extension(scenario_map_name);
    strcpy(path, maps_directory);
    strcat(path, scenario_map_name);
    if (file_dialog_type == MAP_FILE_DIALOG_LOAD) {
        if (!file_exists(path)) {
            file_remove_extension(scenario_map_name);
            file_dialog_error_frames = MAP_FILE_ERROR_FRAMES;
            return;
        }
        scenario_load_map();
    } else {
        game_file_io_write_scenario(path);
    }
    file_remove_extension(scenario_map_name);
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void editor_map_dialog_button(void)
{
    int32_t parameter = button_parameter;
    if (parameter == MAP_FILE_BUTTON_OK) {
        map_file_dialog_ok();
        return;
    }
    if (parameter != MAP_FILE_BUTTON_CANCEL) {
        return;
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

static int32_t tool_build_menu(int32_t tool)
{
    switch (tool) {
        case EDITOR_TOOL_BRUSH_SIZE:
            return BUILD_MENU_EDITOR_BRUSH_SIZE;
        case EDITOR_TOOL_WATER:
            return BUILD_MENU_EDITOR_WATER;
        case EDITOR_TOOL_MIGRATION_POINT:
            return BUILD_MENU_EDITOR_ENTRY_EXIT_POINTS;
        case EDITOR_TOOL_INVASION_POINT:
            return BUILD_MENU_EDITOR_INVASION_POINTS;
        case EDITOR_TOOL_KILLER_TYPE:
            return BUILD_MENU_EDITOR_ANIMALS;
        case EDITOR_TOOL_PLATEAU:
            return BUILD_MENU_EDITOR_ELEVATION;
        case EDITOR_TOOL_RIVER_POINT:
            return BUILD_MENU_EDITOR_RIVER_POINTS;
        case EDITOR_TOOL_NATIVES:
            return BUILD_MENU_EDITOR_NATIVES;
        case EDITOR_TOOL_FISHING_WATERS:
            return BUILD_MENU_EDITOR_SPECIAL_POINTS;
        case EDITOR_TOOL_ROCKS:
            return BUILD_MENU_EDITOR_ROCKS;
        default:
            return -1;
    }
}

static void open_tool_build_menu(int32_t group)
{
    window_current_id = WINDOW_BUILD_MENU;
    build_menu_group = group;
    int32_t count = building_menu_count_items();
    int32_t y_offset = build_menu_types[BUILD_MENU_SUB_POSITIONS_FIRST + count * 2];
    build_menu_item_count = count;
    build_menu_y_offset = y_offset;
    window_redraw_request = 1;
}

static void set_tool_construction_type(int32_t tool)
{
    switch (tool) {
        case EDITOR_TOOL_GRASS:
            construction_building_type = BUILDING_EDITOR_GRASS;
            break;
        case EDITOR_TOOL_TREES:
            construction_building_type = BUILDING_EDITOR_TREES;
            break;
        case EDITOR_TOOL_EARTHQUAKE:
            construction_building_type = BUILDING_EDITOR_EARTHQUAKE;
            break;
        case EDITOR_TOOL_SCRUB:
            construction_building_type = BUILDING_PLATEAU;
            break;
        case EDITOR_TOOL_MEADOW:
            construction_building_type = BUILDING_EDITOR_MEADOW;
            break;
        case EDITOR_TOOL_ROAD:
            construction_building_type = BUILDING_ROAD;
            break;
        case EDITOR_TOOL_INVASION_POINT:
            construction_building_type = BUILDING_EDITOR_INVASION_POINT;
            break;
        case EDITOR_TOOL_FLOODPLAIN:
            construction_building_type = BUILDING_EDITOR_FLOODPLAIN;
            break;
        default:
            break;
    }
}

void editor_sidebar_tool(void)
{
    if (button_parameter != EDITOR_TOOL_BRUSH_SIZE) {
        construction_cancel();
    }
    int32_t group = tool_build_menu(button_parameter);
    if (group >= 0) {
        open_tool_build_menu(group);
        return;
    }
    int32_t tool = button_parameter;
    editor_tool_kind = tool;
    set_tool_construction_type(tool);
    construction_in_progress = 0;
    construction_end_x = 0;
    construction_start_x = 0;
    construction_end_y = 0;
    construction_start_y = 0;
    if (construction_building_type == BUILDING_ROAD) {
        map_routing_update_land_citizen();
    }
    window_redraw_request = 1;
}

void editor_sidebar_preview(void)
{
    construction_building_type = 0;
    editor_map_preview = button_parameter;
    window_redraw_request = 1;
}

void editor_sidebar_enemy(void)
{
    editor_select_list_show(ENEMY_LIST_X, ENEMY_LIST_Y, ENEMY_LIST_ENTRIES, LANG_GROUP_ENEMY_NAMES, 0, 0, &scenario_data.enemy_type, 0, 0, INPUT_TARGET_INT16, 0, 0,
        0, LIST_ROWS_AUTO, 0);
    window_redraw_request = 1;
}

void editor_sidebar_climate(void)
{
    uint8_t climate = (uint8_t) (scenario_data.climate_type + 1);
    scenario_data.climate_type = climate;
    if (climate >= CLIMATE_COUNT) {
        scenario_data.climate_type = 0;
    }
    map_pyramid_redraw_pending = 1;
    editor_set_climate(scenario_data.climate_type);
    window_redraw_request = 1;
}

void editor_sidebar_soldier_images(void)
{
    uint8_t value = scenario_data.soldier_images_alt;
    window_redraw_request = 1;
    scenario_data.soldier_images_alt = value != 1;
}

static void editor_open_window(uint8_t window)
{
    window_current_id = window;
    window_redraw_request = 1;
}

void editor_open_flood(void)
{
    editor_open_window(WINDOW_EDITOR_FLOOD);
}

void editor_open_allowed_buildings(void)
{
    editor_open_window(WINDOW_EDITOR_ALLOWED_BUILDINGS);
}

void editor_open_win_criteria(void)
{
    editor_open_window(WINDOW_EDITOR_WIN_CRITERIA);
}

void editor_open_event_list(void)
{
    editor_open_window(WINDOW_EDITOR_EVENT_LIST);
}

void editor_open_gods(void)
{
    editor_open_window(WINDOW_EDITOR_GODS);
}

void editor_open_settings(void)
{
    editor_open_window(WINDOW_EDITOR_SETTINGS);
}

void editor_button_empire(void)
{
    if (editor_active != 1 && settings_data.is_custom_scenario == 0 && scenario_start_flags[SCENARIO_START_FLAG_EMPIRE_LOCKED] != 0) {
        city_warning_show(WARNING_NOT_AVAILABLE_IN_THIS_ASSIGNMENT);
        return;
    }
    if (window_current_id == WINDOW_CITY) {
        button_empire_open();
    } else {
        button_empire_exit();
    }
}

static void legion_order_for_layout(int32_t layout)
{
    if (layout == FORMATION_DOUBLE_LINE_1 || layout == FORMATION_DOUBLE_LINE_2) {
        button_parameter = LEGION_ORDER_HOLD_TIGHT;
    } else if (layout == FORMATION_SINGLE_LINE_1 || layout == FORMATION_SINGLE_LINE_2) {
        button_parameter = LEGION_ORDER_HOLD_LOOSE;
    } else if (layout == FORMATION_CHARGE) {
        button_parameter = LEGION_ORDER_ENGAGE;
    }
}

void building_info_legion_exit(void)
{
    if (building_info_orders_shown != 0) {
        building_info_orders_shown = 0;
        window_redraw_request = 1;
        return;
    }
    if (building_info_mode == BUILDING_INFO_MODE_LEGION) {
        if (settings_data.paused != 0) {
            return;
        }
        legion_order_for_layout(formation_get(formation_selected)->layout);
        legion_order_button();
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}
