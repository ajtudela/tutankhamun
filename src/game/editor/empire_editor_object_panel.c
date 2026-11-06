// The empire editor's object panel buttons: a new object's image kind, a city's type, name, alignment, route, cost
// and goods, a region text's name, the trade amounts window and a battle marker's path and order. Choosing a good
// for an amount slot gives it a medium amount and takes the amount from the good it replaces.

#include "editor/empire_editor_object_panel.h"
#include "city/resource_type.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_select_list.h"
#include "editor/empire_editor_objects.h"
#include "editor/empire_editor_routes.h"
#include "empire/empire_data.h"
#include "empire/empire_helpers.h"
#include "empire/empire_types.h"
#include "empire/trade_status.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "text/lang.h"
#include "window/input_dialogs.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum object_panel_layout {
    CITY_TYPE_LIST_X = 443,
    CITY_NAME_LIST_X = 20,
    REGION_NAME_LIST_X = 373,
    NUMBER_INPUT_X = 453,
    COST_INPUT_X = 653,
    LIST_FROM_BOTTOM = 314,
    NAME_LIST_FROM_BOTTOM = 534,
    GOODS_LIST_X = 200,
    GOODS_SLOT_LIST_Y = 150,
    GOODS_AMOUNT_LIST_ROWS = 18,
};

enum object_panel_values {
    CITY_EDITED_COUNTDOWN = 10,
    CITY_TYPES = 7,
    REGION_NAMES = 17,
    TEXT_ALIGNMENTS = 4,
    GOODS_LIST_ITEM_NONE = 0,
    TRADE_AMOUNT_ROW_EMPTY = -1,
};

enum empire_editor_image_group {
    IMAGE_GROUP_EMPIRE_EDITOR_CITY = 399,
    IMAGE_GROUP_EMPIRE_EDITOR_BATTLE_MARKER = 446,
    IMAGE_GROUP_EMPIRE_EDITOR_OTHER = 447,
    IMAGE_GROUP_EMPIRE_EDITOR_REGION = 449,
};

static empire_object *edited_object(void)
{
    empire_object *object = &empire_map_objects[g_empire_selected_object];
    if (object->in_use == 0) {
        return 0;
    }
    object->city_type_countdown = CITY_EDITED_COUNTDOWN;
    return object;
}

static int32_t city_name_group(void)
{
    return (settings_data.classical_names != 0) ? LANG_GROUP_CITY_NAMES_CLASSICAL : LANG_GROUP_CITY_NAMES;
}

void empire_editor_choose_image_kind(void)
{
    int32_t kind = button_parameter;
    window_redraw_request = 1;
    empire_editor_image_kind = kind;
    int32_t image;
    switch (kind) {
        case EMPIRE_EDITOR_IMAGE_SIMPLE_GRAPHIC:
            image = image_group(IMAGE_GROUP_EMPIRE_ORNAMENTS_MAX);
            break;
        case EMPIRE_EDITOR_IMAGE_CITY:
            image = image_group(IMAGE_GROUP_EMPIRE_EDITOR_CITY);
            break;
        case EMPIRE_EDITOR_IMAGE_REGION:
            image = image_group(IMAGE_GROUP_EMPIRE_EDITOR_REGION);
            break;
        case EMPIRE_EDITOR_IMAGE_BATTLE_MARKER:
            image = image_group(IMAGE_GROUP_EMPIRE_EDITOR_BATTLE_MARKER);
            break;
        default:
            image = image_group(IMAGE_GROUP_EMPIRE_EDITOR_OTHER);
            break;
    }
    empire_editor_route_dragging = 0;
    empire_editor_image = image;
}

static void show_name_list(int32_t x, int32_t y, int32_t count, int32_t group, uint8_t *target)
{
    editor_select_list_show(x, y, count, group, 0, target, 0, 0, SELECT_LIST_USE_NONE, INPUT_TARGET_INT8, 0, 0, 0, -1, 0);
}

