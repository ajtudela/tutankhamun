// The building info panels of the bazaar, the granary and the storage yard, their cart pushers and their orders panels
// (the goods they accept, get or buy, a scrolled list for the storage yard), the orders button, and the storage yard's
// space state (free, getting full, full at 3200 loads).
//
// What the storage panels draw is not always what they hold:
// - the granary draws its fifth food over its third;
// - the storage yard's down arrow takes the mouse 6 pixels above where it is drawn;
// - an empty entry of the resource list repeats in the remaining rows.

#include "window/building_info_storage.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "graphics/arrow_buttons.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/options_windows.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum bazaar_text {
    BAZAAR_TEXT_NOT_WORKING = 2,
    BAZAAR_TEXT_NO_FOOD = 4,
    BAZAAR_TEXT_SPECIAL_ORDERS = 6,
    BAZAAR_TEXT_INSTRUCTIONS = 7,
    BAZAAR_TEXT_BUY = 8,
    BAZAAR_TEXT_DONT_BUY = 9,
    BAZAAR_TEXT_TRADER_WAITING = 10,
    BAZAAR_TEXT_TRADER_OUT = 11,
    BAZAAR_TEXT_TRADER_RETURNING = 12,
};

enum granary_text {
    GRANARY_TEXT_STORING = 2,
    GRANARY_TEXT_SPACE_FOR = 3,
    GRANARY_TEXT_NOT_NEEDED = 4,
    GRANARY_TEXT_SPECIAL_ORDERS = 5,
    GRANARY_TEXT_INSTRUCTIONS = 6,
};

enum storage_yard_text {
    STORAGE_YARD_TEXT_SPECIAL_ORDERS = 2,
    STORAGE_YARD_TEXT_INSTRUCTIONS = 3,
    STORAGE_YARD_TEXT_ACCEPT_NONE = 7,
    STORAGE_YARD_TEXT_DONT_ACCEPT = 8,
    STORAGE_YARD_TEXT_FULL = 13,
    STORAGE_YARD_TEXT_GETTING_FULL = 14,
    STORAGE_YARD_TEXT_CART_READY = 15,
    STORAGE_YARD_TEXT_CART_DELIVERING = 16,
    STORAGE_YARD_TEXT_CART_RETURNING = 17,
    STORAGE_YARD_TEXT_ACCEPT = 18,
    STORAGE_YARD_TEXT_GET = 19,
    STORAGE_YARD_TEXT_EMPTY_FOOD = 20,
    STORAGE_YARD_TEXT_EMPTY = 21,
    STORAGE_YARD_TEXT_EMPTY_YARD = 22,
    STORAGE_YARD_TEXT_BLOCK_OF = 23,
    STORAGE_YARD_TEXT_BLOCKS_OF = 24,
    STORAGE_YARD_TEXT_QUARTER = 25,
    STORAGE_YARD_TEXT_HALF = 26,
    STORAGE_YARD_TEXT_THREE_QUARTERS = 27,
    STORAGE_YARD_TEXT_ALL = 28,
    STORAGE_YARD_TEXT_YARD = 29,
    STORAGE_YARD_TEXT_GRANARY = 30,
    STORAGE_YARD_TEXT_MAXIMUM = 31,
    STORAGE_YARD_TEXT_FILL = 32,
    STORAGE_YARD_TEXT_CART_GETTING = 33,
};

enum storage_order_limit {
    STORAGE_ORDER_LIMIT_QUARTER = 800,
    STORAGE_ORDER_LIMIT_HALF = 1600,
    STORAGE_ORDER_LIMIT_THREE_QUARTERS = 2400,
};

