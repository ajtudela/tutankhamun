// The fleet advisor: the navy's status (warships, transports, the threat to the city, the kingdom's troop requests by
// sea), a row per warship with its hull, crew and buttons (go to the ship, return it to its wharf, put it into the
// kingdom's service), the "Dispatch now" button and the handlers of the ship buttons. Without a sea request drawing
// the rows clears every ship's service flag.

#include "window/advisors_fleet.h"
#include "building/building.h"
#include "city/resource_type.h"
#include "empire/empire_data.h"
#include "figure/boats.h"
#include "figure/figure.h"
#include "figure/formation_orders.h"
#include "game/game_internal.h"
#include "graphics/button_border.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "io/game_file_internal.h"
#include "map/grid.h"
#include "map/water.h"
#include "scenario/event_manager.h"
#include "scenario/requests.h"
#include "text/lang.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/advisor/military.h"
#include "window/advisor/troop_requests.h"
#include "window/advisors.h"
#include "window/building_info_ships.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>

int32_t advisor_num_ships;

enum fleet_layout {
    FLEET_WIDTH_BLOCKS = 40,
    FLEET_HEIGHT_BLOCKS = 26,
    FLEET_BLOCK = 16,
    FLEET_ROW_HEIGHT = 44,
    FLEET_ROWS_Y = 83,
    FLEET_SHIP_BUTTON_SIZE = 30,
    FLEET_GO_TO_X = 440,
    FLEET_RETURN_X = 500,
    FLEET_SERVICE_X = 560,
    FLEET_BUTTON_IMAGE_INSET = 3,
    FLEET_SHIP_TEXT_WIDTH = 150,
    FLEET_BULLET_X = 50,
    FLEET_STATUS_X = 70,
    FLEET_DISPATCH_TEXT_X = 550,
};

enum fleet_button {
    FLEET_SHIP_BUTTONS = 3,
    FLEET_BUTTON_CLOSE = 19,
    FLEET_BUTTON_DISPATCH = 20,
    FLEET_CLOSE_BUTTON_RECORD = 18,
    FLEET_DISPATCH_BUTTON_RECORD = 19,
};

enum fort_button_image {
    FORT_BUTTON_IMAGE_GO_TO = 0,
    FORT_BUTTON_IMAGE_RETURN = 1,
    FORT_BUTTON_IMAGE_AT_HOME = 2,
    FORT_BUTTON_IMAGE_IN_SERVICE = 3,
    FORT_BUTTON_IMAGE_NOT_IN_SERVICE = 4,
    FORT_BUTTON_IMAGE_NO_SERVICE = 5,
};

enum troop_request_state {
    TROOP_REQUEST_ABROAD = 5,
    TROOP_REQUEST_RETURNING = 6,
    TROOP_REQUEST_RETURNING_LATE = 7,
};

static const int16_t *fleet_button_rect(int32_t record)
{
    return &buttons_rows.fleet_advisor[record].x;
}

static int32_t ship_is_free(int32_t ship)
{
    return figure_get(ship)->request_event_id < 0;
}

static int32_t fleet_threat_text(const city_player *player)
{
    if (player->kingdom_soldiers != 0) {
        return MILITARY_TEXT_KINGDOM_TROOPS_AT_GATES;
    }
    if (player->enemies != 0) {
        return MILITARY_TEXT_ENEMIES_IN_SIGHT;
    }
    return (scenario_invasion_running() != 0) ? MILITARY_TEXT_ENEMIES_APPROACHING : MILITARY_TEXT_NO_THREATS;
}

