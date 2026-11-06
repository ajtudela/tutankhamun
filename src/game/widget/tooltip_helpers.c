// Tooltip hit tests of the goods icons in the mission briefing, the empire map's trade panel and the trade
// prices window (each returns the good and whether it is sold or bought), and the two small city boxes: the
// goods of a map flag on the hovered tile and the city's luxury goods kinds.

#include "widget/tooltip_helpers.h"
#include "building/building_data.h"
#include "city/resource_type.h"
#include "editor/editor_data.h"
#include "empire/empire.h"
#include "empire/empire_data.h"
#include "empire/trade_status.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "text/lang.h"
#include "widget/widget_data.h"
#include "window/hotkeys.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum brief_icons {
    BRIEF_COLUMNS = 17,
    BRIEF_SECOND_ROW_ENTRIES = 16,
    BRIEF_COLUMN_STEP = 35,
    BRIEF_LEFT = 21,
    BRIEF_FIRST_ROW_Y = 50,
    BRIEF_SECOND_ROW_Y = 134,
    BRIEF_ICON_SIZE = 24,
    BRIEF_SECOND_NONE = 27,
    BRIEF_SECOND_END = 36,
};

enum empire_trade_panel {
    EMPIRE_PANEL_WIDTH = 500,
    EMPIRE_PANEL_TOP = 108,
    EMPIRE_PANEL_ROW_LEFT = 100,
    EMPIRE_PANEL_SOLD_ROW_Y = 28,
    EMPIRE_PANEL_BOUGHT_ROW_Y = 50,
    EMPIRE_PANEL_ROW_ICON_WIDTH = 19,
    EMPIRE_PANEL_ROW_ICON_HEIGHT = 17,
    EMPIRE_PANEL_ROW_STEP = 32,
    EMPIRE_PANEL_LIMITS_TOP = 15,
    EMPIRE_PANEL_COLUMN_STEP = 135,
    EMPIRE_PANEL_BOUGHT_LEFT = 280,
    EMPIRE_PANEL_LIMITS_ROW_STEP = 20,
    EMPIRE_PANEL_ICONS_PER_ROW = 2,
    TRADE_ICON_SIZE = 26,
};

enum trade_prices_window {
    PRICE_ROWS_BOUGHT_FIRST = 8,
    PRICE_ICON_LEFT = 223,
    PRICE_ICON_BOTTOM = 114,
};

enum tooltip_small_box {
    SMALL_BOX_HEIGHT = 20,
    SMALL_BOX_PADDING = 5,
    SMALL_BOX_MOUSE_GAP = 20,
    SMALL_BOX_LOWER_HALF_Y = 200,
    SMALL_BOX_BELOW = 10,
    SMALL_BOX_ABOVE = 32,
    SMALL_BOX_FILL_RGB565 = 0xBB88,
    SMALL_BOX_FRAME_RGB565 = 0x58A0,
    SMALL_BOX_TEXT_RGB565 = 0x3922,
    SMALL_BOX_FILL_RGB555 = 0x5DC8,
    SMALL_BOX_FRAME_RGB555 = 0x2C40,
    SMALL_BOX_TEXT_RGB555 = 0x1C82,
    LUXURY_KINDS = 16,
    LUXURY_KIND_WIDTH = 60,
    LUXURY_BOX_EXTRA_WIDTH = 50,
    LUXURY_KIND_TEXT = 39,
    FIGURE_GOODS_TEXT_GROUP = 48,
    FIGURE_GOODS_GRAIN_TEXT_GROUP = 49,
};

static int32_t mouse_on_icon(int32_t x, int32_t y)
{
    return mouse_inside_rect(x, y, TRADE_ICON_SIZE, TRADE_ICON_SIZE);
}

int32_t mission_brief_tooltip_text(void)
{
    const trade_price_list *goods = &trade_goods_list.list;
    for (int32_t i = 0; i < BRIEF_COLUMNS; i++) {
        int32_t first = goods->resources[i + 1];
        int32_t second = (i < BRIEF_SECOND_ROW_ENTRIES) ? goods->resources[i + BRIEF_COLUMNS + 1] : goods->beauty_variant;
        int32_t x = (first * BRIEF_COLUMN_STEP) + BRIEF_LEFT + ui_origin_x;
        if (mouse_inside_rect(x, ui_origin_y + BRIEF_FIRST_ROW_Y, BRIEF_ICON_SIZE, BRIEF_ICON_SIZE)) {
            return first;
        }
        if (second != BRIEF_SECOND_NONE && second >= 0 && second < BRIEF_SECOND_END &&
            mouse_inside_rect(x, ui_origin_y + BRIEF_SECOND_ROW_Y, BRIEF_ICON_SIZE, BRIEF_ICON_SIZE)) {
            return second;
        }
    }
    return 0;
}