enum storage_layout {
    BAZAAR_TEXT_Y = 56,
    BAZAAR_NO_FOOD_Y = 45,
    BAZAAR_GOODS_X = 32,
    BAZAAR_GOODS_SPACING = 110,
    BAZAAR_AMOUNT_DX = 32,
    BAZAAR_FOOD_Y = 84,
    BAZAAR_GOODS_ICON_Y = 104,
    BAZAAR_GOODS_TEXT_Y = 110,
    BAZAAR_WORKERS_PANEL_Y = 128,
    BAZAAR_WORKERS_PANEL_BLOCKS = 5,
    BAZAAR_EMPLOYMENT_Y = 142,
    STORAGE_WALKER_ICON_X = 32,
    STORAGE_WALKER_TEXT_X = 64,
    BAZAAR_WALKER_Y = 184,
    BAZAAR_WALKER_RETURNING_ICON_Y = 190,
    ORDERS_BUTTON_X = 100,
    ORDERS_BUTTON_FROM_BOTTOM = 34,
    ORDERS_BUTTON_HEIGHT = 20,
    ORDERS_BUTTON_TEXT_X = 80,
    ORDERS_BUTTON_TEXT_FROM_BOTTOM = 30,
    ORDERS_PANEL_Y = 32,
    ORDERS_PANEL_WIDTH_BLOCKS = 29,
    ORDERS_PANEL_EXTRA_BLOCKS = 7,
    ORDERS_TITLE_Y = 42,
    ORDERS_INNER_PANEL_Y = 74,
    BAZAAR_ORDERS_FIRST_ROW_Y = 78,
    ORDERS_ROW_HEIGHT = 20,
    ORDERS_ICON_X = 32,
    ORDERS_NAME_X = 72,
    BAZAAR_ORDERS_RIGHT_ICON_X = 408,
    BAZAAR_ORDERS_NAME_DY = 4,
    BAZAAR_ORDERS_STATE_DY = 5,
    BAZAAR_ORDERS_BUY_X = 260,
    BAZAAR_ORDERS_DONT_BUY_X = 240,
    ORDERS_ROW_FOCUS_X = 180,
    BAZAAR_ORDERS_FOCUS_WIDTH = 210,
    ORDERS_ROW_FOCUS_HEIGHT = 20,
    ORDERS_FIRST_ROW_Y = 83,
    ORDERS_ROW_ICON_DY = -5,
    ORDERS_ROW_NAME_DY = -1,
    ORDERS_ROW_BUTTONS_X = 200,
    ORDERS_ROW_BUTTONS_DY = -3,
    ORDERS_ROW_BUTTONS = 2,
    ORDERS_ROW_BUTTONS_TOOLTIP_BASE = 10,
    ORDERS_ACCEPT_NONE_X = 80,
    GRANARY_ORDERS_ACCEPT_NONE_DY = -8,
    ORDERS_ACCEPT_NONE_TEXT_DY = 4,
    GRANARY_ORDERS_FOCUS_WIDTH = 250,
    GRANARY_ORDERS_STATE_X = 250,
    STORAGE_YARD_ORDERS_ICON_X = 25,
    STORAGE_YARD_ORDERS_NAME_X = 60,
    STORAGE_YARD_ORDERS_FOCUS_WIDTH = 230,
    STORAGE_YARD_ORDERS_STATE_X = 240,
    STORAGE_YARD_ORDERS_DOWN_FOCUS_FROM_BOTTOM = 78,
    STORAGE_YARD_ORDERS_DOWN_FROM_BOTTOM = 72,
    STORAGE_YARD_ORDERS_UP_X = 390,
    STORAGE_YARD_ORDERS_UP_Y = 82,
    STORAGE_YARD_ORDERS_TRACK_X = 411,
    STORAGE_YARD_ORDERS_TRACK_WIDTH_BLOCKS = 2,
    STORAGE_YARD_ORDERS_SHORT_PANEL_BLOCKS = 27,
    STORAGE_YARD_ORDERS_SHORT_TRACK_BLOCKS = 16,
    STORAGE_YARD_ORDERS_TRACK_BLOCKS = 24,
    GRANARY_TEXT_Y = 40,
    GRANARY_STORING_X = 34,
    GRANARY_SPACE_X = 220,
    GRANARY_FOOD_LEFT_X = 34,
    GRANARY_FOOD_RIGHT_X = 240,
    GRANARY_FOOD_LEFT_COLUMN = 2,
    GRANARY_FOOD_TOP_Y = 68,
    GRANARY_FOOD_BOTTOM_Y = 92,
    GRANARY_FOOD_TEXT_DX = 34,
    GRANARY_FOOD_TEXT_DY = 7,
    GRANARY_WORKERS_PANEL_Y = 163,
    STORAGE_WORKERS_PANEL_BLOCKS = 5,
    GRANARY_WALKER_Y = 200,
    STORAGE_YARD_TEXT_Y = 56,
    STORAGE_YARD_GOODS_LEFT_X = 10,
    STORAGE_YARD_GOODS_RIGHT_X = 220,
    STORAGE_YARD_GOODS_Y = 45,
    STORAGE_YARD_GOODS_ROW_HEIGHT = 30,
    STORAGE_YARD_GOODS_PER_COLUMN = 4,
    STORAGE_YARD_AMOUNT_DX = 24,
    STORAGE_YARD_AMOUNT_DY = 7,
    STORAGE_YARD_BLOCKS_NAME_DY = 22,
    STORAGE_YARD_WORKERS_PANEL_Y = 183,
    STORAGE_YARD_EMPLOYMENT_Y = 188,
    STORAGE_YARD_WALKER_ICON_Y = 228,
    STORAGE_YARD_WALKER_TEXT_Y = 231,
    STORAGE_YARD_WARNING_FROM_BOTTOM = 85,
    STORAGE_YARD_FIRST_ROW_LAST_FROM_END = 15,
    STORAGE_YARD_FIRST_ROW_MIN_COUNT = 16,
};

enum storage_yard_space {
    STORAGE_YARD_PARTS = 8,
    STORAGE_YARD_FULL_LOADS = 3200,
    STORAGE_YARD_UNITS_PER_BLOCK = 100,
    STORAGE_YARD_PART_LOADS = 400,
};

enum {
    LANG_RESOURCE_PLURAL_NAMES = 54,
    PANEL_BUTTON_SCROLL_THUMB = 39,
};

static const int32_t bazaar_goods[] = { RESOURCE_POTTERY, RESOURCE_LUXURY_GOODS, RESOURCE_LINEN, RESOURCE_BEER };

static int32_t walker_alive(int32_t figure_id)
{
    return figure_id != 0 && figure_get(figure_id)->state == FIGURE_STATE_ALIVE;
}

static void draw_resource_icon(int32_t resource, int32_t x, int32_t y)
{
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource, x, y);
}

static void draw_number(int32_t value, int32_t x, int32_t y, int32_t font)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, postfix_none, x, y, font, 0, building_info_x, building_info_y);
}

static void draw_label(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(group, index, x, y, font, 0, building_info_x, building_info_y);
}

static void draw_walker_text(int32_t group, int32_t index, int32_t x, int32_t y, int32_t width_blocks)
{
    lang_text_draw_multiline_ui(group, index, x, y, width_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
}

static int32_t bazaar_has_food(const building *b)
{
    int32_t total = 0;
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] != RESOURCE_NONE) {
            total += b->data.market.inventory[house_food_inventory_index(city_foods.list[i])];
        }
    }
    return city_foods.count >= 1 && total != 0;
}

