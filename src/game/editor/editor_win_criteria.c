// The scenario editor's win criteria window: drawing and buttons (the monuments button is in editor_monuments.c).
// With open play set the goal rows read "no".
//
// With open play the prosperity row's "no" is drawn 1600 pixels lower, off the panel.

#include "editor/editor_win_criteria.h"
#include "building/house_evolve.h"
#include "building/monument_city.h"
#include "editor/editor_dialogs.h"
#include "editor/editor_text.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "text/lang.h"
#include "window/input_dialogs.h"
#include "window/window_image_group.h"
#include <stdint.h>

static int32_t row_y(int32_t panel_y, int32_t row)
{
    return panel_y + WIN_CRITERIA_FIRST_ROW + (row * WIN_CRITERIA_ROW_SPACING);
}

static int32_t row_text_y(int32_t panel_y, int32_t row)
{
    return row_y(panel_y, row) + WIN_CRITERIA_TEXT_DY;
}

static void draw_criteria_row(int32_t label, int32_t y, int32_t focus_left)
{
    lang_text_draw_ui(LANG_GROUP_EDITOR, label, ui_origin_x + WIN_CRITERIA_LABEL_X, y + WIN_CRITERIA_TEXT_DY, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    button_border_draw_ui(
        ui_origin_x + WIN_CRITERIA_TOGGLE_X, y, WIN_CRITERIA_TOGGLE_WIDTH, WIN_CRITERIA_BUTTON_HEIGHT, focus_left, ui_origin_x, ui_origin_y);
}

static void draw_criteria_yes_no(int32_t value, int32_t y)
{
    lang_text_draw_centered_ui(
        LANG_GROUP_YES_NO, value, ui_origin_x + WIN_CRITERIA_TOGGLE_X, y, WIN_CRITERIA_TOGGLE_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

static void draw_criteria_value_border(int32_t x, int32_t y, int32_t focus)
{
    button_border_draw_ui(x, y, WIN_CRITERIA_VALUE_WIDTH, WIN_CRITERIA_BUTTON_HEIGHT, focus, ui_origin_x, ui_origin_y);
}

static void draw_criteria_value(int32_t value, int32_t x, int32_t y)
{
    text_draw_number_centered(value, NUMBER_PREFIX_NONE, postfix_none, x, y, WIN_CRITERIA_VALUE_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_criteria_years(int32_t years, int32_t y, int32_t px)
{
    g_text_width = 0;
    text_draw_number_ui(years, '+', postfix_none, px + 420, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_year(0, scenario_data.start.year + years, g_text_width + px + 430, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

void window_editor_win_criteria_draw(void)
{
    int32_t px = ui_origin_x + 16;
    int32_t py = ui_origin_y + 32;
    int32_t right = ui_origin_x + WIN_CRITERIA_VALUE_X;
    int32_t plural = 0;
    outer_panel_draw(px, py, 38, 28);
    lang_text_draw_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_WIN_CRITERIA_TITLE, px + 10, py + 10, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CLOSE, px, py + 424, 608, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_criteria_row(EDITOR_TEXT_OPEN_PLAY, row_y(py, WIN_CRITERIA_ROW_OPEN_PLAY), focus_button_id == WIN_CRITERIA_BUTTON_OPEN_PLAY);
    draw_criteria_yes_no(scenario_data.open_play, row_text_y(py, WIN_CRITERIA_ROW_OPEN_PLAY));
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_OPEN_PLAY), focus_button_id == WIN_CRITERIA_BUTTON_OPEN_PLAY_ID);
    draw_criteria_value(scenario_data.open_play_scenario_id, right, row_text_y(py, WIN_CRITERIA_ROW_OPEN_PLAY));
    draw_criteria_row(EDITOR_TEXT_HOUSING_LEVEL, row_y(py, WIN_CRITERIA_ROW_HOUSES), focus_button_id == WIN_CRITERIA_BUTTON_HOUSES);
    text_draw_number_centered(scenario_data.win_houses, NUMBER_PREFIX_NONE, postfix_none, ui_origin_x + WIN_CRITERIA_TOGGLE_X, row_text_y(py, WIN_CRITERIA_ROW_HOUSES),
        WIN_CRITERIA_TOGGLE_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    if (scenario_data.win_houses > 1) {
        plural = HOUSE_LEVEL_COUNT;
    }
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_HOUSES), focus_button_id == WIN_CRITERIA_BUTTON_HOUSES_LEVEL);
    lang_text_draw_centered_ui(LANG_GROUP_HOUSE_LEVELS, scenario_data.win_houses_level + plural, right, row_text_y(py, WIN_CRITERIA_ROW_HOUSES),
        WIN_CRITERIA_VALUE_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_criteria_row(EDITOR_TEXT_CULTURE_NEEDED, row_y(py, WIN_CRITERIA_ROW_CULTURE), focus_button_id == WIN_CRITERIA_BUTTON_CULTURE);
    draw_criteria_yes_no((scenario_data.open_play == 0) ? scenario_data.win_culture_enabled : 0, row_text_y(py, WIN_CRITERIA_ROW_CULTURE));
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_CULTURE), focus_button_id == WIN_CRITERIA_BUTTON_CULTURE_VALUE);
    draw_criteria_value(scenario_data.win_culture, right, row_text_y(py, WIN_CRITERIA_ROW_CULTURE));
    draw_criteria_row(EDITOR_TEXT_PROSPERITY_NEEDED, row_y(py, WIN_CRITERIA_ROW_PROSPERITY), focus_button_id == WIN_CRITERIA_BUTTON_PROSPERITY);
    if (scenario_data.open_play != 0) {
        draw_criteria_yes_no(0, row_text_y(py, WIN_CRITERIA_ROW_PROSPERITY) + WIN_CRITERIA_PROSPERITY_OFF_PANEL);
    } else {
        draw_criteria_yes_no(scenario_data.win_prosperity_enabled, row_text_y(py, WIN_CRITERIA_ROW_PROSPERITY));
    }
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_PROSPERITY), focus_button_id == WIN_CRITERIA_BUTTON_PROSPERITY_VALUE);
    draw_criteria_value(scenario_data.win_prosperity, right, row_text_y(py, WIN_CRITERIA_ROW_PROSPERITY));
    draw_criteria_row(EDITOR_TEXT_NUMBER_OF_MONUMENTS, row_y(py, WIN_CRITERIA_ROW_MONUMENTS), 0);
    text_draw_number_centered(scenario_count_monuments(), NUMBER_PREFIX_NONE, postfix_none, ui_origin_x + WIN_CRITERIA_TOGGLE_X, row_text_y(py, WIN_CRITERIA_ROW_MONUMENTS),
        WIN_CRITERIA_TOGGLE_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_MONUMENTS), focus_button_id == WIN_CRITERIA_BUTTON_MONUMENTS);
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, EDITOR_TEXT_SELECT_MONUMENTS, right, row_text_y(py, WIN_CRITERIA_ROW_MONUMENTS), WIN_CRITERIA_VALUE_WIDTH,
        image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_criteria_row(EDITOR_TEXT_KINGDOM_NEEDED, row_y(py, WIN_CRITERIA_ROW_KINGDOM), focus_button_id == WIN_CRITERIA_BUTTON_KINGDOM);
    draw_criteria_yes_no((scenario_data.open_play != 0) ? 0 : scenario_data.win_kingdom_enabled, row_text_y(py, WIN_CRITERIA_ROW_KINGDOM));
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_KINGDOM), focus_button_id == WIN_CRITERIA_BUTTON_KINGDOM_VALUE);
    draw_criteria_value(scenario_data.win_kingdom, right, row_text_y(py, WIN_CRITERIA_ROW_KINGDOM));
    draw_criteria_row(EDITOR_TEXT_TIME_LIMIT, row_y(py, WIN_CRITERIA_ROW_TIME_LIMIT), focus_button_id == WIN_CRITERIA_BUTTON_TIME_LIMIT);
    draw_criteria_yes_no((scenario_data.open_play != 0) ? 0 : scenario_data.time_limit, row_text_y(py, WIN_CRITERIA_ROW_TIME_LIMIT));
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_TIME_LIMIT), focus_button_id == WIN_CRITERIA_BUTTON_TIME_LIMIT_YEARS);
    draw_criteria_years(scenario_data.time_limit_years, row_text_y(py, WIN_CRITERIA_ROW_TIME_LIMIT), px);
    draw_criteria_row(EDITOR_TEXT_SURVIVAL, row_y(py, WIN_CRITERIA_ROW_SURVIVAL), focus_button_id == WIN_CRITERIA_BUTTON_SURVIVAL);
    draw_criteria_yes_no((scenario_data.open_play != 0) ? 0 : scenario_data.survival, row_text_y(py, WIN_CRITERIA_ROW_SURVIVAL));
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_SURVIVAL), focus_button_id == WIN_CRITERIA_BUTTON_SURVIVAL_YEARS);
    draw_criteria_years(scenario_data.survival_years, row_text_y(py, WIN_CRITERIA_ROW_SURVIVAL), px);
    int32_t y = row_text_y(py, WIN_CRITERIA_ROW_POPULATION);
    draw_criteria_row(EDITOR_TEXT_WINNING_POPULATION, row_y(py, WIN_CRITERIA_ROW_POPULATION), focus_button_id == WIN_CRITERIA_BUTTON_POPULATION);
    draw_criteria_yes_no((scenario_data.open_play != 0) ? 0 : scenario_data.win_population_enabled, y);
    draw_criteria_value_border(right, row_y(py, WIN_CRITERIA_ROW_POPULATION), focus_button_id == WIN_CRITERIA_BUTTON_POPULATION_VALUE);
    draw_criteria_value(scenario_data.win_population, right, y);
}

