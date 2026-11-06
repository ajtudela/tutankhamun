// The religion advisor: its background (the gods, their temples and shrines, the moods), its foreground, the festival
// panel (the next festival or the hold festival button) and the festival dialog (background, foreground and buttons: a
// god, a size with its cost, help, close, confirm).

#include "window/advisor/religion.h"
#include "building/building_storage.h"
#include "city/culture.h"
#include "city/gods.h"
#include "city/resource_type.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/button_border.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/label_animation_slots.h"
#include "graphics/text_draw.h"
#include "window/advisors.h"
#include "window/advisors_internal.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"


enum festival_rules {
    FESTIVAL_TREASURY_LIMIT = -5000,
};

void button_hold_festival(void)
{
    if (city_player_current()->festival_planned_size != 0) {
        return;
    }
    if (city_player_current()->festival_square_placed == 0) {
        window_popup_dialog_show(POPUP_NO_FESTIVAL_SQUARE, button_none, 0);
        return;
    }
    if (city_player_current()->treasury <= FESTIVAL_TREASURY_LIMIT) {
        window_popup_dialog_show(POPUP_CITY_IN_DEBT, button_none, 0);
        return;
    }
    window_redraw_request = 1;
    window_current_id = (button_parameter != 0) ? WINDOW_HOLD_FESTIVAL : WINDOW_FESTIVAL_DIALOG;
}

void button_festival_help(void)
{
    window_message_dialog_show(MESSAGE_DIALOG_FESTIVAL_HELP, 1, 0);
    window_redraw_request = 1;
}

void button_festival_close(void)
{
    window_redraw_request = 1;
    window_current_id = (button_parameter2 != 0) ? WINDOW_BUILDING_INFO : WINDOW_ADVISORS;
}

void button_festival_confirm(void)
{
    int32_t player = city_current_player;
    if (city_player_current()->treasury <= FESTIVAL_TREASURY_LIMIT) {
        return;
    }
    city_player_current()->festival_planned_god = city_player_current()->festival_selected_god;
    city_player_current()->festival_planned_size = city_player_current()->festival_selected_size;
    if (city_player_current()->festival_selected_size == 1) {
        city_player_current()->festival_planned_months_to_go = 2;
    } else if (city_player_current()->festival_selected_size == 2) {
        city_player_current()->festival_planned_months_to_go = 3;
    } else {
        city_player_current()->festival_planned_months_to_go = 4;
    }
    if (city_player_current()->first_festival_effect_months != 0) {
        if (city_player_current()->second_festival_effect_months != 0) {
            city_player_current()->festival_planned_months_to_go += city_player_current()->first_festival_effect_months;
        } else {
            city_player_current()->second_festival_effect_months = 12;
        }
    } else {
        city_player_current()->first_festival_effect_months = 12;
    }
    int32_t cost;
    if (city_player_current()->festival_selected_size == 1) {
        city_player_current()->treasury = city_player_current()->treasury - city_player_current()->festival_cost[0];
        cost = city_player_current()->festival_cost[0];
    } else if (city_player_current()->festival_selected_size == 2) {
        city_player_current()->treasury = city_player_current()->treasury - city_player_current()->festival_cost[1];
        cost = city_player_current()->festival_cost[1];
    } else {
        city_player_current()->treasury = city_player_current()->treasury - city_player_current()->festival_cost[2];
        cost = city_player_current()->festival_cost[2];
    }
    city_player_current()->requests_and_festivals_this_year += cost;
    if (city_player_current()->festival_selected_size == 3) {
        building_storageyards_remove_resource(player, RESOURCE_BEER, city_player_current()->festival_grand_beer);
    }
    window_current_id = WINDOW_ADVISORS;
    if (button_parameter2 != 0) {
        window_current_id = WINDOW_BUILDING_INFO;
    }
    window_redraw_request = 1;
}