static void draw_bazaar_stock(const building *b)
{
    if (!bazaar_has_food(b)) {
        window_building_draw_text(LANG_GROUP_BAZAAR, BAZAAR_TEXT_NO_FOOD, building_info_y + BAZAAR_NO_FOOD_Y);
    }
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        if (resource != RESOURCE_NONE) {
            int32_t x = building_info_x + BAZAAR_GOODS_X + i * BAZAAR_GOODS_SPACING;
            draw_resource_icon(resource, x, building_info_y + BAZAAR_FOOD_Y);
            draw_number(b->data.market.inventory[house_food_inventory_index(resource)], x + BAZAAR_AMOUNT_DX, building_info_y + BAZAAR_FOOD_Y, image_group(IMAGE_GROUP_FONT_BLACK));
        }
    }
    for (int32_t i = 0; i < (int32_t) (sizeof(bazaar_goods) / sizeof(bazaar_goods[0])); i++) {
        int32_t x = building_info_x + BAZAAR_GOODS_X + i * BAZAAR_GOODS_SPACING;
        draw_resource_icon(bazaar_goods[i], x, building_info_y + BAZAAR_GOODS_ICON_Y);
        draw_number(
            b->data.market.inventory[house_food_inventory_index(bazaar_goods[i])], x + BAZAAR_AMOUNT_DX, building_info_y + BAZAAR_GOODS_TEXT_Y, image_group(IMAGE_GROUP_FONT_BLACK));
    }
}

static void draw_bazaar_trader(const building *b)
{
    if (walker_alive(b->figure_id)) {
        const figure *trader = figure_get(b->figure_id);
        int32_t returning = trader->action_state == FIGURE_ACTION_ROAMER_RETURN;
        int32_t icon_y = returning ? BAZAAR_WALKER_RETURNING_ICON_Y : BAZAAR_WALKER_Y;
        draw_resource_icon(trader->resource_id, building_info_x + STORAGE_WALKER_ICON_X, building_info_y + icon_y);
        draw_walker_text(LANG_GROUP_BAZAAR, returning ? BAZAAR_TEXT_TRADER_RETURNING : BAZAAR_TEXT_TRADER_OUT, building_info_x + STORAGE_WALKER_TEXT_X,
            building_info_y + BAZAAR_WALKER_Y, info_w_blocks - 6);
        return;
    }
    if (b->num_workers > 0) {
        draw_walker_text(
            LANG_GROUP_BAZAAR, BAZAAR_TEXT_TRADER_WAITING, building_info_x + BUILDING_INFO_TEXT_X, building_info_y + BAZAAR_WALKER_Y, info_w_blocks - 4);
    }
}

void window_building_draw_bazaar(void)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_BAZAAR];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_BAZAAR, BUILDING_TEXT_TITLE);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + BAZAAR_TEXT_Y);
    } else if (b->num_workers < 1) {
        window_building_draw_text(LANG_GROUP_BAZAAR, BAZAAR_TEXT_NOT_WORKING, building_info_y + BAZAAR_TEXT_Y);
    } else {
        draw_bazaar_stock(b);
    }
    window_building_draw_inner_panel(BAZAAR_WORKERS_PANEL_Y, BAZAAR_WORKERS_PANEL_BLOCKS);
    window_building_draw_employment(building_info_y + BAZAAR_EMPLOYMENT_Y);
    draw_bazaar_trader(b);
}

static void draw_orders_button(int32_t group, int32_t index)
{
    button_border_draw_ui(building_info_x + ORDERS_BUTTON_X, (building_info_h * BUILDING_INFO_BLOCK) - ORDERS_BUTTON_FROM_BOTTOM + building_info_y,
        (info_w_blocks - 12) * BUILDING_INFO_BLOCK, ORDERS_BUTTON_HEIGHT, focus_button_id == 1, building_info_x, building_info_y);
    lang_text_draw_centered_ui(group, index, building_info_x + ORDERS_BUTTON_TEXT_X,
        (building_info_h * BUILDING_INFO_BLOCK) - ORDERS_BUTTON_TEXT_FROM_BOTTOM + building_info_y, (info_w_blocks - 10) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0,
        building_info_x, building_info_y);
}

void window_building_draw_bazaar_foreground(void)
{
    draw_orders_button(LANG_GROUP_BAZAAR, BAZAAR_TEXT_SPECIAL_ORDERS);
}

static int32_t orders_panel_blocks(int32_t rows)
{
    return calc_divide_round_up(rows * 4, 3);
}

static void draw_orders_background(int32_t help, int32_t group, int32_t index, int32_t rows)
{
    int32_t height_blocks = orders_panel_blocks(rows);
    info_help_id = building_info_help_messages[help];
    outer_panel_draw(building_info_x, ORDERS_PANEL_Y, ORDERS_PANEL_WIDTH_BLOCKS, height_blocks + ORDERS_PANEL_EXTRA_BLOCKS);
    lang_text_draw_centered_ui(
        group, index, building_info_x, ORDERS_TITLE_Y, info_w_blocks * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_LARGE), 0, building_info_x, building_info_y);
    inner_panel_draw_ui(building_info_x + BUILDING_INFO_BLOCK, ORDERS_INNER_PANEL_Y, info_w_blocks - 2, height_blocks, building_info_x, building_info_y);
}

