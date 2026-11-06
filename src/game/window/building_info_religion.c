// The building info panels of the religious buildings: temples, temple complexes and shrines of the five gods, the
// oracle, the festival square (the months since the last festival, by festival_square_rules) and the hold festival
// window (the gods, the three sizes with their costs).
//
// Gods and festival sizes are mapped loosely:
// - a temple of a god outside 0..3, and a shrine or god image of a god above 3, are Bast's;
// - while drawing, the hold festival window moves a selected god the scenario lacks to the next god;
// - the sizes are greyed when the treasury is below -4999, the grand one also without enough beer.

#include "window/building_info_religion.h"
#include "building/building.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/kingdom.h"
#include "city/resource_type.h"
#include "game/game_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum entertainment_advisor_festival_text {
    FESTIVAL_TEXT_SINCE_LAST_FESTIVAL = 15,
    FESTIVAL_TEXT_HOLD_NEW_FESTIVAL = 16,
    FESTIVAL_TEXT_FIRST_GOD = 25,
    FESTIVAL_TEXT_SIZE_COSTS = 30,
    FESTIVAL_TEXT_SMALL = 31,
    FESTIVAL_TEXT_LARGE = 32,
    FESTIVAL_TEXT_GRAND = 33,
    FESTIVAL_TEXT_PREPARING_FOR = 34,
    FESTIVAL_TEXT_ORDER_FESTIVAL = 52,
    FESTIVAL_TEXT_ORDERED_FOR = 53,
    FESTIVAL_TEXT_NEXT_MONTH = 54,
};

enum festival_planning_text {
    PLANNING_TEXT_JUST_HELD = 0,
    PLANNING_TEXT_FEW_MONTHS = 1,
    PLANNING_TEXT_QUITE_SOME_TIME = 2,
    PLANNING_TEXT_OVER_A_YEAR = 3,
    PLANNING_TEXT_ALMOST_TWO_YEARS = 4,
    PLANNING_TEXT_OVER_TWO_YEARS = 5,
    PLANNING_TEXT_GOOD_OLD_DAYS = 6,
    PLANNING_TEXT_WONDERFUL_FESTIVAL = 7,
    PLANNING_TEXT_ORDER_ONE_NOW = 8,
    PLANNING_TEXT_ABOUT_TIME = 9,
    PLANNING_TEXT_SMALL_IN_ONE_MONTH = 10,
    PLANNING_TEXT_LARGE_IN_ONE_MONTH = 20,
    PLANNING_TEXT_GRAND_IN_ONE_MONTH = 31,
};

enum festival_ordered_months {
    PLANNING_ORDERED_SMALL_MONTHS = 2,
    PLANNING_ORDERED_LARGE_MONTHS = 3,
    PLANNING_ORDERED_GRAND_MONTHS = 4,
};

enum festival_image {
    FESTIVAL_GOD_IMAGE = 16,
    FESTIVAL_SELECTED_GOD_IMAGE = 21,
};

