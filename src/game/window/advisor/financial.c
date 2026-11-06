// The financial advisor: its background (the tax overview, the columns of last and this year) and the tax rate
// arrows, which the palace and tax collector windows share (0..25, the finance estimates computed again).

#include "window/advisor/financial.h"
#include "city/city_internal.h"
#include "city/finance.h"
#include "city/finance_estimates.h"
#include "game/game_internal.h"
#include "graphics/graphics_present.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "window/advisor/population.h"
#include "window/window_image_group.h"
#include <stdint.h>

static int32_t interest_rate_for_display(void)
{
    int32_t rate = scenario_data.interest_rate;
    if (city_player_current()->has_temple_complex != 0) {
        if (building_temple_god(city_player_current()->temple_complex_building_id) == 1) {
            int32_t reduced = rate - 5;
            if (reduced < 0) {
                rate = 0;
            } else {
                rate = reduced;
            }
        }
    }
    return rate;
}

static void draw_finance_row(int32_t text_id, int32_t y, int32_t last_year, int32_t this_year, int32_t ox, int32_t oy)
{
    lang_text_draw_ui(60, text_id, ox + 80, oy + y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(last_year, NUMBER_PREFIX_NONE, postfix_none, ox + 290, oy + y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(this_year, NUMBER_PREFIX_NONE, postfix_none, ox + 430, oy + y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_advisor_financial_draw_background(void)
{
    int32_t rate = interest_rate_for_display();
    int32_t remain = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    int32_t ox;
    int32_t oy;
    int32_t treasury;

    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 27;
    if (remain >= 5) {
        return;
    }
    if (remain > 0) {
        graphics_draw_rect(remain * 16 + ui_origin_x, ui_origin_y + remain * 8, (remain * -2 + 40) * 16, (remain * -2 + 27) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 27);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 10, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(60, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 64, advisor_y + 48, 34, 4, ui_origin_x, ui_origin_y);
    panel_draw_bar(advisor_x + 64, advisor_y + 104, 34);

    ox = advisor_x;
    oy = advisor_y;
    g_text_width = 0;
    treasury = city_player_current()->treasury;
    if (treasury < 0) {
        lang_text_draw_ui(60, 3, ox + 70, oy + 53, image_group(IMAGE_GROUP_FONT_RED), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_amount(8, 0, -treasury, g_text_width + 72 + ox, oy + 53, image_group(IMAGE_GROUP_FONT_RED), 0);
    } else {
        lang_text_draw_ui(60, 2, ox + 70, oy + 53, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
        lang_text_draw_amount(8, 0, treasury, g_text_width + 72 + ox, oy + 53, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    }

    lang_text_draw_ui(60, 1, ox + 70, oy + 76, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    text_draw_number_ui(city_player_current()->tax_percentage, NUMBER_PREFIX_NONE, pct_postfix, ox + 240, oy + 76, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(60, 4, g_text_width + 240 + ox, oy + 76, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_amount(8, 0, city_player_current()->estimated_tax_income, g_text_width + 240 + ox, oy + 76, image_group(IMAGE_GROUP_FONT_WHITE), 0);

    g_text_width = 0;
    text_draw_number_ui(city_player_current()->percentage_taxed_people, NUMBER_PREFIX_NONE, pct_postfix, ox + 70, oy + 98, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(60, 5, g_text_width + 70 + ox, oy + 98, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    text_draw_number_ui(city_player_current()->yearly_uncollected_plebs + city_player_current()->yearly_uncollected_nobles, NUMBER_PREFIX_NONE, db_format, ox + 365,
        oy + 98, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(60, 23, g_text_width + 365 + ox, oy + 98, image_group(IMAGE_GROUP_FONT_WHITE), 0, ui_origin_x, ui_origin_y);

    lang_text_draw_ui(60, 6, ox + 270, oy + 125, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(60, 7, ox + 400, oy + 125, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    draw_finance_row(8, 140, city_player_current()->taxes_last_year, city_player_current()->taxes_this_year, ox, oy);
    draw_finance_row(9, 155, city_player_current()->exports_last_year, city_player_current()->exports_this_year, ox, oy);
    draw_finance_row(20, 170, city_player_current()->donated_last_year, city_player_current()->donated_this_year, ox, oy);
    draw_finance_row(24, 185, city_player_current()->gold_last_year, city_player_current()->gold_this_year, ox, oy);

    graphics_draw_line(ox + 280, oy + 198, ox + 350, oy + 198, 0);
    graphics_draw_line(ox + 420, oy + 198, ox + 490, oy + 198, 0);

    draw_finance_row(10, 203, city_player_current()->income_last_year, city_player_current()->income_this_year, ox, oy);
    draw_finance_row(11, 227, city_player_current()->imports_last_year, city_player_current()->imports_this_year, ox, oy);
    draw_finance_row(12, 242, city_player_current()->wages_last_year, city_player_current()->wages_this_year, ox, oy);
    draw_finance_row(13, 257, city_player_current()->construction_last_year, city_player_current()->construction_this_year, ox, oy);

    g_text_width = 0;
    lang_text_draw_ui(60, 14, ox + 80, oy + 272, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(rate, NUMBER_PREFIX_NONE, pct_postfix, g_text_width + 80 + ox, oy + 272, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(city_player_current()->interest_last_year, NUMBER_PREFIX_NONE, postfix_none, ox + 290, oy + 272, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    text_draw_number_ui(city_player_current()->interest_this_year, NUMBER_PREFIX_NONE, postfix_none, ox + 430, oy + 272, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);

    draw_finance_row(15, 287, city_player_current()->salary_last_year, city_player_current()->salary_this_year, ox, oy);
    draw_finance_row(16, 302, city_player_current()->stolen_last_year, city_player_current()->stolen_this_year, ox, oy);
    draw_finance_row(21, 317, city_player_current()->tribute_paid, city_player_current()->tribute_estimate, ox, oy);
    draw_finance_row(22, 332, city_player_current()->requests_and_festivals_last_year, city_player_current()->requests_and_festivals_this_year, ox, oy);

    graphics_draw_line(ox + 280, oy + 345, ox + 350, oy + 345, 0);
    graphics_draw_line(ox + 420, oy + 345, ox + 490, oy + 345, 0);

    draw_finance_row(17, 350, city_player_current()->expenses_last_year, city_player_current()->expenses_this_year, ox, oy);
    draw_finance_row(18, 373, city_player_current()->net_in_out_last_year, city_player_current()->net_in_out_this_year, ox, oy);
    draw_finance_row(19, 396, city_player_current()->balance_last_year, city_player_current()->balance_this_year, ox, oy);
}

void advisor_finance_tax_arrow(void)
{
    if (button_parameter == 1) {
        city_player_current()->tax_percentage = city_player_current()->tax_percentage - 1;
    } else if (button_parameter == 0) {
        city_player_current()->tax_percentage = city_player_current()->tax_percentage + 1;
    }
    if (city_player_current()->tax_percentage < 0) {
        city_player_current()->tax_percentage = 0;
    } else if (city_player_current()->tax_percentage > 25) {
        city_player_current()->tax_percentage = 25;
    }
    city_finance_estimate_taxes();
    city_finance_calculate_totals();
    city_finance_calculate_tribute();
    window_redraw_request = 1;
}