static int32_t bazaar_goods_count(void)
{
    int32_t count = 0;
    for (int32_t i = 0; i < city_resources.count; i++) {
        if (resource_is_bazaar_good(city_resources.list[i]) != 0) {
            count++;
        }
    }
    return count;
}

void window_building_draw_bazaar_orders(void)
{
    draw_orders_background(BUILDING_INFO_HELP_BAZAAR, LANG_GROUP_BAZAAR, BAZAAR_TEXT_INSTRUCTIONS, bazaar_goods_count());
}

static void draw_bazaar_order_row(const building_storage *storage, int32_t resource, int32_t row, int32_t y)
{
    draw_resource_icon(resource, building_info_x + ORDERS_ICON_X, y);
    draw_resource_icon(resource, building_info_x + BAZAAR_ORDERS_RIGHT_ICON_X, y);
    draw_label(LANG_GROUP_RESOURCE_NAMES, resource, building_info_x + ORDERS_NAME_X, y + BAZAAR_ORDERS_NAME_DY, image_group(IMAGE_GROUP_FONT_WHITE));
    if (focus_button_id_secondary == row) {
        button_border_draw_ui(
            building_info_x + ORDERS_ROW_FOCUS_X, y, BAZAAR_ORDERS_FOCUS_WIDTH, ORDERS_ROW_FOCUS_HEIGHT, 1, building_info_x, building_info_y);
    }
    uint8_t state = storage->resource_state[resource];
    if (state == STORAGE_STATE_ACCEPT) {
        draw_label(LANG_GROUP_BAZAAR, BAZAAR_TEXT_BUY, building_info_x + BAZAAR_ORDERS_BUY_X, y + BAZAAR_ORDERS_STATE_DY, image_group(IMAGE_GROUP_FONT_WHITE));
    } else if (state == STORAGE_STATE_REFUSE) {
        draw_label(LANG_GROUP_BAZAAR, BAZAAR_TEXT_DONT_BUY, building_info_x + BAZAAR_ORDERS_DONT_BUY_X, y + BAZAAR_ORDERS_STATE_DY, image_group(IMAGE_GROUP_FONT_RED));
    }
}

void window_building_draw_bazaar_orders_foreground(void)
{
    const building_storage *storage = storage_get(building_info_building()->storage_id);
    int32_t row = 1;
    int32_t y = BAZAAR_ORDERS_FIRST_ROW_Y;
    for (int32_t i = 0; i < city_resources.count; i++) {
        int32_t resource = city_resources.list[i];
        if (resource == RESOURCE_NONE || resource_is_bazaar_good(resource) == 0) {
            continue;
        }
        draw_bazaar_order_row(storage, resource, row, y);
        row++;
        y += ORDERS_ROW_HEIGHT;
    }
    window_redraw_request = 1;
}

static int32_t cart_text(const figure *cart)
{
    int32_t action = cart->action_state;
    if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
        action = cart->action_state_before_attack;
    }
    switch (action) {
        case FIGURE_ACTION_STORAGE_CART_RETURNING_EMPTY:
        case FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_FOOD:
        case FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_GOODS:
            return STORAGE_YARD_TEXT_CART_RETURNING;
        case FIGURE_ACTION_STORAGE_CART_TO_YARD_GETTING:
        case FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING:
        case FIGURE_ACTION_STORAGE_CART_TO_GRANARY_GETTING:
        case FIGURE_ACTION_STORAGE_CART_AT_GRANARY_GETTING:
        case FIGURE_ACTION_STORAGE_CART_TO_YARD_GETTING_FOOD:
        case FIGURE_ACTION_STORAGE_CART_AT_YARD_GETTING_FOOD:
            return STORAGE_YARD_TEXT_CART_GETTING;
        default:
            return STORAGE_YARD_TEXT_CART_DELIVERING;
    }
}

static void draw_cart_walker(const building *b, int32_t second_cart_id, int32_t icon_y, int32_t text_y)
{
    int32_t cart_id = b->figure_id;
    if (!walker_alive(cart_id)) {
        cart_id = second_cart_id;
        if (!walker_alive(cart_id)) {
            if (b->num_workers > 0) {
                draw_walker_text(
                    LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_CART_READY, building_info_x + BUILDING_INFO_TEXT_X, building_info_y + text_y, info_w_blocks - 4);
            }
            return;
        }
    }
    const figure *cart = figure_get(cart_id);
    draw_resource_icon(cart->resource_id, building_info_x + STORAGE_WALKER_ICON_X, building_info_y + icon_y);
    draw_walker_text(LANG_GROUP_STORAGE_YARD, cart_text(cart), building_info_x + STORAGE_WALKER_TEXT_X, building_info_y + text_y, info_w_blocks - 6);
}

