// Interface elements drawn at a layout position: lang and number texts, multiline texts, button borders,
// animated labels and inner panels, whose position a window's entry in the layout file dialog.loc can override.
// Also the developer layout editor's mark of the element under the mouse, and the label width and column widths
// measured from the lang texts.
//
// The game ships no dialog.loc and never turns the editor on itself.

#include "graphics/ui_layout.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/layout_buttons.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "io/io_data.h"
#include "platform/platform_file.h"
#include "text/lang.h"
#include "window/building_menu.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t ui_layout_initialized;

int32_t ui_layout_label_columns;

enum ui_layout_metric {
    UI_LAYOUT_ADVISOR_KEYS = 100,
    UI_LAYOUT_TEXT_WIDTH = 100,
    UI_LAYOUT_NUMBER_WIDTH = 24,
    UI_LAYOUT_TEXT_HEIGHT = 24,
    UI_LAYOUT_COLUMN_EXTRA_BLOCKS = 2,
    UI_LAYOUT_BUILD_MENU_BLOCKS = 7,
    UI_LAYOUT_LABEL_WIDTH_EXTRA = 20,
    UI_LAYOUT_FRAME_FOCUSED = 0xFFFF,
};

enum dialog_loc_flag {
    DIALOG_LOC_END = 0x10,
    DIALOG_LOC_ERROR = 0x20,
};

typedef struct ui_layout_spot {
    int32_t key_x;
    int32_t key_y;
    int32_t x;
    int32_t y;
} ui_layout_spot;

int32_t ui_layout_column_groups[UI_LAYOUT_COLUMNS] = { 1, 2, 3, 4, 7, 10 };

int32_t ui_layout_window_key(int32_t window_id)
{
    if (window_id == WINDOW_ADVISORS) {
        window_id = advisor_shown + UI_LAYOUT_ADVISOR_KEYS;
    }
    return window_id;
}

ui_layout_entry *ui_layout_find(int32_t kind, int32_t key, int32_t x, int32_t y)
{
    const ui_layout_window *window = &ui_layout_lists[key];
    ui_layout_entry *e;
    if (kind == UI_LAYOUT_TEXT) {
        e = window->texts;
    } else if (kind == UI_LAYOUT_BUTTON) {
        e = window->buttons;
    } else if (kind == UI_LAYOUT_FRAME || kind == UI_LAYOUT_PANEL) {
        e = window->frames;
    } else {
        return 0;
    }
    for (; e; e = e->next) {
        if (x == e->key_x && y == e->key_y) {
            return e;
        }
    }
    return 0;
}

static int32_t mouse_inside(int32_t x, int32_t y, int32_t width, int32_t height)
{
    return x < mouse_x && mouse_x < x + width && y < mouse_y && mouse_y < y + height;
}

static void mark_element(int32_t base_x, int32_t base_y, int32_t key_x, int32_t key_y, int32_t width, int32_t height)
{
    ui_layout_clear_highlight();
    ui_layout_marked_x = base_x;
    ui_layout_marked_y = base_y;
    ui_layout_marked_key_x = key_x;
    ui_layout_marked_key_y = key_y;
    ui_layout_marked_width = width;
    ui_layout_marked_height = height;
}

static void frame_marked_element(int32_t x, int32_t y, int32_t width, int32_t height)
{
    button_border_draw(x, y, width, height, UI_LAYOUT_FRAME_FOCUSED);
}

void ui_layout_highlight(int32_t kind, int32_t x, int32_t y, int32_t width, int32_t height, int32_t base_x, int32_t base_y, int32_t offset_x, int32_t offset_y)
{
    int32_t screen_x = base_x + offset_x;
    int32_t screen_y = base_y + offset_y;
    if (kind == ui_layout_edit_mode && mouse_inside(screen_x, screen_y, width, height)) {
        mark_element(base_x, base_y, x - offset_x, y - offset_y, width, height);
        frame_marked_element(screen_x, screen_y, width, height);
    }
}

void ui_layout_refresh(void)
{
    if (ui_layout_edit_mode != 0) {
        window_redraw_request = 1;
    }
}

static int32_t dialog_loc_flags(void *file)
{
    return (feof((FILE *) file) ? DIALOG_LOC_END : 0) | (ferror((FILE *) file) ? DIALOG_LOC_ERROR : 0);
}

int32_t ui_layout_group_max_width(int32_t group)
{
    int32_t max = 1;
    int32_t index = 1;
    const char *end = lang_get_string(group + 1, 0);
    const char *text = lang_get_string(group, index);
    do {
        int32_t width = text_get_width((const uint8_t *) text, image_group(IMAGE_GROUP_FONT_BLACK));
        if (width > max) {
            max = width;
        }
        index++;
        text = lang_get_string(group, index);
    } while (end != text);
    return max;
}

