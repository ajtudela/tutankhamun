// The file dialogs over the file list: saved games (save, load, delete) and maps (the editor's save and open), with
// the title, up to 12 rows of the list, the name typed with its cursor, OK and cancel and the scroll bar.
//
// The map dialog's title is centred 32 pixels further left.

#include "window/file_dialog.h"
#include "core/calc.h"
#include "graphics/color.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/file_list.h"
#include "window/text_input.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum file_dialog_layout {
    FILE_DIALOG_OTHER_LAYOUT_ROWS = 15,
    FILE_DIALOG_ROW_HEIGHT = 16,
    FILE_DIALOG_TEXT_X = 160,
    FILE_DIALOG_TITLE_Y = 50,
    FILE_DIALOG_TITLE_WIDTH = 304,
    FILE_DIALOG_NAME_Y = 90,
    FILE_DIALOG_BUTTONS = 4,
    FILE_DIALOG_FILENAME_SIZE = 260,
};

enum file_dialog_scrollbar {
    SCROLLBAR_DOT_IMAGE = 39,
    FILE_DIALOG_SCROLLBAR_RANGE = 130,
    FILE_DIALOG_SCROLLBAR_X = 472,
    FILE_DIALOG_SCROLLBAR_Y = 145,
    OTHER_LAYOUT_SCROLLBAR_RANGE = 164,
    OTHER_LAYOUT_SCROLLBAR_X = 284,
    OTHER_LAYOUT_SCROLLBAR_Y = 245,
};

void window_file_dialog_draw_scrollbar(void)
{
    int32_t rows = (file_dialog_scroll_layout == 0) ? FILE_DIALOG_ROWS : FILE_DIALOG_OTHER_LAYOUT_ROWS;
    int32_t range = file_dialog_scroll_total - rows;
    int32_t percentage;
    if (range <= 0) {
        return;
    }
    if (file_dialog_scroll <= 0) {
        percentage = 0;
    } else if ((file_dialog_scroll + rows) >= file_dialog_scroll_total) {
        percentage = 100;
    } else {
        percentage = calc_percentage(file_dialog_scroll, range);
    }
    int32_t dot = image_group(IMAGE_GROUP_PANEL_BUTTON) + SCROLLBAR_DOT_IMAGE;
    if (file_dialog_scroll_layout == 0) {
        int32_t offset = calc_adjust_with_percentage(FILE_DIALOG_SCROLLBAR_RANGE, percentage);
        image_draw(dot, ui_origin_x + FILE_DIALOG_SCROLLBAR_X, ui_origin_y + offset + FILE_DIALOG_SCROLLBAR_Y);
    } else {
        int32_t offset = calc_adjust_with_percentage(OTHER_LAYOUT_SCROLLBAR_RANGE, percentage);
        image_draw(dot, ui_origin_x + OTHER_LAYOUT_SCROLLBAR_X, ui_origin_y + offset + OTHER_LAYOUT_SCROLLBAR_Y);
    }
}

static void draw_panels(void)
{
    window_ui_redraw = 1;
    outer_panel_draw(ui_origin_x + 128, ui_origin_y + 40, 24, 21);
    inner_panel_draw_ui(ui_origin_x + 144, ui_origin_y + 80, 20, 2, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(ui_origin_x + 144, ui_origin_y + 120, 20, 13, ui_origin_x, ui_origin_y);
}

static void draw_title(int32_t text, int32_t x, int32_t color)
{
    lang_text_draw_centered_ui(
        LANG_GROUP_FILE_DIALOG, text, ui_origin_x + x, ui_origin_y + FILE_DIALOG_TITLE_Y, FILE_DIALOG_TITLE_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), color, ui_origin_x, ui_origin_y);
}

static void draw_proceed(void)
{
    lang_text_draw_ui(LANG_GROUP_FILE_DIALOG, FILE_DIALOG_TEXT_PROCEED, ui_origin_x + 224, ui_origin_y + 342, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_RED, ui_origin_x, ui_origin_y);
}

static void draw_rows(int32_t without_extension)
{
    char name[FILE_DIALOG_FILENAME_SIZE];
    int32_t rows = (file_list_saves.count > FILE_DIALOG_ROWS) ? FILE_DIALOG_ROWS : file_list_saves.count;
    for (int32_t i = 1; i <= rows; i++) {
        int32_t font = (focus_button_id == i) ? image_group(IMAGE_GROUP_FONT_WHITE) : image_group(IMAGE_GROUP_FONT_GREEN);
        strcpy(name, file_list_name(file_dialog_scroll + i - 1));
        if (without_extension) {
            file_remove_extension(name);
        }
        text_draw((const uint8_t *) name, ui_origin_x + FILE_DIALOG_TEXT_X, ui_origin_y + 114 + (i * FILE_DIALOG_ROW_HEIGHT), font, 0);
    }
}

static void draw_name_typed(const char *name)
{
    text_cursor_reset();
    text_draw((const uint8_t *) name, ui_origin_x + FILE_DIALOG_TEXT_X, ui_origin_y + FILE_DIALOG_NAME_Y, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    graphics_draw_text_cursor(ui_origin_x + FILE_DIALOG_TEXT_X, ui_origin_y + FILE_DIALOG_NAME_Y + 1, COLOR_WHITE);
}

void window_file_dialog_draw(void)
{
    draw_panels();
    inner_panel_draw_ui(ui_origin_x + 467, ui_origin_y + 144, 2, 10, ui_origin_x, ui_origin_y);
    if (file_dialog_error_frames != 0) {
        file_dialog_error_frames--;
        draw_title(FILE_DIALOG_TEXT_FILE_DOES_NOT_EXIST, FILE_DIALOG_TEXT_X, COLOR_RED);
    } else {
        draw_title((file_dialog_type == FILE_DIALOG_DELETE) ? FILE_DIALOG_TEXT_DELETE_A_FILE : file_dialog_type, FILE_DIALOG_TEXT_X, COLOR_WHITE);
    }
    draw_proceed();
    draw_rows(0);
    image_buttons_draw(ui_origin_x, ui_origin_y, file_dialog_buttons, FILE_DIALOG_BUTTONS);
    draw_name_typed(file_dialog_name);
    window_file_dialog_draw_scrollbar();
}

void window_map_file_dialog_draw(void)
{
    draw_panels();
    draw_title(FILE_DIALOG_TEXT_SAVING_A_MISSION + file_dialog_type, 128, COLOR_WHITE);
    draw_proceed();
    draw_rows(1);
    image_buttons_draw(ui_origin_x, ui_origin_y, map_dialog_buttons, FILE_DIALOG_BUTTONS);
    draw_name_typed(scenario_map_name);
    window_file_dialog_draw_scrollbar();
}