static void draw_granary_stock(const building *b)
{
    int32_t stored = 0;
    for (int32_t resource = RESOURCE_MIN_FOOD; resource < RESOURCE_MAX_FOOD; resource++) {
        stored += b->data.granary.resource_stored[resource];
    }
    int32_t y = building_info_y + GRANARY_TEXT_Y;
    g_text_width = 0;
    draw_label(LANG_GROUP_GRANARY, GRANARY_TEXT_STORING, building_info_x + GRANARY_STORING_X, y, image_group(IMAGE_GROUP_FONT_BLACK));
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT_PERIOD, stored, g_text_width + GRANARY_STORING_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    g_text_width = 0;
    draw_label(LANG_GROUP_GRANARY, GRANARY_TEXT_SPACE_FOR, building_info_x + GRANARY_SPACE_X, y, image_group(IMAGE_GROUP_FONT_BLACK));
    lang_text_draw_amount(LANG_GROUP_AMOUNTS, AMOUNT_TEXT_UNIT_PERIOD, b->data.granary.resource_stored[RESOURCE_NONE],
        g_text_width + GRANARY_SPACE_X + building_info_x, y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        if (resource == RESOURCE_NONE) {
            continue;
        }
        int32_t x = building_info_x + ((i < GRANARY_FOOD_LEFT_COLUMN) ? GRANARY_FOOD_LEFT_X : GRANARY_FOOD_RIGHT_X);
        int32_t food_y = building_info_y + ((i % 2 == 0) ? GRANARY_FOOD_TOP_Y : GRANARY_FOOD_BOTTOM_Y);
        draw_resource_icon(resource, x, food_y);
        g_text_width = 0;
        draw_number(b->data.granary.resource_stored[resource], x + GRANARY_FOOD_TEXT_DX, food_y + GRANARY_FOOD_TEXT_DY, image_group(IMAGE_GROUP_FONT_BLACK));
        draw_label(LANG_GROUP_RESOURCE_NAMES, resource, g_text_width + GRANARY_FOOD_TEXT_DX + x, food_y + GRANARY_FOOD_TEXT_DY, image_group(IMAGE_GROUP_FONT_BLACK));
    }
}

void window_building_draw_granary(void)
{
    const building *b = building_info_building();
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_GRANARY];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_GRANARY, BUILDING_TEXT_TITLE);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + GRANARY_TEXT_Y);
    } else if (scenario_data.kingdom_supplies_grain == 0) {
        draw_granary_stock(b);
    } else {
        window_building_draw_text(LANG_GROUP_GRANARY, GRANARY_TEXT_NOT_NEEDED, building_info_y + GRANARY_TEXT_Y);
    }
    window_building_draw_inner_panel(GRANARY_WORKERS_PANEL_Y, STORAGE_WORKERS_PANEL_BLOCKS);
    window_building_draw_employment(building_info_y + GRANARY_WORKERS_PANEL_Y);
    draw_cart_walker(b, b->data.granary.second_cart_id, GRANARY_WALKER_Y, GRANARY_WALKER_Y);
}

void window_building_draw_granary_foreground(void)
{
    draw_orders_button(LANG_GROUP_GRANARY, GRANARY_TEXT_SPECIAL_ORDERS);
}

void window_building_draw_granary_orders(void)
{
    draw_orders_background(BUILDING_INFO_HELP_GRANARY, LANG_GROUP_GRANARY, GRANARY_TEXT_INSTRUCTIONS, city_foods.count);
}

static int32_t order_amount_text(int32_t limit, int32_t all_text)
{
    if (limit <= STORAGE_ORDER_LIMIT_QUARTER) {
        return STORAGE_YARD_TEXT_QUARTER;
    }
    if (limit <= STORAGE_ORDER_LIMIT_HALF) {
        return STORAGE_YARD_TEXT_HALF;
    }
    return (limit > STORAGE_ORDER_LIMIT_THREE_QUARTERS) ? all_text : STORAGE_YARD_TEXT_THREE_QUARTERS;
}

static void draw_order_limit(int32_t first_text, int32_t limit, int32_t all_text, int32_t storage_text, int32_t x, int32_t y)
{
    g_text_width = 0;
    draw_label(LANG_GROUP_STORAGE_YARD, first_text, x, y, image_group(IMAGE_GROUP_FONT_WHITE));
    draw_label(LANG_GROUP_STORAGE_YARD, order_amount_text(limit, all_text), g_text_width + x, y, image_group(IMAGE_GROUP_FONT_WHITE));
    if (limit <= STORAGE_ORDER_LIMIT_THREE_QUARTERS) {
        draw_label(LANG_GROUP_STORAGE_YARD, storage_text, g_text_width + x, y, image_group(IMAGE_GROUP_FONT_WHITE));
    }
}

static void draw_order_state(const building_storage *storage, int32_t resource, int32_t x, int32_t y, int32_t storage_text)
{
    switch (storage->resource_state[resource]) {
        case STORAGE_STATE_ACCEPT: {
            int32_t limit = storage->accept_amount[resource];
            int32_t first_text = (limit <= STORAGE_ORDER_LIMIT_THREE_QUARTERS) ? STORAGE_YARD_TEXT_FILL : STORAGE_YARD_TEXT_ACCEPT;
            draw_order_limit(first_text, limit, STORAGE_YARD_TEXT_ALL, storage_text, x, y);
            break;
        }
        case STORAGE_STATE_REFUSE:
            draw_label(LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_DONT_ACCEPT, x, y, image_group(IMAGE_GROUP_FONT_RED));
            break;
        case STORAGE_STATE_GET:
            draw_order_limit(STORAGE_YARD_TEXT_GET, storage->get_amount[resource], STORAGE_YARD_TEXT_MAXIMUM, storage_text, x, y);
            break;
        case STORAGE_STATE_EMPTY: {
            int32_t text = (storage_text == STORAGE_YARD_TEXT_GRANARY) ? STORAGE_YARD_TEXT_EMPTY_FOOD : STORAGE_YARD_TEXT_EMPTY;
            draw_label(LANG_GROUP_STORAGE_YARD, text, x, y, image_group(IMAGE_GROUP_FONT_RED));
            break;
        }
    }
}

