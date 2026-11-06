// The military panels of the info windows: the fort's ground, gatehouses, roadblock, towers, military academy and
// recruiter; the legion's panel with its five order buttons, its return to the fort and the click that gives it a
// target; the order buttons of the warship and the transport. The damage texts follow legion_damage_text_rules.
//
// Orders sometimes act on the figure that happens to be current:
// - ending a charge clears the animation frame of the current figure, not the soldier's;
// - the transport's return removes the route of the current figure, not the ship's;
// - the military academy shows its description while it lacks workers;
// - enemy ships under the mouse are attacked without a target formation.

#include "window/building_info_military.h"
#include "building/building.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/boats.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/formation_orders.h"
#include "figure/image_groups_military.h"
#include "figure/military.h"
#include "figure/route.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "map/water.h"
#include "widget/city_view_rotate.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/building_info_ships.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum military_lang_group {
    LANG_GROUP_FORT = 89,
    LANG_GROUP_MUD_GATEHOUSE = 90,
    LANG_GROUP_MUD_TOWER = 91,
    LANG_GROUP_MILITARY_ACADEMY = 135,
    LANG_GROUP_RECRUITER = 136,
    LANG_GROUP_LEGION = 138,
    LANG_GROUP_ROADBLOCK = 155,
    LANG_GROUP_BRICK_GATEHOUSE = 167,
    LANG_GROUP_CLAY_GATEHOUSE = 168,
    LANG_GROUP_BRICK_TOWER = 169,
    LANG_GROUP_CLAY_TOWER = 170,
};

enum legion_text {
    LEGION_TEXT_NO_SOLDIERS_IN_FORT = 10,
    LEGION_TEXT_NO_SOLDIERS = 11,
    LEGION_TEXT_HOLD_TIGHT = 12,
    LEGION_TEXT_HOLD_TIGHT_HELP = 13,
    LEGION_TEXT_HOLD_LOOSE = 14,
    LEGION_TEXT_HOLD_LOOSE_HELP = 15,
    LEGION_TEXT_ENGAGE = 16,
    LEGION_TEXT_ENGAGE_HELP = 17,
    LEGION_TEXT_MOP_UP = 18,
    LEGION_TEXT_MOP_UP_HELP = 19,
    LEGION_TEXT_CHARGE = 20,
    LEGION_TEXT_CHARGE_HELP = 21,
    LEGION_TEXT_RETURN_HELP = 22,
    LEGION_TEXT_SOLDIERS = 23,
    LEGION_TEXT_DAMAGE = 24,
    LEGION_TEXT_EXPERIENCE = 25,
    LEGION_TEXT_DAMAGE_FIRST = 26,
    LEGION_TEXT_MORALE = 36,
    LEGION_TEXT_MORALE_FIRST = 37,
    LEGION_TEXT_RETURN_TO_FORT = 58,
    LEGION_TEXT_CURSED_BY_SETH = 59,
    LEGION_TEXT_EXPERIENCE_FIRST = 60,
    LEGION_TEXT_COMPANY = 73,
    LEGION_TEXT_INFANTRY_COMPANY = 74,
    LEGION_TEXT_ARCHER_COMPANY = 75,
    LEGION_TEXT_CHARIOTEER_COMPANY = 76,
    LEGION_TEXT_ROTATE_LINE = 77,
};

enum military_building_text {
    FORT_TEXT_CURSED = 1,
    FORT_TEXT_RECRUITS = 2,
    TOWER_TEXT_NO_WORKERS = 2,
    TOWER_TEXT_READY = 3,
    TOWER_TEXT_NO_GUARDS = 4,
    ACADEMY_TEXT_NO_STAFF = 2,
    ACADEMY_TEXT_TRAINING = 3,
    RECRUITER_TEXT_WEAPONS_STORED = 2,
    RECRUITER_TEXT_NO_STAFF = 3,
    RECRUITER_TEXT_NO_REQUESTS = 4,
    RECRUITER_TEXT_NO_WEAPONS_FULL_STAFF = 5,
    RECRUITER_TEXT_FULL_STAFF = 9,
    RECRUITER_TEXT_CHARIOTS_STORED = 13,
    RECRUITER_TEXT_CHARIOT_STORED = 14,
    RECRUITER_TEXT_WEAPON_STORED = 15,
};

