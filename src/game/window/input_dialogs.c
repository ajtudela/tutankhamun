// The input dialogs: the numeric input (a keypad whose OK writes the value into a target of its width, keeping the
// target's sign) and the select list (texts in one or two columns, with the editor's extra button that pages the
// pharaoh names).
//
// The dialogs bend a few rules:
// - an int8 target gets the value as typed, its sign never read;
// - the second column's heading check compares the focus with its first row only;
// - in the editor's list of object kinds, RESOURCE_LUXURY_GOODS stands for the selected object's own kind.

#include "window/input_dialogs.h"
#include "building/building_data.h"
#include "city/resource_type.h"
#include "editor/editor_data.h"
#include "editor/editor_new_scenario.h"
#include "editor/editor_select_list.h"
#include "empire/empire_data.h"
#include "graphics/color.h"
#include "graphics/generic_buttons.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/layout_buttons.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "io/empire_file.h"
#include "io/game_file_internal.h"
#include "text/lang.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum numeric_input_key {
    NUMERIC_INPUT_KEY_ZERO = 10,
    NUMERIC_INPUT_KEY_OK = 11,
    NUMERIC_INPUT_KEY_CANCEL = 12,
    NUMERIC_INPUT_KEY_SIZE = 25,
    NUMERIC_INPUT_ROW_HEIGHT = 30,
    NUMERIC_INPUT_MAX_DIGITS_INT8 = 3,
    NUMERIC_INPUT_MAX_DIGITS = 5,
};

enum numeric_input_text {
    NUMERIC_INPUT_TEXT_OK = 16,
    NUMERIC_INPUT_TEXT_CANCEL = 17,
};

enum select_list_layout {
    SELECT_LIST_ROW_HEIGHT = 20,
    SELECT_LIST_COLUMN_WIDTH = 208,
    SELECT_LIST_COLUMN_BLOCKS = 13,
    SELECT_LIST_EXTRA_WIDTH = 200,
    SELECT_LIST_EXTRA_HEIGHT = 30,
    SELECT_LIST_EXTRA_TEXT_GROUP = 304,
    SELECT_LIST_PHARAOH_NAMES_PAGE_1 = 63,
    SELECT_LIST_PHARAOH_NAMES_PAGE_2 = 62,
    SELECT_LIST_EDITOR_PHARAOH_ROWS = 22,
};

enum editor_empire_map_range {
    EDITOR_EMPIRE_MAP_LAST = 33,
    EDITOR_EMPIRE_MAP_CAMERA_X = 109,
    EDITOR_EMPIRE_MAP_CAMERA_Y = 218,
};

const char numeric_input_key_texts[10][4] = { "0", "9", "8", "7", "6", "5", "4", "3", "2", "1" };

static int32_t key_color(int32_t key)
{
    return focus_button_id == key ? COLOR_BLUE : COLOR_BLACK;
}

static const char *numeric_input_key_text(int32_t key)
{
    return numeric_input_key_texts[NUMERIC_INPUT_KEY_ZERO - key];
}

static void numeric_input_draw_key(int32_t key, int32_t x, int32_t y)
{
    int32_t color = key_color(key);
    graphics_draw_rect(x, y, NUMERIC_INPUT_KEY_SIZE, NUMERIC_INPUT_KEY_SIZE, color);
    text_draw_centered((const uint8_t *) numeric_input_key_text(key), x, y, NUMERIC_INPUT_KEY_SIZE, image_group(IMAGE_GROUP_FONT_LARGE), color);
}

static void numeric_input_draw_text_button(int32_t key, int32_t text, int32_t x, int32_t y, int32_t width)
{
    int32_t color = key_color(key);
    graphics_draw_rect(x, y, width, NUMERIC_INPUT_KEY_SIZE, color);
    lang_text_draw_centered_ui(LANG_GROUP_EDITOR, text, x, y + 6, width, image_group(IMAGE_GROUP_FONT_BLACK), color, ui_origin_x, ui_origin_y);
}

void lang_text_draw_centered_in_rect(
    int32_t group, int32_t index, const generic_button *button, int32_t font, int32_t color, int32_t x, int32_t y, int32_t clip_x, int32_t clip_y)
{
    lang_text_draw_centered_ui(group, index, button->x + x + clip_x, button->y + y + clip_y, button->x_end - button->x, font, color, clip_x, clip_y);
}

