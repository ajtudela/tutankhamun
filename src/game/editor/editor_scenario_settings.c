// The scenario editor's scenario dialogs: the start year, the flood settings, the scenario settings and the trade
// amounts of an empire city.

#include "editor/editor_scenario_settings.h"
#include "editor/editor_data.h"
#include "editor/editor_dialogs.h"
#include "editor/editor_text.h"
#include "empire/empire_helpers.h"
#include "graphics/button.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "map/map_data.h"
#include "text/lang.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

static int32_t *milestone_year(int32_t milestone)
{
    return &(&scenario_data.milestone25_year)[milestone];
}

static int32_t trade_amounts_resource(int32_t row)
{
    return select_list_buttons.trade_amount_resources[row];
}

void window_editor_start_year_draw(void)
{
    int32_t x = ui_origin_x + 128;
    int32_t y = ui_origin_y + 44;
    outer_panel_draw(x, y, 20, 10);
    g_text_width = 0;
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_START_DATE, x + 10, y + 12, 320, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, x, y + 134, 320, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(x + 30, y + 56, 100, 30, focus_button_id == START_YEAR_BUTTON_ERA, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_ERAS, (scenario_data.start.year >= 0) ? ERA_AD : ERA_BC, x + 40, y + 60, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(x + 150, y + 56, 120, 30, focus_button_id == START_YEAR_BUTTON_YEAR, ui_origin_x, ui_origin_y);
    int16_t year = scenario_data.start.year;
    if (year < 0) {
        text_draw_number_centered(-(int32_t) year, NUMBER_PREFIX_NONE, postfix_none, x + 150, y + 60, 120, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    } else {
        text_draw_number_centered(year, NUMBER_PREFIX_NONE, postfix_none, x + 150, y + 60, 120, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    }
}

void editor_start_year_button(void)
{
    numeric_input_show(240, 180, 2, 0, &scenario_data.start.year, 0, 0);
    window_redraw_request = 1;
}

void editor_start_year_toggle_era(void)
{
    window_redraw_request = 1;
    scenario_data.start.year = (int16_t) -scenario_data.start.year;
}

void window_editor_flood_draw(void)
{
    int32_t x = ui_origin_x;
    int32_t y = ui_origin_y + 80;
    outer_panel_draw(x, y, 30, 18);
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_FLOOD_PLAIN_SETTINGS, x, y + 20, 480, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, x, y + 232, 480, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    const generic_button *button = buttons_figure_phrase.editor_flood;
    for (int32_t i = 0; i < FLOOD_SETTINGS_ROWS; i++, button++) {
        int32_t row_y = button->y + y + 8;
        int32_t index;
        if (i == 0) {
            index = (floods_data.settings.season_initial / FLOOD_SEASON_STEP_DAYS) - FLOOD_SEASON_FIRST_STEP;
            lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_FLOODING_STARTS_ON, x + 60, row_y, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
            index += EDITOR_TEXT_EARLY_JUNE;
        } else if (i == 1) {
            index = (floods_data.settings.duration_initial / FLOOD_DURATION_STEP_DAYS) - FLOOD_DURATION_FIRST_MONTHS;
            lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_FLOODING_LASTS_FOR, x + 60, row_y, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
            index += EDITOR_TEXT_TWO_MONTHS;
        } else {
            index = floods_data.settings.quality_initial / FLOOD_QUALITY_STEP;
            lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_FLOODING_QUALITY, x + 60, row_y, image_group(IMAGE_GROUP_FONT_GREEN), 0, ui_origin_x, ui_origin_y);
            index += EDITOR_TEXT_FLOOD_QUALITY_NONE;
        }
        lang_text_draw_centered_in_rect(LANG_GROUP_EDITOR, index, button, image_group(IMAGE_GROUP_FONT_BLACK), 0, 0, 88, ui_origin_x, ui_origin_y);
        button_border_draw_rect(&button->x, focus_button_id == (i + 1), 0, 80);
    }
}

void editor_flood_season_next(void)
{
    int32_t step = (floods_data.settings.season_initial / FLOOD_SEASON_STEP_DAYS) - (FLOOD_SEASON_FIRST_STEP - 1);
    if (step < 0 || step > FLOOD_SEASON_LAST_STEP) {
        step = 0;
    }
    floods_data.settings.season_initial = (step * 3 + 30) * 5;
}

void editor_flood_duration_next(void)
{
    int32_t step = (floods_data.settings.duration_initial / FLOOD_DURATION_STEP_DAYS) + 1;
    if (step > FLOOD_DURATION_LAST_MONTHS) {
        step = FLOOD_DURATION_FIRST_MONTHS;
    }
    floods_data.settings.duration_initial = step * FLOOD_DURATION_STEP_DAYS;
}

void editor_flood_quality_next(void)
{
    int32_t value = floods_data.settings.quality_initial + FLOOD_QUALITY_STEP;
    if (value > FLOOD_QUALITY_PERFECT) {
        value = 0;
    }
    floods_data.settings.quality_initial = value;
}

static void draw_settings_row(int32_t label, int32_t y, int32_t focus)
{
    lang_text_draw_ui(LANG_GROUP_EDITOR, label, ui_origin_x + 32, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(ui_origin_x + 262, y, 200, 30, focus_button_id == focus, ui_origin_x, ui_origin_y);
}

static void draw_milestone_year(int32_t i, int32_t y)
{
    int32_t x0 = ui_origin_x;
    g_text_width = 0;
    text_draw_number_ui(*milestone_year(i), '+', postfix_none, x0 + 277, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_year(0, scenario_data.start.year + *milestone_year(i), g_text_width + x0 + 12 + 275, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

void window_editor_settings_draw(void)
{
    int32_t x0 = ui_origin_x;
    int32_t y0 = ui_origin_y;
    int32_t bx = x0 + 262;
    outer_panel_draw(x0, y0, 30, 34);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_STARTING_CONDITIONS, x0 + 32, y0 + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, x0 + 12, y0 + 507, 480, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_settings_row(EDITOR_TEXT_INITIAL_RANK, y0 + 52, SETTINGS_BUTTON_INITIAL_RANK);
    lang_text_draw_centered_ui(LANG_GROUP_PLAYER_RANKS, scenario_data.initial_rank, bx, y0 + 60, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_settings_row(EDITOR_TEXT_ADJUST_THE_START_DATE, y0 + 92, SETTINGS_BUTTON_START_DATE);
    lang_text_draw_year(0, scenario_data.start.year, x0 + 314, y0 + 96, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    draw_settings_row(EDITOR_TEXT_INITIAL_FUNDS, y0 + 132, SETTINGS_BUTTON_INITIAL_FUNDS);
    text_draw_number_centered(scenario_data.initial_funds, NUMBER_PREFIX_NONE, postfix_none, bx, y0 + 140, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_settings_row(EDITOR_TEXT_PHARAOH_GIFT, y0 + 172, SETTINGS_BUTTON_PHARAOH_GIFT);
    text_draw_number_centered(scenario_data.bailout_amount, NUMBER_PREFIX_NONE, postfix_none, bx, y0 + 180, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_settings_row(EDITOR_TEXT_MILESTONE_25, y0 + 212, SETTINGS_BUTTON_MILESTONE_25);
    draw_milestone_year(0, y0 + 220);
    draw_settings_row(EDITOR_TEXT_MILESTONE_50, y0 + 252, SETTINGS_BUTTON_MILESTONE_50);
    draw_milestone_year(1, y0 + 260);
    draw_settings_row(EDITOR_TEXT_MILESTONE_75, y0 + 292, SETTINGS_BUTTON_MILESTONE_75);
    draw_milestone_year(2, y0 + 300);
    draw_settings_row(EDITOR_TEXT_INTEREST_RATE_ON_DEBT, y0 + 332, SETTINGS_BUTTON_INTEREST_RATE);
    g_text_width = 0;
    text_draw_number_centered(scenario_data.interest_rate, NUMBER_PREFIX_NONE, postfix_none, bx, y0 + 340, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_settings_row(EDITOR_TEXT_CURRENT_PHARAOH, y0 + 372, SETTINGS_BUTTON_CURRENT_PHARAOH);
    lang_text_draw_centered_ui(LANG_GROUP_PHARAOH_NAMES, scenario_data.pharaoh_name_id, bx, y0 + 380, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_settings_row(EDITOR_TEXT_PLAYER_INCARNATION, y0 + 412, SETTINGS_BUTTON_PLAYER_INCARNATION);
    lang_text_draw_centered_ui(LANG_GROUP_PLAYER_INCARNATIONS, scenario_data.player_title_id, bx, y0 + 420, 200, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static void show_settings_number_input(int32_t *target)
{
    numeric_input_show(ui_origin_x + 140, ui_origin_y + 56, 4, 0, 0, target, 0);
    window_redraw_request = 1;
}

void editor_settings_funds(void)
{
    show_settings_number_input(&scenario_data.initial_funds);
}
void editor_settings_bailout(void)
{
    show_settings_number_input(&scenario_data.bailout_amount);
}
void editor_settings_interest(void)
{
    show_settings_number_input(&scenario_data.interest_rate);
}

void editor_settings_empire_map(void)
{
    editor_select_list_show(20, 40, PLAYER_RANK_COUNT, LANG_GROUP_PLAYER_RANKS, 0, 0, &scenario_data.initial_rank, 0, 0, 2, 0, 0, 0, -1, 0);
    window_redraw_request = 1;
}

void editor_settings_pharaoh_name(void)
{
    int32_t extra = select_list_extra_text;
    select_list_extra_on = 1;
    if (extra != 0) {
        editor_select_list_show(0, 0, PHARAOH_NAME_COUNT - 1, LANG_GROUP_PHARAOH_NAMES, PHARAOH_NAME_COUNT, 0, 0, &scenario_data.pharaoh_name_id, 0, 4, 0, 0, 0, 22, 0);
    } else {
        editor_select_list_show(0, 0, PHARAOH_NAME_COUNT, LANG_GROUP_PHARAOH_NAMES, 0, 0, 0, &scenario_data.pharaoh_name_id, 0, 4, 0, 0, 0, 22, 0);
    }
    window_redraw_request = 1;
}

void editor_settings_player_title(void)
{
    editor_select_list_show(0, 0, PLAYER_INCARNATION_COUNT, LANG_GROUP_PLAYER_INCARNATIONS, 0, 0, 0, &scenario_data.player_title_id, 0, 4, 0, 0, 0, 16, 0);
    window_redraw_request = 1;
}

void editor_settings_milestone(void)
{
    numeric_input_show(300, 100, 4, 0, 0, milestone_year(button_parameter), 0);
    window_redraw_request = 1;
}

void editor_settings_start_year(void)
{
    window_current_id = WINDOW_EDITOR_START_YEAR;
    window_redraw_request = 1;
}

void window_editor_trade_amounts_draw(void)
{
    int32_t y0 = ui_origin_y;
    int32_t py = ui_origin_y + 32;
    int32_t px = ui_origin_x + 16;
    outer_panel_draw(px, py, 30, 14);
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_SET_SUPPLY_DEMAND_LEVELS, px, y0 + 48, 480, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    int32_t dx = 0;
    for (int32_t i = 0; i < TRADE_AMOUNTS_PER_ROW * TRADE_AMOUNTS_ROWS; i++, dx += 50) {
        int32_t resource = trade_amounts_resource(i);
        if (resource < 0) {
            continue;
        }
        int32_t amount = empire_object_trade_amount(g_empire_selected_object, resource);
        int32_t focus = (i + 1) == focus_button_id;
        int32_t rx;
        int32_t ry;
        if (i < TRADE_AMOUNTS_PER_ROW) {
            ry = 60;
            rx = dx;
        } else {
            rx = dx - 400;
            ry = 120;
        }
        image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, rx + 40 + px, ry + py);
        int32_t bx = rx + 25 + px;
        button_border_draw_ui(bx, ry + 30 + py, 50, 22, focus, ui_origin_x, ui_origin_y);
        if (amount != 0) {
            lang_text_draw_centered_ui(
                LANG_GROUP_EDITOR, amount + EDITOR_TEXT_TRADE_AMOUNT_NONE, bx, ry + 36 + py, 50, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        }
    }
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, px, y0 + 228, 480, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void editor_trade_amount_next(void)
{
    int32_t resource = trade_amounts_resource(button_parameter);
    int32_t amount = empire_object_trade_amount(g_empire_selected_object, resource) + 1;
    if (amount > TRADE_AMOUNT_HIGH) {
        amount = TRADE_AMOUNT_NONE;
    }
    empire_object_set_trade_amount(g_empire_selected_object, resource, amount);
    window_redraw_request = 1;
}