static int32_t traded(int32_t object, int32_t resource, int32_t bought)
{
    int32_t trades = bought ? empire_object_city_buys_resource(object, resource) : empire_object_city_sells_resource(object, resource);
    return trades && empire_city_has_trade_limit(empire_selected_city, resource);
}

static int32_t row_icon_under_mouse(int32_t object, int32_t left, int32_t y, int32_t bought)
{
    g_text_width = 0;
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (!traded(object, resource, bought)) {
            continue;
        }
        if (mouse_inside_rect(g_text_width + left, y, EMPIRE_PANEL_ROW_ICON_WIDTH, EMPIRE_PANEL_ROW_ICON_HEIGHT)) {
            return resource;
        }
        g_text_width += EMPIRE_PANEL_ROW_STEP;
    }
    return 0;
}

static int32_t column_icon_under_mouse(int32_t object, int32_t left, int32_t y, int32_t bought)
{
    int32_t column = 0;
    int32_t count = 0;
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (!traded(object, resource, bought)) {
            continue;
        }
        if (mouse_on_icon((column * EMPIRE_PANEL_COLUMN_STEP) + left, y)) {
            return resource;
        }
        column++;
        count++;
        if (count % EMPIRE_PANEL_ICONS_PER_ROW == 0) {
            column = 0;
            y += EMPIRE_PANEL_LIMITS_ROW_STEP;
        }
    }
    return 0;
}

int32_t empire_tooltip_text(uint8_t *flag)
{
    int32_t object = empire_selected_object - 1;
    const empire_city *city = empire_city_get(empire_selected_city);
    if (!empire_city_type_is_trading(city->type)) {
        return 0;
    }
    int32_t base_x = (g_screen_width - EMPIRE_PANEL_WIDTH) / 2;
    int32_t y = g_screen_height - EMPIRE_PANEL_TOP;
    int32_t resource;
    if (city->is_open == 0) {
        int32_t left = base_x + EMPIRE_PANEL_ROW_LEFT;
        if ((resource = row_icon_under_mouse(object, left, y + EMPIRE_PANEL_SOLD_ROW_Y, 0)) != 0) {
            *flag = 0;
            return resource;
        }
        if ((resource = row_icon_under_mouse(object, left, y + EMPIRE_PANEL_BOUGHT_ROW_Y, 1)) != 0) {
            *flag = 1;
            return resource;
        }
        return 0;
    }
    y += EMPIRE_PANEL_LIMITS_TOP;
    if ((resource = column_icon_under_mouse(object, base_x, y, 0)) != 0) {
        *flag = 0;
        return resource;
    }
    if ((resource = column_icon_under_mouse(object, base_x + EMPIRE_PANEL_BOUGHT_LEFT, y, 1)) != 0) {
        *flag = 1;
        return resource;
    }
    return 0;
}

static int32_t price_icon_under_mouse(int32_t object, const generic_button *rows, int32_t bought)
{
    int32_t row = 0;
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        int32_t x = rows[row].x + PRICE_ICON_LEFT;
        int32_t y = rows[row].y + g_screen_height - PRICE_ICON_BOTTOM;
        int32_t trades = bought ? empire_object_city_buys_resource(object, resource) : empire_object_city_sells_resource(object, resource);
        if (trades) {
            if (mouse_on_icon(x, y)) {
                return resource;
            }
            row++;
        }
    }
    return 0;
}

int32_t trade_prices_tooltip_text(uint8_t *flag)
{
    int32_t object = g_empire_selected_object;
    uint8_t type = empire_map_objects[object].city_type;
    int32_t resource;
    if (!empire_city_type_is_trading(type)) {
        if (type == EMPIRE_CITY_OURS && (resource = price_icon_under_mouse(object, build_menu_buttons.empire_editor_our_city_sold, 0)) != 0) {
            *flag = 1;
            return resource;
        }
        return 0;
    }
    if ((resource = price_icon_under_mouse(object, build_menu_buttons.empire_editor_trade_city_amounts, 0)) != 0) {
        *flag = 0;
        return resource;
    }
    if ((resource = price_icon_under_mouse(object, &build_menu_buttons.empire_editor_trade_city_amounts[PRICE_ROWS_BOUGHT_FIRST], 1)) != 0) {
        *flag = 1;
        return resource;
    }
    return 0;
}