enum military_layout {
    LEGION_IMAGE_COLUMN_X = 16,
    LEGION_IMAGE_COLUMN_WIDTH = 40,
    LEGION_IMAGES_Y = 16,
    LEGION_TYPE_TITLE_Y = 30,
    LEGION_LINES_X = 100,
    LEGION_VALUES_X = 300,
    LEGION_SOLDIERS_VALUE_X = 294,
    LEGION_EXPERIENCE_ICON_X = 275,
    LEGION_SOLDIERS_Y = 60,
    LEGION_DAMAGE_Y = 80,
    LEGION_EXPERIENCE_Y = 100,
    LEGION_MORALE_Y = 120,
    LEGION_NO_SOLDIERS_Y = 172,
    LEGION_ORDER_X = 21,
    LEGION_ORDER_SPACING = 85,
    LEGION_ORDER_Y = 141,
    LEGION_ORDER_BORDER_X = 19,
    LEGION_ORDER_BORDER_Y = 139,
    LEGION_ORDER_BORDER_SIZE = 84,
    LEGION_ORDER_PANEL_Y = 230,
    LEGION_ORDER_PANEL_BLOCKS = 5,
    LEGION_ORDER_TITLE_X = 24,
    LEGION_ORDER_TITLE_Y = 236,
    LEGION_ORDER_TEXT_Y = 252,
    LEGION_ROTATE_BUTTON_WIDTH = 320,
    LEGION_ROTATE_BUTTON_HEIGHT = 30,
    LEGION_ROTATE_BUTTON_FROM_BOTTOM = 40,
    MILITARY_DESCRIPTION_FROM_BOTTOM = 158,
    MILITARY_TEXT_Y = 56,
    CLAY_TOWER_TEXT_Y = 50,
    CLAY_TOWER_DESCRIPTION_Y = 220,
    RECRUITER_ICON_X = 30,
    RECRUITER_TEXT_X = 58,
    RECRUITER_WEAPONS_ICON_Y = 38,
    RECRUITER_WEAPONS_TEXT_Y = 44,
    RECRUITER_CHARIOTS_ICON_Y = 64,
    RECRUITER_CHARIOTS_TEXT_Y = 70,
    RECRUITER_STATE_Y = 90,
};

enum legion_values {
    LEGION_EXPERIENCE_STEP = 20,
    LEGION_EXPERIENCE_ROUND = 10,
    LEGION_MORALE_STEP = 5,
    LEGION_EXPERIENCE_BANNER_STEP = 5,
    LEGION_BANNER_ROUND = 3,
    LEGION_BANNER_IMAGES = 20,
    LEGION_INFANTRY_FLAG = 18,
    LEGION_ARCHER_FLAG = 9,
    LEGION_HALTED_FLAG = 8,
    LEGION_CHARGE_HELD_IMAGE = 6,
    LEGION_CHARGE_IMAGE = 1,
    LEGION_AT_FORT_RETURN_IMAGE = 5,
    LEGION_CHARGE_END_MORALE = 10,
    RECRUITER_UNITS_PER_ITEM = 100,
    RECRUITER_STAFF_HIGH = 66,
    RECRUITER_STAFF_LOW = 33,
    SHIP_DAMAGED_PERCENT = 15,
};

int32_t legion_damage_text_rules[LEGION_DAMAGE_TEXT_RULE_COUNT] = {
    [LEGION_DAMAGE_TEXT_RULE_PERFECT] = 0,
    [LEGION_DAMAGE_TEXT_RULE_VERY_GOOD] = 20,
    [LEGION_DAMAGE_TEXT_RULE_GOOD] = 40,
    [LEGION_DAMAGE_TEXT_RULE_AVERAGE] = 55,
    [LEGION_DAMAGE_TEXT_RULE_POOR] = 70,
    [LEGION_DAMAGE_TEXT_RULE_VERY_POOR] = 90,
};

static void close_to_city(void)
{
    window_current_id = WINDOW_CITY;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static int32_t charioteers_charging(const formation *legion)
{
    return legion->figure_type == FIGURE_CHARIOTEER && figure_get(legion->figures[0])->action_state == FIGURE_ACTION_SOLDIER_CHARGE;
}

static void end_charge(int32_t legion_id)
{
    formation *legion = formation_get(legion_id);
    int32_t current = figure_current_id;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t soldier_id = legion->figures[i];
        if (soldier_id == 0) {
            break;
        }
        figure *soldier = figure_get(soldier_id);
        if (soldier->action_state == FIGURE_ACTION_CORPSE || legion->months_very_low_morale != 0 || legion->months_low_morale > 1) {
            continue;
        }
        if (legion->months_low_morale == 1) {
            formation_change_morale(legion_id, LEGION_CHARGE_END_MORALE);
            current = figure_current_id;
        }
        soldier->alternative_location_index = 0;
        soldier->charge.steps = 0;
        soldier->charge.tile_x = soldier->x;
        soldier->charge.tile_y = soldier->y;
        soldier->action_state = FIGURE_ACTION_SOLDIER_AT_STANDARD;
        figure_get(current)->animation_frame = 0;
    }
}

