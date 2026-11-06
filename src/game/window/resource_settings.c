// The trade advisor's resource settings window (a resource's industry on or off, its trade state none / import /
// export, the trade amount, the stockpile), its buttons, and the trade advisor's scroll bar input. Straw's industry is
// grain's; import and export need a trade partner and an open route.

#include "window/resource_settings.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "empire/trade_status.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "platform/platform_dialog.h"
#include "text/lang.h"
#include "text/text_lookup.h"
#include "window/main_menu.h"
#include "window/message_box.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum resource_settings_text {
    RESOURCE_SETTINGS_NO_INDUSTRIES = 7,
    RESOURCE_SETTINGS_ONE_WORKING = 8,
    RESOURCE_SETTINGS_MANY_WORKING = 9,
    RESOURCE_SETTINGS_ONE_MOTHBALLED = 10,
    RESOURCE_SETTINGS_MANY_MOTHBALLED = 11,
    RESOURCE_SETTINGS_WORKING_AND = 12,
    RESOURCE_SETTINGS_ONE_IDLE = 13,
    RESOURCE_SETTINGS_MANY_IDLE = 14,
    RESOURCE_SETTINGS_IN_STOCK = 15,
    RESOURCE_SETTINGS_INDUSTRY_ON = 16,
    RESOURCE_SETTINGS_INDUSTRY_OFF = 17,
    RESOURCE_SETTINGS_IMPORTING = 19,
    RESOURCE_SETTINGS_EXPORTING = 20,
    RESOURCE_SETTINGS_NO_INDUSTRY_POSSIBLE = 25,
    RESOURCE_SETTINGS_STOCKPILING = 26,
    RESOURCE_SETTINGS_STOCKPILING_HINT = 27,
    RESOURCE_SETTINGS_NOT_STOCKPILING = 28,
    RESOURCE_SETTINGS_NOT_STOCKPILING_HINT = 29,
    RESOURCE_SETTINGS_IMPORT_NEEDS_ROUTE = 34,
    RESOURCE_SETTINGS_EXPORT_NEEDS_ROUTE = 35,
    RESOURCE_SETTINGS_NOT_IMPORTING = 39,
    RESOURCE_SETTINGS_NOT_EXPORTING = 40,
    RESOURCE_SETTINGS_NOBODY_SELLS = 41,
    RESOURCE_SETTINGS_NOBODY_BUYS = 42,
    RESOURCE_SETTINGS_IMPORTING_AS_NEEDED = 43,
    RESOURCE_SETTINGS_EXPORTING_SURPLUS = 44,
};

enum resource_settings_layout {
    RESOURCE_LIST_ROWS = 15,
    RESOURCE_SCROLLBAR_HEIGHT = 256,
    RESOURCE_AMOUNT_GROUP = 8,
    RESOURCE_AMOUNT_BLOCKS = 56,
    TRADE_BUTTON_IMPORT = 1,
    TRADE_BUTTON_EXPORT = 2,
    TRADE_AMOUNT_UP = 0,
    TRADE_AMOUNT_DOWN = 1,
    TRADE_AMOUNT_STEP = 100,
    TRADE_CYCLE_STATES = 3,
};

static int32_t resource_counted_in_blocks(int32_t resource)
{
    return resource == RESOURCE_STONE || resource == RESOURCE_LIMESTONE || resource == RESOURCE_GRANITE || resource == RESOURCE_SANDSTONE ||
        resource == RESOURCE_MARBLE;
}

static int32_t resource_counted_by_piece(int32_t resource)
{
    return resource == RESOURCE_CHARIOTS || resource == RESOURCE_WEAPONS || resource == RESOURCE_DEBEN;
}

static int32_t resource_industry(int32_t resource)
{
    return resource == RESOURCE_STRAW ? RESOURCE_GRAIN : resource;
}

int32_t window_advisor_trade_handle_scrollbar(void)
{
    int32_t count = city_resources.count;
    if (count <= RESOURCE_LIST_ROWS || mouse_left_down == 0) {
        return 0;
    }
    int32_t max_scroll = count - RESOURCE_LIST_ROWS;
    if (mouse_x < (advisor_x + 592) || mouse_x > (advisor_x + 620)) {
        return 0;
    }
    if (mouse_y < (advisor_y + 78) || mouse_y > (advisor_y + 366)) {
        return 0;
    }
    int32_t dy = mouse_y - advisor_y - 78;
    if (dy > RESOURCE_SCROLLBAR_HEIGHT) {
        dy = RESOURCE_SCROLLBAR_HEIGHT;
    }
    int32_t percent = calc_percentage(dy, RESOURCE_SCROLLBAR_HEIGHT);
    int32_t scroll = calc_adjust_with_percentage(max_scroll, percent);
    window_redraw_request = 2;
    city_player_current()->resource_list_scroll = scroll + 1;
    return 1;
}

