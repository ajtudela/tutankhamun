// The select list and numeric input (editor_select_list.c): their functions, what the editor does with a choice
// (select_list_use) and the most kinds of food a city may trade (FOOD_KINDS_MAX).

#ifndef EDITOR_EDITOR_SELECT_LIST_H
#define EDITOR_EDITOR_SELECT_LIST_H

#include "window/input_dialogs.h"
#include <stdbool.h>
#include <stdint.h>

enum select_list_use {
    SELECT_LIST_USE_NONE = 0,
    SELECT_LIST_USE_NEW_SCENARIO = 2,
};

enum empire_food_kinds {
    FOOD_KINDS_MAX = 4,
};

void editor_select_list_show(int32_t x, int32_t y, int32_t count, int32_t group, int32_t text_base, uint8_t *target8, int16_t *target16, int32_t *target32,
    int32_t use, int32_t target_kind, bool keep_items, input_dialog_callback on_choose, int32_t in_editor, int32_t rows, int32_t heading);
void numeric_input_show(int32_t x, int32_t y, int32_t kind, uint8_t *target8, int16_t *target16, int32_t *target32, int32_t use);
void editor_select_list_click(void);

#endif
