// The military advisor: its background, its foreground (the buttons of each legion without a target, the troop
// requests), the count of the legions, and its buttons: go to a legion, return it to its fort, put it into the
// empire's service, and send help for a troops request (also the fleet advisor's button).
//
// Sending help walks five request rows; a row past the waiting requests keeps the last row's request id, so that
// request can be dispatched again.

#include "window/advisor/military.h"
#include "city/resource_type.h"
#include "empire/empire_route.h"
#include "empire/empire_types.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
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
#include "scenario/event_manager.h"
#include "scenario/requests.h"
#include "scenario/scenario_internal.h"
#include "text/lang.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/advisor/chief.h"
#include "window/advisor/troop_requests.h"
#include "window/advisors.h"
#include "window/advisors_internal.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>

int32_t advisor_num_legions;

enum company_text {
    COMPANY_TEXT_CHARIOTEERS = 33,
    COMPANY_TEXT_INFANTRY = 34,
    COMPANY_TEXT_ARCHERS = 35,
    COMPANY_TEXT_MORALE = 36,
    COMPANY_TEXT_MORALE_FIRST = 37,
};

enum military_layout {
    MILITARY_WIDTH_BLOCKS = 40,
    MILITARY_HEIGHT_BLOCKS = 26,
    MILITARY_BLOCK = 16,
    MILITARY_OPEN_STEPS = 5,
    MILITARY_ROW_HEIGHT = 44,
    MILITARY_BUTTON_SIZE = 30,
    MILITARY_BUTTON_IMAGE_INSET = 3,
    MILITARY_ROW_BUTTON_Y = 83,
    MILITARY_ROW_IMAGE_Y = 86,
    MILITARY_EXPERIENCE_X = 360,
    MILITARY_GO_TO_X = 440,
    MILITARY_RETURN_X = 500,
    MILITARY_SERVICE_X = 560,
    MILITARY_SERVICE_TEXT_X = 550,
    MILITARY_BULLET_X = 50,
    MILITARY_STATUS_X = 70,
    MILITARY_MORALE_WIDTH = 150,
    MILITARY_MORALE_PER_TEXT = 5,
    MILITARY_EXPERIENCE_PER_IMAGE = 20,
    MILITARY_NUMBER_WIDTH = 64,
};

enum military_button {
    MILITARY_ROW_BUTTONS = 3,
    MILITARY_BUTTON_CLOSE = 19,
    MILITARY_BUTTON_SEND_HELP = 20,
};

enum fort_button_image {
    FORT_BUTTON_IMAGE_GO_TO = 0,
    FORT_BUTTON_IMAGE_RETURN = 1,
    FORT_BUTTON_IMAGE_AT_HOME = 2,
    FORT_BUTTON_IMAGE_IN_SERVICE = 3,
    FORT_BUTTON_IMAGE_NOT_IN_SERVICE = 4,
    FORT_BUTTON_IMAGE_NO_SERVICE = 5,
};

enum military_button_record {
    MILITARY_CLOSE_BUTTON_RECORD = 18,
    MILITARY_SEND_HELP_BUTTON_RECORD = 19,
};

static const int16_t *military_button_rect(int32_t record)
{
    return &buttons_rows.military_advisor[record].x;
}

static scenario_event *next_request(scenario_event *previous)
{
    return scenario_request_next_active(&scenario_events, previous);
}

static int32_t legion_has_no_request(int32_t formation_id)
{
    return formation_array[formation_id].request_event_id < 0;
}

static int32_t enemy_status_text_id(void)
{
    if (city_player_current()->kingdom_soldiers != 0) {
        return MILITARY_TEXT_KINGDOM_TROOPS_AT_GATES;
    }
    if (city_player_current()->enemies != 0) {
        return MILITARY_TEXT_ENEMIES_IN_SIGHT;
    }
    if (scenario_invasion_running() != 0) {
        return MILITARY_TEXT_ENEMIES_APPROACHING;
    }
    return MILITARY_TEXT_NO_THREATS;
}

