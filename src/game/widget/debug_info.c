// Developer overlays of the city window: the city, evolve, building and figure info panels and the grid values
// over the city view, each drawn only while its flag is on (never in a normal game). Also debug_frame_update,
// which times the frame and drives the two debug sprite objects (null in the game).
//
// The city info panel walks eight player records from player 1, past the two records of city_players.

#include "widget/debug_info.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_population.h"
#include "city/city_data.h"
#include "city/city_player.h"
#include "figure/figure.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "graphics/color.h"
#include "graphics/debug_sprite.h"
#include "graphics/graphics_data.h"
#include "graphics/image_blit.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_blocks.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "io/io_data.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/service_visit.h"
#include "platform/platform_input.h"
#include "platform/platform_time.h"
#include "text/lang.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

static const float DEBUG_KEY_STEP = 1.0f;
static const float DEBUG_KEY_MIN = 10.0f;
static const float DEBUG_KEY_MAX = 80.0f;

int32_t debug_road_total_connections;

int32_t debug_road_connects;

debug_test_value_set debug_test_values;

int32_t debug_was_in_set;

enum debug_text_color {
    DEBUG_COLOR_LABEL = COLOR_WHITE,
    DEBUG_COLOR_VALUE = 0x3EFA,
    DEBUG_COLOR_WARNING = COLOR_RED,
};

enum debug_number_prefix {
    DEBUG_NUMBER_PREFIX_HOUSE_MODEL = '(',
    DEBUG_NUMBER_PREFIX_GRID = ' ',
};

enum debug_panel_layout {
    DEBUG_PANEL_WIDTH = 192,
    DEBUG_PANEL_WIDTH_BLOCKS = 12,
    DEBUG_PANEL_HEIGHT_BLOCKS = 15,
    DEBUG_BUILDING_PANEL_HEIGHT_BLOCKS = 19,
    DEBUG_PANEL_STYLE = 1,
    DEBUG_PANEL_Y = 220,
    DEBUG_BUILDING_PANEL_Y = 156,
    DEBUG_COLUMN_LEFT = 8,
    DEBUG_COLUMN_RIGHT = 100,
    DEBUG_COLUMN_RIGHT_WIDE = 108,
    DEBUG_COLUMN_RIGHT_FAR = 110,
    DEBUG_HOUSE_COLUMN_2 = 50,
    DEBUG_HOUSE_COLUMN_4 = 150,
    DEBUG_HOUSE_COLUMN_5 = 200,
};

enum debug_city_info_layout {
    DEBUG_CITY_FIRST_PLAYER = 1,
    DEBUG_CITY_FIRST_ROW_Y = 94,
    DEBUG_CITY_ROWS_END_Y = 445,
    DEBUG_CITY_PLAYER_ROWS_HEIGHT = 50,
    DEBUG_CITY_LINE_HEIGHT = 14,
};

enum debug_grid {
    DEBUG_GRID_BUILDING_IDS = 0,
    DEBUG_GRID_TERRAIN = 1,
    DEBUG_GRID_BITFIELDS = 2,
    DEBUG_GRID_IMAGES = 3,
    DEBUG_GRID_EDGE = 4,
    DEBUG_GRID_CANALS = 5,
    DEBUG_GRID_RANDOM = 6,
    DEBUG_GRID_FIGURE_IDS = 7,
    DEBUG_GRID_SPRITE = 8,
    DEBUG_GRID_ROUTING_LAND_CITIZEN = 9,
    DEBUG_GRID_ROUTING_DISTANCE = 10,
    DEBUG_GRID_BUILDING_WATER_ACCESS = 11,
    DEBUG_GRID_BUILDING_HOUSES_COVERED = 12,
    DEBUG_GRID_BUILDING_DISTANCE_FROM_ENTRY = 13,
    DEBUG_GRID_BUILDING_DAMAGE_RISK = 14,
    DEBUG_GRID_BUILDING_POPULATION = 15,
    DEBUG_GRID_DESIRABILITY = 16,
    DEBUG_GRID_ELEVATION = 17,
    DEBUG_GRID_ROUTING_WATER = 18,
    DEBUG_GRID_ROUTING_LAND_NONCITIZEN = 19,
    DEBUG_GRID_BUILDING_DAMAGE = 20,
    DEBUG_GRID_FIGURE_COUNT = 21,
    DEBUG_GRID_OLD_TYPE = 22,
    DEBUG_GRID_SOLDIER_STRENGTH = 23,
    DEBUG_GRID_ROUTING_WALLS = 24,
    DEBUG_GRID_ROAD_NETWORK = 25,
    DEBUG_GRID_ROAD_SEGMENT = 26,
    DEBUG_GRID_SERVICE_VISITS = 27,
};

