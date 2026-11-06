// The top menu bar: its entries, the rotate button and the panel strip, the entry under the mouse and its
// click, the width of a menu's text group, and the open drop-down (item focus, click, draw, close). On screen
// sizes other than the standard ones the panel strip is extended to the screen's left edge.

#include "widget/menu_bar.h"
#include "editor/editor_data.h"
#include "graphics/button.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/menu.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "text/lang.h"
#include "text/text_lookup.h"
#include "widget/screen_layout.h"
#include "widget/widget_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

menu_bar_entry *menu_open_entry;

int32_t menu_return_window;

int32_t menu_item_focus;

enum menu_layout {
    MENU_BAR_FIRST_X = 10,
    MENU_BAR_Y = 6,
    MENU_BAR_ENTRY_HEIGHT = 12,
    MENU_BAR_ENTRY_SPACING = 10,
    MENU_DROPDOWN_PANEL_TOP = 18,
    MENU_DROPDOWN_ITEMS_TOP = 30,
    MENU_DROPDOWN_ITEM_HEIGHT = 15,
    MENU_DROPDOWN_ITEM_PITCH = 20,
    MENU_DROPDOWN_TEXT_INDENT = 8,
    MENU_DROPDOWN_FOCUS_TEXT_HEIGHT = 16,
    MENU_DEFAULT_WIDTH_BLOCKS = 10,
    MENU_COLUMNS_BY_ENTRY = 5,
};

enum menu_text_color {
    MENU_TEXT_COLOR_DEFAULT = 0,
    MENU_TEXT_COLOR_FOCUS_SHADOW = 0xFAC1,
};

enum rotate_button_image {
    ROTATE_BUTTON_IMAGE_FOCUSED = 1,
    ROTATE_BUTTON_IMAGE_PRESSED = 4,
};

enum panel_strip {
    PANEL_STRIP_IMAGE = 8,
    PANEL_STRIP_X_640 = -360,
    PANEL_STRIP_X_800 = -240,
    PANEL_STRIP_X_1024 = 0,
};

static menu_item *menu_bar_items(const menu_bar_entry *entry)
{
    return &g_menu_items[entry->first_item];
}

void menu_bar_draw(menu_bar_entry *entries, int32_t count)
{
    g_text_width = MENU_BAR_FIRST_X;
    menu_bar_entry *entry = entries;
    for (int32_t number = 1; number <= count; number++, entry++) {
        int32_t group = entry->text_group;
        int32_t y = MENU_BAR_Y;
        entry->x_start = (int16_t) g_text_width;
        int32_t x = entry->x_start;
        const char *title = lang_get_string(group, 0);
        if (number == menu_bar_focus_item) {
            text_draw_shadow = 1;
            text_get_width((const uint8_t *) title, image_group(IMAGE_GROUP_FONT_SMALL));
            lang_text_draw(group, 0, x, y, image_group(IMAGE_GROUP_FONT_BLACK), MENU_TEXT_COLOR_FOCUS_SHADOW);
            text_draw_shadow = 0;
        } else {
            lang_text_draw(group, 0, x, y, image_group(IMAGE_GROUP_FONT_BLACK), MENU_TEXT_COLOR_DEFAULT);
        }
        entry->x_end = (int16_t) g_text_width;
        g_text_width = g_text_width + MENU_BAR_ENTRY_SPACING;
    }
}

void menu_bar_draw_side_button(void)
{
    const image_button *button = &rotate_button_x[0];
    int32_t image = image_group(button->image_group);
    int32_t x = button->x;
    g_draw_x = x;
    int32_t y = button->y;
    g_draw_y = y;
    g_last_image_id = image;
    int32_t part_under_mouse = button->parameter1;
    if (button->pressed != 0) {
        if (button->pressed == IMAGE_BUTTON_PRESSED_FRAMES) {
            g_force_refresh = 1;
        }
        image = image + part_under_mouse + ROTATE_BUTTON_IMAGE_PRESSED;
        g_last_image_id = image;
        image_draw(image, x, y);
        return;
    }
    if (button->focused != 0) {
        image = image + part_under_mouse + ROTATE_BUTTON_IMAGE_FOCUSED;
        g_last_image_id = image;
    }
    image_draw(image, x, y);
}

