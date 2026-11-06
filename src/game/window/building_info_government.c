// The building info panels of the government buildings: the mansions (money kept, the salary button), the palaces
// (money kept, the tax rate with its arrows), the courthouse and the tax collector, which collects only while a
// placed palace works.

#include "window/building_info_government.h"
#include "building/building.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "graphics/arrow_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum money_text {
    PALACE_TEXT_VAULTS_HOLD = 2,
    TAX_COLLECTOR_TEXT_VAULTS_HOLD = 2,
    COURTHOUSE_TEXT_VAULT_HOLDS = 9,
    FINANCE_TEXT_TAX_RATE = 1,
};

enum courthouse_text {
    COURTHOUSE_TEXT_NO_STAFF = 2,
    COURTHOUSE_TEXT_FULL_STAFF = 6,
    COURTHOUSE_TEXT_MAGISTRATE_OUT = 7,
    COURTHOUSE_TEXT_MAGISTRATE_IN = 8,
};

enum tax_collector_text {
    TAX_COLLECTOR_TEXT_FULL_STAFF = 5,
    TAX_COLLECTOR_TEXT_NO_STAFF = 10,
    TAX_COLLECTOR_TEXT_NEEDS_PALACE = 11,
    TAX_COLLECTOR_TEXT_NEEDS_WORKING_PALACE = 12,
};

enum mansion_text {
    MANSION_TEXT_DEBEN_PER_MONTH = 2,
    MANSION_TEXT_FIRST_SALARY_RANK = 3,
};

enum {
    TAX_ARROWS_X = 226,
    TAX_ARROWS_Y = -37,
    TAX_ARROW_BUTTONS = 2,
    TAX_ARROW_FOCUS_BASE = 10,
    MONEY_ICON_X = 16,
    MONEY_ICON_Y = 36,
    MONEY_Y = 43,
    PALACE_MONEY_X = 32,
    TAX_COLLECTOR_MONEY_X = 35,
    TAX_RATE_X = 260,
    COURTHOUSE_TEXT_Y = 63,
    COURTHOUSE_STAFF_TEXT_Y = 83,
    COURTHOUSE_DESCRIPTION_FROM_BOTTOM = 113,
    MANSION_MONEY_X = 48,
    MANSION_MONEY_Y = 47,
    MANSION_DESCRIPTION_FROM_BOTTOM = 143,
    SALARY_BUTTON_X = 40,
    SALARY_BUTTON_Y = 61,
    SALARY_BUTTON_WIDTH = 400,
    SALARY_BUTTON_HEIGHT = 25,
    SALARY_BUTTON_FOCUS = 1,
    SALARY_TEXT_X = 44,
    SALARY_TEXT_Y = 65,
};

static int32_t text_y_from_bottom(int32_t from_bottom)
{
    return (building_info_h * BUILDING_INFO_BLOCK) - from_bottom + building_info_y;
}

void window_building_draw_palace_foreground(void)
{
    int32_t x = building_info_x + TAX_ARROWS_X;
    int32_t y = building_info_y + TAX_ARROWS_Y;
    arrow_buttons_get_focus(x, y, advisor_finance_tax_arrow_buttons, TAX_ARROW_BUTTONS, TAX_ARROW_FOCUS_BASE);
    arrow_buttons_handle_mouse(x, y, advisor_finance_tax_arrow_buttons, TAX_ARROW_BUTTONS);
}

static void draw_money_icon(void)
{
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_GOLD, building_info_x + MONEY_ICON_X, building_info_y + MONEY_ICON_Y);
}

static void draw_money_kept(int32_t group, int32_t index, int32_t dx, int32_t dy)
{
    const building *b = building_info_building();
    int32_t y = building_info_y + dy;
    g_text_width = 0;
    lang_text_draw_ui(group, index, building_info_x + dx, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_DEBEN, b->tax_income_or_storage, g_text_width + dx + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_money_and_tax(int32_t group, int32_t index, int32_t dx)
{
    int32_t y = building_info_y + MONEY_Y;
    draw_money_kept(group, index, dx, MONEY_Y);
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_FINANCE_ADVISOR, FINANCE_TEXT_TAX_RATE, building_info_x + TAX_RATE_X, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    text_draw_number_ui(city_player_current()->tax_percentage, NUMBER_PREFIX_NONE, pct_postfix, g_text_width + TAX_RATE_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x,
        building_info_y);
    arrow_buttons_draw(building_info_x + TAX_ARROWS_X, building_info_y + TAX_ARROWS_Y, advisor_finance_tax_arrow_buttons, TAX_ARROW_BUTTONS);
}

void window_building_draw_palace(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_PALACE];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_BUILDING_TYPES, building_info_building()->type);
    draw_money_icon();
    draw_money_and_tax(LANG_GROUP_PALACE, PALACE_TEXT_VAULTS_HOLD, PALACE_MONEY_X);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    }
    window_building_draw_workers_panel();
}