static void set_order(formation *legion, int32_t order)
{
    switch (order) {
        case LEGION_ORDER_HOLD_TIGHT:
            legion->layout = (int16_t) ((legion->alternate_layout != 0) ? FORMATION_DOUBLE_LINE_2 : FORMATION_DOUBLE_LINE_1);
            break;
        case LEGION_ORDER_HOLD_LOOSE:
            if (legion->figure_type != FIGURE_CHARIOTEER) {
                legion->layout = (int16_t) ((legion->alternate_layout != 0) ? FORMATION_SINGLE_LINE_2 : FORMATION_SINGLE_LINE_1);
            } else if (legion->has_move_order == 0) {
                if (legion->layout != FORMATION_COLUMN) {
                    legion->saved_layout = legion->layout;
                }
                legion->layout = FORMATION_COLUMN;
            }
            break;
        case LEGION_ORDER_ENGAGE:
            legion->layout = FORMATION_CHARGE;
            break;
        case LEGION_ORDER_MOP_UP:
            legion->layout = FORMATION_MOP_UP;
            break;
        default:
            break;
    }
}

void legion_order_button(void)
{
    int32_t legion_id = building_info_formation_id;
    formation *legion = formation_get(legion_id);
    if (legion->request_event_id >= 0 || settings_data.paused != 0) {
        return;
    }
    if (button_parameter == LEGION_ORDER_MOP_UP && legion->layout != FORMATION_MOP_UP) {
        legion->prev_layout = legion->layout;
    }
    if (charioteers_charging(legion) && button_parameter != LEGION_ORDER_HOLD_LOOSE) {
        end_charge(legion_id);
    }
    set_order(legion, button_parameter);
    formation_selected = legion_id;
    if (legion->layout == FORMATION_MOP_UP) {
        legion->attack_formation_id = -1;
        close_to_city();
        return;
    }
    if (legion->layout == FORMATION_COLUMN) {
        legion->attack_formation_id = -1;
        window_current_id = WINDOW_CITY_LEGION_ORDER;
        window_redraw_request = WINDOW_REDRAW_ALL;
        return;
    }
    if (legion->has_move_order != 0 && button_parameter == LEGION_ORDER_HOLD_LOOSE) {
        window_current_id = WINDOW_BUILDING_INFO;
    } else {
        window_current_id = WINDOW_CITY_LEGION_SELECTED;
    }
    window_redraw_request = WINDOW_REDRAW_ALL;
    legion->attack_formation_id = -1;
}

void legion_return_button(void)
{
    int32_t legion_id = building_info_formation_id;
    const formation *legion = formation_get(legion_id);
    if (legion->request_event_id >= 0 || legion->is_at_fort == 1 || settings_data.paused != 0) {
        return;
    }
    if (charioteers_charging(legion) && button_parameter != LEGION_ORDER_HOLD_LOOSE) {
        end_charge(legion_id);
    }
    formation_legion_to_standard(legion_id, FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_TO_LANDING);
    close_to_city();
}

static int32_t ship_damaged_at_dock(const figure *ship, int32_t going_to_dock, int32_t queued_at_dock)
{
    int32_t health = calc_percentage(ship->ship_hit_points, ship->ship_max_hit_points);
    int32_t action = ship->action_state;
    return (action == going_to_dock || action == queued_at_dock) && health <= SHIP_DAMAGED_PERCENT;
}

void warship_order_button(void)
{
    int32_t ship_id = building_info_warship_id;
    if (ship_id <= 0 || settings_data.paused != 0) {
        return;
    }
    const figure *ship = figure_get(ship_id);
    if (ship_damaged_at_dock(ship, FIGURE_ACTION_WARSHIP_GOING_TO_DOCK, FIGURE_ACTION_WARSHIP_QUEUED_AT_DOCK)) {
        return;
    }
    if (button_parameter == SHIP_ORDER_RETURN && ship_is_docked(ship_id) != 0) {
        return;
    }
    if (ship->request_event_id >= 0) {
        return;
    }
    if (button_parameter == SHIP_ORDER_RETURN) {
        selected_warship_active = 0;
        figure_warship_return_to_dock(ship_id);
    }
    close_to_city();
}

static void transport_ship_return(figure *ship)
{
    ship->action_state = FIGURE_ACTION_TRANSPORT_SHIP_SAILING;
    const building *dock = building_get(ship->building_id);
    int32_t old_x = map_result_x;
    int32_t old_y = map_result_y;
    selected_transport_active = 0;
    map_water_tile_next_to(dock->x, dock->y, dock->size);
    ship->source_x = ship->x;
    ship->source_y = ship->y;
    ship->destination_x = (int16_t) map_result_x;
    ship->destination_y = (int16_t) map_result_y;
    map_result_x = old_x;
    map_result_y = old_y;
    figure_route_remove(figure_current_id);
}