void empire_editor_city_type(void)
{
    empire_object *city = edited_object();
    if (city == 0) {
        return;
    }
    empire_editor_route_dragging = 0;
    show_name_list(CITY_TYPE_LIST_X, g_screen_height - LIST_FROM_BOTTOM, CITY_TYPES, LANG_GROUP_EMPIRE_CITY_TYPES, &city->city_type);
    window_redraw_request = 1;
}

void empire_editor_city_name(void)
{
    empire_object *city = edited_object();
    if (city == 0) {
        return;
    }
    empire_editor_route_dragging = 0;
    show_name_list(CITY_NAME_LIST_X, g_screen_height - NAME_LIST_FROM_BOTTOM, EMPIRE_CITY_NAMES, city_name_group(), &city->name_id);
    window_redraw_request = 1;
}

void empire_editor_city_text_align(void)
{
    empire_object *city = edited_object();
    if (city == 0) {
        return;
    }
    uint8_t align = (uint8_t) (city->text_align + 1);
    empire_editor_route_dragging = 0;
    city->text_align = align;
    if (align >= TEXT_ALIGNMENTS) {
        city->text_align = 0;
    }
    window_redraw_request = 1;
}

void empire_editor_region_name(void)
{
    empire_object *region = edited_object();
    if (region == 0) {
        return;
    }
    empire_editor_route_dragging = 0;
    show_name_list(REGION_NAME_LIST_X, g_screen_height - NAME_LIST_FROM_BOTTOM, REGION_NAMES, LANG_GROUP_EMPIRE_REGION_NAMES, &region->name_id);
    window_redraw_request = 1;
}

void empire_editor_city_route(void)
{
    empire_object *city = edited_object();
    if (city == 0) {
        return;
    }
    empire_editor_route_dragging = 0;
    numeric_input_show_int8(NUMBER_INPUT_X, g_screen_height - LIST_FROM_BOTTOM, &city->route_id, empire_editor_city_route_changed);
    window_redraw_request = 1;
}

void empire_editor_trade_amounts(void)
{
    window_current_id = WINDOW_EDITOR_TRADE_AMOUNTS;
    empire_objects_fix_trade_amounts();
    int32_t object = g_empire_selected_object;
    int32_t n = 0;
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (empire_object_city_sells_resource(object, resource) != 0 || empire_object_city_buys_resource(object, resource) != 0) {
            select_list_buttons.trade_amount_resources[n] = resource;
            n++;
        }
    }
    for (int32_t i = n; i < SELECT_LIST_TRADE_AMOUNT_ROWS; i++) {
        select_list_buttons.trade_amount_resources[i] = TRADE_AMOUNT_ROW_EMPTY;
    }
    window_redraw_request = 1;
}

void empire_editor_city_cost(void)
{
    empire_object *city = edited_object();
    if (city == 0) {
        return;
    }
    empire_editor_route_dragging = 0;
    numeric_input_show(COST_INPUT_X, g_screen_height - LIST_FROM_BOTTOM, INPUT_TARGET_INT16, 0, &city->cost_to_open, 0, 0);
    window_redraw_request = 1;
}

static int32_t goods_slot_offers(int32_t resource)
{
    return empire_editor_goods_slot_offered[resource] != 0;
}

static int32_t amount_slot_offers(int32_t resource)
{
    return resource != RESOURCE_UNUSED && resource >= 0 && resource < RESOURCE_MAX;
}

static int32_t in_other_slot(const uint8_t *goods, int32_t slot, int32_t slots, int32_t resource)
{
    if (resource == goods[slot]) {
        return 0;
    }
    for (int32_t i = 0; i < slots; i++) {
        if (goods[i] == resource) {
            return 1;
        }
    }
    return 0;
}

static int32_t goods_list_fill(const uint8_t *goods, int32_t slot, int32_t slots, int32_t goods_slot_list)
{
    int32_t n = 1;
    select_list.items[0] = GOODS_LIST_ITEM_NONE;
    for (int32_t resource = goods_slot_list ? 0 : RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        int32_t offered = goods_slot_list ? goods_slot_offers(resource) : amount_slot_offers(resource);
        if (offered && !in_other_slot(goods, slot, slots, resource)) {
            select_list.items[n] = resource;
            n++;
        }
    }
    return n;
}