enum debug_grid_value_layout {
    DEBUG_GRID_SERVICE = 1,
    DEBUG_GRID_VALUE_X = 24,
    DEBUG_GRID_VALUE_Y = 10,
    DEBUG_GRID_COLUMN_GAP = 2,
};

enum debug_granary_layout {
    DEBUG_GRANARY_RESOURCES = 7,
    DEBUG_GRANARY_RESOURCE_STEP_BACK = 4,
};

enum debug_key {
    DEBUG_KEY_PRESSED = 0x8001,
};

enum debug_sprite_place {
    DEBUG_OBJECT_X = 350,
    DEBUG_OBJECT_Y = 300,
    DEBUG_SPRITE_X = 450,
    DEBUG_SPRITE_Y = 300,
};

typedef struct {
    uint8_t reserved_1;
    uint8_t has_offset_record;
    int16_t width;
    int16_t height;
    uint8_t reserved_2[2];
    union {
        int16_t dx;
        const int16_t *offset_record;
    } x;
    const uint8_t *data;
} debug_sprite_draw_frame;

static void num(int32_t value, const char *label, int32_t x, int32_t y, int32_t color)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, label, x, y, image_group(IMAGE_GROUP_FONT_SMALL), color, ui_origin_x, ui_origin_y);
}

static void label(const char *text, int32_t x, int32_t y, int32_t color)
{
    text_draw((const uint8_t *) text, x, y, image_group(IMAGE_GROUP_FONT_SMALL), color);
}

static void title(const char *text, int32_t y, int32_t font, int32_t color)
{
    text_draw_centered((const uint8_t *) text, 0, y, DEBUG_PANEL_WIDTH, font, color);
}

static void house_model_value(int32_t value, int32_t x, int32_t y)
{
    text_draw_number_ui(value, DEBUG_NUMBER_PREFIX_HOUSE_MODEL, ")", x, y, image_group(IMAGE_GROUP_FONT_SMALL), DEBUG_COLOR_VALUE, ui_origin_x, ui_origin_y);
}