void ui_layout_init_column_widths(void)
{
    for (int32_t i = 0; i < UI_LAYOUT_COLUMNS; i++) {
        ui_layout_column_widths[i] = (ui_layout_group_max_width(ui_layout_column_groups[i]) / UI_BLOCK_SIZE) + UI_LAYOUT_COLUMN_EXTRA_BLOCKS;
    }
}

int32_t ui_layout_init_label_width(void)
{
    int32_t width = ui_layout_group_max_width(LANG_GROUP_BUILDING_NAMES);
    int32_t blocks = width / UI_BLOCK_SIZE;
    ui_layout_label_columns = blocks;
    if ((width % UI_BLOCK_SIZE) != 0) {
        blocks++;
        ui_layout_label_columns = blocks;
    }
    generic_button *buttons = build_menu_buttons.build_menu;
    for (int32_t i = 0; i < BUILD_MENU_ITEMS; i++) {
        buttons[i].x_end = (int16_t) ((blocks + UI_LAYOUT_BUILD_MENU_BLOCKS) * UI_BLOCK_SIZE);
    }
    ui_layout_label_width = blocks * UI_BLOCK_SIZE;
    return ui_layout_label_width;
}

void dialog_loc_read_list(void *file, int32_t count, ui_layout_entry **list)
{
    if (dialog_loc_flags(file) & DIALOG_LOC_ERROR) {
        return;
    }
    *list = 0;
    for (; count > 0; count--) {
        ui_layout_entry *e = malloc(sizeof(ui_layout_entry));
        fread(&e->key_x, 4, 1, file);
        fread(&e->key_y, 4, 1, file);
        fread(&e->x, 4, 1, file);
        fread(&e->y, 4, 1, file);
        fread(&e->width, 4, 1, file);
        fread(&e->height, 4, 1, file);
        e->next = *list;
        if (dialog_loc_flags(file) & (DIALOG_LOC_END | DIALOG_LOC_ERROR)) {
            free(e);
        } else {
            *list = e;
        }
    }
}

static void read_counted_list(FILE *file, ui_layout_entry **list)
{
    int32_t count = 0;
    fread(&count, 4, 1, file);
    dialog_loc_read_list(file, count, list);
}

static void clear_window_layout(ui_layout_window *window)
{
    window->buttons = 0;
    window->texts = 0;
    window->frames = 0;
    window->buttons_applied = 1;
}

int32_t dialog_loc_load(void)
{
    FILE *file = platform_fopen(dialog_loc_name, file_mode_read_binary);
    if (file == 0) {
        return 0;
    }
    for (int32_t key = 0; key < UI_LAYOUT_KEYS; key++) {
        ui_layout_window *window = &ui_layout_lists[key];
        read_counted_list(file, &window->buttons);
        if (window->buttons != 0) {
            window->buttons_applied = 0;
        }
        read_counted_list(file, &window->texts);
        read_counted_list(file, &window->frames);
        if (dialog_loc_flags(file) & (DIALOG_LOC_END | DIALOG_LOC_ERROR)) {
            clear_window_layout(window);
            break;
        }
    }
    return fclose(file);
}

void ui_layout_init(void)
{
    for (int32_t key = 0; key < UI_LAYOUT_KEYS; key++) {
        clear_window_layout(&ui_layout_lists[key]);
    }
    ui_layout_initialized = 1;
    ui_layout_init_column_widths();
    ui_layout_init_label_width();
    dialog_loc_load();
}

static const ui_layout_entry *place(int32_t kind, int32_t x, int32_t y, int32_t offset_x, int32_t offset_y, ui_layout_spot *spot)
{
    if (ui_layout_initialized == 0) {
        ui_layout_init();
    }
    spot->key_x = x - offset_x;
    spot->key_y = y - offset_y;
    spot->x = spot->key_x;
    spot->y = spot->key_y;
    const ui_layout_entry *e = ui_layout_find(kind, ui_layout_window_key(window_current_id), spot->key_x, spot->key_y);
    if (e) {
        spot->x = e->x;
        spot->y = e->y;
    }
    return e;
}

void lang_text_draw_centered_ui(
    int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    const ui_layout_entry *e = place(UI_LAYOUT_TEXT, x, y, offset_x, offset_y, &spot);
    if (e) {
        box_width = e->width;
    }
    ui_layout_highlight(UI_LAYOUT_TEXT, x, y, box_width, UI_LAYOUT_TEXT_HEIGHT, spot.x, spot.y, offset_x, offset_y);
    lang_text_draw_centered(group, index, spot.x + offset_x, spot.y + offset_y, box_width, font, color);
}