static int32_t fleet_request_text(bool requested_by_land, bool requested_by_sea)
{
    uint8_t marching_back = 0;
    uint8_t sailing_back = 0;
    uint8_t marching_abroad = 0;
    uint8_t sailing_abroad = 0;
    for (scenario_event *e = scenario_request_next_active(&scenario_events, NULL); e != NULL;
        e = scenario_request_next_active(&scenario_events, e)) {
        if (e->item.value != RESOURCE_TROOPS) {
            continue;
        }
        int32_t route = e->route.value;
        int32_t by_sea = 0;
        if (route >= 0 && route < EMPIRE_ROUTES) {
            by_sea = empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA;
        }
        int16_t state = e->state;
        if (state == TROOP_REQUEST_ABROAD) {
            if (by_sea) {
                sailing_abroad = 1;
            } else {
                marching_abroad = 1;
            }
        }
        if (state == TROOP_REQUEST_RETURNING || state == TROOP_REQUEST_RETURNING_LATE) {
            if (by_sea) {
                sailing_back = 1;
            } else {
                marching_back = 1;
            }
        }
    }
    if (marching_back) {
        return MILITARY_TEXT_MARCHING_BACK;
    }
    if (sailing_back) {
        return MILITARY_TEXT_SAILING_BACK;
    }
    if (marching_abroad) {
        return MILITARY_TEXT_MARCHING_TO_FIGHT;
    }
    if (sailing_abroad) {
        return MILITARY_TEXT_SAILING_TO_FIGHT;
    }
    if (requested_by_land) {
        return MILITARY_TEXT_TROOPS_NEEDED_ABROAD;
    }
    if (requested_by_sea) {
        return MILITARY_TEXT_NAVY_NEEDED;
    }
    return MILITARY_TEXT_NO_REQUESTS_ABROAD;
}

static void draw_military_text(int32_t text, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, text, x, y, font, 0, ui_origin_x, ui_origin_y);
}

static void draw_ship_buttons(int32_t ship, int32_t y, bool requested_by_sea)
{
    int32_t image_y = y + FLEET_BUTTON_IMAGE_INSET;
    figure *f = figure_get(ship);
    button_border_draw_ui(advisor_x + FLEET_GO_TO_X, y, FLEET_SHIP_BUTTON_SIZE, FLEET_SHIP_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
    image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_GO_TO, advisor_x + FLEET_GO_TO_X + FLEET_BUTTON_IMAGE_INSET, image_y);
    button_border_draw_ui(advisor_x + FLEET_RETURN_X, y, FLEET_SHIP_BUTTON_SIZE, FLEET_SHIP_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
    int32_t return_image = advisor_ship_at_dock(ship) ? FORT_BUTTON_IMAGE_AT_HOME : FORT_BUTTON_IMAGE_RETURN;
    image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + return_image, advisor_x + FLEET_RETURN_X + FLEET_BUTTON_IMAGE_INSET, image_y);
    button_border_draw_ui(advisor_x + FLEET_SERVICE_X, y, FLEET_SHIP_BUTTON_SIZE, FLEET_SHIP_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
    int32_t service_image;
    if (requested_by_sea == 0) {
        f->answers_requests = 0;
        service_image = FORT_BUTTON_IMAGE_NO_SERVICE;
    } else if (f->answers_requests == 0) {
        service_image = FORT_BUTTON_IMAGE_NOT_IN_SERVICE;
    } else {
        service_image = FORT_BUTTON_IMAGE_IN_SERVICE;
    }
    image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + service_image, advisor_x + FLEET_SERVICE_X + FLEET_BUTTON_IMAGE_INSET, image_y);
}

