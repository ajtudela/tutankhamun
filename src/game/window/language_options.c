// The language window of the Options menu, drawn as the display options' list of resolutions
// (display_resolution_list.c): the title, a row per language (the current one in red) and the display options' row
// back to the city. Choosing a language lays its texts over the game's (text/lang_pack.c), remembers it and redraws
// everything in it.

#include "window/language_options.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "text/lang_pack.h"
#include "window/options_buttons.h"
#include "window/options_windows.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum language_options_layout {
    LANGUAGES_MAX = 16,
    TEXT_GROUP_DISPLAY = 42,
    TEXT_BACK = 6,
    PANEL_X = 96,
    PANEL_Y = 80,
    PANEL_WIDTH_BLOCKS = 18,
    PANEL_BLOCK_SIZE = 16,
    PANEL_HEIGHT_OUTSIDE_ROWS = 72,
    TITLE_Y = 94,
    ROW_X = 128,
    ROW_WIDTH = 224,
    ROW_LABEL_Y = 112,
    ROW_TEXT_Y = 116,
    ROW_HEIGHT = 24,
    ROW_LABEL_WIDTH_BLOCKS = 14,
    BUTTON_X = 144,
    BUTTON_X_END = 336,
    BUTTON_HEIGHT = 20,
    BUTTON_TYPE_TEXT = 1,
    LABEL_DISPLAY = 2,
    LABEL_FOCUSED = 1,
    LABEL_NORMAL = 2,
    LABEL_GROWN = 50,
};

static generic_button buttons[LANGUAGES_MAX + 1];
static int32_t button_count;

static void choose_language(void)
{
    if (button_parameter != lang_pack_current()) {
        lang_pack_select(button_parameter);
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static void set_button(int32_t row, button_handler handler, int32_t parameter)
{
    generic_button *button = &buttons[row - 1];
    button->x = BUTTON_X;
    button->y = (int16_t) (ROW_LABEL_Y + row * ROW_HEIGHT);
    button->x_end = BUTTON_X_END;
    button->y_end = (int16_t) (button->y + BUTTON_HEIGHT);
    button->left_click_handler = handler;
    button->right_click_handler = 0;
    button->reserved_1 = 0;
    button->pressed = 0;
    button->reserved_2 = 0;
    button->type = BUTTON_TYPE_TEXT;
    button->parameter1 = parameter;
    button->parameter2 = 0;
}

static void build_buttons(void)
{
    int32_t languages = lang_pack_count();
    if (languages > LANGUAGES_MAX) {
        languages = LANGUAGES_MAX;
    }
    for (int32_t i = 0; i < languages; i++) {
        set_button(i + 1, choose_language, i);
    }
    set_button(languages + 1, window_go_city, 1);
    button_count = languages + 1;
}

void window_language_options_show(void)
{
    build_buttons();
    window_current_id = WINDOW_LANGUAGE_OPTIONS;
    label_animation[LABEL_SLOT_DISPLAY_OPTIONS] = 0;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

generic_button *language_options_buttons(void)
{
    return buttons;
}

int32_t language_options_button_count(void)
{
    return button_count;
}

static int32_t row_color(int32_t row)
{
    return focus_button_id == row ? COLOR_ORANGE : COLOR_BLUE;
}

void window_language_options_draw(void)
{
    if (button_count == 0) {
        build_buttons();
    }
    int32_t rows = button_count;
    int32_t height = rows * ROW_HEIGHT + PANEL_HEIGHT_OUTSIDE_ROWS;
    outer_panel_draw(ui_origin_x + PANEL_X, ui_origin_y + PANEL_Y, PANEL_WIDTH_BLOCKS, (height + PANEL_BLOCK_SIZE - 1) / PANEL_BLOCK_SIZE);
    for (int32_t row = 1; row <= rows; row++) {
        label_draw_animated_ui(LABEL_DISPLAY, ui_origin_x + ROW_X, ui_origin_y + ROW_LABEL_Y + row * ROW_HEIGHT, ROW_LABEL_WIDTH_BLOCKS, 0,
            focus_button_id == row ? LABEL_FOCUSED : LABEL_NORMAL, ui_origin_x, ui_origin_y);
    }
    lang_text_draw_centered_ui(LANG_GROUP_MENU_OPTIONS, LANG_PACK_MENU_TEXT, ui_origin_x + ROW_X, ui_origin_y + TITLE_Y, ROW_WIDTH,
        image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    int32_t current = lang_pack_current();
    for (int32_t row = 1; row < rows; row++) {
        int32_t color = row_color(row);
        if (row - 1 == current && focus_button_id != row) {
            color = COLOR_RED;
        }
        text_draw_centered((const uint8_t *) lang_pack_name(row - 1), ui_origin_x + ROW_X, ui_origin_y + ROW_TEXT_Y + row * ROW_HEIGHT, ROW_WIDTH,
            image_group(IMAGE_GROUP_FONT_BLACK), color);
    }
    lang_text_draw_centered_ui(TEXT_GROUP_DISPLAY, TEXT_BACK, ui_origin_x + ROW_X, ui_origin_y + ROW_TEXT_Y + rows * ROW_HEIGHT, ROW_WIDTH,
        image_group(IMAGE_GROUP_FONT_BLACK), row_color(rows), ui_origin_x, ui_origin_y);
    graphics_reset_clip_rectangle();
    label_animation[LABEL_SLOT_DISPLAY_OPTIONS] = LABEL_GROWN;
}