void window_numeric_input_draw(int32_t x, int32_t y)
{
    static const int32_t columns[3] = { 5, 35, 65 };
    outer_panel_draw(x - 16, y - 16, 8, 14);
    graphics_fill_rect(x, y, 96, 30, COLOR_BLACK);
    if (numeric_input_digits > 0) {
        text_draw_number_centered(numeric_input_value, NUMBER_PREFIX_NONE, postfix_none, x - 8, y + 3, 96, image_group(IMAGE_GROUP_FONT_LARGE), COLOR_RED);
    }
    for (int32_t key = 1; key <= 9; key++) {
        int32_t key_x = x + columns[(key - 1) % 3];
        int32_t key_y = y + 35 + (((key - 1) / 3) * NUMERIC_INPUT_ROW_HEIGHT);
        numeric_input_draw_key(key, key_x, key_y);
    }
    numeric_input_draw_key(NUMERIC_INPUT_KEY_ZERO, x + 5, y + 125);
    numeric_input_draw_text_button(NUMERIC_INPUT_KEY_OK, NUMERIC_INPUT_TEXT_OK, x + 35, y + 125, 55);
    numeric_input_draw_text_button(NUMERIC_INPUT_KEY_CANCEL, NUMERIC_INPUT_TEXT_CANCEL, x + 5, y + 155, 85);
}

static int32_t select_list_item_text(int32_t index, int32_t object)
{
    int32_t item = select_list.items[index];
    if (editor_active == 1 && select_list_group == LANG_GROUP_RESOURCE_NAMES && item == RESOURCE_LUXURY_GOODS) {
        return empire_culture_table[empire_map_objects[object].name_id].luxury_kind;
    }
    return item + select_list_text_base;
}

static int32_t select_list_item_color(int32_t index, int32_t heading_row)
{
    if (select_list_heading != 0 && focus_button_id == heading_row) {
        return COLOR_BLACK;
    }
    return key_color(index + 1);
}

static void select_list_draw_item(int32_t index, int32_t text, int32_t color, int32_t x, int32_t y)
{
    lang_text_draw_centered_in_rect(select_list_group, text, &select_list_buttons.items[index], image_group(IMAGE_GROUP_FONT_BLACK), color, x, y, ui_origin_x, ui_origin_y);
}