static int32_t distant_battle_text_id(
    bool returning_by_land, bool returning_by_sea, bool received_by_land, bool received_by_sea, bool has_by_land, bool has_by_sea)
{
    if (returning_by_land) {
        return MILITARY_TEXT_MARCHING_BACK;
    }
    if (returning_by_sea) {
        return MILITARY_TEXT_SAILING_BACK;
    }
    if (received_by_land) {
        return MILITARY_TEXT_MARCHING_TO_FIGHT;
    }
    if (received_by_sea) {
        return MILITARY_TEXT_SAILING_TO_FIGHT;
    }
    if (has_by_land) {
        return MILITARY_TEXT_TROOPS_NEEDED_ABROAD;
    }
    if (has_by_sea) {
        return MILITARY_TEXT_NAVY_NEEDED;
    }
    return MILITARY_TEXT_NO_REQUESTS_ABROAD;
}

void window_advisor_military_draw_background(void)
{
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = MILITARY_HEIGHT_BLOCKS;

    int32_t anim = MILITARY_OPEN_STEPS - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    if (anim >= MILITARY_OPEN_STEPS) {
        advisor_height_blocks = MILITARY_HEIGHT_BLOCKS;
        advisor_y = ui_origin_y;
        advisor_x = ui_origin_x;
        return;
    }
    if (anim > 0) {
        graphics_draw_rect((anim * MILITARY_BLOCK) + ui_origin_x, ui_origin_y + (anim * (MILITARY_BLOCK / 2)), (MILITARY_WIDTH_BLOCKS - (anim * 2)) * MILITARY_BLOCK,
            (MILITARY_HEIGHT_BLOCKS - (anim * 2)) * MILITARY_BLOCK, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, MILITARY_WIDTH_BLOCKS, MILITARY_HEIGHT_BLOCKS);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 1, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_TITLE, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_GO_TO, advisor_x + 425, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_GO_TO_COMPANY, advisor_x + 425, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_RETURN, advisor_x + 490, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_RETURN_TO_FORT, advisor_x + 490, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_COMPANY, advisor_x + 350, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_EXPERIENCE, advisor_x + 330, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_COMPANIES, COMPANY_TEXT_MORALE, advisor_x + 250, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    advisor_num_legions = formation_get_num_legions();

    bool has_by_land = false;
    bool has_by_sea = false;
    advisor_has_troop_requests(&has_by_land, &has_by_sea);

    bool returning_by_land = false;
    bool returning_by_sea = false;
    bool received_by_land = false;
    bool received_by_sea = false;
    for (scenario_event *e = next_request(0); e != 0; e = next_request(e)) {
        if (e->item.value != RESOURCE_TROOPS) {
            continue;
        }
        int32_t route = e->route.value;
        int32_t by_sea = 0;
        if (route >= 0 && route < EMPIRE_ROUTES) {
            by_sea = empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA;
        }
        int16_t state = e->state;
        if (state == REQUEST_STATE_RECEIVED) {
            if (by_sea) {
                received_by_sea = true;
            } else {
                received_by_land = true;
            }
        }
        if (state == REQUEST_STATE_TROOPS_RETURN || state == REQUEST_STATE_TROOPS_RETURN_TOO_LATE) {
            if (by_sea) {
                returning_by_sea = true;
            } else {
                returning_by_land = true;
            }
        }
    }

    int32_t enemy_id = enemy_status_text_id();
    int32_t battle_id = distant_battle_text_id(returning_by_land, returning_by_sea, received_by_land, received_by_sea, has_by_land, has_by_sea);

    if (advisor_num_legions < 1) {
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + MILITARY_BULLET_X, advisor_y + 359);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, enemy_id, advisor_x + MILITARY_STATUS_X, advisor_y + 358, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + MILITARY_BULLET_X, advisor_y + 389);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, battle_id, advisor_x + MILITARY_STATUS_X, advisor_y + 388, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    } else {
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + MILITARY_BULLET_X, advisor_y + 349);
        g_text_width = 0;
        lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_SOLDIER, city_player_current()->total_soldiers, advisor_x + MILITARY_STATUS_X, advisor_y + 348, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_IN, g_text_width + MILITARY_STATUS_X + advisor_x, advisor_y + 348, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_COMPANY, city_player_current()->total_legions, g_text_width + MILITARY_STATUS_X + advisor_x, advisor_y + 348, image_group(IMAGE_GROUP_FONT_BLACK), 0);

        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + MILITARY_BULLET_X, advisor_y + 369);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, enemy_id, advisor_x + MILITARY_STATUS_X, advisor_y + 368, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + MILITARY_BULLET_X, advisor_y + 389);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, battle_id, advisor_x + MILITARY_STATUS_X, advisor_y + 388, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    }

    inner_panel_draw_ui(advisor_x + 32, advisor_y + 70, 36, 17, ui_origin_x, ui_origin_y);

    if (advisor_num_legions < 1) {
        lang_text_draw_multiline_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_NO_COMPANIES, advisor_x + 64, advisor_y + 200, 496, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
    } else {
        int32_t row_y = 0;
        for (int32_t i = 0; i < advisor_num_legions; i++) {
            int32_t formation_id = formation_for_legion(i + 1);
            formation *m = &formation_array[formation_id];
            int32_t on_ship = formation_legion_is_on_ship(formation_id);
            int32_t standard_base = on_ship ? image_group(IMAGE_GROUP_FORT_STANDARD_B) : image_group(IMAGE_GROUP_FORT_LEGION_STANDARDS);
            uint8_t legion = m->legion_id;

            image_draw((int32_t) legion + standard_base, advisor_x + 48, row_y + 82 + advisor_y);
            lang_text_draw_ui(LANG_GROUP_COMPANIES, legion, advisor_x + 100, row_y + 83 + advisor_y, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);

            g_text_width = 0;
            text_draw_number_ui(
                m->num_figures, MILITARY_NUMBER_WIDTH, postfix_none, advisor_x + 100, row_y + 100 + advisor_y, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);

            int32_t type_text;
            int16_t ftype = m->figure_type;
            if (ftype == FIGURE_CHARIOTEER) {
                type_text = COMPANY_TEXT_CHARIOTEERS;
            } else if (ftype == FIGURE_ARCHER) {
                type_text = COMPANY_TEXT_ARCHERS;
            } else {
                type_text = COMPANY_TEXT_INFANTRY;
            }
            lang_text_draw_ui(LANG_GROUP_COMPANIES, type_text, g_text_width + 100 + advisor_x, row_y + 100 + advisor_y, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);

            lang_text_draw_centered_ui(
                LANG_GROUP_COMPANIES, (m->morale / MILITARY_MORALE_PER_TEXT) + COMPANY_TEXT_MORALE_FIRST, advisor_x + 200, row_y + 91 + advisor_y, MILITARY_MORALE_WIDTH, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);

            if (m->request_event_id < 0) {
                button_border_draw_ui(advisor_x + MILITARY_GO_TO_X, row_y + MILITARY_ROW_BUTTON_Y + advisor_y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
                image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_GO_TO, advisor_x + MILITARY_GO_TO_X + MILITARY_BUTTON_IMAGE_INSET, row_y + MILITARY_ROW_IMAGE_Y + advisor_y);

                button_border_draw_ui(advisor_x + MILITARY_RETURN_X, row_y + MILITARY_ROW_BUTTON_Y + advisor_y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
                int32_t return_img = m->is_at_fort == 0 ? image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_RETURN : image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_AT_HOME;
                image_draw(return_img, advisor_x + MILITARY_RETURN_X + MILITARY_BUTTON_IMAGE_INSET, row_y + MILITARY_ROW_IMAGE_Y + advisor_y);

                button_border_draw_ui(advisor_x + MILITARY_SERVICE_X, row_y + MILITARY_ROW_BUTTON_Y + advisor_y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
                bool has_requests = on_ship ? has_by_sea : has_by_land;
                if (has_requests == 0) {
                    m->empire_service = 0;
                    image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_NO_SERVICE, advisor_x + MILITARY_SERVICE_X + MILITARY_BUTTON_IMAGE_INSET, row_y + MILITARY_ROW_IMAGE_Y + advisor_y);
                } else if (m->empire_service == 0) {
                    image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_NOT_IN_SERVICE, advisor_x + MILITARY_SERVICE_X + MILITARY_BUTTON_IMAGE_INSET, row_y + MILITARY_ROW_IMAGE_Y + advisor_y);
                } else {
                    image_draw(image_group(IMAGE_GROUP_FORT_BUTTONS) + FORT_BUTTON_IMAGE_IN_SERVICE, advisor_x + MILITARY_SERVICE_X + MILITARY_BUTTON_IMAGE_INSET, row_y + MILITARY_ROW_IMAGE_Y + advisor_y);
                }
            } else {
                lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_ABROAD, advisor_x + MILITARY_RETURN_X, row_y + MILITARY_ROW_IMAGE_Y + advisor_y, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
            }

            button_border_draw_ui(advisor_x + MILITARY_EXPERIENCE_X, row_y + MILITARY_ROW_BUTTON_Y + advisor_y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, 0, ui_origin_x, ui_origin_y);
            image_draw(((m->experience + (MILITARY_EXPERIENCE_PER_IMAGE / 2)) / MILITARY_EXPERIENCE_PER_IMAGE) + image_group(IMAGE_GROUP_FORT_EXPERIENCE_ICONS),
                advisor_x + MILITARY_EXPERIENCE_X + MILITARY_BUTTON_IMAGE_INSET, row_y + MILITARY_ROW_IMAGE_Y + advisor_y);

            row_y += MILITARY_ROW_HEIGHT;
        }
    }

    image_draw(image_group(IMAGE_GROUP_CLOSE) + 1, advisor_x + 532, advisor_y + 347);
}

