// The ships and their buildings on the info windows: shipwright, dock, fishing wharf, transport and warship wharves,
// ferry landing; the transport's and warship's panels (hull, crew, the company on board) with their five order buttons
// and clicks, the target clicks of the ship windows and the click on a fort for a legion. The hull texts follow
// ship_hull_text_rules.
//
// Ship panels:
// - the warship's attack order leaves the window without a redraw request;
// - the crew fatigue is read from the ship's move mode byte.

#include "window/building_info_ships.h"
#include "building/building.h"
#include "building/shipwright.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/formation_orders.h"
#include "figure/military.h"
#include "figure/route.h"
#include "figure/ships_player.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "map/water.h"
#include "text/lang.h"
#include "widget/city_selection.h"
#include "window/building_info.h"
#include "window/building_info_military.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum ship_lang_group {
    LANG_GROUP_SHIPWRIGHT = 100,
    LANG_GROUP_DOCK = 101,
    LANG_GROUP_FISHING_WHARF = 102,
    LANG_GROUP_LEGION_NAMES = 138,
    LANG_GROUP_FERRY_LANDING = 159,
    LANG_GROUP_TRANSPORT_WHARF = 174,
    LANG_GROUP_WARSHIP_WHARF = 175,
};

enum ship_text {
    SHIP_TEXT_TRANSPORT = 0,
    SHIP_TEXT_WARSHIP = 1,
    SHIP_TEXT_HULL_STRENGTH = 2,
    SHIP_TEXT_HULL_VERY_STRONG = 3,
    SHIP_TEXT_HULL_WEAK = 8,
    SHIP_TEXT_HOLD_POSITION = 9,
    SHIP_TEXT_ENGAGE = 11,
    SHIP_TEXT_SEEK_AND_DESTROY = 13,
    SHIP_TEXT_REPAIR = 15,
    SHIP_TEXT_RETURN_TO_WHARF = 17,
    SHIP_TEXT_TRANSPORT_HOLD_POSITION = 19,
    SHIP_TEXT_EVADE = 21,
    SHIP_TEXT_EMBARK = 23,
    SHIP_TEXT_DISEMBARK = 25,
    SHIP_TEXT_CREW_FATIGUE = 27,
    SHIP_TEXT_CREW_RESTED = 28,
    SHIP_TEXT_CREW_TIRED = 29,
    SHIP_TEXT_CREW_EXHAUSTED = 30,
    SHIP_TEXT_ARCHER_COMPANY = 31,
    SHIP_TEXT_CHARIOT_COMPANY = 32,
    SHIP_TEXT_INFANTRY_COMPANY = 33,
    SHIP_TEXT_ON_BOARD = 34,
};

enum ship_building_text {
    SHIPWRIGHT_TEXT_PRODUCTION_IS = 2,
    SHIPWRIGHT_TEXT_COMPLETE = 3,
    SHIPWRIGHT_TEXT_NO_DEMAND = 4,
    SHIPWRIGHT_TEXT_BUILDING = 5,
    SHIPWRIGHT_TEXT_REPAIRING = 6,
    SHIPWRIGHT_TEXT_STORED_WOOD = 7,
    SHIPWRIGHT_TEXT_ORDER_NEEDS_WOOD = 9,
    SHIPWRIGHT_TEXT_REPAIRS_NEED_WOOD = 10,
    DOCK_TEXT_SERVICING_NO_STAFF = 2,
    DOCK_TEXT_WAITING_NO_STAFF = 6,
    FISHING_WHARF_TEXT_WAITING_FOR_BOAT = 2,
    FISHING_WHARF_TEXT_SAILING_OUT = 3,
    FISHING_WHARF_TEXT_FISHING = 4,
    FISHING_WHARF_TEXT_SAILING_BACK = 5,
    FISHING_WHARF_TEXT_RESTOCKING = 6,
    FISHING_WHARF_TEXT_RETURNING_WITH_FISH = 7,
    FISHING_WHARF_TEXT_NO_FISHING_GROUNDS = 8,
    FISHING_WHARF_TEXT_HALTED = 9,
    FISHING_WHARF_TEXT_POLLUTED_WATER = 10,
    FERRY_TEXT_NO_LINKED_LANDING = 1,
    FERRY_TEXT_LINKED_NO_ROAD = 2,
    FERRY_TEXT_LINKED_NO_WORKERS = 3,
};