void empire_editor_goods_slot(void)
{
    empire_object *city = &empire_map_objects[g_empire_selected_object];
    if (city->in_use == 0) {
        return;
    }
    int32_t slot = button_parameter;
    empire_goods_list_slot = slot;
    city->city_type_countdown = CITY_EDITED_COUNTDOWN;
    empire_editor_route_dragging = 0;
    int32_t n = goods_list_fill(city->sells, slot, EMPIRE_OBJECT_SOLD_RESOURCES, 1);
    editor_select_list_show(GOODS_LIST_X, GOODS_SLOT_LIST_Y, n, LANG_GROUP_RESOURCE_NAMES, 0, &city->sells[slot], 0, 0, SELECT_LIST_USE_NONE, INPUT_TARGET_INT8,
        1, 0, 1, (n / 2) + 1, 0);
}

void empire_editor_goods_chosen(int32_t bought)
{
    int32_t object = g_empire_selected_object;
    const empire_object *city = &empire_map_objects[object];
    int32_t resource = bought ? city->buys[empire_goods_list_slot] : city->sells[empire_goods_list_slot];
    int32_t previous = empire_goods_list_previous;
    if (previous != 0) {
        if (resource == previous) {
            return;
        }
        int32_t other = (uint8_t) bought ? empire_object_city_sells_resource(object, previous) : empire_object_city_buys_resource(object, previous);
        if (other == 0) {
            empire_object_set_trade_amount(object, empire_goods_list_previous, 0);
        }
    }
    if (empire_object_trade_amount(object, resource) != 0) {
        return;
    }
    int32_t other = (uint8_t) bought ? empire_object_city_sells_resource(object, resource) : empire_object_city_buys_resource(object, resource);
    if (other == 0) {
        empire_object_set_trade_amount(object, resource, TRADE_AMOUNT_MEDIUM);
    }
}

void empire_editor_goods_chosen_sold(void)
{
    empire_editor_goods_chosen(0);
}

void empire_editor_goods_chosen_bought(void)
{
    empire_editor_goods_chosen(1);
}

static void goods_amount_slot(uint8_t *goods, int32_t slots, input_dialog_callback on_choose)
{
    empire_object *city = &empire_map_objects[g_empire_selected_object];
    if (city->in_use == 0) {
        return;
    }
    int32_t slot = button_parameter;
    empire_goods_list_slot = slot;
    int32_t n = goods_list_fill(goods, slot, slots, 0);
    uint8_t *target = &goods[slot];
    city->city_type_countdown = CITY_EDITED_COUNTDOWN;
    empire_editor_route_dragging = 0;
    empire_goods_list_previous = *target;
    editor_select_list_show(
        GOODS_LIST_X, 0, n, LANG_GROUP_RESOURCE_NAMES, 0, target, 0, 0, SELECT_LIST_USE_NONE, INPUT_TARGET_INT8, 1, on_choose, 1, GOODS_AMOUNT_LIST_ROWS, 0);
}

void empire_editor_goods_sold_amount_slot(void)
{
    goods_amount_slot(empire_map_objects[g_empire_selected_object].sells, EMPIRE_OBJECT_SOLD_RESOURCES, empire_editor_goods_chosen_sold);
}

void empire_editor_goods_bought_amount_slot(void)
{
    goods_amount_slot(empire_map_objects[g_empire_selected_object].buys, EMPIRE_OBJECT_BOUGHT_RESOURCES, empire_editor_goods_chosen_bought);
}

static void show_battle_marker_number(uint8_t *target)
{
    empire_editor_route_dragging = 0;
    numeric_input_show(NUMBER_INPUT_X, g_screen_height - LIST_FROM_BOTTOM, INPUT_TARGET_INT8, target, 0, 0, 0);
    window_redraw_request = 1;
}

void empire_editor_battle_path(void)
{
    empire_object *marker = edited_object();
    if (marker != 0) {
        show_battle_marker_number(&marker->invasion_path_id);
    }
}

void empire_editor_battle_order(void)
{
    empire_object *marker = edited_object();
    if (marker != 0) {
        show_battle_marker_number(&marker->invasion_years);
    }
}