static void draw_panel_strip_of_screen_layout(int32_t image)
{
    int32_t right_x = screen_layout_sidebar_right() - SCREEN_LAYOUT_TOP_STRIP_WIDTH;
    int32_t x = right_x;
    while (x > 0) {
        x -= SCREEN_LAYOUT_TOP_STRIP_REPEAT;
    }
    for (; x < right_x; x += SCREEN_LAYOUT_TOP_STRIP_REPEAT) {
        image_draw(image, x, 0);
    }
    image_draw(image, right_x, 0);
}

void top_menu_draw_panel_strip(void)
{
    int32_t image = image_group(IMAGE_GROUP_PANEL) + PANEL_STRIP_IMAGE;
    if (!screen_layout_is_standard_size()) {
        draw_panel_strip_of_screen_layout(image);
    } else if (g_screen_width == 640) {
        image_draw(image, PANEL_STRIP_X_640, 0);
    } else if (g_screen_width == 800) {
        image_draw(image, PANEL_STRIP_X_800, 0);
    } else if (g_screen_width == 1024) {
        image_draw(image, PANEL_STRIP_X_1024, 0);
    }
}

int32_t ui_layout_column_width(int32_t index)
{
    if (index < 0 || index >= MENU_COLUMNS_BY_ENTRY) {
        return MENU_DEFAULT_WIDTH_BLOCKS;
    }
    return ui_layout_column_widths[index];
}

int32_t ui_layout_menu_width(int32_t group)
{
    switch (group) {
        case LANG_GROUP_MENU_FILE:
            return ui_layout_column_widths[0];
        case LANG_GROUP_MENU_OPTIONS:
            return ui_layout_column_widths[1];
        case LANG_GROUP_MENU_HELP:
            return ui_layout_column_widths[2];
        case LANG_GROUP_MENU_OVERSEERS:
            return ui_layout_column_widths[3];
        case LANG_GROUP_EDITOR_MENU_FILE:
            return ui_layout_column_widths[4];
        case LANG_GROUP_EDITOR_MENU_RESETS:
            return ui_layout_column_widths[5];
        default:
            return MENU_DEFAULT_WIDTH_BLOCKS;
    }
}