void debug_draw_city_info(void)
{
    if (debug_city_info_enabled != 1) {
        return;
    }
    int32_t player = DEBUG_CITY_FIRST_PLAYER;
    for (int32_t y = DEBUG_CITY_FIRST_ROW_Y; y < DEBUG_CITY_ROWS_END_Y; y += DEBUG_CITY_PLAYER_ROWS_HEIGHT, player++) {
        const city_player *record = city_player_get(player);
        int32_t color = DEBUG_COLOR_LABEL;
        g_text_width = 0;
        if (record->in_use != 1) {
            continue;
        }
        int32_t y0 = y - DEBUG_CITY_LINE_HEIGHT;
        g_text_width = 0;
        label("Pop is ", DEBUG_COLUMN_LEFT, y0, DEBUG_COLOR_LABEL);
        num(record->population, postfix_none, g_text_width + 8, y0, DEBUG_COLOR_LABEL);
        label("  tot cap  ", g_text_width + 8, y0, DEBUG_COLOR_LABEL);
        num(record->total_capacity, postfix_none, g_text_width + 8, y0, DEBUG_COLOR_LABEL);
        label("  spare cap  ", g_text_width + 8, y0, DEBUG_COLOR_LABEL);
        num(record->room_in_houses, postfix_none, g_text_width + 8, y0, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        label("  births  ", DEBUG_COLUMN_LEFT, y, DEBUG_COLOR_LABEL);
        num(record->yearly_births, postfix_none, g_text_width + 8, y, DEBUG_COLOR_LABEL);
        label("  deaths  ", g_text_width + 8, y, DEBUG_COLOR_LABEL);
        num(record->yearly_deaths, postfix_none, g_text_width + 8, y, DEBUG_COLOR_LABEL);
        int32_t in_houses = house_population_calculate_people_per_type(city_current_player);
        if (in_houses != record->population) {
            color = DEBUG_COLOR_WARNING;
        }
        label("  in houses ", g_text_width + 8, y, color);
        num(in_houses, postfix_none, g_text_width + 8, y, color);
        const city_player *current = city_player_current();
        int32_t y2 = y + DEBUG_CITY_LINE_HEIGHT;
        g_text_width = 0;
        label("nof_docks=  ", DEBUG_COLUMN_LEFT, y2, DEBUG_COLOR_LABEL);
        num(building_count_total[BUILDING_DOCK], "   ", g_text_width + 8, y2, DEBUG_COLOR_LABEL);
        label("docks_list=  ", g_text_width + 8, y2, DEBUG_COLOR_LABEL);
        for (int32_t i = 0; i < (int32_t) (sizeof(current->working_dock_ids) / sizeof(current->working_dock_ids[0])); i++) {
            num(current->working_dock_ids[i], ",  ", g_text_width + 4, y2, DEBUG_COLOR_LABEL);
        }
        g_text_width = 0;
        label("wine sources =  ", DEBUG_COLUMN_LEFT, y + 28, DEBUG_COLOR_LABEL);
        num(current->luxury_goods_kinds, "   ", g_text_width + 8, y + 28, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(map_info.orientation, " MAP ROTATION", DEBUG_COLUMN_LEFT, y + 42, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(city_overlay, " map_style", DEBUG_COLUMN_LEFT, y + 56, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(current->natives_hostile, " natives_are_restless", DEBUG_COLUMN_LEFT, y + 70, DEBUG_COLOR_LABEL);
        num(current->mission_post_operational, " mission_in_place", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 70, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(phrase_weight_block.was_in_int, " was_in_int", DEBUG_COLUMN_LEFT, y + 84, DEBUG_COLOR_LABEL);
        num(debug_was_in_set, " was_in_set", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 84, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(city_message_problem_count, " alert_messages", DEBUG_COLUMN_LEFT, y + 98, DEBUG_COLOR_LABEL);
        num(city_message_problem_area_b, " last_alert_message_shown", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 98, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(buildings_without_id_cleared, "Auto removed man mades tiles", DEBUG_COLUMN_LEFT, y + 112, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(debug_test_values.value1a, " test_value1a", DEBUG_COLUMN_LEFT, y + 126, DEBUG_COLOR_LABEL);
        num(debug_test_values.value2a, " test_value2a", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 126, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(debug_test_values.value3a, " test_value3a", DEBUG_COLUMN_LEFT, y + 140, DEBUG_COLOR_LABEL);
        num(debug_test_values.value4a, " test_value4a", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 140, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(mouse_x, " mouse_x", DEBUG_COLUMN_LEFT, y + 154, DEBUG_COLOR_LABEL);
        num(mouse_y, " mouse_y", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 154, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(mouse_moved, " mouse_movement", DEBUG_COLUMN_LEFT, y + 168, DEBUG_COLOR_LABEL);
        num(city_view_width_expanded, " side_panel_x1", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y + 168, DEBUG_COLOR_LABEL);
        g_text_width = 0;
        num(construction_in_progress, " building_in_progress", DEBUG_COLUMN_LEFT, y + 182, DEBUG_COLOR_LABEL);
    }
}

void debug_draw_evolve_info(void)
{
    if (debug_evolve_info_enabled != 1) {
        return;
    }
    int32_t y = (ui_origin_y * 2) + DEBUG_PANEL_Y;
    panel_draw_blocks(PANEL_BLOCKS_DEBUG, 0, y, DEBUG_PANEL_WIDTH_BLOCKS, DEBUG_PANEL_HEIGHT_BLOCKS, DEBUG_PANEL_STYLE);
    title("Evolve info (F3)", y + 6, image_group(IMAGE_GROUP_FONT_BLACK), DEBUG_COLOR_LABEL);
    num(game_time_tick, " = tick", DEBUG_COLUMN_LEFT, y + 26, DEBUG_COLOR_VALUE);
    num(game_time_day, " = week", DEBUG_COLUMN_RIGHT_FAR, y + 26, DEBUG_COLOR_VALUE);
    num(game_time_month, " = month", DEBUG_COLUMN_LEFT, y + 40, DEBUG_COLOR_VALUE);
    num(game_time_year, " = year", DEBUG_COLUMN_RIGHT_FAR, y + 40, DEBUG_COLOR_VALUE);
    num(building_highest_id, " = last building", DEBUG_COLUMN_LEFT, y + 54, DEBUG_COLOR_VALUE);
    num(building_highest_id_ever, " = ever", DEBUG_COLUMN_RIGHT_FAR, y + 54, DEBUG_COLOR_VALUE);
    num(debug_road_connects - 1, " = no of connects", DEBUG_COLUMN_LEFT, y + 68, DEBUG_COLOR_VALUE);
    num(debug_road_total_connections, " = total connections", DEBUG_COLUMN_LEFT, y + 96, DEBUG_COLOR_VALUE);
    int32_t error = debug_evolve_error;
    int32_t color = error != 0 ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE;
    num(error, " = evolve error", DEBUG_COLUMN_LEFT, y + 110, color);
    num(routing_stats_total_routes_calculated, " = routings", DEBUG_COLUMN_LEFT, y + 124, color);
    num(routing_stats_enemy_routes_calculated, " = barb", DEBUG_COLUMN_LEFT, y + 138, color);
}

static void draw_no_building_info(int32_t y)
{
    title("Not over a building", y + 120, image_group(IMAGE_GROUP_FONT_SMALL), DEBUG_COLOR_VALUE);
    num(map_info.selected_grid, " = over ptr", DEBUG_COLUMN_LEFT, y + 140, DEBUG_COLOR_LABEL);
    lang_text_draw_centered_ui(
        LANG_GROUP_DEBUG_GRIDS, settings_data.debug_grid_overlay, DEBUG_COLUMN_LEFT, y + 160, DEBUG_PANEL_WIDTH, image_group(IMAGE_GROUP_FONT_SMALL), DEBUG_COLOR_LABEL, ui_origin_x, ui_origin_y);
    num(undo_ready, " = undo_ready", DEBUG_COLUMN_LEFT, y + 180, DEBUG_COLOR_LABEL);
    num(undo_available, " = undo_valid", DEBUG_COLUMN_LEFT, y + 200, DEBUG_COLOR_LABEL);
    num(undo_num_buildings, " = undo_buildings", DEBUG_COLUMN_LEFT, y + 220, DEBUG_COLOR_LABEL);
    num(building_get(undo_building_ids[0])->state, " = alive", DEBUG_COLUMN_LEFT, y + 240, DEBUG_COLOR_LABEL);
    num(undo_buildings[0].state, " = alive", DEBUG_COLUMN_RIGHT, y + 240, DEBUG_COLOR_LABEL);
    g_text_width = 0;
    for (int32_t direction = 0; direction < DIRECTION_COUNT; direction++) {
        num(routing_grids.water[map_info.selected_grid + map_grid_direction_delta[direction]], (const char *) str_comma, g_text_width + 8, y + 260, DEBUG_COLOR_VALUE);
    }
    num(window_ui_redraw, " = redraw", DEBUG_COLUMN_RIGHT, y + 280, DEBUG_COLOR_LABEL);
}

static int32_t shows_raw_materials(int32_t type)
{
    if (type >= BUILDING_BREWERY && type <= BUILDING_POTTER) {
        return 1;
    }
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_MORTUARY:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_JEWELER:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_ZOO:
        case BUILDING_ARTISANS_GUILD:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
            return 1;
        default:
            return 0;
    }
}

static void draw_building_stock(const building *b, int32_t y)
{
    if (b->type == BUILDING_WATER_LIFT) {
        g_text_width = 0;
        label("aqua links", DEBUG_COLUMN_LEFT, y, DEBUG_COLOR_VALUE);
        for (int32_t i = 0; i < (int32_t) (sizeof(b->data.water_lift.aqueduct_links) / sizeof(b->data.water_lift.aqueduct_links[0])); i++) {
            num(b->data.water_lift.aqueduct_links[i], postfix_none, g_text_width + 8, y, DEBUG_COLOR_VALUE);
        }
    } else if (b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP) {
        g_text_width = 0;
        label("food", DEBUG_COLUMN_LEFT, y, DEBUG_COLOR_VALUE);
        for (int32_t i = 0; i < DEBUG_GRANARY_RESOURCES; i++) {
            num(b->data.granary.resource_stored[i], postfix_none, g_text_width + 8, y, DEBUG_COLOR_VALUE);
            g_text_width = g_text_width - DEBUG_GRANARY_RESOURCE_STEP_BACK;
        }
    } else if (shows_raw_materials(b->type)) {
        g_text_width = 0;
        label("raw_materials", DEBUG_COLUMN_LEFT, y, DEBUG_COLOR_VALUE);
        num(b->loads_stored, postfix_none, g_text_width + 8, y, DEBUG_COLOR_VALUE);
    }
}

static void draw_entertainment_info(const building *b, int32_t y)
{
    num(b->data.entertainment.num_shows, " shows", DEBUG_COLUMN_LEFT, y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(b->data.entertainment.show_days_left[0], ", ", DEBUG_COLUMN_RIGHT_WIDE, y, DEBUG_COLOR_VALUE);
    num(b->data.entertainment.show_days_left[1], "  ", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y, DEBUG_COLOR_VALUE);
    num(b->data.entertainment.show_days_left[2], "  ", g_text_width + DEBUG_COLUMN_RIGHT_WIDE, y, DEBUG_COLOR_VALUE);
}

static void draw_house_info(const building *b, int32_t y)
{
    const building_house *house = &b->data.house;
    int32_t services_y = y + 192;
    g_text_width = 0;
    num(house->juggler, "th", DEBUG_COLUMN_LEFT, services_y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(house->musician, (const char *) str_comma, DEBUG_HOUSE_COLUMN_2, services_y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(house->dancer, postfix_none, DEBUG_COLUMN_RIGHT, services_y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(house->senet, "hi", DEBUG_HOUSE_COLUMN_4, services_y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(house->zoo, "hi", DEBUG_HOUSE_COLUMN_5, services_y, DEBUG_COLOR_VALUE);
    int32_t levels_y = y + 240;
    num(house->entertainment, " en", DEBUG_COLUMN_LEFT, levels_y, DEBUG_COLOR_VALUE);
    num(house->education, " ed", DEBUG_HOUSE_COLUMN_2, levels_y, DEBUG_COLOR_VALUE);
    num(house->health, " he", DEBUG_COLUMN_RIGHT, levels_y, DEBUG_COLOR_VALUE);
    num(house->num_gods, " re", DEBUG_HOUSE_COLUMN_4, levels_y, DEBUG_COLOR_VALUE);
    int32_t model_y = y + 252;
    const house_model *model = &house_models[b->subtype];
    house_model_value(model->entertainment, DEBUG_COLUMN_LEFT, model_y);
    house_model_value(model->education, DEBUG_HOUSE_COLUMN_2, model_y);
    house_model_value(model->religion, DEBUG_COLUMN_RIGHT, model_y);
    house_model_value(model->courthouse, DEBUG_HOUSE_COLUMN_4, model_y);
}

static void draw_goods(const int16_t *inventory, int32_t y)
{
    g_text_width = 0;
    num(inventory[INVENTORY_GRAIN], " ,", DEBUG_COLUMN_LEFT, y, DEBUG_COLOR_VALUE);
    num(inventory[INVENTORY_POTTERY], " ,", g_text_width + 8, y, DEBUG_COLOR_VALUE);
    num(inventory[INVENTORY_JEWELRY], " ,", g_text_width + 8, y, DEBUG_COLOR_VALUE);
    num(inventory[INVENTORY_LINEN], " ,", g_text_width + 8, y, DEBUG_COLOR_VALUE);
    num(inventory[INVENTORY_BEER], " goods", g_text_width + 8, y, DEBUG_COLOR_VALUE);
}

void debug_draw_building_info(void)
{
    if (debug_building_info_enabled != 1) {
        return;
    }
    int32_t y = (ui_origin_y * 2) + DEBUG_BUILDING_PANEL_Y;
    panel_draw_blocks(PANEL_BLOCKS_DEBUG, 0, y, DEBUG_PANEL_WIDTH_BLOCKS, DEBUG_BUILDING_PANEL_HEIGHT_BLOCKS, DEBUG_PANEL_STYLE);
    map_info.selected_grid = city_view_mouse_to_grid_offset();
    int32_t id = map_info.selected_grid == 0 ? 0 : map_grids.building_ids[map_info.selected_grid];
    title("Building info (F4)", y + 6, image_group(IMAGE_GROUP_FONT_BLACK), DEBUG_COLOR_LABEL);
    if (id == 0) {
        draw_no_building_info(y);
        return;
    }
    const building *b = building_get(id);
    num(id, " = building no", DEBUG_COLUMN_LEFT, y + 24, b->state != BUILDING_STATE_IN_USE ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE);
    num(b->type, " = type", DEBUG_COLUMN_LEFT, y + 60, DEBUG_COLOR_VALUE);
    num(b->subtype, " = sub_type", DEBUG_COLUMN_RIGHT, y + 60, DEBUG_COLOR_VALUE);
    draw_building_stock(b, y + 72);
    num(b->house_population, " = pop", DEBUG_COLUMN_LEFT, y + 84, DEBUG_COLOR_VALUE);
    num(b->loads_stored, " = goods", DEBUG_COLUMN_RIGHT, y + 84, DEBUG_COLOR_VALUE);
    num(b->houses_covered, " = acc", DEBUG_COLUMN_LEFT, y + 96, DEBUG_COLOR_VALUE);
    num(b->percentage_houses_covered, " = rel", DEBUG_COLUMN_RIGHT, y + 96, DEBUG_COLOR_VALUE);
    num(b->immigrant_figure_id, " = immigrant", DEBUG_COLUMN_LEFT, y + 108, DEBUG_COLOR_VALUE);
    if (b->figure_id2 != 0) {
        num(b->figure_id2, " = SUB agent", DEBUG_COLUMN_RIGHT, y + 108, DEBUG_COLOR_VALUE);
    } else if (b->figure_id != 0) {
        num(b->figure_id, " = agent", DEBUG_COLUMN_RIGHT, y + 108, DEBUG_COLOR_VALUE);
    }
    num(b->distance_from_entry, " = rome", DEBUG_COLUMN_LEFT, y + 120, DEBUG_COLOR_VALUE);
    num(b->road_network_id, " = roadnet", DEBUG_COLUMN_RIGHT, y + 120, DEBUG_COLOR_VALUE);
    num(b->house_population_room, " = spare room", DEBUG_COLUMN_LEFT, y + 132, DEBUG_COLOR_VALUE);
    num(b->damage_risk, " = damage", DEBUG_COLUMN_RIGHT, y + 132, DEBUG_COLOR_VALUE);
    if (b->has_water_access != 0) {
        num(b->has_water_access, " = water sup", DEBUG_COLUMN_LEFT, y + 144, DEBUG_COLOR_VALUE);
    } else if (b->has_well_access != 0) {
        num(b->has_well_access, " = well sup", DEBUG_COLUMN_LEFT, y + 144, DEBUG_COLOR_VALUE);
    } else {
        num(0, " = no water", DEBUG_COLUMN_LEFT, y + 144, DEBUG_COLOR_VALUE);
    }
    num(b->water_dependency, " = water dep", DEBUG_COLUMN_RIGHT, y + 144, DEBUG_COLOR_VALUE);
    num(b->unreachable_ticks, " = no contact with rome", DEBUG_COLUMN_LEFT, y + 156, DEBUG_COLOR_VALUE);
    num(b->num_workers, " = employees", DEBUG_COLUMN_RIGHT, y + 168, DEBUG_COLOR_VALUE);
    num(b->house_tax_coverage, " = tax cvr", DEBUG_COLUMN_LEFT, y + 180, DEBUG_COLOR_VALUE);
    num(b->house_highest_population, " = max pop", DEBUG_COLUMN_RIGHT, y + 180, DEBUG_COLOR_VALUE);
    int32_t is_house = b->house_size != 0;
    if (is_house) {
        draw_house_info(b, y);
    } else {
        draw_entertainment_info(b, y + 192);
    }
    num(map_info.selected_grid, " = over ptr", DEBUG_COLUMN_LEFT, y + 204, DEBUG_COLOR_LABEL);
    if (is_house) {
        draw_goods(b->data.house.inventory, y + 220);
    } else if (b->type == BUILDING_BAZAAR || b->type == BUILDING_BAZAAR_UP) {
        draw_goods(b->data.market.inventory, y + 220);
    }
}

void debug_draw_figure_info(void)
{
    if (debug_figure_info_enabled != 1) {
        return;
    }
    int32_t y = (ui_origin_y * 2) + DEBUG_PANEL_Y;
    panel_draw_blocks(PANEL_BLOCKS_DEBUG, 0, y, DEBUG_PANEL_WIDTH_BLOCKS, DEBUG_PANEL_HEIGHT_BLOCKS, DEBUG_PANEL_STYLE);
    map_info.selected_grid = city_view_mouse_to_grid_offset();
    if (map_info.selected_grid == 0) {
        empire_markers.watched_figure_id = 0;
    } else if (map_grids.figure_ids[map_info.selected_grid] != 0) {
        empire_markers.watched_figure_id = map_grids.figure_ids[map_info.selected_grid];
    }
    title("Figure info (F1)", y + 6, image_group(IMAGE_GROUP_FONT_BLACK), DEBUG_COLOR_LABEL);
    int32_t id = empire_markers.watched_figure_id;
    if (id == 0) {
        title("Not watching a figure", y + 24, image_group(IMAGE_GROUP_FONT_BLACK), DEBUG_COLOR_LABEL);
        num(figure_count_processed, " Figs", DEBUG_COLUMN_LEFT, y + 48, DEBUG_COLOR_VALUE);
        num(figure_highest_id, " = ever", DEBUG_COLUMN_RIGHT_FAR, y + 48, DEBUG_COLOR_VALUE);
        num(figure_route_count_free(), " free route slots", DEBUG_COLUMN_LEFT, y + 60, DEBUG_COLOR_VALUE);
        return;
    }
    const figure *f = figure_get(id);
    num(id, " = figure no", DEBUG_COLUMN_LEFT, y + 24, f->state != FIGURE_STATE_ALIVE ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE);
    num(f->faction_id, " = ruler", DEBUG_COLUMN_RIGHT_FAR, y + 24, DEBUG_COLOR_VALUE);
    int32_t position_y = y + 48;
    g_text_width = 0;
    num(f->cross_country.x, (const char *) str_comma, DEBUG_COLUMN_LEFT, position_y, DEBUG_COLOR_VALUE);
    num(f->cross_country.y, "x,y", g_text_width + 8, position_y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(f->x, (const char *) str_comma, DEBUG_COLUMN_RIGHT, position_y, DEBUG_COLOR_VALUE);
    num(f->y, " map x,y", g_text_width + DEBUG_COLUMN_RIGHT, position_y, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(f->cross_country.x & 0xF, (const char *) str_comma, DEBUG_COLUMN_LEFT, y + 72, DEBUG_COLOR_VALUE);
    num(f->cross_country.y & 0xF, "x,y bits", g_text_width + 8, y + 72, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(f->building_id, " = agent_for ", DEBUG_COLUMN_LEFT, y + 84, DEBUG_COLOR_VALUE);
    num(f->immigrant_building_id, " = immigrant_to ", g_text_width + 8, y + 84, DEBUG_COLOR_VALUE);
    g_text_width = 0;
    num(f->migrant_num_people, " = people ", DEBUG_COLUMN_LEFT, y + 96, DEBUG_COLOR_VALUE);
    num(f->is_ghost, " = ghost ", g_text_width + 8, y + 96, DEBUG_COLOR_VALUE);
    int32_t dockers = 0;
    for (int32_t figure_id = 1; figure_id < MAX_FIGURES; figure_id++) {
        const figure *other = figure_get(figure_id);
        if (other->state == FIGURE_STATE_ALIVE && other->type == FIGURE_DOCKER) {
            dockers++;
        }
    }
    g_text_width = 0;
    num(dockers, " = dockers ", DEBUG_COLUMN_LEFT, y + 108, DEBUG_COLOR_VALUE);
}

static int32_t debug_key_pressed(int32_t key)
{
    return ((uint16_t) platform_key_state(key) & DEBUG_KEY_PRESSED) == DEBUG_KEY_PRESSED;
}

void debug_frame_update(void)
{
    int32_t now = (int32_t) platform_time_ms();
    int32_t last = debug_frame.time_last;
    debug_frame.time_current = now;
    debug_frame.time_last = now;
    frame_time_delta = now - last;
    if (debug_keys_enabled != 0) {
        if (debug_key_pressed(PLATFORM_KEY_F2)) {
            debug_key_toggle = debug_key_toggle == 0;
        }
        if (debug_key_pressed(PLATFORM_KEY_F5)) {
            double value = (double) debug_key_value - (double) DEBUG_KEY_STEP;
            debug_key_value = (float) value;
            if (!(value >= (double) DEBUG_KEY_MIN)) {
                debug_key_value = DEBUG_KEY_MIN;
            }
        }
        if (debug_key_pressed(PLATFORM_KEY_F6)) {
            double value = (double) debug_key_value + (double) DEBUG_KEY_STEP;
            debug_key_value = (float) value;
            if (!(value <= (double) DEBUG_KEY_MAX)) {
                debug_key_value = DEBUG_KEY_MAX;
            }
        }
    }
    if (debug_sprite_a) {
        debug_sprite_update(debug_sprite_a);
        debug_sprite_owned_object *object = debug_sprite_a;
        object->ops->draw(object, DEBUG_OBJECT_X, DEBUG_OBJECT_Y);
    }
    const debug_sprite *sprite = (const debug_sprite *) debug_sprite_b;
    if (sprite && sprite->model && sprite->model->data) {
        debug_sprite_update(debug_sprite_b);
        sprite = (const debug_sprite *) debug_sprite_b;
        int16_t frame = (int16_t) (int32_t) sprite->frame;
        const debug_sprite_draw_frame *record = &((const debug_sprite_draw_frame *) sprite->direction->frames)[frame];
        int16_t dx = record->has_offset_record != 0 ? record->x.offset_record[1] : record->x.dx;
        g_img_width = record->width;
        g_img_height = record->height;
        g_draw_x = dx + DEBUG_SPRITE_X;
        g_draw_y = DEBUG_SPRITE_Y;
        graphics_clip_horizontal();
        graphics_clip_vertical();
        if (g_clip_y != IMAGE_CLIP_INVISIBLE) {
            if (g_clip_x == IMAGE_CLIP_LEFT) {
                image_blit_compressed_left(record->data);
            } else if (g_clip_x == IMAGE_CLIP_RIGHT) {
                image_blit_compressed_right(record->data);
            } else {
                image_blit_compressed(record->data);
            }
        }
    }
}

static int32_t grid_value(int32_t building_id)
{
    int32_t offset = city_view.grid_offset;
    switch (settings_data.debug_grid_overlay) {
        case DEBUG_GRID_BUILDING_IDS:
            return building_id;
        case DEBUG_GRID_TERRAIN:
            return (int32_t) map_grids.terrain[offset];
        case DEBUG_GRID_BITFIELDS:
            return map_grids.bitfields[offset];
        case DEBUG_GRID_IMAGES:
            return map_grids.images[offset];
        case DEBUG_GRID_EDGE:
            return map_grids.edge[offset];
        case DEBUG_GRID_CANALS:
            return map_grids.canals[offset];
        case DEBUG_GRID_RANDOM:
            return map_grids.random[offset];
        case DEBUG_GRID_FIGURE_IDS:
            return map_grids.figure_ids[offset];
        case DEBUG_GRID_SPRITE:
            return map_grids.sprite[offset];
        case DEBUG_GRID_ROUTING_LAND_CITIZEN:
            return routing_grids.land_citizen[offset];
        case DEBUG_GRID_ROUTING_DISTANCE:
            return routing_grids.distance[offset];
        case DEBUG_GRID_BUILDING_WATER_ACCESS:
            return building_get(building_id)->has_water_access;
        case DEBUG_GRID_BUILDING_HOUSES_COVERED:
            return building_get(building_id)->houses_covered;
        case DEBUG_GRID_BUILDING_DISTANCE_FROM_ENTRY:
            return building_get(building_id)->distance_from_entry;
        case DEBUG_GRID_BUILDING_DAMAGE_RISK:
            return building_get(building_id)->damage_risk;
        case DEBUG_GRID_BUILDING_POPULATION:
            return building_get(building_id)->house_population;
        case DEBUG_GRID_DESIRABILITY:
            return map_grids.desirability[offset];
        case DEBUG_GRID_ELEVATION:
            return map_grids.elevation[offset];
        case DEBUG_GRID_ROUTING_WATER:
            return routing_grids.water[offset];
        case DEBUG_GRID_ROUTING_LAND_NONCITIZEN:
            return routing_grids.land_noncitizen[offset];
        case DEBUG_GRID_BUILDING_DAMAGE:
            return (int16_t) map_grids.building_damage[offset];
        case DEBUG_GRID_FIGURE_COUNT:
            return map_figure_count_at(offset);
        case DEBUG_GRID_OLD_TYPE:
            return map_grids.rubble_building_type[offset];
        case DEBUG_GRID_SOLDIER_STRENGTH:
            return map_grids.soldier_strength[offset];
        case DEBUG_GRID_ROUTING_WALLS:
            return routing_grids.walls[offset];
        case DEBUG_GRID_ROAD_NETWORK:
            return map_grids.road_network[offset];
        case DEBUG_GRID_ROAD_SEGMENT:
            return map_grids.road_segment[offset];
        case DEBUG_GRID_SERVICE_VISITS:
            return map_service_visit_get(DEBUG_GRID_SERVICE, offset);
        default:
            return 0;
    }
}

void debug_draw_grid_value(int32_t x, int32_t y)
{
    int32_t value = grid_value(map_grids.building_ids[city_view.grid_offset]);
    if (value == 0) {
        return;
    }
    int32_t color = DEBUG_COLOR_LABEL;
    if (value < 0) {
        value = -value;
        color = DEBUG_COLOR_WARNING;
    }
    text_draw_number(value, DEBUG_NUMBER_PREFIX_GRID, postfix_none, x + DEBUG_GRID_VALUE_X, y + DEBUG_GRID_VALUE_Y, image_group(IMAGE_GROUP_FONT_SMALL), color);
}

void debug_draw_grid_values(void)
{
    if (!debug_building_info_enabled && !debug_evolve_info_enabled && !debug_figure_info_enabled) {
        return;
    }
    int32_t y = city_view.origin_y - city_view.tile_half_height;
    int32_t odd = 0;
    city_view.row = city_view.camera_row;
    for (int32_t row = 0; row < city_view.height_tiles; row++) {
        int32_t x = city_view.origin_x;
        if (odd) {
            x = city_view.origin_x - city_view.tile_half_width - 1;
        }
        odd = !odd;
        city_view.col = city_view.camera_col;
        for (int32_t col = 0; col <= city_view.width_tiles; col++) {
            city_view.grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
            if (city_view.grid_offset < VIEW_LOOKUP_EMPTY) {
                debug_draw_grid_value(x, y);
            }
            city_view.col++;
            x += city_view.tile_width + DEBUG_GRID_COLUMN_GAP;
        }
        y += city_view.tile_half_height;
        city_view.row++;
    }
}