enum {
    TEMPLE_TITLE_Y = 12,
    TEMPLE_PANEL_Y = 56,
    TEMPLE_PANEL_BLOCKS = 4,
    TEMPLE_EMPLOYMENT_Y = 62,
    TEMPLE_GOD_IMAGE_X = 190,
    TEMPLE_GOD_IMAGE_FROM_BOTTOM = 182,
    TEMPLE_NO_ROAD_BLOCKS_FROM_BOTTOM = 8,
    SHRINE_TEXTS_PER_GOD = 2,
    ORACLE_TEXT_FROM_BOTTOM = 143,
    FESTIVAL_BUTTON_X = 82,
    FESTIVAL_BUTTON_Y = 153,
    FESTIVAL_BUTTON_WIDTH = 300,
    FESTIVAL_BUTTON_HEIGHT = 20,
    FESTIVAL_BUTTON_FOCUS = 1,
    FESTIVAL_WINDOW_X = 78,
    FESTIVAL_WINDOW_Y = 78,
    FESTIVAL_WINDOW_WIDTH_BLOCKS = 34,
    FESTIVAL_WINDOW_HEIGHT_BLOCKS = 20,
    FESTIVAL_GODS_X = 100,
    FESTIVAL_GOD_SPACING = 100,
    FESTIVAL_GOD_Y = 126,
    FESTIVAL_GOD_GREY_DX = 5,
    FESTIVAL_GOD_GREY_Y = 131,
    FESTIVAL_GOD_GREY_WIDTH = 72,
    FESTIVAL_GOD_GREY_HEIGHT = 82,
    FESTIVAL_GOD_BORDER_DX = -4,
    FESTIVAL_GOD_BORDER_Y = 122,
    FESTIVAL_GOD_BORDER_WIDTH = 90,
    FESTIVAL_GOD_BORDER_HEIGHT = 100,
    FESTIVAL_GOD_NAME_Y = 90,
    FESTIVAL_GOD_NAME_WIDTH = 544,
    FESTIVAL_SIZE_TEXT_X = 210,
    FESTIVAL_SIZE_TEXT_Y = 352,
    FESTIVAL_SIZE_ROW_X = 132,
    FESTIVAL_SIZE_ROW_Y = 246,
    FESTIVAL_SIZE_ROW_SPACING = 30,
    FESTIVAL_SIZE_ROW_WIDTH = 430,
    FESTIVAL_SIZE_ROW_HEIGHT = 26,
    FESTIVAL_SIZE_FIRST_FOCUS = 6,
    FESTIVAL_SIZE_LABEL_X = 140,
    FESTIVAL_SIZE_LABEL_DY = 8,
    FESTIVAL_BEER_DX = 10,
    FESTIVAL_BEER_ICON_DY = 3,
    FESTIVAL_GREY_X = 134,
    FESTIVAL_GREY_DY = 2,
    FESTIVAL_GREY_WIDTH = 426,
    FESTIVAL_GREY_HEIGHT = 22,
    FESTIVAL_MIN_TREASURY = -4999,
    SQUARE_TEXT_Y = 55,
    SQUARE_PANEL_X = 40,
    SQUARE_PANEL_Y = 118,
    SQUARE_PANEL_BLOCKS_NARROWER = 5,
    SQUARE_PANEL_HEIGHT_BLOCKS = 6,
    SQUARE_MONTHS_X = 112,
    SQUARE_MONTHS_Y = 130,
    SQUARE_STATE_X = 82,
    SQUARE_STATE_Y = 154,
    SQUARE_STATE_WIDTH = 300,
    SQUARE_ORDERED_X = 102,
    SQUARE_PLANNING_X = 56,
    SQUARE_PLANNING_Y = 175,
    SQUARE_PLANNING_WIDTH = 380,
    MONTHS_PER_YEAR = 12,
};

int32_t festival_square_rules[FESTIVAL_SQUARE_RULE_COUNT] = {
    [FESTIVAL_SQUARE_RULE_MONTHS_FEW] = 2,
    [FESTIVAL_SQUARE_RULE_MONTHS_QUITE_SOME_TIME] = 7,
    [FESTIVAL_SQUARE_RULE_MONTHS_OVER_A_YEAR] = 13,
    [FESTIVAL_SQUARE_RULE_MONTHS_ALMOST_TWO_YEARS] = 19,
    [FESTIVAL_SQUARE_RULE_MONTHS_OVER_TWO_YEARS] = 25,
    [FESTIVAL_SQUARE_RULE_MONTHS_GOOD_OLD_DAYS] = 31,
    [FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ORDER_NOW] = 2,
    [FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ABOUT_TIME] = 7,
};

void window_building_draw_oracle(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_ORACLE];
    info_first_draw = 0;
    window_building_draw_panel_title_at(LANG_GROUP_ORACLE, BUILDING_TEXT_TITLE, TEMPLE_TITLE_Y);
    window_building_draw_right_click_hint();
    window_building_draw_text(
        LANG_GROUP_ORACLE, BUILDING_TEXT_DESCRIPTION, (building_info_h * BUILDING_INFO_BLOCK) - ORACLE_TEXT_FROM_BOTTOM + building_info_y);
}

static int32_t is_shrine(int32_t type)
{
    return type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_GOD_9;
}