void menu_bar_clear(void)
{
    menu_item_focus = 0;
    menu_bar_focus_item = 0;
    menu_open_entry = 0;
    g_top_menu_force = 1;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void menu_bar_close(void)
{
    window_current_id = (uint8_t) menu_return_window;
    menu_item_focus = 0;
    menu_bar_focus_item = 0;
    menu_open_entry = 0;
    g_top_menu_force = 1;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

int32_t menu_dropdown_focus(const menu_item *items, int32_t count, int32_t x, int32_t y)
{
    const menu_item *item = items;
    for (int32_t number = 1; number <= count; number++, item++) {
        int32_t top = item->y + y + MENU_DROPDOWN_ITEMS_TOP;
        if (x > mouse_x) {
            continue;
        }
        int32_t width = ui_layout_column_width(menu_bar_focus_item - 1);
        if ((width * UI_BLOCK_SIZE) + x > mouse_x && top <= mouse_y && top + MENU_DROPDOWN_ITEM_HEIGHT > mouse_y) {
            return number;
        }
    }
    return 0;
}

int32_t menu_dropdown_handle_mouse(menu_bar_entry *menu, int32_t count)
{
    popup.menu_dropdown_keep_open = 0;
    if (mouse_left_went_down != 0) {
        if (menu_item_focus == 0) {
            menu_bar_close();
            return 1;
        }
        window_current_id = (uint8_t) menu_return_window;
        menu_item *item = &menu_bar_items(menu_open_entry)[menu_item_focus - 1];
        popup.menu_dropdown_item = item;
        button_parameter = item->parameter;
        item->handler();
        if (popup.menu_dropdown_keep_open == 1) {
            window_redraw_request = WINDOW_REDRAW_ALL;
            g_top_menu_force = 1;
            window_current_id = WINDOW_TOP_MENU;
            return 1;
        }
        menu_bar_clear();
        return 1;
    }
    int32_t focus = menu_bar_focus(menu, count);
    if (focus != menu_bar_focus_item && focus != 0) {
        menu_bar_focus_item = focus;
        menu_item_focus = 0;
        menu_open_entry = &menu[focus - 1];
        g_top_menu_force = 1;
        window_redraw_request = WINDOW_REDRAW_ALL;
    }
    const menu_bar_entry *entry = menu_open_entry;
    menu_item_focus = menu_dropdown_focus(menu_bar_items(entry), entry->item_count, entry->x_start, MENU_BAR_Y);
    return 0;
}

void menu_dropdown_draw(void)
{
    if (window_current_id != WINDOW_TOP_MENU || menu_bar_focus_item == 0) {
        return;
    }
    const menu_bar_entry *entry = menu_open_entry;
    popup.menu_dropdown_item = menu_bar_items(entry);
    int32_t count = entry->item_count;
    int32_t x = entry->x_start;
    int32_t y = MENU_BAR_Y;
    int32_t group = entry->text_group;
    int16_t height = (int16_t) ((count + 1) * MENU_DROPDOWN_ITEM_PITCH);
    g_img_height = height;
    int16_t width = (int16_t) ui_layout_menu_width(group);
    g_img_width = width;
    panel_draw_unbordered(x, y + MENU_DROPDOWN_PANEL_TOP, width, height / UI_BLOCK_SIZE);
    for (int32_t number = 1; number <= count; number++) {
        const menu_item *item = popup.menu_dropdown_item;
        int32_t item_y = item->y + y + MENU_DROPDOWN_ITEMS_TOP;
        if (number == menu_item_focus) {
            text_draw_shadow = 1;
            g_img_width = width;
            g_img_height = MENU_DROPDOWN_FOCUS_TEXT_HEIGHT;
            lang_text_draw(group, item->text_number, x + MENU_DROPDOWN_TEXT_INDENT, item_y, image_group(IMAGE_GROUP_FONT_RED), MENU_TEXT_COLOR_FOCUS_SHADOW);
            text_draw_shadow = 0;
        } else {
            lang_text_draw(group, item->text_number, x + MENU_DROPDOWN_TEXT_INDENT, item_y, image_group(IMAGE_GROUP_FONT_BLACK), MENU_TEXT_COLOR_DEFAULT);
        }
        popup.menu_dropdown_item++;
    }
}

int32_t menu_bar_focus(const menu_bar_entry *menu, int32_t count)
{
    const menu_bar_entry *entry = menu;
    for (int32_t index = 0; entry && index < count; index++, entry++) {
        if (entry->x_start <= mouse_x && mouse_x < entry->x_end && MENU_BAR_Y <= mouse_y && mouse_y < MENU_BAR_Y + MENU_BAR_ENTRY_HEIGHT) {
            return index + 1;
        }
    }
    return 0;
}

int32_t menu_bar_handle_mouse(menu_bar_entry *menu, int32_t count)
{
    if (!mouse_left_went_down || top_menu_focus_menu_id <= 0) {
        return 0;
    }
    menu_item_focus = 0;
    menu_return_window = window_current_id;
    window_current_id = WINDOW_TOP_MENU;
    menu_open_entry = &menu[menu_bar_focus_item - 1];
    g_top_menu_force = 1;
    window_redraw_request = WINDOW_REDRAW_ALL;
    return 1;
}