static int32_t select_list_extra_button_handled(int32_t x, int32_t y, int32_t shown_rows)
{
    generic_button *button = &select_list_extra_buttons[0];
    int32_t button_x = (select_list_column_blocks * SELECT_LIST_COLUMN_WIDTH) + x - 218;
    int32_t button_y = y + (shown_rows * SELECT_LIST_ROW_HEIGHT) - 25;
    button->x = (int16_t) button_x;
    button->y = (int16_t) button_y;
    button->x_end = (int16_t) (button_x + SELECT_LIST_EXTRA_WIDTH);
    button->y_end = (int16_t) (button_y + SELECT_LIST_EXTRA_HEIGHT);
    button->left_click_handler = select_list_extra_text == 0 ? select_list_pharaoh_names_page_2 : select_list_pharaoh_names_page_1;
    int32_t focus = generic_buttons_get_focus(0, 0, button, 1);
    focus = ui_layout_buttons(0, 0, (uint8_t *) button, 1, focus);
    if (generic_buttons_handle_mouse(0, 0, button, 1) != 0) {
        return 1;
    }
    button_border_draw_ui(button_x, button_y, SELECT_LIST_EXTRA_WIDTH, SELECT_LIST_EXTRA_HEIGHT, focus == 1, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(SELECT_LIST_EXTRA_TEXT_GROUP, select_list_extra_text, button_x, button_y + 8, SELECT_LIST_EXTRA_WIDTH,
        image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK, ui_origin_x, ui_origin_y);
    return 0;
}

void window_select_list_draw(int32_t x, int32_t y, int32_t rows)
{
    int32_t object = g_empire_selected_object;
    int32_t two_columns = rows < select_list_count;
    if (two_columns) {
        outer_panel_draw(x, y, select_list_column_blocks * SELECT_LIST_COLUMN_BLOCKS, ((rows * SELECT_LIST_ROW_HEIGHT) + 24) / 16);
    } else {
        outer_panel_draw(x, y, SELECT_LIST_COLUMN_BLOCKS, ((select_list_count * 21) + 24) / 16);
    }
    if (select_list_extra_on != 0 && select_list_extra_button_handled(x, y, two_columns ? rows : select_list_count)) {
        return;
    }
    int32_t relative_x = x - ui_origin_x;
    int32_t relative_y = y - ui_origin_y;
    if (!two_columns) {
        for (int32_t i = 0; i < select_list_count; i++) {
            int32_t color = (select_list_heading != 0 && focus_button_id == rows) ? COLOR_BLACK : select_list_item_color(i, 1);
            select_list_draw_item(i, select_list.items[i] + select_list_text_base, color, relative_x, relative_y);
        }
        return;
    }
    for (int32_t i = 0; i < rows; i++) {
        select_list_draw_item(i, select_list_item_text(i, object), select_list_item_color(i, 1), relative_x, relative_y);
    }
    for (int32_t i = rows; i < select_list_count; i++) {
        select_list_draw_item(i, select_list_item_text(i, object), select_list_item_color(i, rows + 1), relative_x, relative_y);
    }
}

void numeric_input_digit(void)
{
    int32_t max_digits = numeric_input_width == INPUT_TARGET_INT8 ? NUMERIC_INPUT_MAX_DIGITS_INT8 : NUMERIC_INPUT_MAX_DIGITS;
    if (numeric_input_digits < max_digits) {
        window_redraw_request = 1;
        numeric_input_value = button_parameter + (numeric_input_value * 10);
        numeric_input_digits++;
    }
}

static int32_t numeric_input_write_target(void)
{
    if (numeric_input_width == INPUT_TARGET_INT16 && *numeric_input_target16 < 0) {
        *numeric_input_target16 = (int16_t) numeric_input_value;
        return -1;
    }
    if (numeric_input_width == INPUT_TARGET_INT32 && *numeric_input_target32 < 0) {
        *numeric_input_target32 = numeric_input_value;
        return -1;
    }
    if (numeric_input_width == INPUT_TARGET_INT8) {
        *numeric_input_target8 = (int8_t) numeric_input_value;
    } else if (numeric_input_width == INPUT_TARGET_INT16) {
        *numeric_input_target16 = (int16_t) numeric_input_value;
    } else if (numeric_input_width == INPUT_TARGET_INT32) {
        *numeric_input_target32 = numeric_input_value;
    }
    return 1;
}

static int32_t sign_differs(int32_t value, int32_t sign)
{
    return (value < 0 && sign == 1) || (value > 0 && sign == -1);
}

static void numeric_input_restore_sign(int32_t sign)
{
    if (numeric_input_width == INPUT_TARGET_INT8) {
        if (*numeric_input_target8 != 0 && sign == -1) {
            *numeric_input_target8 = (int8_t) -*numeric_input_target8;
        }
    } else if (numeric_input_width == INPUT_TARGET_INT16) {
        int16_t value = *numeric_input_target16;
        if (sign_differs(value, sign)) {
            *numeric_input_target16 = (int16_t) -value;
        }
    } else if (numeric_input_width == INPUT_TARGET_INT32) {
        int32_t value = *numeric_input_target32;
        if (sign_differs(value, sign)) {
            *numeric_input_target32 = -value;
        }
    }
}

static void numeric_input_editor_apply(void)
{
    if (numeric_input_editor_use == NUMERIC_INPUT_EDITOR_EMPIRE_MAP) {
        int32_t index = --editor_empire_map_number;
        if (index >= 0 && index <= EDITOR_EMPIRE_MAP_LAST) {
            map_info.map_id = index;
            editor_select_empire_map(index);
            map_info.camera_x = EDITOR_EMPIRE_MAP_CAMERA_X;
            map_info.camera_y = EDITOR_EMPIRE_MAP_CAMERA_Y;
        }
    } else if (numeric_input_editor_use == NUMERIC_INPUT_EDITOR_EMPIRE_FILE) {
        empire_file_select(editor_empire_file_number - 1);
    }
}

void numeric_input_ok(void)
{
    numeric_input_restore_sign(numeric_input_write_target());
    numeric_input_editor_apply();
    if (numeric_input_on_ok != 0) {
        numeric_input_on_ok();
        numeric_input_on_ok = 0;
    }
    window_current_id = numeric_input_return_window.id;
    window_redraw_request = 1;
}

void numeric_input_cancel(void)
{
    window_redraw_request = 1;
    window_current_id = numeric_input_return_window.id;
}

static void select_list_show_pharaoh_names(int32_t page, int32_t count, int32_t first_name)
{
    select_list_extra_text = page;
    window_current_id = select_list_return_window.id;
    editor_select_list_show(0, 0, count, LANG_GROUP_PHARAOH_NAMES, first_name, 0, 0, &scenario_data.pharaoh_name_id, NUMERIC_INPUT_EDITOR_NONE, INPUT_TARGET_INT32, 0, 0, 0,
        SELECT_LIST_EDITOR_PHARAOH_ROWS, 0);
    window_redraw_request = 1;
}

void select_list_pharaoh_names_page_1(void)
{
    select_list_show_pharaoh_names(0, SELECT_LIST_PHARAOH_NAMES_PAGE_1, 0);
}

void select_list_pharaoh_names_page_2(void)
{
    select_list_show_pharaoh_names(1, SELECT_LIST_PHARAOH_NAMES_PAGE_2, SELECT_LIST_PHARAOH_NAMES_PAGE_1);
}