void lang_text_draw_ui(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    place(UI_LAYOUT_TEXT, x, y, offset_x, offset_y, &spot);
    ui_layout_highlight(UI_LAYOUT_TEXT, x, y, UI_LAYOUT_TEXT_WIDTH, UI_LAYOUT_TEXT_HEIGHT, spot.x, spot.y, offset_x, offset_y);
    lang_text_draw(group, index, spot.x + offset_x, spot.y + offset_y, font, color);
}

void text_draw_number_ui(
    int32_t value, int32_t prefix, const char *postfix, int32_t x, int32_t y, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    place(UI_LAYOUT_TEXT, x, y, offset_x, offset_y, &spot);
    ui_layout_highlight(UI_LAYOUT_TEXT, x, y, UI_LAYOUT_NUMBER_WIDTH, UI_LAYOUT_TEXT_HEIGHT, spot.x, spot.y, offset_x, offset_y);
    text_draw_number(value, prefix, postfix, spot.x + offset_x, spot.y + offset_y, font, color);
}

void lang_text_draw_multiline_ui(
    int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    const ui_layout_entry *e = place(UI_LAYOUT_TEXT, x, y, offset_x, offset_y, &spot);
    if (e) {
        box_width = e->width;
    }
    ui_layout_highlight(UI_LAYOUT_TEXT, x, y, box_width, UI_LAYOUT_TEXT_HEIGHT, spot.x, spot.y, offset_x, offset_y);
    lang_text_draw_multiline(group, index, spot.x + offset_x, spot.y + offset_y, box_width, font, color);
}

void button_border_draw_ui(int32_t x, int32_t y, int32_t width, int32_t height, int32_t has_focus, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    const ui_layout_entry *e = place(UI_LAYOUT_FRAME, x, y, offset_x, offset_y, &spot);
    if (e) {
        width = e->width;
        height = e->height;
    }
    ui_layout_highlight(UI_LAYOUT_FRAME, x, y, width, height, spot.x, spot.y, offset_x, offset_y);
    button_border_draw(spot.x + offset_x, spot.y + offset_y, width, height, has_focus);
}

void label_draw_animated_ui(int32_t id, int32_t x, int32_t y, int32_t width_blocks, int32_t force, int32_t type, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    const ui_layout_entry *e = place(UI_LAYOUT_FRAME, x, y, offset_x, offset_y, &spot);
    if (e) {
        width_blocks = e->width;
    }
    int32_t screen_x = offset_x + spot.x;
    int32_t screen_y = offset_y + spot.y;
    int32_t width = width_blocks * UI_BLOCK_SIZE;
    if (ui_layout_edit_mode == UI_LAYOUT_FRAME && mouse_inside(screen_x, screen_y, width, UI_LAYOUT_TEXT_HEIGHT)) {
        mark_element(spot.x, spot.y, spot.key_x, spot.key_y, width_blocks, UI_LAYOUT_TEXT_HEIGHT);
        frame_marked_element(screen_x, screen_y, width, UI_LAYOUT_TEXT_HEIGHT);
    }
    label_draw_animated(id, screen_x, screen_y, width_blocks, force, type);
}

void inner_panel_draw_ui(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks, int32_t offset_x, int32_t offset_y)
{
    ui_layout_spot spot;
    const ui_layout_entry *e = place(UI_LAYOUT_PANEL, x, y, offset_x, offset_y, &spot);
    if (e) {
        width_blocks = e->width;
        height_blocks = e->height;
    }
    int32_t screen_x = spot.x + offset_x;
    int32_t screen_y = spot.y + offset_y;
    int32_t width = width_blocks * UI_BLOCK_SIZE;
    int32_t height = height_blocks * UI_BLOCK_SIZE;
    if (ui_layout_edit_mode == UI_LAYOUT_PANEL && mouse_inside(screen_x, screen_y, width, height)) {
        mark_element(spot.x, spot.y, spot.key_x, spot.key_y, width_blocks, height_blocks);
        frame_marked_element(screen_x, screen_y, width, height);
    }
    inner_panel_draw(screen_x, screen_y, width_blocks, height_blocks);
}

int32_t ui_layout_label_width_get(void)
{
    return ui_layout_label_width + UI_LAYOUT_LABEL_WIDTH_EXTRA;
}

int32_t ui_layout_label_columns_get(void)
{
    return ui_layout_label_columns;
}