static int32_t tooltip_box_place(int32_t width, int32_t *fill)
{
    tooltip_box_width = width;
    tooltip_box_height = SMALL_BOX_HEIGHT;
    if (mouse_x < width + SMALL_BOX_MOUSE_GAP) {
        tooltip_box_x = mouse_x + SMALL_BOX_MOUSE_GAP;
    } else {
        tooltip_box_x = mouse_x - width - SMALL_BOX_MOUSE_GAP;
    }
    tooltip_box_y = (mouse_y < SMALL_BOX_LOWER_HALF_Y) ? mouse_y + SMALL_BOX_BELOW : mouse_y - SMALL_BOX_ABOVE;
    if (display_mode == DISPLAY_MODE_RGB565) {
        g_draw_color = SMALL_BOX_TEXT_RGB565;
        *fill = SMALL_BOX_FILL_RGB565;
        return SMALL_BOX_FRAME_RGB565;
    }
    g_draw_color = SMALL_BOX_TEXT_RGB555;
    *fill = SMALL_BOX_FILL_RGB555;
    return SMALL_BOX_FRAME_RGB555;
}

static void draw_small_box(int32_t width)
{
    int32_t fill;
    int32_t frame = tooltip_box_place(width, &fill);
    graphics_draw_rect(tooltip_box_x, tooltip_box_y, width, SMALL_BOX_HEIGHT, frame);
    graphics_fill_rect(tooltip_box_x + 1, tooltip_box_y + 1, tooltip_box_width - 2, tooltip_box_height - 2, fill);
}

static int32_t figure_goods_text(uint8_t goods, int32_t *group)
{
    *group = FIGURE_GOODS_TEXT_GROUP;
    if (goods == 1) {
        *group = FIGURE_GOODS_GRAIN_TEXT_GROUP;
        return 3;
    }
    if (goods == 2) {
        return 5;
    }
    if (goods == 3) {
        return 6;
    }
    if (goods == 12) {
        return 18;
    }
    if (goods == 13) {
        return 19;
    }
    if (goods >= 14 && goods <= 25) {
        return goods + 9;
    }
    if (goods >= 26 && goods <= 29) {
        return goods + 13;
    }
    if (goods >= 30 && goods <= 34) {
        return goods + 26;
    }
    if ((int32_t) goods - 50 < 8) {
        return goods - 40;
    }
    return goods - 10;
}

void tooltip_draw_figure_goods(void)
{
    int32_t figure_id = map_grids.figure_ids[map_info.selected_grid];
    if (figure_id == 0) {
        return;
    }
    const figure *f = figure_get(figure_id);
    while (f->type != FIGURE_MAP_FLAG) {
        figure_id = f->next_figure_id_on_same_tile;
        if (figure_id == 0) {
            return;
        }
        f = figure_get(figure_id);
    }
    int32_t group;
    int32_t index = figure_goods_text(f->resource_id, &group);
    const char *text = lang_get_string(group, index);
    int32_t width = text_get_width((const uint8_t *) text, image_group(IMAGE_GROUP_FONT_SMALL)) + SMALL_BOX_MOUSE_GAP;
    int16_t saved_color = (int16_t) g_draw_color;
    draw_small_box(width);
    g_text_width = 0;
    text_draw((const uint8_t *) text, tooltip_box_x + SMALL_BOX_PADDING, tooltip_box_y + SMALL_BOX_PADDING, image_group(IMAGE_GROUP_FONT_SMALL), (int16_t) g_draw_color);
    g_draw_color = (uint16_t) saved_color;
}

void tooltip_draw_luxury_goods(void)
{
    int32_t kinds[LUXURY_KINDS];
    memset(kinds, 0, sizeof(kinds));
    int32_t count = empire_count_luxury_goods_kinds(kinds);
    int32_t width = (count * LUXURY_KIND_WIDTH) + LUXURY_BOX_EXTRA_WIDTH;
    int16_t saved_color = (int16_t) g_draw_color;
    draw_small_box(width);
    int32_t drawn = 0;
    g_text_width = 0;
    for (int32_t kind = 0; kind < LUXURY_KINDS; kind++) {
        if (kinds[kind] == 0) {
            continue;
        }
        int32_t y = tooltip_box_y + SMALL_BOX_PADDING;
        if (drawn != 0) {
            text_draw((const uint8_t *) str_comma, g_text_width + tooltip_box_x + SMALL_BOX_PADDING, y, image_group(IMAGE_GROUP_FONT_SMALL), (int16_t) g_draw_color);
        }
        lang_text_draw(
            LANG_GROUP_RESOURCE_NAMES, kind + LUXURY_KIND_TEXT, g_text_width + tooltip_box_x + SMALL_BOX_PADDING, y, image_group(IMAGE_GROUP_FONT_SMALL), (int16_t) g_draw_color);
        drawn++;
    }
    g_draw_color = (uint16_t) saved_color;
}