void window_building_draw_temple(int32_t god)
{
    int32_t shown_god = (god > GOD_SETH) ? GOD_BAST : god;
    int32_t panel_blocks = 0;
    info_first_draw = 0;
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_TEMPLE];
    if (is_shrine(building_info_building()->type)) {
        window_building_draw_panel_title_at(LANG_GROUP_SHRINES, shown_god * SHRINE_TEXTS_PER_GOD, TEMPLE_TITLE_Y);
    } else {
        int32_t temple_god = (god >= GOD_OSIRIS && god <= GOD_SETH) ? god : GOD_BAST;
        window_building_draw_panel_title_at(LANG_GROUP_TEMPLE_OSIRIS + temple_god, BUILDING_TEXT_TITLE, TEMPLE_TITLE_Y);
        panel_blocks = TEMPLE_PANEL_BLOCKS;
        window_building_draw_inner_panel(TEMPLE_PANEL_Y, panel_blocks);
        window_building_draw_employment(building_info_y + TEMPLE_EMPLOYMENT_Y);
    }
    if (bi_road_access_block.has_road_access > 0) {
        int32_t image_y = ((building_info_h + panel_blocks) * BUILDING_INFO_BLOCK) - TEMPLE_GOD_IMAGE_FROM_BOTTOM + building_info_y;
        image_draw(shown_god + FESTIVAL_SELECTED_GOD_IMAGE + image_group(IMAGE_GROUP_PANEL_WINDOWS), building_info_x + TEMPLE_GOD_IMAGE_X, image_y);
        return;
    }
    window_building_draw_no_road_text(((building_info_h - TEMPLE_NO_ROAD_BLOCKS_FROM_BOTTOM) * BUILDING_INFO_BLOCK) + building_info_y);
}

void window_building_festival_button(void)
{
    const city_player *player = city_player_current();
    if (player->festival_planned_size != 0) {
        return;
    }
    int32_t focus = (player->festival_square_placed != 0) ? (focus_button_id == FESTIVAL_BUTTON_FOCUS) : 0;
    button_border_draw_ui(building_info_x + FESTIVAL_BUTTON_X, building_info_y + FESTIVAL_BUTTON_Y, FESTIVAL_BUTTON_WIDTH, FESTIVAL_BUTTON_HEIGHT, focus,
        building_info_x, building_info_y);
}

