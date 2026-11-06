// The labor advisor: its background, its rows (a button per labor category with its priority, name, workers and
// needed workers; also the background of the priority dialog), the wages arrows (0..100, the finance estimates
// computed again) and the priority dialog (window 0x2A: the boxes 1..9 and "no priority").

#include "window/advisor/labor.h"
#include "building/temple_complex.h"
#include "city/city_internal.h"
#include "city/finance.h"
#include "city/finance_estimates.h"
#include "city/labor.h"
#include "figure/action.h"
#include "game/game_internal.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "platform/log.h"
#include "window/advisors.h"
#include "window/advisors_internal.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t labor_priority_order[9];

void window_advisor_labor_draw_background(void)
{
    int32_t labor_bonus = city_temple_complex_wage_bonus();
    int32_t remain = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    int32_t ox;
    int32_t oy;

    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 26;
    if (remain >= 5) {
        return;
    }
    if (remain > 0) {
        graphics_draw_rect(remain * 16 + ui_origin_x, ui_origin_y + remain * 8, (remain * -2 + 40) * 16, (remain * -2 + 26) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 26);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS), advisor_x + 10, advisor_y + 10);

    ox = advisor_x;
    oy = advisor_y;

    lang_text_draw_ui(50, 0, ox + 60, oy + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 21, ox + 60, oy + 56, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 22, ox + 170, oy + 56, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 23, ox + 400, oy + 56, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 24, ox + 500, oy + 56, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    g_text_width = 0;
    text_draw_number_ui(city_player_current()->workers_employed, NUMBER_PREFIX_NONE, postfix_none, ox + 32, oy + 320, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 12, g_text_width + 32 + ox, oy + 320, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(
        city_player_current()->workers_unemployed, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 50 + ox, oy + 320, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 13, g_text_width + 50 + ox, oy + 320, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(city_player_current()->unemployment_percentage, NUMBER_PREFIX_NONE, unemployment_format, g_text_width + 50 + ox, oy + 320, image_group(IMAGE_GROUP_FONT_BLACK), 0,
        ui_origin_x, ui_origin_y);

    inner_panel_draw_ui(ox + 64, oy + 350, 32, 2, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    lang_text_draw_ui(50, 14, ox + 70, oy + 359, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    text_draw_number_ui(city_player_current()->wages, NUMBER_PREFIX_NONE, postfix_none, ox + 230, oy + 359, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 15, g_text_width + 230 + ox, oy + 359, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(50, 18, g_text_width + 230 + ox, oy + 359, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(city_player_current()->wages_kingdom + labor_bonus, NUMBER_PREFIX_NONE, wages_rome_format, g_text_width + 230 + ox, oy + 359, image_group(IMAGE_GROUP_FONT_WHITE), 0,
        ui_origin_x, ui_origin_y);

    g_text_width = 0;
    lang_text_draw_ui(50, 19, ox + 64, oy + 390, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(
        city_player_current()->estimated_wages, NUMBER_PREFIX_NONE, money_format, g_text_width + 64 + ox, oy + 390, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void advisor_labor_wages_arrow(void)
{
    if (button_parameter == 1) {
        city_player_current()->wages = city_player_current()->wages - 1;
    } else if (button_parameter == 0) {
        city_player_current()->wages = city_player_current()->wages + 1;
    }
    if (city_player_current()->wages < 0) {
        city_player_current()->wages = 0;
    } else if (city_player_current()->wages > 100) {
        city_player_current()->wages = 100;
    }
    city_finance_estimate_wages();
    city_finance_calculate_totals();
    city_finance_calculate_tribute();
    window_redraw_request = 1;
}

void button_labor_priority_open(void)
{
    int32_t category = button_parameter;
    labor_focus = category;
    advisor_labor_priority_max = 0;
    int32_t count = 0;
    for (int32_t i = 0; i < 9; i++) {
        if (city_player_current()->labor_categories[i].priority > 0) {
            count++;
        }
    }
    advisor_labor_priority_max = count;
    if (count < 9) {
        count++;
        advisor_labor_priority_max = count;
        if (city_player_current()->labor_categories[category].priority != 0) {
            count--;
            advisor_labor_priority_max = count;
        }
    }
    window_current_id = WINDOW_LABOR_PRIORITY;
    window_redraw_request = 1;
}

void button_labor_priority_set(void)
{
    int32_t category = labor_focus;
    int32_t new_priority = button_parameter;
    window_current_id = WINDOW_ADVISORS;
    window_redraw_request = 1;
    int32_t old_priority = city_player_current()->labor_categories[category].priority;
    if (old_priority == new_priority) {
        return;
    }
    int32_t direction;
    if (old_priority == 0 && new_priority != 0) {
        direction = 1;
    } else if (old_priority != 0 && new_priority == 0) {
        direction = -1;
    } else {
        direction = old_priority >= new_priority ? 1 : -1;
    }
    city_player_current()->labor_categories[category].priority = new_priority;
    for (int32_t i = 0; i < 9; i++) {
        labor_priority_order[i] = i;
    }
    for (int32_t pass = 9; pass > 0; pass--) {
        for (int32_t i = 1; i < 9; i++) {
            int32_t current = labor_priority_order[i];
            int32_t previous = labor_priority_order[i - 1];
            int32_t current_priority = city_player_current()->labor_categories[current].priority;
            if (current_priority == 0) {
                continue;
            }
            int32_t swap = 0;
            int32_t previous_priority = city_player_current()->labor_categories[previous].priority;
            if (previous_priority == 0) {
                swap = 1;
            }
            if (previous_priority > current_priority) {
                swap = 1;
            }
            if (previous_priority == current_priority) {
                if (labor_focus == current && direction == 1) {
                    swap = 1;
                } else if (labor_focus == previous && direction == -1) {
                    swap = 1;
                }
            }
            if (swap) {
                labor_priority_order[i - 1] = current;
                labor_priority_order[i] = previous;
            }
        }
    }
    for (int32_t i = 0; i < 9; i++) {
        int32_t c = labor_priority_order[i];
        if (city_player_current()->labor_categories[c].priority != 0) {
            city_player_current()->labor_categories[c].priority = i + 1;
        }
    }
    city_labor_allocate_workers_to_categories();
    city_labor_set_building_worker_weight();
    city_labor_allocate_workers_to_buildings();
}

void window_advisor_labor_draw_foreground(void)
{
    if (label_animation[LABEL_SLOT_ADVISOR_OPEN] <= 5) {
        return;
    }
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 70, 36, 15, ui_origin_x, ui_origin_y);
    int32_t i = 0;
    for (int32_t dy = 0; dy < 225; dy += 25) {
        int32_t focus = window_current_id == WINDOW_CITY ? focus_button_id - 1 : labor_focus;
        button_border_draw_ui(advisor_x + 40, dy + advisor_y + 77, 560, 22, i == focus, ui_origin_x, ui_origin_y);
        if (city_player_current()->labor_categories[i].priority != 0) {
            image_draw(image_group(IMAGE_GROUP_LABOR_PRIORITY), advisor_x + 70, dy + advisor_y + 80);
            text_draw_number_ui(city_player_current()->labor_categories[i].priority, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 90, dy + advisor_y + 82, image_group(IMAGE_GROUP_FONT_WHITE), 0,
                ui_origin_x, ui_origin_y);
        }
        i++;
        g_text_width = 0;
        lang_text_draw_ui(50, i, advisor_x + 170, dy + advisor_y + 82, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        text_draw_number_ui(city_player_current()->labor_categories[i - 1].workers_needed, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 410, dy + advisor_y + 82,
            image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        int32_t font = image_group(IMAGE_GROUP_FONT_WHITE);
        if (city_player_current()->labor_categories[i - 1].workers_allocated != city_player_current()->labor_categories[i - 1].workers_needed) {
            font = image_group(IMAGE_GROUP_FONT_RED);
        }
        text_draw_number_ui(city_player_current()->labor_categories[i - 1].workers_allocated, NUMBER_PREFIX_NONE, postfix_none, advisor_x + 510, dy + advisor_y + 82,
            font, 0, ui_origin_x, ui_origin_y);
    }
}

void window_labor_priority_draw_background(void)
{
    outer_panel_draw(advisor_x + 160, advisor_y + 176, 20, 9);
    lang_text_draw_centered_ui(50, 25, advisor_x + 160, advisor_y + 185, 320, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    int32_t i = 0;
    for (int32_t dx = 0; dx < 288; dx += 32, i++) {
        graphics_draw_rect(dx + advisor_x + 178, advisor_y + 221, 27, 27, 0);
        lang_text_draw_centered_ui(50, i + 27, dx + advisor_x + 178, advisor_y + 224, 27, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
        if (i >= advisor_labor_priority_max) {
            graphics_grey_rect(dx + advisor_x + 179, advisor_y + 222, 25, 25, 1);
        }
    }
    graphics_draw_rect(advisor_x + 180, advisor_y + 256, 280, 25, 0);
    lang_text_draw_centered_ui(50, 26, advisor_x + 180, advisor_y + 263, 280, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(13, 3, advisor_x + 160, advisor_y + 296, 320, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_labor_priority_draw_foreground(void)
{
    int32_t i = 0;
    for (int32_t dx = 0; dx < 288; dx += 32, i++) {
        graphics_draw_rect(dx + advisor_x + 178, advisor_y + 221, 27, 27, i == focus_button_id - 2 ? 0xF800 : 0);
    }
    for (int32_t k = i; k < 10; k++) {
        graphics_draw_rect(advisor_x + 180, advisor_y + 256, 280, 25, focus_button_id == 1 ? 0xF800 : 0);
    }
}