static void draw_order_buttons(const building_storage *storage, int32_t resource, int32_t row, int32_t y)
{
    uint8_t state = storage->resource_state[resource];
    if (state == STORAGE_STATE_GET || state == STORAGE_STATE_ACCEPT) {
        image_button *buttons = &order_buttons.limit_buttons[row * ORDERS_ROW_BUTTONS];
        int32_t x = building_info_x + ORDERS_ROW_BUTTONS_X;
        image_buttons_set_tooltip(x, y + ORDERS_ROW_BUTTONS_DY, buttons, ORDERS_ROW_BUTTONS, ORDERS_ROW_BUTTONS_TOOLTIP_BASE);
        image_buttons_handle_mouse(x, y + ORDERS_ROW_BUTTONS_DY, buttons, ORDERS_ROW_BUTTONS);
        image_buttons_draw(x, y + ORDERS_ROW_BUTTONS_DY, buttons, ORDERS_ROW_BUTTONS);
    }
}

static void draw_accept_none_button(int32_t y)
{
    button_border_draw_ui(building_info_x + ORDERS_ACCEPT_NONE_X, y, (info_w_blocks - 10) * BUILDING_INFO_BLOCK, ORDERS_BUTTON_HEIGHT, focus_button_id == 1,
        building_info_x, building_info_y);
    lang_text_draw_centered_ui(LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_ACCEPT_NONE, building_info_x + ORDERS_ACCEPT_NONE_X, y + ORDERS_ACCEPT_NONE_TEXT_DY,
        (info_w_blocks - 10) * BUILDING_INFO_BLOCK, image_group(IMAGE_GROUP_FONT_BLACK), 0, building_info_x, building_info_y);
}

static void draw_order_row_name(int32_t resource, int32_t icon_x, int32_t name_x, int32_t y)
{
    draw_resource_icon(resource, building_info_x + icon_x, y + ORDERS_ROW_ICON_DY);
    draw_label(LANG_GROUP_RESOURCE_NAMES, resource, building_info_x + name_x, y + ORDERS_ROW_NAME_DY, image_group(IMAGE_GROUP_FONT_WHITE));
}

static void draw_order_row_focus(int32_t row, int32_t width, int32_t y)
{
    if (focus_button_id_secondary == row + 1) {
        button_border_draw_ui(
            building_info_x + ORDERS_ROW_FOCUS_X, y + ORDERS_ROW_ICON_DY, width, ORDERS_ROW_FOCUS_HEIGHT, 1, building_info_x, building_info_y);
    }
}

void window_building_draw_granary_orders_foreground(void)
{
    const building_storage *storage = storage_get(building_info_building()->storage_id);
    int32_t panel_bottom = (orders_panel_blocks(city_foods.count) + ORDERS_PANEL_EXTRA_BLOCKS) * BUILDING_INFO_BLOCK;
    draw_accept_none_button(panel_bottom + GRANARY_ORDERS_ACCEPT_NONE_DY);
    int32_t y = ORDERS_FIRST_ROW_Y;
    for (int32_t row = 0; row < city_foods.count; row++) {
        int32_t resource = city_foods.list[row];
        draw_order_row_name(resource, ORDERS_ICON_X, ORDERS_NAME_X, y);
        draw_order_row_focus(row, GRANARY_ORDERS_FOCUS_WIDTH, y);
        draw_order_state(storage, resource, building_info_x + GRANARY_ORDERS_STATE_X, y, STORAGE_YARD_TEXT_GRANARY);
        draw_order_buttons(storage, resource, row, y);
        y += ORDERS_ROW_HEIGHT;
    }
    window_redraw_request = 1;
}

static int32_t storage_yard_goods(int32_t building_id, int32_t *amounts, int32_t *parts_used)
{
    int32_t total = 0;
    int32_t part_id = building_id;
    for (int32_t i = 0; i < STORAGE_YARD_PARTS; i++) {
        part_id = building_get(part_id)->next_part_building_id;
        if (part_id > 0) {
            const building *part = building_get(part_id);
            if (part->loads_stored > 0) {
                amounts[part->subtype] += part->loads_stored;
                total += part->loads_stored;
                (*parts_used)++;
            }
        }
    }
    return total;
}

static int32_t is_counted_in_blocks(int32_t resource)
{
    return resource == RESOURCE_STONE || resource == RESOURCE_LIMESTONE || resource == RESOURCE_GRANITE || resource == RESOURCE_SANDSTONE ||
        resource == RESOURCE_MARBLE;
}

static int32_t is_counted_in_units(int32_t resource)
{
    return is_counted_in_blocks(resource) || resource == RESOURCE_CHARIOTS || resource == RESOURCE_WEAPONS || resource == RESOURCE_DEBEN;
}