void window_hold_festival_draw_background(void)
{
    city_player *player = city_player_current();
    outer_panel_draw(FESTIVAL_WINDOW_X, FESTIVAL_WINDOW_Y, FESTIVAL_WINDOW_WIDTH_BLOCKS, FESTIVAL_WINDOW_HEIGHT_BLOCKS);
    for (int32_t god = GOD_OSIRIS; god < GOD_COUNT; god++) {
        int32_t x = FESTIVAL_GODS_X + (god * FESTIVAL_GOD_SPACING);
        if (scenario_data.god_status[god] == 0) {
            if (god == player->festival_selected_god) {
                player->festival_selected_god++;
            }
            image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + FESTIVAL_GOD_IMAGE + god, x, FESTIVAL_GOD_Y);
            graphics_grey_rect(x + FESTIVAL_GOD_GREY_DX, FESTIVAL_GOD_GREY_Y, FESTIVAL_GOD_GREY_WIDTH, FESTIVAL_GOD_GREY_HEIGHT, 0);
        } else if (god == player->festival_selected_god) {
            button_border_draw_ui(x + FESTIVAL_GOD_BORDER_DX, FESTIVAL_GOD_BORDER_Y, FESTIVAL_GOD_BORDER_WIDTH, FESTIVAL_GOD_BORDER_HEIGHT, 1,
                building_info_x, building_info_y);
            image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + FESTIVAL_SELECTED_GOD_IMAGE + god, x, FESTIVAL_GOD_Y);
        } else {
            image_draw(image_group(IMAGE_GROUP_PANEL_WINDOWS) + FESTIVAL_GOD_IMAGE + god, x, FESTIVAL_GOD_Y);
        }
    }
    if (player->festival_selected_god < GOD_COUNT) {
        lang_text_draw_centered_ui(LANG_GROUP_ENTERTAINMENT_ADVISOR, player->festival_selected_god + FESTIVAL_TEXT_FIRST_GOD, FESTIVAL_WINDOW_X,
            FESTIVAL_GOD_NAME_Y, FESTIVAL_GOD_NAME_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
    }
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_ENTERTAINMENT_ADVISOR, player->festival_selected_size + FESTIVAL_TEXT_SIZE_COSTS, FESTIVAL_SIZE_TEXT_X, FESTIVAL_SIZE_TEXT_Y,
        image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static int32_t festival_row_y(int32_t row)
{
    return FESTIVAL_SIZE_ROW_Y + (row * FESTIVAL_SIZE_ROW_SPACING);
}

static int32_t draw_festival_size_row(int32_t row)
{
    int32_t label_y = festival_row_y(row) + FESTIVAL_SIZE_LABEL_DY;
    button_border_draw_ui(FESTIVAL_SIZE_ROW_X, festival_row_y(row), FESTIVAL_SIZE_ROW_WIDTH, FESTIVAL_SIZE_ROW_HEIGHT,
        focus_button_id == (FESTIVAL_SIZE_FIRST_FOCUS + row), building_info_x, building_info_y);
    g_text_width = 0;
    lang_text_draw_ui(
        LANG_GROUP_ENTERTAINMENT_ADVISOR, FESTIVAL_TEXT_SMALL + row, FESTIVAL_SIZE_LABEL_X, label_y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    return label_y;
}

static void grey_festival_size_row(int32_t row)
{
    graphics_grey_rect(FESTIVAL_GREY_X, festival_row_y(row) + FESTIVAL_GREY_DY, FESTIVAL_GREY_WIDTH, FESTIVAL_GREY_HEIGHT, 0);
}

void window_hold_festival_draw_foreground(void)
{
    const city_player *player = city_player_current();
    int32_t grand_row = FESTIVAL_GRAND - FESTIVAL_SMALL;
    for (int32_t row = 0; row < grand_row; row++) {
        int32_t label_y = draw_festival_size_row(row);
        lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_DEBEN, player->festival_cost[row], g_text_width + FESTIVAL_SIZE_LABEL_X, label_y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
    int32_t label_y = draw_festival_size_row(grand_row);
    int32_t width = g_text_width;
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_DEBEN, player->festival_cost[grand_row], width + FESTIVAL_SIZE_LABEL_X, label_y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    width += g_text_width;
    g_text_width = width;
    int32_t beer_x = FESTIVAL_SIZE_LABEL_X + FESTIVAL_BEER_DX;
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT, player->festival_grand_beer, width + beer_x, label_y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    g_text_width += width;
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_BEER, g_text_width + beer_x, festival_row_y(grand_row) + FESTIVAL_BEER_ICON_DY);
    if (player->treasury < FESTIVAL_MIN_TREASURY) {
        grey_festival_size_row(0);
        grey_festival_size_row(1);
        grey_festival_size_row(grand_row);
        return;
    }
    for (int32_t row = 0; row < grand_row; row++) {
        if (player->treasury < player->festival_cost[row]) {
            grey_festival_size_row(row);
        }
    }
    if (player->treasury < player->festival_cost[grand_row] || player->festival_not_enough_beer != 0) {
        grey_festival_size_row(grand_row);
    }
}

static int32_t months_since_festival_text(int32_t months)
{
    if (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_FEW]) {
        return PLANNING_TEXT_JUST_HELD;
    }
    if (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_QUITE_SOME_TIME]) {
        return PLANNING_TEXT_FEW_MONTHS;
    }
    if (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_OVER_A_YEAR]) {
        return PLANNING_TEXT_QUITE_SOME_TIME;
    }
    if (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_ALMOST_TWO_YEARS]) {
        return PLANNING_TEXT_OVER_A_YEAR;
    }
    if (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_OVER_TWO_YEARS]) {
        return PLANNING_TEXT_ALMOST_TWO_YEARS;
    }
    return (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_GOOD_OLD_DAYS]) ? PLANNING_TEXT_OVER_TWO_YEARS : PLANNING_TEXT_GOOD_OLD_DAYS;
}

static int32_t festival_effect_text(int32_t months)
{
    if (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ORDER_NOW]) {
        return PLANNING_TEXT_WONDERFUL_FESTIVAL;
    }
    return (months < festival_square_rules[FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ABOUT_TIME]) ? PLANNING_TEXT_ORDER_ONE_NOW : PLANNING_TEXT_ABOUT_TIME;
}