void button_fleet_send_help(void)
{
    button_military_send_help();
}

void button_military_go_to_legion(void)
{
    int32_t f = formation_for_legion(button_parameter);
    if (f == 0 || !legion_has_no_request(f)) {
        return;
    }
    city_view_go_to_grid_offset(map_info.grid_start + formation_array[f].x + formation_array[f].y * 228);
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void button_military_return_to_fort(void)
{
    int32_t f = formation_for_legion(button_parameter);
    if (f == 0 || !legion_has_no_request(f)) {
        return;
    }
    formation_legion_to_standard(f, FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_TO_LANDING);
    window_redraw_request = 1;
}

void button_military_empire_service(void)
{
    int32_t f = formation_for_legion(button_parameter);
    if (f == 0) {
        return;
    }
    bool has_by_land;
    bool has_by_sea;
    advisor_has_troop_requests(&has_by_land, &has_by_sea);
    bool on_ship = formation_legion_is_on_ship(f) != 0;
    if (!legion_has_no_request(f)) {
        return;
    }
    bool troops_needed_here = on_ship ? has_by_sea : has_by_land;
    if (troops_needed_here) {
        formation_array[f].empire_service = formation_array[f].empire_service == 0;
    } else {
        int32_t text;
        if (on_ship) {
            text = has_by_land ? POPUP_LAND_TROOPS_NEEDED : POPUP_NO_TROOPS_NEEDED;
        } else {
            text = has_by_sea ? POPUP_SEA_TROOPS_NEEDED : POPUP_NO_TROOPS_NEEDED;
        }
        window_popup_dialog_show(text, button_none, 1);
        formation_array[f].empire_service = 0;
    }
    formation_calculate_figures();
    window_redraw_request = 1;
}

void button_military_send_help(void)
{
    if (military_empire_button_shown == 0) {
        return;
    }
    int32_t first_by_land;
    int32_t first_by_sea;
    int32_t count_by_land;
    int32_t count_by_sea;
    advisor_count_troop_requests(&first_by_land, &first_by_sea, &count_by_land, &count_by_sea);
    if (city_player_current()->empire_service_legions_not_fighting == 0) {
        first_by_land = -1;
    }
    if (city_player_current()->empire_service_legions_fighting == 0 && city_player_current()->empire_service_warships == 0) {
        first_by_sea = -1;
    }
    if ((count_by_land > 1 && first_by_land >= 0) || (count_by_sea > 1 && first_by_sea >= 0)) {
        button_parameter = 4;
        window_advisors_show_checked();
        window_popup_dialog_show(POPUP_SEVERAL_TROOP_REQUESTS, button_none, 0);
        return;
    }
    int32_t event_id = -2;
    for (button_parameter = 0; button_parameter < 5; button_parameter++) {
        scenario_request_status(button_parameter, &event_id);
        if (event_id == first_by_land || event_id == first_by_sea) {
            scenario_current_request = event_id;
            scenario_request_dispatch(event_id);
        }
    }
    window_redraw_request = 1;
}

int32_t formation_get_num_legions(void)
{
    int32_t count = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        if (formation_array[id].in_use == 1 && formation_array[id].is_legion) {
            count++;
        }
    }
    return count;
}

