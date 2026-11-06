// The scenario editor's select list and numeric input: showing the list (its texts, targets, callback, rows and
// heading) in columns, writing the chosen item into the target, and showing the numeric input. In the empire editor's
// goods lists the food rules apply: a city trades at most four kinds of food, and meat needs straw the empire can
// produce or import.

#include "editor/editor_select_list.h"
#include "city/resource_type.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_new_scenario.h"
#include "editor/editor_warning.h"
#include "empire/empire_data.h"
#include "empire/empire_helpers.h"
#include "empire/empire_types.h"
#include "game/scenario_start.h"
#include "graphics/button.h"
#include "window/empire_editor_input.h"
#include "window/input_dialogs.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>

enum select_list_layout {
    SELECT_LIST_ROWS_DEFAULT = 26,
    SELECT_LIST_COLUMN_WIDTH = 200,
    SELECT_LIST_ROW_HEIGHT = 20,
    SELECT_LIST_BUTTON_X = 5,
    SELECT_LIST_BUTTON_Y = 14,
    SELECT_LIST_BUTTON_WIDTH = 196,
    SELECT_LIST_BUTTON_HEIGHT = 18,
    SELECT_LIST_INPUT_DELAY_FRAMES = 20,
};

void editor_select_list_show(int32_t x, int32_t y, int32_t count, int32_t group, int32_t text_base, uint8_t *target8, int16_t *target16, int32_t *target32,
    int32_t use, int32_t target_kind, bool keep_items, input_dialog_callback on_choose, int32_t in_editor, int32_t rows, int32_t heading)
{
    if (count > SELECT_LIST_MAX_ITEMS) {
        count = SELECT_LIST_MAX_ITEMS;
    }
    if (rows <= 0) {
        rows = SELECT_LIST_ROWS_DEFAULT;
    }
    select_list_x = x;
    select_list.target32 = target32;
    select_list_y = y;
    select_list.target8 = target8;
    select_list.target16 = target16;
    select_list_group = group;
    select_list.target_kind = target_kind;
    select_list.use = use;
    select_list_text_base = text_base;
    select_list_heading = (uint8_t) heading;
    select_list_on_choose = on_choose;
    select_list_count = count;
    select_list_rows = rows;
    select_list_column_blocks = (count + rows - 1) / rows;
    if (!keep_items) {
        for (int32_t i = 0; i < count; i++) {
            select_list.items[i] = i;
        }
    }
    window_input_delay_frames = SELECT_LIST_INPUT_DELAY_FRAMES;
    select_list_return_window.word = window_current_id;
    window_current_id = ((uint8_t) in_editor != 0) ? WINDOW_EDITOR_SELECT_LIST : WINDOW_SELECT_LIST;
    for (int32_t i = 0; i < count; i++) {
        generic_button *button = &select_list_buttons.items[i];
        int32_t column = i / rows;
        int32_t row = i % rows;
        int16_t x0 = (int16_t) ((column * SELECT_LIST_COLUMN_WIDTH) + SELECT_LIST_BUTTON_X);
        int16_t y0 = (int16_t) ((row * SELECT_LIST_ROW_HEIGHT) + SELECT_LIST_BUTTON_Y);
        button->x = x0;
        button->y = y0;
        button->x_end = (int16_t) (x0 + SELECT_LIST_BUTTON_WIDTH);
        button->y_end = (int16_t) (y0 + SELECT_LIST_BUTTON_HEIGHT);
        button->left_click_handler = editor_select_list_click;
        button->right_click_handler = button_none;
        button->reserved_1 = 0;
        button->pressed = 0;
        button->reserved_2 = 0;
        button->type = GENERIC_BUTTON_RELEASE;
        button->parameter1 = i;
        button->parameter2 = 0;
    }
}

static void show_resource_warning(int32_t text)
{
    window_current_id = WINDOW_EDITOR_RESOURCE_WARNING;
    editor_warning_draw(text);
}

static uint8_t city_slot_good(const empire_object *city, const uint8_t *target, int32_t slots)
{
    uint8_t old = 0;
    for (int32_t i = 0; i < slots; i++) {
        if (target == &city->sells[i]) {
            old = city->sells[i];
        }
    }
    return old;
}

static int32_t choose_city_good(int32_t item)
{
    const empire_object *city = &empire_map_objects[g_empire_selected_object];
    uint8_t *target = select_list.target8;
    if (item < RESOURCE_MIN_FOOD || item >= RESOURCE_MAX_FOOD) {
        *target = (uint8_t) item;
        return 1;
    }
    int32_t short_list = city->city_type != EMPIRE_CITY_OURS;
    uint8_t old = city_slot_good(city, target, short_list ? EMPIRE_OBJECT_BOUGHT_RESOURCES : EMPIRE_OBJECT_SOLD_RESOURCES);
    *target = 0;
    int32_t foods = empire_count_food_kinds();
    int32_t too_many_foods = (short_list && foods > FOOD_KINDS_MAX) || (foods == FOOD_KINDS_MAX && !empire_resource_available(item));
    if (too_many_foods) {
        *target = old;
        show_resource_warning(EDITOR_WARNING_FOOD_KINDS_FULL);
        return 0;
    }
    if (!short_list && item == RESOURCE_MEAT && !empire_resource_available(RESOURCE_STRAW)) {
        show_resource_warning(EDITOR_WARNING_MEAT_NEEDS_STRAW);
        return 0;
    }
    *target = (uint8_t) item;
    return 1;
}

static int32_t write_choice(int32_t item)
{
    switch (select_list.target_kind) {
        case INPUT_TARGET_INT8:
            if (window_current_id != WINDOW_EDITOR_SELECT_LIST) {
                *select_list.target8 = (uint8_t) item;
                return 1;
            }
            return choose_city_good(item);
        case INPUT_TARGET_INT16:
            *select_list.target16 = (int16_t) item;
            return 1;
        case INPUT_TARGET_INT32:
            *select_list.target32 = item + select_list_text_base;
            return 1;
        default:
            return 1;
    }
}

void editor_select_list_click(void)
{
    int32_t parameter = button_parameter;
    if (select_list_heading != 0 && (parameter == 0 || parameter == select_list_rows)) {
        return;
    }
    if (select_list_extra_on != 0) {
        select_list_extra_on = 0;
    }
    if (!write_choice(select_list.items[parameter])) {
        return;
    }
    if (select_list.use == SELECT_LIST_USE_NEW_SCENARIO && editor_new_map_size < NEW_MAP_SIZES) {
        game_file_editor_create_scenario(editor_new_map_size);
        if (editor_active == 1) {
            traders_clear();
            figure_create_editor_flags();
        }
    }
    window_input_delay_frames = SELECT_LIST_INPUT_DELAY_FRAMES;
    window_current_id = select_list_return_window.id;
    window_redraw_request = 1;
    if (select_list_on_choose != 0) {
        select_list_on_choose();
        select_list_on_choose = 0;
    }
}

void numeric_input_show(int32_t x, int32_t y, int32_t kind, uint8_t *target8, int16_t *target16, int32_t *target32, int32_t use)
{
    numeric_input_x = x;
    numeric_input_y = y;
    numeric_input_editor_use = use;
    numeric_input_target8 = (int8_t *) target8;
    numeric_input_width = kind;
    numeric_input_target32 = target32;
    numeric_input_target16 = target16;
    numeric_input_on_ok = 0;
    numeric_input_return_window.word = window_current_id;
    numeric_input_value = 0;
    numeric_input_digits = 0;
    window_current_id = WINDOW_NUMERIC_INPUT;
}