static int32_t draw_unplanned_festival(const city_player *player)
{
    int32_t x = building_info_x + SQUARE_STATE_X;
    int32_t y = building_info_y + SQUARE_STATE_Y;
    if (player->first_festival_effect_months == 0 || player->second_festival_effect_months == 0) {
        lang_text_draw_centered_ui(
            LANG_GROUP_ENTERTAINMENT_ADVISOR, FESTIVAL_TEXT_HOLD_NEW_FESTIVAL, x, y, SQUARE_STATE_WIDTH, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
        return months_since_festival_text(player->months_since_festival);
    }
    lang_text_draw_centered_ui(
        LANG_GROUP_ENTERTAINMENT_ADVISOR, FESTIVAL_TEXT_ORDER_FESTIVAL, x, y, SQUARE_STATE_WIDTH, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
    return festival_effect_text(player->months_since_festival);
}

static int32_t draw_planned_festival(const city_player *player, int32_t size)
{
    int32_t ordered_months;
    int32_t one_month_text;
    if (size == FESTIVAL_SMALL) {
        ordered_months = PLANNING_ORDERED_SMALL_MONTHS;
        one_month_text = PLANNING_TEXT_SMALL_IN_ONE_MONTH;
    } else if (size == FESTIVAL_LARGE) {
        ordered_months = PLANNING_ORDERED_LARGE_MONTHS;
        one_month_text = PLANNING_TEXT_LARGE_IN_ONE_MONTH;
    } else {
        ordered_months = PLANNING_ORDERED_GRAND_MONTHS;
        one_month_text = PLANNING_TEXT_GRAND_IN_ONE_MONTH;
    }
    int32_t x = building_info_x + SQUARE_ORDERED_X;
    int32_t y = building_info_y + SQUARE_STATE_Y;
    int32_t state_text = (ordered_months < player->festival_planned_months_to_go) ? FESTIVAL_TEXT_ORDERED_FOR : FESTIVAL_TEXT_PREPARING_FOR;
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_ENTERTAINMENT_ADVISOR, state_text, x, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
    int32_t months_to_go = player->festival_planned_months_to_go;
    if (months_to_go == 1) {
        lang_text_draw_ui(LANG_GROUP_ENTERTAINMENT_ADVISOR, FESTIVAL_TEXT_NEXT_MONTH, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
    } else {
        lang_text_draw_ui(
            LANG_GROUP_MONTHS, (months_to_go + game_time_month) % MONTHS_PER_YEAR, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
    }
    return one_month_text - 1 + months_to_go;
}

void window_building_draw_festival_square(void)
{
    const city_player *player = city_player_current();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_FESTIVAL_SQUARE];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_FESTIVAL_SQUARE, BUILDING_TEXT_TITLE);
    window_building_draw_text(LANG_GROUP_FESTIVAL_SQUARE, BUILDING_TEXT_DESCRIPTION, building_info_y + SQUARE_TEXT_Y);
    inner_panel_draw_ui(building_info_x + SQUARE_PANEL_X, building_info_y + SQUARE_PANEL_Y, info_w_blocks - SQUARE_PANEL_BLOCKS_NARROWER,
        SQUARE_PANEL_HEIGHT_BLOCKS, building_info_x, building_info_y);
    int32_t months_y = building_info_y + SQUARE_MONTHS_Y;
    g_text_width = 0;
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_MONTH, player->months_since_festival, building_info_x + SQUARE_MONTHS_X, months_y, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    lang_text_draw_ui(LANG_GROUP_ENTERTAINMENT_ADVISOR, FESTIVAL_TEXT_SINCE_LAST_FESTIVAL, g_text_width + SQUARE_MONTHS_X + building_info_x, months_y,
        image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
    int32_t size = player->festival_planned_size;
    int32_t text = (size == 0) ? draw_unplanned_festival(player) : draw_planned_festival(player, size);
    lang_text_draw_multiline_ui(LANG_GROUP_FESTIVAL_PLANNING, text, building_info_x + SQUARE_PLANNING_X, building_info_y + SQUARE_PLANNING_Y,
        SQUARE_PLANNING_WIDTH, image_group(IMAGE_GROUP_FONT_WHITE), 0, building_info_x, building_info_y);
}