void transport_ship_order_button(void)
{
    int32_t ship_id = building_info_transport_id;
    selected_transport_id = ship_id;
    if (ship_id <= 0) {
        return;
    }
    figure *ship = figure_get(ship_id);
    if (ship_damaged_at_dock(ship, FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK, FIGURE_ACTION_TRANSPORT_SHIP_QUEUED_AT_DOCK)) {
        return;
    }
    if (button_parameter == SHIP_ORDER_RETURN) {
        if (ship_is_docked(ship_id) != 0) {
            return;
        }
        transport_ship_return(ship);
    }
    close_to_city();
}

static int32_t order_button_of_layout(int32_t layout)
{
    switch (layout) {
        case FORMATION_DOUBLE_LINE_1:
        case FORMATION_DOUBLE_LINE_2:
            return LEGION_ORDER_HOLD_TIGHT;
        case FORMATION_SINGLE_LINE_1:
        case FORMATION_SINGLE_LINE_2:
            return LEGION_ORDER_HOLD_LOOSE;
        case FORMATION_CHARGE:
            return LEGION_ORDER_ENGAGE;
        default:
            return -1;
    }
}

void legion_order_button_toggle(void)
{
    int32_t selected = formation_selected;
    int32_t legion_id = selected;
    if (legion_id == 0 || selected != building_info_formation_id) {
        legion_id = building_info_formation_id;
    }
    if (settings_data.paused != 0) {
        return;
    }
    formation *legion = formation_get(legion_id);
    legion->alternate_layout = (uint8_t) (1 - legion->alternate_layout);
    int32_t order = order_button_of_layout(legion->layout);
    if (order >= 0) {
        button_parameter = order;
    }
    if (button_parameter2 != 1) {
        return;
    }
    building_info_formation_id = selected;
    legion_order_button();
}

static int32_t is_enemy_ship(int32_t type)
{
    return type == FIGURE_ENEMY_TRANSPORT_SHIP || type == FIGURE_ENEMY_EGYPTIAN_WARSHIP || type == FIGURE_ENEMY_WARSHIP ||
        type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP;
}

void legion_target_handle_input(void)
{
    if (settings_data.paused != 0) {
        return;
    }
    if (map_info.selected_grid == 0) {
        close_to_city();
        return;
    }
    int32_t grid_offset = map_grid_offset(map_info.cursor_tile_x, map_info.cursor_tile_y);
    int32_t legion_id = formation_selected;
    const formation *legion = formation_get(legion_id);
    if (legion->request_event_id >= 0 || legion->cursed_by_seth != 0) {
        return;
    }
    int32_t fort_legion = formation_legion_at_building(grid_offset);
    int32_t fighter_id = figure_fighter_at_grid_offset(grid_offset);
    legion_id = formation_selected;
    if (fort_legion != 0 && fort_legion == legion_id) {
        formation_legion_to_standard(legion_id, FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_TO_LANDING);
        close_to_city();
        return;
    }
    const figure *fighter = figure_get(fighter_id);
    int32_t target = is_enemy_ship(fighter->type) ? -1 : fighter->formation_id;
    if (fighter_id != 0) {
        formation_legion_attack_near(map_info.cursor_tile_x, map_info.cursor_tile_y, legion_id, target);
    } else {
        formation_legion_move_to(map_info.cursor_tile_x, map_info.cursor_tile_y, legion_id, 0);
    }
    close_to_city();
}