void editor_criteria_open_play_toggle(void)
{
    window_redraw_request = 1;
    scenario_data.open_play = scenario_data.open_play == 0;
}

static void show_criteria_number_input(int32_t *target)
{
    numeric_input_show(300, 200, 4, 0, 0, target, 0);
    window_redraw_request = 1;
}

void editor_criteria_open_play_id(void)
{
    numeric_input_show(300, 200, INPUT_TARGET_INT8, &scenario_data.open_play_scenario_id, 0, 0, 0);
    window_redraw_request = 1;
}

void editor_criteria_houses(void)
{
    show_criteria_number_input(&scenario_data.win_houses);
}

void editor_criteria_houses_level(void)
{
    editor_select_list_show(300, 100, HOUSE_LEVEL_COUNT, LANG_GROUP_HOUSE_LEVELS, 0, 0, 0, &scenario_data.win_houses_level, 0, 4, 0, 0, 0, -1, 0);
    window_redraw_request = 1;
}

void editor_criteria_goal_toggle(void)
{
    int32_t i = button_parameter;
    window_redraw_request = 1;
    (&scenario_data.win_culture_enabled)[i] = (&scenario_data.win_culture_enabled)[i] == 0;
}

void editor_criteria_goal(void)
{
    show_criteria_number_input(&(&scenario_data.win_culture)[button_parameter]);
}

void editor_criteria_time_limit_toggle(void)
{
    if (scenario_data.time_limit != 0) {
        scenario_data.time_limit = 0;
        window_redraw_request = 1;
        return;
    }
    scenario_data.survival = 0;
    scenario_data.time_limit = 1;
    window_redraw_request = 1;
}

void editor_criteria_time_limit_years(void)
{
    show_criteria_number_input(&scenario_data.time_limit_years);
}

void editor_criteria_survival_toggle(void)
{
    if (scenario_data.survival != 0) {
        scenario_data.survival = 0;
        window_redraw_request = 1;
        return;
    }
    scenario_data.time_limit = 0;
    scenario_data.survival = 1;
    window_redraw_request = 1;
}

void editor_criteria_survival_years(void)
{
    show_criteria_number_input(&scenario_data.survival_years);
}

void editor_criteria_population_toggle(void)
{
    scenario_data.win_population_enabled = scenario_data.win_population_enabled == 0;
    window_redraw_request = 1;
}

void editor_criteria_population(void)
{
    show_criteria_number_input(&scenario_data.win_population);
}