static void draw_storage_yard_good(int32_t building_id, int32_t resource, int32_t row, int32_t parts_used)
{
    int32_t amount = building_storageyard_get_amount(building_id, resource);
    int32_t right_column = row >= STORAGE_YARD_GOODS_PER_COLUMN;
    int32_t x = building_info_x + (right_column ? STORAGE_YARD_GOODS_RIGHT_X : STORAGE_YARD_GOODS_LEFT_X);
    int32_t y = building_info_y + STORAGE_YARD_GOODS_Y + (row - (right_column ? STORAGE_YARD_GOODS_PER_COLUMN : 0)) * STORAGE_YARD_GOODS_ROW_HEIGHT;
    draw_resource_icon(resource, x, y);
    int32_t font = (amount % STORAGE_YARD_PART_LOADS != 0 || parts_used != STORAGE_YARD_PARTS) ? image_group(IMAGE_GROUP_FONT_BLACK) : image_group(IMAGE_GROUP_FONT_RED);
    g_text_width = 0;
    if (is_counted_in_units(resource)) {
        amount /= STORAGE_YARD_UNITS_PER_BLOCK;
    }
    int32_t text_y = y + STORAGE_YARD_AMOUNT_DY;
    draw_number(amount, x + STORAGE_YARD_AMOUNT_DX, text_y, font);
    int32_t name = resource;
    int32_t name_x = g_text_width + STORAGE_YARD_AMOUNT_DX + x;
    if (is_counted_in_blocks(resource)) {
        draw_label(LANG_GROUP_STORAGE_YARD, (amount == 1) ? STORAGE_YARD_TEXT_BLOCK_OF : STORAGE_YARD_TEXT_BLOCKS_OF, name_x, text_y, font);
        text_y = y + STORAGE_YARD_BLOCKS_NAME_DY;
    } else if ((resource == RESOURCE_CHARIOTS || resource == RESOURCE_WEAPONS) && amount != 1) {
        name = resource + LANG_RESOURCE_PLURAL_NAMES;
    }
    draw_label(LANG_GROUP_RESOURCE_NAMES, name, name_x, text_y, font);
}

static void clamp_storage_yard_first_row(building *b)
{
    if (b->info_orders_toggle == 0) {
        b->info_orders_toggle = 1;
    }
    if (city_resources.count <= b->info_orders_toggle) {
        b->info_orders_toggle = (city_resources.count < STORAGE_YARD_FIRST_ROW_MIN_COUNT)
            ? 1
            : city_resources.count - STORAGE_YARD_FIRST_ROW_LAST_FROM_END;
    }
}

void window_building_draw_storage_yard(void)
{
    int32_t amounts[RESOURCE_MAX];
    memset(amounts, 0, sizeof(amounts));
    int32_t parts_used = 0;
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_STORAGE_YARD];
    info_first_draw = 0;
    window_building_draw_panel_title(LANG_GROUP_STORAGE_YARD, BUILDING_TEXT_TITLE);
    int32_t id = building_info_building_id;
    building *b = building_get(id);
    clamp_storage_yard_first_row(b);
    int32_t total = storage_yard_goods(id, amounts, &parts_used);
    if (bi_road_access_block.has_road_access < 1) {
        window_building_draw_no_road_text(building_info_y + STORAGE_YARD_TEXT_Y);
    } else if (total == 0) {
        window_building_draw_text(LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_EMPTY_YARD, building_info_y + STORAGE_YARD_TEXT_Y);
    } else {
        int32_t row = 0;
        for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (amounts[resource] != 0) {
                draw_storage_yard_good(id, resource, row, parts_used);
                row++;
            }
        }
    }
    window_building_draw_inner_panel(STORAGE_YARD_WORKERS_PANEL_Y, STORAGE_WORKERS_PANEL_BLOCKS);
    window_building_draw_employment(building_info_y + STORAGE_YARD_EMPLOYMENT_Y);
    draw_cart_walker(b, b->data.storage_yard.second_cart_id, STORAGE_YARD_WALKER_ICON_Y, STORAGE_YARD_WALKER_TEXT_Y);
    int32_t warning_y = (building_info_h * BUILDING_INFO_BLOCK) - STORAGE_YARD_WARNING_FROM_BOTTOM + building_info_y;
    if (bi_storage_state == STORAGE_YARD_SPACE_FULL) {
        window_building_draw_text(LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_FULL, warning_y);
    } else if (bi_storage_state == STORAGE_YARD_SPACE_GETTING_FULL) {
        window_building_draw_text(LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_GETTING_FULL, warning_y);
    }
}

void window_building_draw_storage_yard_foreground(void)
{
    draw_orders_button(LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_SPECIAL_ORDERS);
}

int32_t storage_orders_rows(void)
{
    switch (settings_data.display_mode) {
        case DISPLAY_MODE_640X480:
            return 15;
        case DISPLAY_MODE_800X600:
            return 21;
        case DISPLAY_MODE_1024X768:
            return 24;
        default:
            return 10;
    }
}

static int32_t storage_yard_rows_shown(void)
{
    return (storage_orders_rows() < city_resources.count) ? storage_orders_rows() : city_resources.count;
}

int32_t storage_orders_scrollbar_height(void)
{
    return (orders_panel_blocks(storage_orders_rows()) - 6) * BUILDING_INFO_BLOCK;
}

int32_t building_info_orders_rows(void)
{
    int16_t type = building_info_building()->type;
    if (type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP) {
        return storage_yard_rows_shown();
    }
    if (type != BUILDING_BAZAAR && type != BUILDING_BAZAAR_UP) {
        return city_foods.count;
    }
    return bazaar_goods_count();
}

void window_building_draw_storage_scroll_thumb(void)
{
    int32_t hidden = city_resources.count - storage_orders_rows();
    if (hidden <= 0) {
        return;
    }
    int32_t percentage;
    int32_t first_row = building_info_building()->info_orders_scroll;
    if (first_row < 2) {
        percentage = 0;
    } else if (city_resources.count < storage_orders_rows() + first_row) {
        percentage = 100;
    } else {
        percentage = calc_percentage(first_row - 1, hidden);
    }
    int32_t y = calc_adjust_with_percentage(storage_orders_scrollbar_height(), percentage);
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + PANEL_BUTTON_SCROLL_THUMB, building_info_x + STORAGE_ORDERS_SCROLLBAR_X,
        y + STORAGE_ORDERS_SCROLLBAR_Y);
}