void window_building_draw_courthouse(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_COURTHOUSE];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_COURTHOUSE, BUILDING_TEXT_TITLE);
    draw_money_kept(LANG_GROUP_COURTHOUSE, COURTHOUSE_TEXT_VAULT_HOLDS, BUILDING_INFO_TEXT_X, MONEY_Y);
    const building *b = building_info_building();
    int32_t y = building_info_y + COURTHOUSE_TEXT_Y;
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(y);
    } else if (b->num_workers == 0) {
        window_building_draw_text(LANG_GROUP_COURTHOUSE, COURTHOUSE_TEXT_NO_STAFF, y);
    } else {
        window_building_draw_text(LANG_GROUP_COURTHOUSE, (b->figure_id != 0) ? COURTHOUSE_TEXT_MAGISTRATE_OUT : COURTHOUSE_TEXT_MAGISTRATE_IN, y);
        int32_t staffing_step = window_building_worker_percentage_text(0);
        window_building_draw_text(LANG_GROUP_COURTHOUSE, COURTHOUSE_TEXT_FULL_STAFF - staffing_step, building_info_y + COURTHOUSE_STAFF_TEXT_Y);
    }
    window_building_draw_workers_panel();
    window_building_draw_text(LANG_GROUP_COURTHOUSE, BUILDING_TEXT_DESCRIPTION, text_y_from_bottom(COURTHOUSE_DESCRIPTION_FROM_BOTTOM));
}

static int32_t tax_collector_text(const city_player *player, int32_t palace_worker_percentage)
{
    if (player->palace_placed == 0) {
        return TAX_COLLECTOR_TEXT_NEEDS_PALACE;
    }
    if (palace_worker_percentage == 0) {
        return TAX_COLLECTOR_TEXT_NEEDS_WORKING_PALACE;
    }
    if (building_info_building()->num_workers == 0) {
        return TAX_COLLECTOR_TEXT_NO_STAFF;
    }
    return window_building_worker_percentage_text(TAX_COLLECTOR_TEXT_FULL_STAFF);
}

void window_building_draw_tax_collector(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_TAX_COLLECTOR];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_TAX_COLLECTOR, BUILDING_TEXT_TITLE);
    draw_money_icon();
    draw_money_and_tax(LANG_GROUP_TAX_COLLECTOR, TAX_COLLECTOR_TEXT_VAULTS_HOLD, TAX_COLLECTOR_MONEY_X);
    const city_player *player = city_player_current();
    const building *palace = building_get(player->palace_building_id);
    int32_t palace_worker_percentage = calc_percentage(palace->num_workers, building_model_table[palace->type].laborers);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BUILDING_INFO_TEXT_Y);
    } else {
        window_building_draw_text(
            LANG_GROUP_TAX_COLLECTOR, tax_collector_text(player, palace_worker_percentage), building_info_y + BUILDING_INFO_SECOND_TEXT_Y);
    }
    window_building_draw_workers_panel();
}

void window_building_draw_mansion(void)
{
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_MANSION];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_BUILDING_TYPES, building_info_building()->type);
    draw_money_icon();
    draw_money_kept(LANG_GROUP_TAX_COLLECTOR, TAX_COLLECTOR_TEXT_VAULTS_HOLD, MANSION_MONEY_X, MANSION_MONEY_Y);
    window_building_draw_text(LANG_GROUP_MANSION, BUILDING_TEXT_DESCRIPTION, text_y_from_bottom(MANSION_DESCRIPTION_FROM_BOTTOM));
}

void window_building_draw_mansion_foreground(void)
{
    const city_player *player = city_player_current();
    int32_t x = building_info_x + SALARY_TEXT_X;
    int32_t y = building_info_y + SALARY_TEXT_Y;
    button_border_draw_ui(building_info_x + SALARY_BUTTON_X, building_info_y + SALARY_BUTTON_Y, SALARY_BUTTON_WIDTH, SALARY_BUTTON_HEIGHT,
        focus_button_id == SALARY_BUTTON_FOCUS, building_info_x, building_info_y);
    g_text_width = 0;
    lang_text_draw_ui(LANG_GROUP_MANSION, player->salary_rank + MANSION_TEXT_FIRST_SALARY_RANK, x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    text_draw_number_ui(player->salary_amount, NUMBER_PREFIX_NONE, postfix_none, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
    lang_text_draw_ui(LANG_GROUP_MANSION, MANSION_TEXT_DEBEN_PER_MONTH, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}