void window_resource_settings_help(void)
{
    window_message_dialog_show(MESSAGE_DIALOG_TRADE_RESOURCE_HELP, 1, 0);
    window_redraw_request = 1;
}

void window_resource_settings_back(void)
{
    window_current_id = WINDOW_ADVISORS;
    window_redraw_request = 1;
}

static void draw_text(int32_t text, int32_t x, int32_t y)
{
    lang_text_draw_ui(LANG_GROUP_RESOURCE_SETTINGS, text, x, y, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK, ui_origin_x, ui_origin_y);
}

static void draw_text_centered(int32_t text, int32_t x, int32_t y, int32_t width)
{
    lang_text_draw_centered_ui(LANG_GROUP_RESOURCE_SETTINGS, text, x, y, width, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK, ui_origin_x, ui_origin_y);
}

static void draw_number(int32_t value, int32_t x, int32_t y)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, postfix_none, x, y, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK, ui_origin_x, ui_origin_y);
}

static void draw_trade_amount(int32_t x_blocks, int32_t x_number)
{
    int32_t resource = trade_resource_index;
    int32_t amount = city_player_current()->trading_amount[resource];
    if (resource_counted_in_blocks(resource)) {
        lang_text_draw_amount(
            RESOURCE_AMOUNT_GROUP, RESOURCE_AMOUNT_BLOCKS, amount, g_text_width + advisor_x + x_blocks, advisor_y + 221, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
    } else if (resource_counted_by_piece(resource)) {
        draw_number(amount, g_text_width + advisor_x + x_number, advisor_y + 221);
    } else {
        draw_number(amount * TRADE_AMOUNT_STEP, g_text_width + advisor_x + x_number, advisor_y + 221);
    }
}

static int32_t import_text(int32_t status, uint8_t *button_on)
{
    if (status == TRADE_STATUS_IMPORT) {
        return RESOURCE_SETTINGS_IMPORTING;
    }
    if (status == TRADE_STATUS_IMPORT_AS_NEEDED) {
        return RESOURCE_SETTINGS_IMPORTING_AS_NEEDED;
    }
    if (empire_can_import_resource(trade_resource_index)) {
        return RESOURCE_SETTINGS_NOT_IMPORTING;
    }
    *button_on = 0;
    return empire_can_import_resource_potentially(trade_resource_index) ? RESOURCE_SETTINGS_IMPORT_NEEDS_ROUTE : RESOURCE_SETTINGS_NOBODY_SELLS;
}

static int32_t export_text(int32_t status, uint8_t *button_on)
{
    if (status == TRADE_STATUS_EXPORT) {
        return RESOURCE_SETTINGS_EXPORTING;
    }
    if (status != TRADE_STATUS_NONE && status != TRADE_STATUS_IMPORT && status != TRADE_STATUS_IMPORT_AS_NEEDED) {
        return RESOURCE_SETTINGS_EXPORTING_SURPLUS;
    }
    if (empire_can_export_resource(trade_resource_index)) {
        return RESOURCE_SETTINGS_NOT_EXPORTING;
    }
    *button_on = 0;
    return empire_can_export_resource_potentially(trade_resource_index) ? RESOURCE_SETTINGS_EXPORT_NEEDS_ROUTE : RESOURCE_SETTINGS_NOBODY_BUYS;
}

static void draw_industry(int32_t industry, const city_player *player)
{
    int32_t x = advisor_x + 98;
    int32_t y = advisor_y + 172;
    g_text_width = 0;
    if (!empire_can_produce_resource(industry)) {
        draw_text(RESOURCE_SETTINGS_NO_INDUSTRY_POSSIBLE, x, y);
        return;
    }
    int32_t total = building_count_industry_total[industry];
    int32_t active = building_count_industry_active[industry];
    if (total <= 0) {
        draw_text(RESOURCE_SETTINGS_NO_INDUSTRIES, x, y);
    } else if (player->mothballed[trade_resource_index] == 1) {
        draw_number(total, x, y);
        draw_text(total == 1 ? RESOURCE_SETTINGS_ONE_MOTHBALLED : RESOURCE_SETTINGS_MANY_MOTHBALLED, g_text_width + x, y);
    } else if (total == active) {
        draw_number(total, x, y);
        draw_text(total == 1 ? RESOURCE_SETTINGS_ONE_WORKING : RESOURCE_SETTINGS_MANY_WORKING, g_text_width + x, y);
    } else {
        draw_number(active, x, y);
        draw_text(RESOURCE_SETTINGS_WORKING_AND, g_text_width + x, y);
        draw_number(total - active, g_text_width + x, y);
        draw_text(active == 1 ? RESOURCE_SETTINGS_ONE_IDLE : RESOURCE_SETTINGS_MANY_IDLE, g_text_width + x, y);
    }
}

static void draw_stock(const city_player *player)
{
    int32_t resource = trade_resource_index;
    int32_t stored = player->storage_stored[resource];
    int32_t x = advisor_x + 98;
    int32_t y = advisor_y + 192;
    g_text_width = 0;
    if (resource_counted_in_blocks(resource)) {
        lang_text_draw_amount(RESOURCE_AMOUNT_GROUP, RESOURCE_AMOUNT_BLOCKS, stored / 100, x, y, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
    } else {
        draw_number(resource_counted_by_piece(resource) ? stored / 100 : stored, x, y);
    }
    draw_text(RESOURCE_SETTINGS_IN_STOCK, g_text_width + x, y);
}

static void draw_trade_state(int32_t status, int32_t text, int32_t shown_status, int32_t text_x, int32_t centered_x, int32_t x_blocks, int32_t x_number)
{
    if (status == shown_status) {
        g_text_width = 0;
        draw_text(text, advisor_x + text_x, advisor_y + 221);
        draw_trade_amount(x_blocks, x_number);
    } else {
        draw_text_centered(text, advisor_x + centered_x, advisor_y + 221, 200);
    }
}

void window_resource_settings_draw_foreground(void)
{
    city_player *player = city_player_current();
    uint8_t import_on = 1;
    uint8_t export_on = 1;
    player->resource_settings_step = 1;
    outer_panel_draw(advisor_x + 16, advisor_y + 128, 38, 15);
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + trade_resource_index, advisor_x + 26, advisor_y + 136);
    lang_text_draw_ui(LANG_GROUP_RESOURCE_NAMES, trade_resource_index, advisor_x + 60, advisor_y + 137, image_group(IMAGE_GROUP_FONT_LARGE), COLOR_BLACK, ui_origin_x, ui_origin_y);
    int32_t status = player->trade_status[trade_resource_index];
    int32_t import = import_text(status, &import_on);
    int32_t export = export_text(status, &export_on);
    int32_t industry = resource_industry(trade_resource_index);
    draw_industry(industry, player);
    draw_stock(player);
    if (import_on) {
        button_border_draw_ui(advisor_x + 26, advisor_y + 212, 291, 30, focus_button_id_secondary == TRADE_BUTTON_IMPORT ? COLOR_RED : 0, ui_origin_x, ui_origin_y);
    }
    if (export_on) {
        button_border_draw_ui(advisor_x + 321, advisor_y + 212, 291, 30, focus_button_id_secondary == TRADE_BUTTON_EXPORT ? COLOR_RED : 0, ui_origin_x, ui_origin_y);
    }
    draw_trade_state(player->trade_status[trade_resource_index], import, TRADE_STATUS_IMPORT, 31, 38, 24, 26);
    draw_trade_state(player->trade_status[trade_resource_index], export, TRADE_STATUS_EXPORT, 331, 334, 324, 326);
    if (building_count_industry_total[industry] > 0) {
        button_border_draw_ui(advisor_x + 98, advisor_y + 250, 432, 30, focus_button_id != 0, ui_origin_x, ui_origin_y);
        int32_t text = player->mothballed[trade_resource_index] == 0 ? RESOURCE_SETTINGS_INDUSTRY_ON : RESOURCE_SETTINGS_INDUSTRY_OFF;
        draw_text_centered(text, advisor_x + 114, advisor_y + 259, 400);
    }
    button_border_draw_ui(advisor_x + 98, advisor_y + 288, 432, 50, focus_button_id_tertiary != 0, ui_origin_x, ui_origin_y);
    if (player->stockpiled[trade_resource_index] == 0) {
        draw_text_centered(RESOURCE_SETTINGS_NOT_STOCKPILING, advisor_x + 114, advisor_y + 296, 400);
        draw_text_centered(RESOURCE_SETTINGS_NOT_STOCKPILING_HINT, advisor_x + 114, advisor_y + 316, 400);
    } else {
        draw_text_centered(RESOURCE_SETTINGS_STOCKPILING, advisor_x + 114, advisor_y + 296, 400);
        draw_text_centered(RESOURCE_SETTINGS_STOCKPILING_HINT, advisor_x + 114, advisor_y + 316, 400);
    }
}

void window_resource_settings_toggle_industry(void)
{
    int32_t resource = trade_resource_index;
    if (building_count_industry_total[resource_industry(resource)] > 0) {
        city_player *player = city_player_current();
        player->mothballed[resource] = (int16_t) (player->mothballed[resource] == 0);
    }
}

void window_resource_settings_cycle_trade(void)
{
    city_player *player = city_player_current();
    int16_t *status = &player->trade_status[trade_resource_index];
    (*status)++;
    if (*status >= TRADE_CYCLE_STATES) {
        *status = TRADE_STATUS_NONE;
    }
    if (*status == TRADE_STATUS_IMPORT && !empire_can_import_resource(trade_resource_index)) {
        *status = TRADE_STATUS_EXPORT;
    }
    if (*status == TRADE_STATUS_EXPORT) {
        if (!empire_can_export_resource(trade_resource_index)) {
            *status = TRADE_STATUS_NONE;
        }
        player->stockpiled[trade_resource_index] = 0;
    }
}

static void show_trade_error(int32_t title, int32_t text)
{
    const char *title_text = lang_get_string(LANG_GROUP_POPUP, title);
    const char *message = lang_get_string(LANG_GROUP_POPUP, text);
    window_message_box_show(title_text, message, PLATFORM_BOX_OK);
}

static void trade_keep_current_amount(city_player *player)
{
    empire_update_export_keep();
    player->trading_amount[trade_resource_index] = (int16_t) (trade_export_keep[trade_resource_index] / TRADE_AMOUNT_STEP);
}

void window_resource_settings_toggle_import(void)
{
    if (!empire_can_import_resource_potentially(trade_resource_index)) {
        show_trade_error(POPUP_RESOURCE_NOT_SOLD, POPUP_RESOURCE_NOT_SOLD + 1);
        return;
    }
    if (!empire_can_import_resource(trade_resource_index)) {
        show_trade_error(POPUP_TRADE_ROUTE_CLOSED, POPUP_TRADE_ROUTE_CLOSED_IMPORT_TEXT);
        return;
    }
    city_player *player = city_player_current();
    int16_t *status = &player->trade_status[trade_resource_index];
    switch (*status) {
        case TRADE_STATUS_IMPORT:
            *status = TRADE_STATUS_NONE;
            return;
        case TRADE_STATUS_IMPORT_AS_NEEDED:
            *status = TRADE_STATUS_IMPORT;
            trade_keep_current_amount(player);
            return;
        case TRADE_STATUS_NONE:
        case TRADE_STATUS_EXPORT:
        case TRADE_STATUS_EXPORT_SURPLUS:
            *status = TRADE_STATUS_IMPORT_AS_NEEDED;
            return;
        default:
            return;
    }
}

void window_resource_settings_toggle_export(void)
{
    if (!empire_can_export_resource_potentially(trade_resource_index)) {
        show_trade_error(POPUP_RESOURCE_NOT_BOUGHT, POPUP_RESOURCE_NOT_BOUGHT + 1);
        return;
    }
    if (!empire_can_export_resource(trade_resource_index)) {
        show_trade_error(POPUP_TRADE_ROUTE_CLOSED, POPUP_TRADE_ROUTE_CLOSED_EXPORT_TEXT);
        return;
    }
    city_player *player = city_player_current();
    int16_t *status = &player->trade_status[trade_resource_index];
    switch (*status) {
        case TRADE_STATUS_EXPORT:
            *status = TRADE_STATUS_NONE;
            return;
        case TRADE_STATUS_EXPORT_SURPLUS:
            *status = TRADE_STATUS_EXPORT;
            trade_keep_current_amount(player);
            return;
        case TRADE_STATUS_NONE:
        case TRADE_STATUS_IMPORT:
        case TRADE_STATUS_IMPORT_AS_NEEDED:
            player->stockpiled[trade_resource_index] = 0;
            *status = TRADE_STATUS_EXPORT_SURPLUS;
            return;
        default:
            return;
    }
}

void window_resource_settings_change_trade_amount(void)
{
    int16_t *amount = &city_player_current()->trading_amount[trade_resource_index];
    if (button_parameter == TRADE_AMOUNT_DOWN) {
        (*amount)--;
    } else if (button_parameter == TRADE_AMOUNT_UP) {
        (*amount)++;
    }
    int32_t capped = city_resource_cap_amount(trade_resource_index, *amount * TRADE_AMOUNT_STEP);
    *amount = (int16_t) (capped / TRADE_AMOUNT_STEP);
}

void window_resource_settings_toggle_stockpile(void)
{
    city_player *player = city_player_current();
    if (player->stockpiled[trade_resource_index] == 0) {
        player->stockpiled[trade_resource_index] = 1;
        int16_t status = player->trade_status[trade_resource_index];
        if (status == TRADE_STATUS_EXPORT || status == TRADE_STATUS_EXPORT_SURPLUS) {
            player->trade_status[trade_resource_index] = TRADE_STATUS_NONE;
        }
    } else {
        player->stockpiled[trade_resource_index] = 0;
    }
    window_redraw_request = 1;
}