enum ship_layout {
    SHIP_LINES_X = 100,
    SHIP_VALUES_X = 300,
    TRANSPORT_HULL_Y = 40,
    TRANSPORT_COMPANY_Y = 60,
    TRANSPORT_SOLDIERS_Y = 76,
    TRANSPORT_DAMAGE_Y = 92,
    TRANSPORT_EXPERIENCE_Y = 108,
    TRANSPORT_MORALE_Y = 124,
    WARSHIP_HULL_Y = 60,
    WARSHIP_CREW_Y = 80,
    SHIP_ORDER_IMAGE_X = 21,
    SHIP_ORDER_IMAGE_Y = 141,
    SHIP_ORDER_SPACING = 85,
    SHIP_ORDER_BORDER_X = 19,
    SHIP_ORDER_BORDER_Y = 139,
    SHIP_ORDER_BORDER_SIZE = 84,
    SHIP_ORDER_PANEL_Y = 230,
    SHIP_ORDER_PANEL_BLOCKS = 5,
    SHIP_ORDER_TITLE_X = 24,
    SHIP_ORDER_TITLE_Y = 236,
    SHIP_ORDER_TEXT_Y = 252,
    TRANSPORT_EMBARKED_IMAGE = 3,
    TRANSPORT_DOCKED_RETURN_IMAGE = 6,
    WARSHIP_DOCKED_RETURN_IMAGE = 5,
    SHIPWRIGHT_ICON_X = 30,
    SHIPWRIGHT_ICON_Y = 38,
    SHIPWRIGHT_STOCK_X = 58,
    SHIPWRIGHT_STOCK_Y = 44,
    SHIPWRIGHT_STATE_Y = 62,
    SHIPWRIGHT_TEXT_Y = 86,
    SHIP_BUILDING_TEXT_Y = 56,
    WHARF_NO_ROAD_Y = 40,
    WHARF_TEXT_Y = 76,
    FERRY_TEXT_Y = 60,
    TITLE_ICON_X = 10,
    TITLE_ICON_Y = 10,
};

enum ship_values {
    SHIP_DAMAGED_PERCENT = 15,
    SHIP_UNDAMAGED_PERCENT = 100,
    TRANSPORT_EVADE_WAIT_TICKS = 75,
    SHIPWRIGHT_PROGRESS_MAX = 160,
    SHIPWRIGHT_WOOD_PER_BOAT = 100,
    DOCK_STAFF_FEW = 50,
    DOCK_STAFF_MOST = 75,
};

ship_hull_text_rule ship_hull_text_rules[SHIP_HULL_TEXT_RULE_COUNT] = {
    [SHIP_HULL_TEXT_RULE_WEAK] = { .transport = 0, .warship = 15 },
    [SHIP_HULL_TEXT_RULE_FAIR] = { .transport = 30, .warship = 30 },
    [SHIP_HULL_TEXT_RULE_AVERAGE] = { .transport = 50, .warship = 50 },
    [SHIP_HULL_TEXT_RULE_GOOD] = { .transport = 70, .warship = 70 },
    [SHIP_HULL_TEXT_RULE_STRONG] = { .transport = 90, .warship = 90 },
};

static int32_t mouse_grid_offset(void)
{
    return map_grid_offset(map_info.cursor_tile_x, map_info.cursor_tile_y);
}