static void draw_ship_rows(bool requested_by_sea)
{
    int32_t dy = 0;
    for (int32_t index = 1; index <= advisor_num_ships; index++) {
        int32_t ship = figure_warship_by_index(index);
        int32_t row_y = advisor_y + FLEET_ROWS_Y + dy;
        image_draw(image_group(IMAGE_GROUP_FLEET_SHIP_ROW), advisor_x + 48, row_y - 1);
        draw_military_text(MILITARY_TEXT_WARSHIP, advisor_x + 100, row_y, image_group(IMAGE_GROUP_FONT_WHITE));
        lang_text_draw_centered_ui(
            LANG_GROUP_SHIPS, window_building_ship_crew_text(ship), advisor_x + 140, row_y + 8, FLEET_SHIP_TEXT_WIDTH, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
        if (ship_is_free(ship)) {
            draw_ship_buttons(ship, row_y, requested_by_sea);
        } else {
            draw_military_text(MILITARY_TEXT_ABROAD, advisor_x + FLEET_RETURN_X, row_y + 3, image_group(IMAGE_GROUP_FONT_GREEN));
        }
        lang_text_draw_centered_ui(LANG_GROUP_SHIPS, window_building_ship_damage_text(ship), advisor_x + 273, row_y + 8, FLEET_SHIP_TEXT_WIDTH, image_group(IMAGE_GROUP_FONT_GREEN), 0,
            ui_origin_x, ui_origin_y);
        dy += FLEET_ROW_HEIGHT;
    }
}

void window_advisor_fleet_draw_background(void)
{
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = FLEET_HEIGHT_BLOCKS;
    int32_t frame = ADVISOR_OPEN_LAST_STEP - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    if (frame >= ADVISOR_OPEN_LAST_STEP) {
        return;
    }
    if (frame > 0) {
        graphics_draw_rect((frame * FLEET_BLOCK) + ui_origin_x, ui_origin_y + (frame * 8), (FLEET_WIDTH_BLOCKS - (frame * 2)) * FLEET_BLOCK,
            (FLEET_HEIGHT_BLOCKS - (frame * 2)) * FLEET_BLOCK, 0);
        return;
    }
    const city_player *player = city_player_current();
    outer_panel_draw(ui_origin_x, ui_origin_y, FLEET_WIDTH_BLOCKS, FLEET_HEIGHT_BLOCKS);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 1, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_NAVY_STATUS, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    draw_military_text(MILITARY_TEXT_GO_TO, advisor_x + 423, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_WARSHIP, advisor_x + 423, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_RETURN, advisor_x + 480, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_TO_WHARF, advisor_x + 480, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_HULL, advisor_x + 315, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_STRENGTH, advisor_x + 300, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_CREW_STATUS, advisor_x + 170, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK));
    advisor_num_ships = player->total_warships;
    bool requested_by_land;
    bool requested_by_sea;
    advisor_has_troop_requests(&requested_by_land, &requested_by_sea);
    int32_t request_text = fleet_request_text(requested_by_land, requested_by_sea);
    if (advisor_num_ships <= 0) {
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + FLEET_BULLET_X, advisor_y + 359);
        draw_military_text(fleet_threat_text(player), advisor_x + FLEET_STATUS_X, advisor_y + 358, image_group(IMAGE_GROUP_FONT_BLACK));
    } else {
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + FLEET_BULLET_X, advisor_y + 349);
        g_text_width = 0;
        lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_WARSHIP, player->total_warships, advisor_x + FLEET_STATUS_X, advisor_y + 348, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        if (player->total_transport_ships != 0) {
            draw_military_text(MILITARY_TEXT_AND, g_text_width + advisor_x + FLEET_STATUS_X, advisor_y + 348, image_group(IMAGE_GROUP_FONT_BLACK));
            lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_TRANSPORT, player->total_transport_ships, g_text_width + advisor_x + FLEET_STATUS_X,
                advisor_y + 348, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        }
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + FLEET_BULLET_X, advisor_y + 369);
        draw_military_text(fleet_threat_text(player), advisor_x + FLEET_STATUS_X, advisor_y + 368, image_group(IMAGE_GROUP_FONT_BLACK));
    }
    image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + FLEET_BULLET_X, advisor_y + 389);
    draw_military_text(request_text, advisor_x + FLEET_STATUS_X, advisor_y + 388, image_group(IMAGE_GROUP_FONT_BLACK));
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 70, 36, 17, ui_origin_x, ui_origin_y);
    if (advisor_num_ships <= 0) {
        lang_text_draw_multiline_ui(
            LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_NO_WARSHIPS, advisor_x + 64, advisor_y + 200, 496, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
    } else {
        draw_ship_rows(requested_by_sea);
    }
    image_draw(image_group(IMAGE_GROUP_CLOSE), advisor_x + 532, advisor_y + 347);
}