void window_building_draw_storage_yard_orders(void)
{
    draw_orders_background(BUILDING_INFO_HELP_STORAGE_YARD, LANG_GROUP_STORAGE_YARD, STORAGE_YARD_TEXT_INSTRUCTIONS, storage_yard_rows_shown());
}

static int32_t storage_yard_first_row(void)
{
    int32_t first_row = building_info_building()->info_orders_scroll;
    if (first_row == 0) {
        first_row = 1;
    }
    if (city_resources.count <= first_row) {
        first_row = (storage_orders_rows() + 1 < city_resources.count) ? city_resources.count - storage_orders_rows() - 1 : 1;
    }
    return first_row;
}

static void draw_storage_yard_scroll_arrows(int32_t height_blocks)
{
    int32_t panel_bottom = height_blocks * BUILDING_INFO_BLOCK;
    int32_t x = building_info_x + STORAGE_ORDERS_SCROLLBAR_X;
    int32_t down_focus_y = panel_bottom - STORAGE_YARD_ORDERS_DOWN_FOCUS_FROM_BOTTOM;
    arrow_buttons_get_focus(x, down_focus_y, storage_orders_scroll_down, 1, ORDERS_ROW_BUTTONS_TOOLTIP_BASE);
    arrow_buttons_handle_mouse(x, down_focus_y, storage_orders_scroll_down, 1);
    arrow_buttons_draw(x, panel_bottom - STORAGE_YARD_ORDERS_DOWN_FROM_BOTTOM, storage_orders_scroll_down, 1);
    int32_t up_x = building_info_x + STORAGE_YARD_ORDERS_UP_X;
    arrow_buttons_get_focus(up_x, STORAGE_YARD_ORDERS_UP_Y, storage_orders_scroll_up, 1, ORDERS_ROW_BUTTONS_TOOLTIP_BASE);
    arrow_buttons_handle_mouse(up_x, STORAGE_YARD_ORDERS_UP_Y, storage_orders_scroll_up, 1);
    arrow_buttons_draw(up_x, STORAGE_YARD_ORDERS_UP_Y, storage_orders_scroll_up, 1);
    int32_t track_blocks = (height_blocks == STORAGE_YARD_ORDERS_SHORT_PANEL_BLOCKS) ? STORAGE_YARD_ORDERS_SHORT_TRACK_BLOCKS
                                                                                     : STORAGE_YARD_ORDERS_TRACK_BLOCKS;
    inner_panel_draw_ui(building_info_x + STORAGE_YARD_ORDERS_TRACK_X, STORAGE_ORDERS_SCROLLBAR_Y, STORAGE_YARD_ORDERS_TRACK_WIDTH_BLOCKS, track_blocks,
        building_info_x, building_info_y);
}

void window_building_draw_storage_yard_orders_foreground(void)
{
    const building_storage *storage = storage_get(building_info_building()->storage_id);
    int32_t height_blocks = orders_panel_blocks(storage_yard_rows_shown()) + ORDERS_PANEL_EXTRA_BLOCKS;
    draw_accept_none_button(height_blocks * BUILDING_INFO_BLOCK);
    int32_t first_row = storage_yard_first_row();
    if (storage_orders_rows() < city_resources.count) {
        draw_storage_yard_scroll_arrows(height_blocks);
    }
    int32_t index = first_row - 1;
    int32_t y = ORDERS_FIRST_ROW_Y;
    for (int32_t row = 0; row < storage_orders_rows(); row++, y += ORDERS_ROW_HEIGHT) {
        int32_t resource = city_resources.list[index];
        if (resource == RESOURCE_NONE) {
            continue;
        }
        draw_order_row_name(resource, STORAGE_YARD_ORDERS_ICON_X, STORAGE_YARD_ORDERS_NAME_X, y);
        draw_order_row_focus(row, STORAGE_YARD_ORDERS_FOCUS_WIDTH, y);
        draw_order_state(storage, resource, building_info_x + STORAGE_YARD_ORDERS_STATE_X, y, STORAGE_YARD_TEXT_YARD);
        draw_order_buttons(storage, resource, row, y);
        first_row++;
        index++;
        if (city_resources.count < first_row) {
            break;
        }
    }
    window_redraw_request = 1;
    window_building_draw_storage_scroll_thumb();
}

int32_t building_storageyard_get_space_info(int32_t building_id)
{
    int32_t part_id = building_id;
    int32_t empty_parts = 0;
    int32_t total = 0;
    for (int32_t i = 0; i < STORAGE_YARD_PARTS; i++) {
        part_id = building_get(part_id)->next_part_building_id;
        if (part_id <= 0) {
            return STORAGE_YARD_SPACE_FREE;
        }
        const building *part = building_get(part_id);
        if (part->subtype == RESOURCE_NONE) {
            empty_parts++;
        } else {
            total += part->loads_stored;
        }
    }
    if (empty_parts > 0) {
        return STORAGE_YARD_SPACE_FREE;
    }
    return (total < STORAGE_YARD_FULL_LOADS) ? STORAGE_YARD_SPACE_GETTING_FULL : STORAGE_YARD_SPACE_FULL;
}

void window_building_info_storage_special_button(void)
{
    int32_t building_id = building_info_building_id;
    int32_t storage = building_get(building_id)->storage_id;
    if (button_parameter == 1) {
        city_player_current()->trade_center_building_id = building_id;
    } else if (button_parameter == 2 && city_resources.count > 0) {
        for (int32_t i = 0; i < city_resources.count; i++) {
            storage_get(storage)->resource_state[city_resources.list[i]] = 1;
        }
    }
    window_redraw_request = 1;
}
