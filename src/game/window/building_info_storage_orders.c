// The input of the storage buildings' orders panel: the orders button, a row's state (accept -> get -> empty -> don't
// accept) and its limit arrows (800..3200 by 800), "accept none", and the storage yard's scroll arrows and scroll bar.
//
// Order clicks have side effects:
// - "accept none" refuses as many entries from the start of the resource list as the city has foods;
// - a click on a storage yard row first pulls the first row shown back into range.

#include "window/building_info_storage_orders.h"
#include "building/building.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "input/input_data.h"
#include "window/building_info.h"
#include "window/building_info_storage.h"
#include "window/window_data.h"
#include <stdint.h>

enum storage_order_limit_arrow {
    STORAGE_ORDER_LIMIT_MORE = 0,
    STORAGE_ORDER_LIMIT_LESS = 1,
};

enum storage_orders_scroll_arrow {
    STORAGE_ORDERS_SCROLL_UP = 0,
};

enum {
    STORAGE_ORDER_LIMIT_STEP = 800,
    STORAGE_ORDER_LIMIT_MIN = 800,
    STORAGE_ORDER_LIMIT_MAX = 3200,
    STORAGE_YARD_TOGGLE_LAST_FIRST_ROW = 14,
    CITY_RESOURCE_LIST_SIZE = 36,
};

static int32_t is_storage_yard(int32_t type)
{
    return type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP;
}

static int32_t is_bazaar(int32_t type)
{
    return type == BUILDING_BAZAAR || type == BUILDING_BAZAAR_UP;
}

void storage_orders_accept_none(void)
{
    building_storage *storage = storage_get(building_info_building()->storage_id);
    int32_t count = city_foods.count;
    for (int32_t i = 0; i < count; i++) {
        storage->resource_state[city_resources.list[i]] = STORAGE_STATE_REFUSE;
    }
    window_redraw_request = 1;
}

void building_orders_scroll_arrow(void)
{
    building *b = building_info_building();
    if (button_parameter == STORAGE_ORDERS_SCROLL_UP) {
        b->info_orders_scroll--;
        if (b->info_orders_scroll <= 0) {
            b->info_orders_scroll = 1;
        }
        window_redraw_request = 1;
        return;
    }
    b->info_orders_scroll++;
    if (storage_orders_rows() + b->info_orders_scroll - 1 > city_resources.count) {
        b->info_orders_scroll = city_resources.count - storage_orders_rows() + 1;
    }
    window_redraw_request = 1;
}

void building_info_show_orders(void)
{
    building_info_orders_shown = 1;
    window_redraw_request = 1;
}

static int32_t bazaar_good_of_row(int32_t row)
{
    int32_t goods[CITY_RESOURCE_LIST_SIZE] = { 0 };
    int32_t count = 0;
    for (int32_t i = 0; i < CITY_RESOURCE_LIST_SIZE; i++) {
        if (resource_is_bazaar_good(city_resources.list[i]) != 0) {
            goods[count++] = city_resources.list[i];
        }
    }
    return goods[row - 1];
}

static int32_t toggled_state(int32_t state, int32_t type)
{
    switch (state) {
        case STORAGE_STATE_ACCEPT:
            return is_bazaar(type) ? STORAGE_STATE_REFUSE : STORAGE_STATE_GET;
        case STORAGE_STATE_REFUSE:
            return STORAGE_STATE_ACCEPT;
        case STORAGE_STATE_GET:
            return STORAGE_STATE_EMPTY;
        case STORAGE_STATE_EMPTY:
            return STORAGE_STATE_REFUSE;
        default:
            return state;
    }
}

void storage_orders_toggle_resource(void)
{
    building *b = building_info_building();
    int32_t resource;
    if (is_storage_yard(b->type)) {
        if (b->info_orders_scroll == 0) {
            b->info_orders_scroll = 1;
        } else if (b->info_orders_scroll > STORAGE_YARD_TOGGLE_LAST_FIRST_ROW) {
            b->info_orders_scroll = city_resources.count - (STORAGE_YARD_TOGGLE_LAST_FIRST_ROW - 1);
        }
        resource = city_resources.list[b->info_orders_scroll + button_parameter - 2];
    } else if (is_bazaar(b->type)) {
        resource = bazaar_good_of_row(button_parameter);
    } else {
        resource = city_foods.list[button_parameter - 1];
    }
    uint8_t *state = &storage_get(b->storage_id)->resource_state[resource];
    *state = (uint8_t) toggled_state(*state, b->type);
    window_redraw_request = 1;
}

static int32_t changed_limit(int32_t limit)
{
    if (button_parameter == STORAGE_ORDER_LIMIT_LESS) {
        limit -= STORAGE_ORDER_LIMIT_STEP;
        return (limit < STORAGE_ORDER_LIMIT_MIN) ? STORAGE_ORDER_LIMIT_MIN : limit;
    }
    limit += STORAGE_ORDER_LIMIT_STEP;
    return (limit > STORAGE_ORDER_LIMIT_MAX) ? STORAGE_ORDER_LIMIT_MAX : limit;
}

void storage_orders_limit_arrow(void)
{
    building *b = building_info_building();
    if (b->info_orders_scroll == 0) {
        b->info_orders_scroll = 1;
    } else if (b->info_orders_scroll > storage_orders_rows()) {
        b->info_orders_scroll = city_resources.count - storage_orders_rows() + 1;
    }
    int32_t resource;
    if (is_storage_yard(b->type)) {
        resource = city_resources.list[b->info_orders_scroll + button_parameter2 - 1];
    } else {
        resource = city_foods.list[button_parameter2];
    }
    if (button_parameter == STORAGE_ORDER_LIMIT_LESS || button_parameter == STORAGE_ORDER_LIMIT_MORE) {
        building_storage *storage = storage_get(b->storage_id);
        if (storage->resource_state[resource] == STORAGE_STATE_ACCEPT) {
            storage->accept_amount[resource] = (int16_t) changed_limit(storage->accept_amount[resource]);
        } else {
            storage->get_amount[resource] = (int16_t) changed_limit(storage->get_amount[resource]);
        }
    }
    window_redraw_request = 1;
}

int32_t storage_orders_scrollbar(void)
{
    if (city_resources.count <= storage_orders_rows() || mouse_left_down == 0) {
        return 0;
    }
    int32_t hidden = city_resources.count - storage_orders_rows();
    int32_t height = storage_orders_scrollbar_height();
    if (mouse_x < building_info_x + STORAGE_ORDERS_SCROLLBAR_X || mouse_x > building_info_x + STORAGE_ORDERS_SCROLLBAR_X + STORAGE_ORDERS_SCROLLBAR_WIDTH) {
        return 0;
    }
    if (mouse_y < STORAGE_ORDERS_SCROLLBAR_Y || mouse_y > height + STORAGE_ORDERS_SCROLLBAR_Y + STORAGE_ORDERS_SCROLLBAR_THUMB) {
        return 0;
    }
    int32_t y = mouse_y - STORAGE_ORDERS_SCROLLBAR_Y;
    if (y > height) {
        y = height;
    }
    int32_t first_hidden_shown = calc_adjust_with_percentage(hidden, calc_percentage(y, height));
    window_redraw_request = 2;
    building_info_building()->info_orders_scroll = first_hidden_shown + 1;
    return 1;
}