void window_advisor_fleet_draw_foreground(void)
{
    if (ADVISOR_OPEN_LAST_STEP - label_animation[LABEL_SLOT_ADVISOR_OPEN] > 0) {
        return;
    }
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_num_ships = city_player_current()->total_warships;
    advisor_height_blocks = FLEET_HEIGHT_BLOCKS;
    bool requested_by_land;
    bool requested_by_sea;
    advisor_has_troop_requests(&requested_by_land, &requested_by_sea);
    int32_t any_in_service = 0;
    int32_t dy = 0;
    int32_t first_button = 1;
    for (int32_t index = 1; index <= advisor_num_ships; index++) {
        int32_t ship = figure_warship_by_index(index);
        int32_t row_y = advisor_y + FLEET_ROWS_Y + dy;
        if (ship_is_free(ship)) {
            button_border_draw_ui(
                advisor_x + FLEET_GO_TO_X, row_y, FLEET_SHIP_BUTTON_SIZE, FLEET_SHIP_BUTTON_SIZE, focus_button_id == first_button, ui_origin_x, ui_origin_y);
            if (figure_get(ship)->answers_requests != 0) {
                any_in_service = 1;
            }
            button_border_draw_ui(advisor_x + FLEET_RETURN_X, row_y, FLEET_SHIP_BUTTON_SIZE, FLEET_SHIP_BUTTON_SIZE, focus_button_id == (first_button + 1),
                ui_origin_x, ui_origin_y);
            button_border_draw_ui(advisor_x + FLEET_SERVICE_X, row_y, FLEET_SHIP_BUTTON_SIZE, FLEET_SHIP_BUTTON_SIZE,
                requested_by_sea != 0 && focus_button_id == (first_button + 2), ui_origin_x, ui_origin_y);
        }
        first_button += FLEET_SHIP_BUTTONS;
        dy += FLEET_ROW_HEIGHT;
    }
    button_border_draw_rect(fleet_button_rect(FLEET_CLOSE_BUTTON_RECORD), focus_button_id == FLEET_BUTTON_CLOSE, 0, 0);
    if (any_in_service) {
        button_border_draw_rect(fleet_button_rect(FLEET_DISPATCH_BUTTON_RECORD), focus_button_id == FLEET_BUTTON_DISPATCH, 0, 0);
        draw_military_text(MILITARY_TEXT_DISPATCH, advisor_x + FLEET_DISPATCH_TEXT_X, advisor_y + 40, image_group(IMAGE_GROUP_FONT_BLACK));
        draw_military_text(MILITARY_TEXT_DISPATCH_NOW, advisor_x + FLEET_DISPATCH_TEXT_X, advisor_y + 54, image_group(IMAGE_GROUP_FONT_BLACK));
        military_empire_button_shown = 1;
        return;
    }
    draw_military_text(MILITARY_TEXT_KINGDOM, advisor_x + FLEET_DISPATCH_TEXT_X, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK));
    draw_military_text(MILITARY_TEXT_KINGDOM_SERVICE, advisor_x + FLEET_DISPATCH_TEXT_X, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK));
    military_empire_button_shown = 0;
}

void button_fleet_go_to_ship(void)
{
    int32_t ship = figure_warship_by_index(button_parameter);
    if (ship == 0 || !ship_is_free(ship)) {
        return;
    }
    const figure *f = figure_get(ship);
    city_view_go_to_grid_offset(map_grid_offset(f->x, f->y));
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void button_fleet_return_to_dock(void)
{
    int32_t ship = figure_warship_by_index(button_parameter);
    if (ship == 0 || !ship_is_free(ship)) {
        return;
    }
    figure_warship_return_to_dock(ship);
    window_redraw_request = 1;
}

void button_fleet_empire_service(void)
{
    int32_t ship = figure_warship_by_index(button_parameter);
    if (ship == 0) {
        return;
    }
    bool requested_by_land;
    bool requested_by_sea;
    advisor_has_troop_requests(&requested_by_land, &requested_by_sea);
    if (!ship_is_free(ship)) {
        return;
    }
    figure *f = figure_get(ship);
    if (requested_by_sea) {
        f->answers_requests = f->answers_requests == 0;
    } else {
        window_popup_dialog_show(requested_by_land ? POPUP_LAND_TROOPS_NEEDED : POPUP_NO_TROOPS_NEEDED, button_none, 1);
        f->answers_requests = 0;
    }
    advisor_military_count_ships();
    window_redraw_request = 1;
}

int32_t advisor_ship_at_dock(int32_t ship)
{
    int32_t saved_x = map_result_x;
    int32_t saved_y = map_result_y;
    int32_t result = 1;
    const figure *f = figure_get(ship);
    const building *wharf = building_get(f->building_id);
    map_water_tile_next_to(wharf->x, wharf->y, wharf->size);
    if (f->destination_x != map_result_x || f->destination_y != map_result_y) {
        result = 0;
    }
    map_result_x = saved_x;
    map_result_y = saved_y;
    return result;
}