static void draw_label(int32_t index, int32_t x, int32_t y)
{
    lang_text_draw_ui(LANG_GROUP_LEGION, index, x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static int32_t damage_text(int32_t damage_percentage)
{
    for (int32_t rule = 0; rule < LEGION_DAMAGE_TEXT_RULE_COUNT; rule++) {
        if (damage_percentage <= legion_damage_text_rules[rule]) {
            return LEGION_TEXT_DAMAGE_FIRST + rule;
        }
    }
    return LEGION_TEXT_DAMAGE_FIRST + LEGION_DAMAGE_TEXT_RULE_COUNT;
}

void window_building_draw_legion_lines(int32_t formation_id, int32_t soldiers_y, int32_t damage_y, int32_t experience_y, int32_t morale_y)
{
    const formation *legion = formation_get(formation_id);
    int32_t x = building_info_x + LEGION_LINES_X;
    draw_label(LEGION_TEXT_SOLDIERS, x, building_info_y + soldiers_y);
    text_draw_number_ui(legion->num_figures, NUMBER_PREFIX_NONE, postfix_none, building_info_x + LEGION_SOLDIERS_VALUE_X, building_info_y + soldiers_y, image_group(IMAGE_GROUP_FONT_BLACK), 0,
        building_info_x, building_info_y);
    int32_t damage = calc_percentage(legion->total_damage, legion->max_total_damage);
    draw_label(LEGION_TEXT_DAMAGE, x, building_info_y + damage_y);
    draw_label(damage_text(damage), building_info_x + LEGION_VALUES_X, building_info_y + damage_y);
    g_text_width = 0;
    draw_label(LEGION_TEXT_COMPANY, x, building_info_y + experience_y);
    draw_label(LEGION_TEXT_EXPERIENCE, g_text_width + x, building_info_y + experience_y);
    int32_t experience_level = (legion->experience + LEGION_EXPERIENCE_ROUND) / LEGION_EXPERIENCE_STEP;
    image_draw(experience_level + image_group(IMAGE_GROUP_FORT_EXPERIENCE_ICONS), building_info_x + LEGION_EXPERIENCE_ICON_X, building_info_y + experience_y);
    draw_label(experience_level + LEGION_TEXT_EXPERIENCE_FIRST, building_info_x + LEGION_VALUES_X, building_info_y + experience_y);
    if (legion->cursed_by_seth == 0) {
        g_text_width = 0;
        draw_label(LEGION_TEXT_COMPANY, x, building_info_y + morale_y);
        draw_label(LEGION_TEXT_MORALE, g_text_width + x, building_info_y + morale_y);
        draw_label(legion->morale / LEGION_MORALE_STEP + LEGION_TEXT_MORALE_FIRST, building_info_x + LEGION_VALUES_X, building_info_y + morale_y);
    } else {
        draw_label(LEGION_TEXT_CURSED_BY_SETH, x, building_info_y + morale_y);
    }
}

static void draw_legion_image(int32_t image, int32_t y)
{
    image_draw(image, (LEGION_IMAGE_COLUMN_WIDTH - image_header_get(image)->width) / 2 + LEGION_IMAGE_COLUMN_X + building_info_x, y);
}

static int32_t legion_type_title(const formation *legion)
{
    if (legion->figure_type == FIGURE_CHARIOTEER) {
        return LEGION_TEXT_CHARIOTEER_COMPANY;
    }
    return (legion->figure_type == FIGURE_INFANTRY) ? LEGION_TEXT_INFANTRY_COMPANY : LEGION_TEXT_ARCHER_COMPANY;
}

static int32_t legion_type_flag(const formation *legion)
{
    int32_t image = image_group(IMAGE_GROUP_FIGURE_STANDARD_FLAG);
    if (legion->figure_type == FIGURE_INFANTRY) {
        image += LEGION_INFANTRY_FLAG;
    } else if (legion->figure_type != FIGURE_CHARIOTEER) {
        image += LEGION_ARCHER_FLAG;
    }
    return (legion->is_halted != 0) ? image + LEGION_HALTED_FLAG : image;
}

static void draw_legion_banners(const formation *legion)
{
    int32_t standard = legion->legion_id + image_group(IMAGE_GROUP_FORT_LEGION_STANDARDS);
    int32_t standard_height = image_header_get(standard)->height;
    draw_legion_image(standard, building_info_y + LEGION_IMAGES_Y);
    int32_t flag = legion_type_flag(legion);
    int32_t flag_height = image_header_get(flag)->height;
    draw_legion_image(flag, building_info_y + LEGION_IMAGES_Y + standard_height);
    lang_text_draw_centered_ui(LANG_GROUP_LEGION, legion_type_title(legion), building_info_x, building_info_y + LEGION_TYPE_TITLE_Y,
        info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
    int32_t banners_y = building_info_y + flag_height + LEGION_IMAGES_Y + standard_height;
    draw_legion_image(image_group(IMAGE_GROUP_FIGURE_STANDARD) - legion->morale / LEGION_MORALE_STEP + LEGION_BANNER_IMAGES, banners_y);
    draw_legion_image(
        image_group(IMAGE_GROUP_FORT_FLAG) - (legion->experience + LEGION_BANNER_ROUND) / LEGION_EXPERIENCE_BANNER_STEP + LEGION_BANNER_IMAGES, banners_y);
}

static void draw_order_image(const formation *legion, int32_t order)
{
    int32_t base = (legion->figure_type == FIGURE_CHARIOTEER) ? image_group(IMAGE_GROUP_LEGION_ORDERS_D)
        : (legion->figure_type == FIGURE_ARCHER)              ? image_group(IMAGE_GROUP_LEGION_ORDERS_B)
                                                              : image_group(IMAGE_GROUP_LEGION_ORDERS);
    if (legion->figure_type == FIGURE_CHARIOTEER && order == LEGION_ORDER_HOLD_LOOSE) {
        int32_t image = image_group(IMAGE_GROUP_LEGION_ORDERS_D) + ((legion->has_move_order != 0) ? LEGION_CHARGE_HELD_IMAGE : LEGION_CHARGE_IMAGE);
        image_draw(image, g_draw_x, g_draw_y);
        return;
    }
    int32_t straight = map_info.orientation == VIEW_ORIENTATION_NORTH || map_info.orientation == VIEW_ORIENTATION_SOUTH;
    int32_t mirrored;
    if (legion->alternate_layout != 0 && order < LEGION_ORDER_RETURN) {
        mirrored = !straight;
    } else if (order == LEGION_ORDER_RETURN && legion->is_at_fort != 0) {
        image_draw(base + LEGION_AT_FORT_RETURN_IMAGE, g_draw_x, g_draw_y);
        return;
    } else {
        mirrored = straight;
    }
    if (mirrored) {
        image_draw_mirrored(base + order, g_draw_x, g_draw_y);
    } else {
        image_draw(base + order, g_draw_x, g_draw_y);
    }
}

void window_building_draw_legion(int32_t formation_id)
{
    const formation *legion = formation_get(formation_id);
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_LEGION];
    building_info_details_shown = 0;
    outer_panel_draw(building_info_x, building_info_y, info_w_blocks, building_info_h);
    lang_text_draw_centered_ui(LANG_GROUP_LEGION, legion->legion_id, building_info_x, building_info_y + BUILDING_INFO_TITLE_Y,
        info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
    draw_legion_banners(legion);
    window_building_draw_legion_lines(formation_id, LEGION_SOLDIERS_Y, LEGION_DAMAGE_Y, LEGION_EXPERIENCE_Y, LEGION_MORALE_Y);
    if (legion->num_figures == 0) {
        int32_t group = LANG_GROUP_LEGION;
        int32_t text;
        if (legion->cursed_by_seth != 0) {
            group = LANG_GROUP_FORT;
            text = FORT_TEXT_CURSED;
        } else {
            text = (building_count_active[BUILDING_RECRUITER] == 0) ? LEGION_TEXT_NO_SOLDIERS : LEGION_TEXT_NO_SOLDIERS_IN_FORT;
        }
        window_building_draw_text(group, text, building_info_y + LEGION_NO_SOLDIERS_Y);
        return;
    }
    for (int32_t order = 0; order < LEGION_ORDER_BUTTONS; order++) {
        g_draw_x = order * LEGION_ORDER_SPACING + LEGION_ORDER_X + building_info_x;
        g_draw_y = building_info_y + LEGION_ORDER_Y;
        draw_order_image(legion, order);
    }
}

static int32_t order_shown_without_focus(const formation *legion, int32_t order)
{
    int32_t layout = legion->layout;
    int32_t current = order_button_of_layout(layout);
    if (layout == FORMATION_COLUMN && legion->has_move_order == 0) {
        current = LEGION_ORDER_HOLD_LOOSE;
    } else if (layout == FORMATION_MOP_UP) {
        current = LEGION_ORDER_MOP_UP;
    }
    return order == current;
}

static void order_texts_of_button(const formation *legion, int32_t order, int32_t *title, int32_t *text)
{
    switch (order) {
        case LEGION_ORDER_HOLD_TIGHT:
            *title = LEGION_TEXT_HOLD_TIGHT;
            break;
        case LEGION_ORDER_HOLD_LOOSE:
            *title = (legion->figure_type == FIGURE_CHARIOTEER) ? LEGION_TEXT_CHARGE : LEGION_TEXT_HOLD_LOOSE;
            break;
        case LEGION_ORDER_ENGAGE:
            *title = LEGION_TEXT_ENGAGE;
            break;
        case LEGION_ORDER_MOP_UP:
            *title = LEGION_TEXT_MOP_UP;
            break;
        default:
            *title = LEGION_TEXT_RETURN_TO_FORT;
            *text = LEGION_TEXT_RETURN_HELP;
            return;
    }
    *text = *title + 1;
}

static void order_texts_of_layout(int32_t layout, int32_t *title, int32_t *text)
{
    switch (layout) {
        case FORMATION_DOUBLE_LINE_1:
        case FORMATION_DOUBLE_LINE_2:
            *title = LEGION_TEXT_HOLD_TIGHT;
            break;
        case FORMATION_SINGLE_LINE_1:
        case FORMATION_SINGLE_LINE_2:
            *title = LEGION_TEXT_HOLD_LOOSE;
            break;
        case FORMATION_COLUMN:
            *title = LEGION_TEXT_CHARGE;
            break;
        case FORMATION_CHARGE:
            *title = LEGION_TEXT_ENGAGE;
            break;
        case FORMATION_MOP_UP:
            *title = LEGION_TEXT_MOP_UP;
            break;
        default:
            *title = LEGION_TEXT_RETURN_TO_FORT;
            *text = LEGION_TEXT_RETURN_HELP;
            return;
    }
    *text = *title + 1;
}

void window_building_draw_legion_foreground(int32_t formation_id)
{
    const formation *legion = formation_get(formation_id);
    if (legion->num_figures == 0) {
        return;
    }
    for (int32_t order = 0; order < LEGION_ORDER_BUTTONS; order++) {
        int32_t focus = (focus_button_id == 0) ? order_shown_without_focus(legion, order) : order == focus_button_id - 1;
        button_border_draw_ui(order * LEGION_ORDER_SPACING + LEGION_ORDER_BORDER_X + building_info_x, building_info_y + LEGION_ORDER_BORDER_Y,
            LEGION_ORDER_BORDER_SIZE, LEGION_ORDER_BORDER_SIZE, focus, building_info_x, building_info_y);
    }
    window_building_draw_inner_panel(LEGION_ORDER_PANEL_Y, LEGION_ORDER_PANEL_BLOCKS);
    int32_t title;
    int32_t text;
    if (focus_button_id >= 1 && focus_button_id <= LEGION_ORDER_BUTTONS) {
        order_texts_of_button(legion, focus_button_id - 1, &title, &text);
    } else {
        order_texts_of_layout(legion->layout, &title, &text);
    }
    lang_text_draw_ui(LANG_GROUP_LEGION, title, building_info_x + LEGION_ORDER_TITLE_X, building_info_y + LEGION_ORDER_TITLE_Y, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x,
        building_info_y);
    lang_text_draw_multiline_ui(LANG_GROUP_LEGION, text, building_info_x + LEGION_ORDER_TITLE_X, building_info_y + LEGION_ORDER_TEXT_Y,
        (info_w_blocks - 4) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
    int32_t rotate_x = ((info_w_blocks - 20) * BUILDING_INFO_BLOCK) / 2 + building_info_x;
    button_border_draw_ui(rotate_x, (building_info_h * BUILDING_INFO_BLOCK) - LEGION_ROTATE_BUTTON_FROM_BOTTOM + building_info_y, LEGION_ROTATE_BUTTON_WIDTH,
        LEGION_ROTATE_BUTTON_HEIGHT, focus_button_id_secondary == 1, building_info_x, building_info_y);
    lang_text_draw_centered_ui(LANG_GROUP_LEGION, LEGION_TEXT_ROTATE_LINE, rotate_x, (building_info_h - 2) * BUILDING_INFO_BLOCK + building_info_y,
        LEGION_ROTATE_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_description_panel(int32_t help, int32_t group)
{
    info_help_id = building_info_help_messages[help];
    info_first_draw = 0;
    window_building_draw_panel_title(group, BUILDING_TEXT_TITLE);
}

void window_building_draw_fort(void)
{
    const formation *legion = formation_get(building_info_formation_id);
    draw_description_panel(BUILDING_INFO_HELP_FORT, LANG_GROUP_FORT);
    window_building_draw_text(LANG_GROUP_FORT, (legion->cursed_by_seth != 0) ? FORT_TEXT_CURSED : FORT_TEXT_RECRUITS,
        (building_info_h * BUILDING_INFO_BLOCK) - MILITARY_DESCRIPTION_FROM_BOTTOM + building_info_y);
}

static void draw_described_building(int32_t help, int32_t group)
{
    draw_description_panel(help, group);
    window_building_draw_text(group, BUILDING_TEXT_DESCRIPTION, (building_info_h * BUILDING_INFO_BLOCK) - MILITARY_DESCRIPTION_FROM_BOTTOM + building_info_y);
}

void window_building_draw_mud_gatehouse(void)
{
    draw_described_building(BUILDING_INFO_HELP_GATEHOUSE, LANG_GROUP_MUD_GATEHOUSE);
}

void window_building_draw_brick_gatehouse(void)
{
    draw_described_building(BUILDING_INFO_HELP_GATEHOUSE, LANG_GROUP_BRICK_GATEHOUSE);
}

void window_building_draw_clay_gatehouse(void)
{
    draw_described_building(BUILDING_INFO_HELP_GATEHOUSE, LANG_GROUP_CLAY_GATEHOUSE);
}

void window_building_draw_roadblock(void)
{
    draw_described_building(BUILDING_INFO_HELP_ROADBLOCK, LANG_GROUP_ROADBLOCK);
}

static void draw_tower(int32_t group, int32_t text_y)
{
    const building *b = building_info_building();
    draw_description_panel(BUILDING_INFO_HELP_TOWER, group);
    int32_t y = building_info_y + text_y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
        return;
    }
    int32_t text = (b->num_workers < 1) ? TOWER_TEXT_NO_WORKERS : (b->figure_id < 1) ? TOWER_TEXT_NO_GUARDS : TOWER_TEXT_READY;
    window_building_draw_text(group, text, y);
}

void window_building_draw_mud_tower(void)
{
    draw_tower(LANG_GROUP_MUD_TOWER, MILITARY_TEXT_Y);
    window_building_draw_workers_panel();
}

void window_building_draw_brick_tower(void)
{
    draw_tower(LANG_GROUP_BRICK_TOWER, MILITARY_TEXT_Y);
    window_building_draw_workers_panel();
}

void window_building_draw_clay_tower(void)
{
    draw_tower(LANG_GROUP_CLAY_TOWER, CLAY_TOWER_TEXT_Y);
    window_building_draw_text(LANG_GROUP_CLAY_TOWER, BUILDING_TEXT_DESCRIPTION, building_info_y + CLAY_TOWER_DESCRIPTION_Y);
    window_building_draw_workers_panel();
}

void window_building_draw_military_academy(void)
{
    const building *b = building_info_building();
    draw_description_panel(BUILDING_INFO_HELP_MILITARY_ACADEMY, LANG_GROUP_MILITARY_ACADEMY);
    int32_t y = building_info_y + MILITARY_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        int32_t text = (b->num_workers < 1)                        ? ACADEMY_TEXT_NO_STAFF
            : (b->num_workers < building_model_table[b->type].laborers) ? BUILDING_TEXT_DESCRIPTION
                                                                   : ACADEMY_TEXT_TRAINING;
        window_building_draw_text(LANG_GROUP_MILITARY_ACADEMY, text, y);
    }
    window_building_draw_workers_panel();
}

static void draw_recruiter_stock(int32_t resource, int32_t loads, int32_t one_text, int32_t more_text, int32_t icon_y, int32_t text_y)
{
    int32_t items = loads / RECRUITER_UNITS_PER_ITEM;
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, building_info_x + RECRUITER_ICON_X, building_info_y + icon_y);
    g_text_width = 0;
    text_draw_number_ui(items, NUMBER_PREFIX_NONE, empty_text, building_info_x + RECRUITER_TEXT_X, building_info_y + text_y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_ui(LANG_GROUP_RECRUITER, (items == 1) ? one_text : more_text, g_text_width + RECRUITER_TEXT_X + building_info_x, building_info_y + text_y,
        image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static int32_t recruiter_staff_text(int32_t full_staff_text)
{
    int32_t percentage = bi_road_access_block.worker_percentage;
    if (percentage >= 100) {
        return full_staff_text;
    }
    if (percentage >= RECRUITER_STAFF_HIGH) {
        return full_staff_text + 1;
    }
    return (percentage >= RECRUITER_STAFF_LOW) ? full_staff_text + 2 : full_staff_text + 3;
}

void window_building_draw_recruiter(void)
{
    const building *b = building_info_building();
    draw_description_panel(BUILDING_INFO_HELP_RECRUITER, LANG_GROUP_RECRUITER);
    draw_recruiter_stock(
        RESOURCE_WEAPONS, b->loads_stored, RECRUITER_TEXT_WEAPON_STORED, RECRUITER_TEXT_WEAPONS_STORED, RECRUITER_WEAPONS_ICON_Y, RECRUITER_WEAPONS_TEXT_Y);
    if (b->second_input_stock != 0) {
        draw_recruiter_stock(RESOURCE_CHARIOTS, b->second_input_stock, RECRUITER_TEXT_CHARIOT_STORED, RECRUITER_TEXT_CHARIOTS_STORED, RECRUITER_CHARIOTS_ICON_Y,
            RECRUITER_CHARIOTS_TEXT_Y);
    }
    int32_t y = building_info_y + RECRUITER_STATE_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else {
        int32_t text;
        if (b->num_workers < 1) {
            text = RECRUITER_TEXT_NO_STAFF;
        } else if (info_has_academy == 0) {
            text = RECRUITER_TEXT_NO_REQUESTS;
        } else {
            text = recruiter_staff_text((b->loads_stored < 1) ? RECRUITER_TEXT_NO_WEAPONS_FULL_STAFF : RECRUITER_TEXT_FULL_STAFF);
        }
        window_building_draw_text(LANG_GROUP_RECRUITER, text, y);
    }
    window_building_draw_workers_panel();
}