void button_festival_god(void)
{
    int32_t god = button_parameter;
    if (scenario_data.god_status[god] != 0) {
        city_player_current()->festival_selected_god = god;
    }
    window_redraw_request = 1;
}

void button_festival_size(void)
{
    if (city_player_current()->treasury <= FESTIVAL_TREASURY_LIMIT) {
        return;
    }
    int32_t size = button_parameter;
    if (city_player_current()->festival_not_enough_beer != 0 && size == 3) {
        return;
    }
    if (city_player_current()->festival_square_x == -1 || city_player_current()->festival_square_y == -1) {
        return;
    }
    city_player_current()->festival_selected_size = size;
    window_redraw_request = 1;
}

void window_advisor_religion_draw_foreground(void)
{
    if (label_animation[LABEL_SLOT_ADVISOR_OPEN] > 5 && city_player_current()->festival_planned_size == 0) {
        button_border_draw_ui(advisor_x + 102, advisor_y + 348, 300, 20, city_player_current()->festival_square_placed != 0 && focus_button_id == 1,
            ui_origin_x, ui_origin_y);
    }
}

void window_advisor_religion_draw_festival(void)
{
    inner_panel_draw_ui(advisor_x + 48, advisor_y + 320, 34, 6, ui_origin_x, ui_origin_y);
    image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + 15, advisor_x + 460, advisor_y + 323);
    g_text_width = 0;
    lang_text_draw_amount(8, 4, city_player_current()->months_since_festival, advisor_x + 112, advisor_y + 328, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    lang_text_draw_ui(58, 15, g_text_width + advisor_x + 112, advisor_y + 328, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    int32_t text;
    int32_t size = city_player_current()->festival_planned_size;
    if (size == 0) {
        if (city_player_current()->first_festival_effect_months == 0 || city_player_current()->second_festival_effect_months == 0) {
            lang_text_draw_centered_ui(58, 16, advisor_x + 102, advisor_y + 352, 300, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            int32_t months = city_player_current()->months_since_festival;
            if (months < 2) {
                text = 0;
            } else if (months < 7) {
                text = 1;
            } else if (months < 13) {
                text = 2;
            } else if (months < 19) {
                text = 3;
            } else if (months < 25) {
                text = 4;
            } else {
                text = (months > 30) + 5;
            }
        } else {
            lang_text_draw_centered_ui(58, 52, advisor_x + 102, advisor_y + 352, 300, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            if (city_player_current()->months_since_festival < 2) {
                text = 7;
            } else {
                text = (city_player_current()->months_since_festival > 6) + 8;
            }
        }
    } else {
        int32_t threshold;
        int32_t base;
        if (size == 1) {
            threshold = 2;
            base = 10;
        } else if (size == 2) {
            threshold = 3;
            base = 20;
        } else {
            threshold = 4;
            base = 31;
        }
        g_text_width = 0;
        lang_text_draw_ui(58, threshold < city_player_current()->festival_planned_months_to_go ? 53 : 34, advisor_x + 102, advisor_y + 352, image_group(IMAGE_GROUP_FONT_WHITE),
            0, ui_origin_x, ui_origin_y);
        int32_t months = city_player_current()->festival_planned_months_to_go;
        text = base - 1 + months;
        if (months == 1) {
            lang_text_draw_ui(58, 54, g_text_width + advisor_x + 102, advisor_y + 352, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        } else {
            lang_text_draw_ui(
                160, (months + game_time_month) % 12, g_text_width + advisor_x + 102, advisor_y + 352, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
    }
    lang_text_draw_multiline_ui(295, text, advisor_x + 56, advisor_y + 373, 380, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    window_advisor_religion_draw_foreground();
}

static void text_draw_number_centered_at(int32_t value, int32_t x, int32_t dy)
{
    text_draw_number_centered(value, NUMBER_PREFIX_NONE, postfix_none, x, dy + advisor_y + 66, 50, image_group(IMAGE_GROUP_FONT_WHITE), 0);
}

void window_advisor_religion_draw_background(void)
{
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 27;
    int32_t anim = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    if (anim > 4) {
        return;
    }
    if (anim > 0) {
        graphics_draw_rect(anim * 16 + ui_origin_x, ui_origin_y + anim * 8, (anim * -2 + 40) * 16, (anim * -2 + 27) * 16, 0);
        return;
    }
    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 27);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 9, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(59, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(59, 5, advisor_x + 180, advisor_y + 32, 65, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(28, 150, advisor_x + 325, advisor_y + 46, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(59, 1, advisor_x + 255, advisor_y + 46, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(59, 2, advisor_x + 180, advisor_y + 46, 65, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(59, 3, advisor_x + 490, advisor_y + 46, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_centered_ui(59, 6, advisor_x + 390, advisor_y + 18, 65, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(59, 8, advisor_x + 390, advisor_y + 32, 65, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(59, 7, advisor_x + 390, advisor_y + 46, 65, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 60, 36, 13, ui_origin_x, ui_origin_y);
    building_info_details_shown = 0;
    int32_t dy = 0;
    for (int32_t god = 0; god < 5; god++, dy += 40) {
        lang_text_draw_rect_ui(157, god, &row_nile2.religion_gods[god].x, image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
        if (building_info_details_shown != 0) {
            text_draw_number_centered_at(city_player_current()->god_wrath_bolts[god], advisor_x + 210, dy);
            text_draw_number_centered_at(city_player_current()->god_happy_bolts[god], advisor_x + 245, dy);
            text_draw_number_centered_at(city_player_current()->god_target_mood[god], advisor_x + 310, dy);
            text_draw_number_centered_at(city_player_current()->god_mood[god], advisor_x + 360, dy);
            continue;
        }
        lang_text_draw_ui(187, scenario_data.god_status[god], advisor_x + 100, dy + advisor_y + 67, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_ui(158, god, advisor_x + 40, dy + advisor_y + 87, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
        if (scenario_data.god_status[god] == 0) {
            static const int32_t none_x[] = { 260, 330, 390, 460 };
            for (int32_t k = 0; k < 4; k++) {
                lang_text_draw_centered_ui(59, 37, advisor_x + none_x[k], dy + advisor_y + 66, 50, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            }
        } else {
            text_draw_number_centered_at(building_count_total[building_type_for_god(god, 0)], advisor_x + 260, dy);
            text_draw_number_centered_at(building_count_total[building_type_for_god(god, 2)], advisor_x + 330, dy);
            text_draw_number_centered_at(city_player_current()->god_months_since_festival[god], advisor_x + 390, dy);
            g_text_width = 0;
            lang_text_draw_ui(
                59, city_player_current()->god_mood[god] / 10 + 20, advisor_x + 450, dy + advisor_y + 66, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
            for (int32_t i = 0, x = 450; i < city_player_current()->god_wrath_bolts[god] / 10; i++, x += 10) {
                image_draw(image_group(IMAGE_GROUP_GOD_ANGER_BOLT), g_text_width + advisor_x + x, dy + advisor_y + 62);
            }
            for (int32_t i = 0, x = 450; i < city_player_current()->god_happy_bolts[god] / 10; i++, x += 10) {
                image_draw(image_group(IMAGE_GROUP_GOD_HAPPY_BOLT), g_text_width + advisor_x + x, dy + advisor_y + 62);
            }
        }
        if (scenario_data.temple_complexes_allowed[god] == 1) {
            text_draw_number_centered_at(building_count_total[building_type_for_god(god, 1)], advisor_x + 200, dy);
        } else {
            lang_text_draw_centered_ui(59, 37, advisor_x + 200, dy + advisor_y + 66, 50, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        }
    }
    city_gods_find_least_happy();
    int32_t least = city_player_current()->least_happy_god;
    int32_t text;
    if (least > 0 && city_player_current()->god_wrath_bolts[least - 1] > 4) {
        text = least + 14;
    } else if (city_player_current()->religion_demand == 1) {
        text = (city_player_current()->houses_requiring_religion != 0) + 9;
    } else if (city_player_current()->religion_demand == 2) {
        text = 11;
    } else if (city_player_current()->religion_demand == 3) {
        text = 12;
    } else if (city_player_current()->houses_requiring_religion == 0) {
        text = 13;
    } else {
        text = least + 14;
    }
    lang_text_draw_multiline_ui(59, text, advisor_x + 60, advisor_y + 272, 512, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    window_advisor_religion_draw_festival();
}

void window_festival_dialog_draw_foreground(void)
{
    button_border_draw_ui(advisor_x + 102, advisor_y + 216, 430, 26, focus_button_id == 6, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(58, 31, advisor_x + 110, advisor_y + 224, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_amount(8, 0, city_player_current()->festival_cost[0], g_text_width + advisor_x + 110, advisor_y + 224, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw_ui(advisor_x + 102, advisor_y + 246, 430, 26, focus_button_id == 7, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(58, 32, advisor_x + 110, advisor_y + 254, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_amount(8, 0, city_player_current()->festival_cost[1], g_text_width + advisor_x + 110, advisor_y + 254, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw_ui(advisor_x + 102, advisor_y + 276, 430, 26, focus_button_id == 8, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(58, 33, advisor_x + 110, advisor_y + 284, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    int32_t width = g_text_width;
    lang_text_draw_amount(8, 0, city_player_current()->festival_cost[2], g_text_width + advisor_x + 110, advisor_y + 284, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    g_text_width += width;
    text_draw_number_ui(city_player_current()->festival_grand_beer, NUMBER_PREFIX_NONE, postfix_none, g_text_width + advisor_x + 120, advisor_y + 284, image_group(IMAGE_GROUP_FONT_BLACK), 0,
        ui_origin_x, ui_origin_y);
    lang_text_draw_ui(23, 15, g_text_width + advisor_x + 120, advisor_y + 284, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    if (city_player_current()->treasury <= FESTIVAL_TREASURY_LIMIT) {
        graphics_grey_rect(advisor_x + 104, advisor_y + 218, 426, 22, 0);
        graphics_grey_rect(advisor_x + 104, advisor_y + 248, 426, 22, 0);
        graphics_grey_rect(advisor_x + 104, advisor_y + 278, 426, 22, 0);
        return;
    }
    if (city_player_current()->festival_not_enough_beer != 0) {
        graphics_grey_rect(advisor_x + 104, advisor_y + 278, 426, 22, 0);
    }
}

void window_festival_dialog_draw_background(void)
{
    outer_panel_draw(advisor_x + 48, advisor_y + 48, 34, 20);
    int32_t dx = 0;
    for (int32_t god = 0; god < 5; god++, dx += 100) {
        if (scenario_data.god_status[god] == 0) {
            if (god == city_player_current()->festival_selected_god) {
                city_player_current()->festival_selected_god = city_player_current()->festival_selected_god + 1;
            }
            image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + 16 + god, dx + advisor_x + 70, advisor_y + 96);
            graphics_grey_rect(dx + advisor_x + 75, advisor_y + 101, 72, 82, 0);
        } else if (god == city_player_current()->festival_selected_god) {
            button_border_draw_ui(dx + advisor_x + 66, advisor_y + 92, 90, 100, 1, ui_origin_x, ui_origin_y);
            image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + 21 + god, dx + advisor_x + 70, advisor_y + 96);
        } else {
            image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + 16 + god, dx + advisor_x + 70, advisor_y + 96);
        }
    }
    if (city_player_current()->festival_selected_god < 5) {
        lang_text_draw_centered_ui(
            58, city_player_current()->festival_selected_god + 25, advisor_x + 48, advisor_y + 60, 544, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    }
    window_festival_dialog_draw_foreground();
    g_text_width = 0;
    lang_text_draw_ui(58, city_player_current()->festival_selected_size + 30, advisor_x + 180, advisor_y + 322, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}