static void close_to_city(void)
{
    window_current_id = WINDOW_CITY;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static void open_window(uint8_t window)
{
    window_current_id = window;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static int32_t hull_percentage(const figure *ship)
{
    return calc_percentage(ship->ship_hit_points, ship->ship_max_hit_points);
}

static int32_t going_to_dock_action(int32_t is_warship)
{
    return is_warship ? FIGURE_ACTION_WARSHIP_GOING_TO_DOCK : FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK;
}

static int32_t queued_at_dock_action(int32_t is_warship)
{
    return is_warship ? FIGURE_ACTION_WARSHIP_QUEUED_AT_DOCK : FIGURE_ACTION_TRANSPORT_SHIP_QUEUED_AT_DOCK;
}

static int32_t is_dock_trip(const figure *ship, int32_t is_warship)
{
    int32_t action = ship->action_state;
    return action == going_to_dock_action(is_warship) || action == queued_at_dock_action(is_warship);
}

static int32_t orders_disabled(const figure *ship, int32_t is_warship)
{
    return is_dock_trip(ship, is_warship) && hull_percentage(ship) <= SHIP_DAMAGED_PERCENT;
}

static int32_t repair_disabled(const figure *ship, int32_t is_warship)
{
    return is_dock_trip(ship, is_warship) || hull_percentage(ship) == SHIP_UNDAMAGED_PERCENT;
}

void transport_ship_button(void)
{
    int32_t ship_id = building_info_transport_id;
    selected_transport_id = ship_id;
    if (ship_id <= 0 || settings_data.paused != 0) {
        return;
    }
    figure *ship = figure_get(ship_id);
    if (orders_disabled(ship, 0)) {
        return;
    }
    if (button_parameter == SHIP_ORDER_REPAIR && repair_disabled(ship, 0)) {
        return;
    }
    switch (button_parameter) {
        case SHIP_ORDER_FIRST:
            ship->action_state = FIGURE_ACTION_TRANSPORT_SHIP_DOCKED;
            selected_transport_active = 1;
            open_window(WINDOW_CITY_TRANSPORT_SELECTED);
            return;
        case SHIP_ORDER_SECOND:
            ship->action_state = FIGURE_ACTION_TRANSPORT_SHIP_EVADING;
            ship->wait_ticks = TRANSPORT_EVADE_WAIT_TICKS;
            ship->warship_secondary_action = WARSHIP_MODE_WAITING;
            selected_transport_active = 0;
            close_to_city();
            return;
        case SHIP_ORDER_THIRD:
            ship->action_state = (uint8_t) ((ship->has_troops_on_board != 0) ? FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING
                                                                             : FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION);
            selected_transport_active = 1;
            open_window(WINDOW_CITY_TRANSPORT_SELECTED);
            return;
        case SHIP_ORDER_REPAIR:
            selected_transport_active = 0;
            ship_go_to_best_dock(ship_id);
            open_window(WINDOW_CITY_TRANSPORT_SELECTED);
            return;
        default:
            return;
    }
}

void warship_button(void)
{
    if (settings_data.paused != 0) {
        return;
    }
    int32_t ship_id = building_info_warship_id;
    selected_warship_id = ship_id;
    if (ship_id <= 0) {
        return;
    }
    figure *ship = figure_get(ship_id);
    if (orders_disabled(ship, 1)) {
        return;
    }
    if (button_parameter == SHIP_ORDER_REPAIR && repair_disabled(ship, 1)) {
        return;
    }
    if (ship->request_event_id >= 0) {
        return;
    }
    switch (button_parameter) {
        case SHIP_ORDER_FIRST:
            ship->action_state = FIGURE_ACTION_WARSHIP_DOCKED;
            selected_warship_active = 1;
            open_window(WINDOW_CITY_WARSHIP_SELECTED);
            return;
        case SHIP_ORDER_SECOND:
            ship->action_state = FIGURE_ACTION_WARSHIP_PATROLLING;
            ship->warship_secondary_action = WARSHIP_MODE_PATROL;
            selected_warship_active = 1;
            open_window(WINDOW_CITY_WARSHIP_SELECTED);
            return;
        case SHIP_ORDER_THIRD:
            ship->action_state = FIGURE_ACTION_WARSHIP_HUNTING;
            ship->warship_secondary_action = WARSHIP_MODE_WAITING;
            selected_warship_active = 0;
            close_to_city();
            return;
        case SHIP_ORDER_REPAIR:
            selected_warship_active = 0;
            ship_go_to_best_dock(ship_id);
            open_window(WINDOW_CITY_WARSHIP_SELECTED);
            return;
        default:
            return;
    }
}

static void sail_towards_mouse(figure *ship, int32_t *x, int32_t *y)
{
    map_water_farthest_on_line(ship->x, ship->y, map_info.cursor_tile_x, map_info.cursor_tile_y, x, y);
    ship->destination_x = (int16_t) *x;
    ship->destination_y = (int16_t) *y;
}

void warship_target_click(void)
{
    int32_t ship_id = selected_warship_id;
    figure *ship = figure_get(ship_id);
    if (ship_id <= 0 || ship->request_event_id >= 0 || ship->action_state == FIGURE_ACTION_WARSHIP_WAITING || map_info.selected_grid == 0) {
        close_to_city();
        return;
    }
    int32_t target = figure_fighter_at_grid_offset(mouse_grid_offset());
    if (target != 0) {
        ship->action_state_before_attack = ship->action_state;
        ship->action_state = FIGURE_ACTION_WARSHIP_ATTACKING_SHIP;
        ship->warship_secondary_action = WARSHIP_MODE_WAITING;
        ship->target_figure_id = (int16_t) target;
        window_current_id = WINDOW_CITY;
        selected_warship_active = 0;
        return;
    }
    int32_t x = 0;
    int32_t y = 0;
    switch (ship->action_state) {
        case FIGURE_ACTION_WARSHIP_STRAIGHT:
            sail_towards_mouse(ship, &x, &y);
            figure_route_remove(ship_id);
            window_redraw_request = WINDOW_REDRAW_ALL;
            return;
        case FIGURE_ACTION_WARSHIP_HUNTING:
        case FIGURE_ACTION_WARSHIP_SAILING:
        case FIGURE_ACTION_WARSHIP_EVADING:
        case FIGURE_ACTION_WARSHIP_GOING_TO_DOCK:
        case FIGURE_ACTION_WARSHIP_DOCKED:
        case FIGURE_ACTION_WARSHIP_PATROLLING:
            sail_towards_mouse(ship, &x, &y);
            ship->patrol_x = ship->destination_x;
            ship->patrol_y = ship->destination_y;
            figure_route_remove(ship_id);
            window_redraw_request = WINDOW_REDRAW_ALL;
            if (ship->action_state == FIGURE_ACTION_WARSHIP_PATROLLING) {
                ship->warship_secondary_action = WARSHIP_MODE_PATROL;
            } else if (ship->action_state == FIGURE_ACTION_WARSHIP_HUNTING) {
                ship->warship_secondary_action = WARSHIP_MODE_WAITING;
                window_current_id = WINDOW_CITY;
                selected_warship_active = 0;
            }
            return;
        default:
            return;
    }
}

static void embark_legion_under_mouse(figure *ship)
{
    int32_t x = 0;
    int32_t y = 0;
    map_water_farthest_on_line(ship->x, ship->y, map_info.cursor_tile_x, map_info.cursor_tile_y, &x, &y);
    int32_t legion = formation_legion_at_grid_offset(mouse_grid_offset());
    if (legion == 0) {
        ship->destination_x = (int16_t) x;
        ship->destination_y = (int16_t) y;
        return;
    }
    int32_t land_x = 0;
    int32_t land_y = 0;
    map_water_landing_point(map_info.cursor_tile_x, map_info.cursor_tile_y, x, y, &land_x, &land_y);
    if (formation_legion_move_to(land_x, land_y, legion, 0) != 0) {
        ship->ship_formation_id = (uint8_t) legion;
        ship->destination_x = (int16_t) x;
        ship->destination_y = (int16_t) y;
        figure_route_remove(selected_transport_id);
    }
}

static void choose_transport_order_by_cursor(figure *ship)
{
    if (selected_transport_active == 0) {
        return;
    }
    void *cursor = cursor_current;
    if (cursor == cursor_handles[CURSOR_SLOT_TRANSPORT_TO_LEGION]) {
        ship->action_state = FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION;
    }
    if (cursor == cursor_handles[CURSOR_SLOT_TRANSPORT_TO_LANDING]) {
        ship->action_state = FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING;
    }
}

void transport_target_click(void)
{
    if (settings_data.paused != 0) {
        return;
    }
    int32_t ship_id = selected_transport_id;
    figure *ship = figure_get(ship_id);
    if (ship_id <= 0 || ship->action_state == FIGURE_ACTION_TRANSPORT_SHIP_WAITING || map_info.selected_grid == 0) {
        close_to_city();
        return;
    }
    choose_transport_order_by_cursor(ship);
    int32_t x = 0;
    int32_t y = 0;
    switch (ship->action_state) {
        case FIGURE_ACTION_TRANSPORT_SHIP_SAILING:
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK:
        case FIGURE_ACTION_TRANSPORT_SHIP_DOCKED:
            sail_towards_mouse(ship, &x, &y);
            figure_route_remove(ship_id);
            window_redraw_request = WINDOW_REDRAW_ALL;
            return;
        case FIGURE_ACTION_TRANSPORT_SHIP_EVADING:
            sail_towards_mouse(ship, &x, &y);
            ship->direction = (int8_t) calc_missile_shooter_direction(ship->x, ship->y, x, y);
            ship->warship_secondary_action = WARSHIP_MODE_FOLLOW_TARGET;
            figure_route_remove(ship_id);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION:
            embark_legion_under_mouse(ship);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING:
            if (ship->has_troops_on_board == 0) {
                return;
            }
            if ((map_grids.terrain[mouse_grid_offset()] & TERRAIN_WATER) == 0) {
                sail_towards_mouse(ship, &x, &y);
                ship->ship_landing_x = (int16_t) map_info.cursor_tile_x;
                ship->ship_landing_y = (int16_t) map_info.cursor_tile_y;
                figure_route_remove(ship_id);
            }
            break;
        default:
            return;
    }
    close_to_city();
    selected_transport_active = 0;
}

void legion_fort_click(void)
{
    if (settings_data.paused != 0) {
        return;
    }
    if (map_info.selected_grid != 0) {
        int32_t grid_offset = mouse_grid_offset();
        const formation *legion = formation_get(formation_selected);
        if (legion->request_event_id >= 0 || legion->cursed_by_seth != 0) {
            return;
        }
        int32_t fort_legion = formation_legion_at_building(grid_offset);
        int32_t legion_id = formation_selected;
        if (fort_legion != 0 && fort_legion == legion_id) {
            formation_legion_to_standard(legion_id, FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_TO_LANDING);
            close_to_city();
            return;
        }
        formation_legion_return_to_fort(map_info.cursor_tile_x, map_info.cursor_tile_y, legion_id);
    }
    close_to_city();
}

static int32_t current_order_button(const figure *ship, int32_t is_warship)
{
    int32_t action = ship->action_state;
    if (is_warship) {
        switch (action) {
            case FIGURE_ACTION_WARSHIP_HUNTING:
                return SHIP_ORDER_THIRD;
            case FIGURE_ACTION_WARSHIP_GOING_TO_DOCK:
                return SHIP_ORDER_REPAIR;
            case FIGURE_ACTION_WARSHIP_DOCKED:
                return SHIP_ORDER_FIRST;
            case FIGURE_ACTION_WARSHIP_PATROLLING:
                return SHIP_ORDER_SECOND;
            default:
                return SHIP_ORDER_RETURN;
        }
    }
    switch (action) {
        case FIGURE_ACTION_TRANSPORT_SHIP_EVADING:
            return SHIP_ORDER_SECOND;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK:
            return SHIP_ORDER_REPAIR;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION:
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING:
            return SHIP_ORDER_THIRD;
        case FIGURE_ACTION_TRANSPORT_SHIP_DOCKED:
            return SHIP_ORDER_FIRST;
        default:
            return SHIP_ORDER_RETURN;
    }
}

static int32_t highlighted_order_button(int32_t ship_id, const figure *ship, int32_t is_warship)
{
    if (orders_disabled(ship, is_warship)) {
        return SHIP_ORDER_SECOND;
    }
    int32_t focused = focus_button_id - 1;
    if (focused >= SHIP_ORDER_FIRST && focused <= SHIP_ORDER_THIRD) {
        return focused;
    }
    if (focused == SHIP_ORDER_REPAIR && !repair_disabled(ship, is_warship)) {
        return SHIP_ORDER_REPAIR;
    }
    if (focused == SHIP_ORDER_RETURN && ship_is_docked(ship_id) == 0) {
        return SHIP_ORDER_RETURN;
    }
    return current_order_button(ship, is_warship);
}

static int32_t order_text_of_action(const figure *ship, int32_t is_warship)
{
    int32_t action = ship->action_state;
    if (is_warship) {
        switch (action) {
            case FIGURE_ACTION_WARSHIP_HUNTING:
                return SHIP_TEXT_SEEK_AND_DESTROY;
            case FIGURE_ACTION_WARSHIP_GOING_TO_DOCK:
                return SHIP_TEXT_REPAIR;
            case FIGURE_ACTION_WARSHIP_DOCKED:
                return SHIP_TEXT_HOLD_POSITION;
            case FIGURE_ACTION_WARSHIP_PATROLLING:
                return SHIP_TEXT_ENGAGE;
            default:
                return SHIP_TEXT_RETURN_TO_WHARF;
        }
    }
    switch (action) {
        case FIGURE_ACTION_TRANSPORT_SHIP_EVADING:
            return SHIP_TEXT_EVADE;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK:
            return SHIP_TEXT_REPAIR;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION:
            return SHIP_TEXT_EMBARK;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING:
            return SHIP_TEXT_DISEMBARK;
        case FIGURE_ACTION_TRANSPORT_SHIP_DOCKED:
            return SHIP_TEXT_TRANSPORT_HOLD_POSITION;
        default:
            return SHIP_TEXT_RETURN_TO_WHARF;
    }
}

static int32_t order_text(const figure *ship, int32_t is_warship)
{
    switch (focus_button_id) {
        case 0:
            return order_text_of_action(ship, is_warship);
        case SHIP_ORDER_FIRST + 1:
            return is_warship ? SHIP_TEXT_HOLD_POSITION : SHIP_TEXT_TRANSPORT_HOLD_POSITION;
        case SHIP_ORDER_SECOND + 1:
            return is_warship ? SHIP_TEXT_ENGAGE : SHIP_TEXT_EVADE;
        case SHIP_ORDER_THIRD + 1:
            if (is_warship) {
                return SHIP_TEXT_SEEK_AND_DESTROY;
            }
            return (ship->has_troops_on_board != 0) ? SHIP_TEXT_DISEMBARK : SHIP_TEXT_EMBARK;
        case SHIP_ORDER_REPAIR + 1:
            return SHIP_TEXT_REPAIR;
        default:
            return SHIP_TEXT_RETURN_TO_WHARF;
    }
}

static void draw_ship_orders(int32_t ship_id, int32_t is_warship)
{
    const figure *ship = figure_get(ship_id);
    window_building_draw_inner_panel(SHIP_ORDER_PANEL_Y, SHIP_ORDER_PANEL_BLOCKS);
    int32_t highlighted = highlighted_order_button(ship_id, ship, is_warship);
    for (int32_t order = 0; order < SHIP_ORDER_BUTTONS; order++) {
        int32_t x = order * SHIP_ORDER_SPACING + SHIP_ORDER_BORDER_X + building_info_x;
        int32_t y = building_info_y + SHIP_ORDER_BORDER_Y;
        button_border_draw_ui(x, y, SHIP_ORDER_BORDER_SIZE, SHIP_ORDER_BORDER_SIZE, highlighted == order, building_info_x, building_info_y);
        int32_t greyed = orders_disabled(ship, is_warship) || (order == SHIP_ORDER_REPAIR && repair_disabled(ship, is_warship)) ||
            (order == SHIP_ORDER_RETURN && ship_is_docked(ship_id) != 0);
        if (greyed) {
            graphics_grey_rect(x, y, SHIP_ORDER_BORDER_SIZE, SHIP_ORDER_BORDER_SIZE, 0);
        }
    }
    int32_t text = order_text(ship, is_warship);
    lang_text_draw_ui(
        LANG_GROUP_SHIPS, text, building_info_x + SHIP_ORDER_TITLE_X, building_info_y + SHIP_ORDER_TITLE_Y, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
    lang_text_draw_multiline_ui(LANG_GROUP_SHIPS, text + 1, building_info_x + SHIP_ORDER_TITLE_X, building_info_y + SHIP_ORDER_TEXT_Y,
        (info_w_blocks - 4) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
}

void window_building_draw_boat_orders(int32_t figure_id)
{
    draw_ship_orders(figure_id, 0);
}

void window_building_draw_ship_orders(int32_t figure_id)
{
    draw_ship_orders(figure_id, 1);
}

static int32_t hull_text(const figure *ship, int32_t is_warship)
{
    int32_t percentage = hull_percentage(ship);
    for (int32_t rule = 0; rule < SHIP_HULL_TEXT_RULE_COUNT; rule++) {
        const ship_hull_text_rule *threshold = &ship_hull_text_rules[rule];
        if (percentage <= (is_warship ? threshold->warship : threshold->transport)) {
            return SHIP_TEXT_HULL_WEAK - rule;
        }
    }
    return SHIP_TEXT_HULL_VERY_STRONG;
}

static void draw_line(int32_t group, int32_t index, int32_t x, int32_t y)
{
    lang_text_draw_ui(group, index, x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_ship_title(int32_t help, int32_t title)
{
    info_help_id = building_info_help_messages[help];
    outer_panel_draw(building_info_x, building_info_y, info_w_blocks, building_info_h);
    lang_text_draw_centered_ui(LANG_GROUP_SHIPS, title, building_info_x, building_info_y + BUILDING_INFO_TITLE_Y, info_w_blocks * BUILDING_INFO_BLOCK,
        image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
}

static int32_t company_text(int32_t figure_type)
{
    if (figure_type == FIGURE_ARCHER) {
        return SHIP_TEXT_ARCHER_COMPANY;
    }
    return (figure_type == FIGURE_INFANTRY) ? SHIP_TEXT_INFANTRY_COMPANY : SHIP_TEXT_CHARIOT_COMPANY;
}

static void draw_company_on_board(int32_t formation_id)
{
    const formation *legion = formation_get(formation_id);
    int32_t y = building_info_y + TRANSPORT_COMPANY_Y;
    g_text_width = 0;
    draw_line(LANG_GROUP_SHIPS, company_text(legion->figure_type), building_info_x + SHIP_LINES_X, y);
    draw_line(LANG_GROUP_LEGION_NAMES, legion->legion_id, g_text_width + SHIP_LINES_X + building_info_x, y);
    draw_line(LANG_GROUP_SHIPS, SHIP_TEXT_ON_BOARD, g_text_width + SHIP_LINES_X + building_info_x, y);
    window_building_draw_legion_lines(formation_id, TRANSPORT_SOLDIERS_Y, TRANSPORT_DAMAGE_Y, TRANSPORT_EXPERIENCE_Y, TRANSPORT_MORALE_Y);
}

static int32_t transport_order_image(int32_t ship_id, const figure *ship, int32_t order)
{
    if (order == SHIP_ORDER_THIRD && ship->has_troops_on_board != 0) {
        return image_group(IMAGE_GROUP_BOAT_ORDERS) + TRANSPORT_EMBARKED_IMAGE;
    }
    if (order == SHIP_ORDER_RETURN && ship_is_docked(ship_id) != 0) {
        return image_group(IMAGE_GROUP_BOAT_ORDERS) + TRANSPORT_DOCKED_RETURN_IMAGE;
    }
    return boat_order_image_offsets[order] + image_group(IMAGE_GROUP_BOAT_ORDERS);
}

void window_building_draw_boat(void)
{
    int32_t ship_id = building_info_transport_id;
    const figure *ship = figure_get(ship_id);
    draw_ship_title(BUILDING_INFO_HELP_TRANSPORT_SHIP, SHIP_TEXT_TRANSPORT);
    draw_line(LANG_GROUP_SHIPS, SHIP_TEXT_HULL_STRENGTH, building_info_x + SHIP_LINES_X, building_info_y + TRANSPORT_HULL_Y);
    draw_line(LANG_GROUP_SHIPS, hull_text(ship, 0), building_info_x + SHIP_VALUES_X, building_info_y + TRANSPORT_HULL_Y);
    if (ship->has_troops_on_board != 0) {
        draw_company_on_board(ship->ship_formation_id);
    }
    for (int32_t order = 0; order < SHIP_ORDER_BUTTONS; order++) {
        int32_t x = order * SHIP_ORDER_SPACING + SHIP_ORDER_IMAGE_X + building_info_x;
        image_draw(transport_order_image(ship_id, ship, order), x, building_info_y + SHIP_ORDER_IMAGE_Y);
    }
    window_building_draw_boat_orders(ship_id);
}

int32_t window_building_ship_damage_text(int32_t figure_id)
{
    return hull_text(figure_get(figure_id), 1);
}

int32_t window_building_ship_crew_text(int32_t figure_id)
{
    int32_t fatigue = figure_get(figure_id)->ship_move_mode;
    if (fatigue == 0) {
        return SHIP_TEXT_CREW_RESTED;
    }
    return (fatigue == 1) ? SHIP_TEXT_CREW_TIRED : SHIP_TEXT_CREW_EXHAUSTED;
}

void window_building_draw_ship(void)
{
    int32_t ship_id = building_info_warship_id;
    draw_ship_title(BUILDING_INFO_HELP_WARSHIP, SHIP_TEXT_WARSHIP);
    draw_line(LANG_GROUP_SHIPS, SHIP_TEXT_HULL_STRENGTH, building_info_x + SHIP_LINES_X, building_info_y + WARSHIP_HULL_Y);
    draw_line(LANG_GROUP_SHIPS, window_building_ship_damage_text(ship_id), building_info_x + SHIP_VALUES_X, building_info_y + WARSHIP_HULL_Y);
    draw_line(LANG_GROUP_SHIPS, SHIP_TEXT_CREW_FATIGUE, building_info_x + SHIP_LINES_X, building_info_y + WARSHIP_CREW_Y);
    draw_line(LANG_GROUP_SHIPS, window_building_ship_crew_text(ship_id), building_info_x + SHIP_VALUES_X, building_info_y + WARSHIP_CREW_Y);
    for (int32_t order = 0; order < SHIP_ORDER_BUTTONS; order++) {
        g_draw_y = building_info_y + SHIP_ORDER_IMAGE_Y;
        g_draw_x = order * SHIP_ORDER_SPACING + SHIP_ORDER_IMAGE_X + building_info_x;
        int32_t image = (order == SHIP_ORDER_RETURN && ship_is_docked(ship_id) != 0) ? image_group(IMAGE_GROUP_SHIP_ORDERS) + WARSHIP_DOCKED_RETURN_IMAGE : image_group(IMAGE_GROUP_SHIP_ORDERS) + order;
        image_draw(image, g_draw_x, g_draw_y);
    }
    window_building_draw_ship_orders(ship_id);
}

static void start_panel(int32_t help, int32_t group)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
}

static int32_t shipwright_text(const building *b)
{
    int32_t order = b->data.shipyard.boat_order;
    switch (order & BOAT_ORDER_KIND_MASK) {
        case 0: {
            int32_t needed_type;
            if (shipwright_boat_needed(&needed_type) == 0) {
                return SHIPWRIGHT_TEXT_NO_DEMAND;
            }
            return (b->loads_stored < SHIPWRIGHT_WOOD_PER_BOAT && needed_type != FIGURE_FISHING_BOAT) ? SHIPWRIGHT_TEXT_ORDER_NEEDS_WOOD : -1;
        }
        case BOAT_ORDER_FOR_CITY: {
            int32_t progress = calc_percentage(b->data.shipyard.progress, SHIPWRIGHT_PROGRESS_MAX);
            int32_t y = building_info_y + SHIPWRIGHT_STATE_Y;
            g_text_width = 0;
            draw_line(LANG_GROUP_SHIPWRIGHT, SHIPWRIGHT_TEXT_PRODUCTION_IS, building_info_x + BUILDING_INFO_TEXT_X, y);
            text_draw_number_ui(
                progress, NUMBER_PREFIX_NONE, pct_postfix, g_text_width + BUILDING_INFO_TEXT_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
            draw_line(LANG_GROUP_SHIPWRIGHT, SHIPWRIGHT_TEXT_COMPLETE, g_text_width + BUILDING_INFO_TEXT_X + building_info_x, y);
            int32_t needs_wood = b->loads_stored < SHIPWRIGHT_WOOD_PER_BOAT && (order & BOAT_ORDER_TYPE_MASK) != FIGURE_FISHING_BOAT;
            return needs_wood ? SHIPWRIGHT_TEXT_ORDER_NEEDS_WOOD : SHIPWRIGHT_TEXT_BUILDING;
        }
        case BOAT_ORDER_REPAIR:
            return (b->loads_stored < 1) ? SHIPWRIGHT_TEXT_REPAIRS_NEED_WOOD : SHIPWRIGHT_TEXT_REPAIRING;
        default:
            return -1;
    }
}

void window_building_draw_shipwright(void)
{
    const building *b = building_info_building();
    start_panel(BUILDING_INFO_HELP_SHIPWRIGHT, LANG_GROUP_SHIPWRIGHT);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_TIMBER, building_info_x + SHIPWRIGHT_ICON_X, building_info_y + SHIPWRIGHT_ICON_Y);
    g_text_width = 0;
    draw_line(LANG_GROUP_SHIPWRIGHT, SHIPWRIGHT_TEXT_STORED_WOOD, building_info_x + SHIPWRIGHT_STOCK_X, building_info_y + SHIPWRIGHT_STOCK_Y);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT, (b->loads_stored < 1) ? 0 : b->loads_stored,
        g_text_width + SHIPWRIGHT_STOCK_X + building_info_x, building_info_y + SHIPWRIGHT_STOCK_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + SHIPWRIGHT_STATE_Y);
    } else {
        int32_t text = shipwright_text(b);
        if (text >= 0) {
            lang_text_draw_multiline_ui(LANG_GROUP_SHIPWRIGHT, text, building_info_x + BUILDING_INFO_TEXT_X, building_info_y + SHIPWRIGHT_TEXT_Y,
                (info_w_blocks - 6) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
        }
    }
    window_building_draw_workers_panel();
}

static int32_t dock_staff_step(void)
{
    int32_t percentage = bi_road_access_block.worker_percentage;
    if (percentage < 1) {
        return 0;
    }
    if (percentage < DOCK_STAFF_FEW) {
        return 1;
    }
    return (percentage < DOCK_STAFF_MOST) ? 2 : 3;
}

void window_building_draw_dock(void)
{
    start_panel(BUILDING_INFO_HELP_DOCK, LANG_GROUP_DOCK);
    int32_t y = building_info_y + SHIP_BUILDING_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        int32_t first = (building_info_building()->data.dock.ship_id == 0) ? DOCK_TEXT_WAITING_NO_STAFF : DOCK_TEXT_SERVICING_NO_STAFF;
        window_building_draw_text(LANG_GROUP_DOCK, first + dock_staff_step(), y);
    }
    window_building_draw_workers_panel();
}

static int32_t fishing_boat_text(int32_t boat_id)
{
    switch ((int8_t) figure_get(boat_id)->action_state) {
        case FIGURE_ACTION_FISHING_BOAT_GOING_TO_FISH:
            return FISHING_WHARF_TEXT_SAILING_OUT;
        case FIGURE_ACTION_FISHING_BOAT_FISHING:
            return FISHING_WHARF_TEXT_FISHING;
        case FIGURE_ACTION_FISHING_BOAT_GOING_TO_WHARF:
            return FISHING_WHARF_TEXT_SAILING_BACK;
        case FIGURE_ACTION_FISHING_BOAT_RETURNING_WITH_FISH:
            return FISHING_WHARF_TEXT_RETURNING_WITH_FISH;
        case FIGURE_ACTION_FISHING_BOAT_AT_WHARF:
            return (map_sweep_countdown < 1) ? FISHING_WHARF_TEXT_RESTOCKING : FISHING_WHARF_TEXT_POLLUTED_WATER;
        default:
            return FISHING_WHARF_TEXT_NO_FISHING_GROUNDS;
    }
}

void window_building_draw_fishing_wharf(void)
{
    start_panel(BUILDING_INFO_HELP_FISHING_WHARF, LANG_GROUP_FISHING_WHARF);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_FISH, building_info_x + TITLE_ICON_X, building_info_y + TITLE_ICON_Y);
    int32_t y = building_info_y + SHIP_BUILDING_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        int32_t boat_id = building_info_building()->data.industry.worker_figure_id;
        int32_t text;
        if (city_player_current()->mothballed[RESOURCE_FISH] != 0) {
            text = FISHING_WHARF_TEXT_HALTED;
        } else if (boat_id < 1) {
            text = FISHING_WHARF_TEXT_WAITING_FOR_BOAT;
        } else {
            text = fishing_boat_text(boat_id);
        }
        window_building_draw_text(LANG_GROUP_FISHING_WHARF, text, y);
    }
    window_building_draw_workers_panel();
}

static int32_t has_road_access(int32_t building_id)
{
    const building *b = building_get(building_id);
    return map_has_road_access(b->x, b->y, b->size);
}

static void draw_wharf(int32_t help, int32_t group)
{
    start_panel(help, group);
    if (has_road_access(building_info_building_id) == 0) {
        window_building_draw_no_road_text(building_info_y + WHARF_NO_ROAD_Y);
    }
    window_building_draw_text(group, BUILDING_TEXT_DESCRIPTION, building_info_y + WHARF_TEXT_Y);
    window_building_draw_workers_panel();
}

void window_building_draw_transport_wharf(void)
{
    draw_wharf(BUILDING_INFO_HELP_TRANSPORT_WHARF, LANG_GROUP_TRANSPORT_WHARF);
}

void window_building_draw_warship_wharf(void)
{
    draw_wharf(BUILDING_INFO_HELP_WARSHIP_WHARF, LANG_GROUP_WARSHIP_WHARF);
}

static int32_t ferry_text(int32_t other_id)
{
    if (other_id < 1) {
        return FERRY_TEXT_NO_LINKED_LANDING;
    }
    if (has_road_access(other_id) == 0) {
        return FERRY_TEXT_LINKED_NO_ROAD;
    }
    return (building_get(other_id)->num_workers != 0) ? -1 : FERRY_TEXT_LINKED_NO_WORKERS;
}

void window_building_draw_ferry(void)
{
    int32_t other_id = building_info_building()->linked_building_id;
    start_panel(BUILDING_INFO_HELP_FERRY_LANDING, LANG_GROUP_FERRY_LANDING);
    int32_t y = building_info_y + FERRY_TEXT_Y;
    if (has_road_access(building_info_building_id) == 0) {
        window_building_draw_no_road_text(y);
    } else {
        int32_t text = ferry_text(other_id);
        if (text >= 0) {
            window_building_draw_text(LANG_GROUP_FERRY_LANDING, text, y);
        }
    }
    window_building_draw_workers_panel();
}