void window_advisor_military_draw_foreground(void)
{
    if (MILITARY_OPEN_STEPS - label_animation[LABEL_SLOT_ADVISOR_OPEN] > 0) {
        return;
    }
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = MILITARY_HEIGHT_BLOCKS;
    advisor_num_legions = formation_get_num_legions();
    bool has_by_land;
    bool has_by_sea;
    advisor_has_troop_requests(&has_by_land, &has_by_sea);
    int32_t in_service = 0;
    int32_t dy = 0;
    int32_t focus_base = 2;
    for (int32_t index = 1; index <= advisor_num_legions; index++) {
        int32_t formation = formation_for_legion(index);
        if (legion_has_no_request(formation)) {
            button_border_draw_ui(advisor_x + MILITARY_GO_TO_X, dy + advisor_y + MILITARY_ROW_BUTTON_Y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, focus_button_id == focus_base - 1, ui_origin_x, ui_origin_y);
            button_border_draw_ui(advisor_x + MILITARY_RETURN_X, dy + advisor_y + MILITARY_ROW_BUTTON_Y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, focus_button_id == focus_base, ui_origin_x, ui_origin_y);
            int32_t on_ship = formation_legion_is_on_ship(formation) != 0;
            int32_t focus = 0;
            if (legion_has_no_request(formation)) {
                bool requests = on_ship ? has_by_sea : has_by_land;
                if (requests != 0) {
                    focus = focus_button_id == focus_base + 1;
                }
            }
            button_border_draw_ui(advisor_x + MILITARY_SERVICE_X, dy + advisor_y + MILITARY_ROW_BUTTON_Y, MILITARY_BUTTON_SIZE, MILITARY_BUTTON_SIZE, focus, ui_origin_x, ui_origin_y);
            if (formation_array[formation].empire_service != 0) {
                in_service = 1;
            }
        }
        focus_base += MILITARY_ROW_BUTTONS;
        dy += MILITARY_ROW_HEIGHT;
    }
    button_border_draw_rect(military_button_rect(MILITARY_CLOSE_BUTTON_RECORD), focus_button_id == MILITARY_BUTTON_CLOSE, 0, 0);
    if (in_service) {
        button_border_draw_rect(military_button_rect(MILITARY_SEND_HELP_BUTTON_RECORD), focus_button_id == MILITARY_BUTTON_SEND_HELP, 0, 0);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_DISPATCH, advisor_x + MILITARY_SERVICE_TEXT_X, advisor_y + 40, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_DISPATCH_NOW, advisor_x + MILITARY_SERVICE_TEXT_X, advisor_y + 54, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        military_empire_button_shown = 1;
        return;
    }
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_KINGDOM, advisor_x + MILITARY_SERVICE_TEXT_X, advisor_y + 43, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_MILITARY_ADVISOR, MILITARY_TEXT_KINGDOM_SERVICE, advisor_x + MILITARY_SERVICE_TEXT_X, advisor_y + 58, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    military_empire_button_shown = 0;
}

void advisor_military_count_ships(void)
{
    int32_t player = city_current_player;
    city_player_current()->empire_service_warships = 0;
    city_player_current()->total_warships = 0;
    city_player_current()->total_transport_ships = 0;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = &city_figures.records[id];
        if (f->state == 0 || f->faction_id != player) {
            continue;
        }
        if (f->type == FIGURE_WARSHIP) {
            city_player_current()->total_warships++;
            if (f->answers_requests != 0) {
                city_player_current()->empire_service_warships++;
            }
        } else if (f->type == FIGURE_TRANSPORT_SHIP) {
            city_player_current()->total_transport_ships++;
        }
    }
}
