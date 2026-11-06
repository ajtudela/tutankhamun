// The numeric input and the select list (input_dialogs.c): their draws, buttons and state.

#ifndef WINDOW_INPUT_DIALOGS_H
#define WINDOW_INPUT_DIALOGS_H

#include "graphics/button.h"
#include <stdint.h>

enum select_list_size {
    SELECT_LIST_MAX_ITEMS = 78,
    SELECT_LIST_TRADE_AMOUNT_ROWS = 16,
    SELECT_LIST_EXTRA_BUTTONS = 2,
};

enum input_target_width {
    INPUT_TARGET_INT8 = 1,
    INPUT_TARGET_INT16 = 2,
    INPUT_TARGET_INT32 = 4,
};

enum numeric_input_editor_use {
    NUMERIC_INPUT_EDITOR_NONE = 0,
    NUMERIC_INPUT_EDITOR_EMPIRE_MAP = 1,
    NUMERIC_INPUT_EDITOR_EMPIRE_FILE = 2,
};

typedef void (*input_dialog_callback)(void);

typedef void (*input_dialog_callback)(void);

typedef struct {
    int32_t items[SELECT_LIST_MAX_ITEMS];
    int32_t target_kind;
    uint8_t *target8;
    int16_t *target16;
    int32_t *target32;
    int32_t use;
} select_list_state;

typedef struct {
    generic_button items[SELECT_LIST_MAX_ITEMS];
    int32_t trade_amount_resources[SELECT_LIST_TRADE_AMOUNT_ROWS];
} select_list_button_table;

void lang_text_draw_centered_in_rect(
    int32_t group, int32_t index, const generic_button *button, int32_t font, int32_t color, int32_t x, int32_t y, int32_t clip_x, int32_t clip_y);
void numeric_input_digit(void);
void numeric_input_ok(void);
void numeric_input_cancel(void);
void select_list_pharaoh_names_page_1(void);
void select_list_pharaoh_names_page_2(void);
void window_numeric_input_draw(int32_t x, int32_t y);
void window_select_list_draw(int32_t x, int32_t y, int32_t rows);

#endif
