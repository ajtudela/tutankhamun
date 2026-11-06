// The play mode's display options window with a list of resolutions, after Akhenaten's display options
// (src/scripts/ui_display_options_window.js and get_video_modes in src/platform/renderer.cpp, AGPL-3.0), its rows
// drawn the way Julius' select list draws them (src/window/select_list.c, AGPL-3.0).
//
// The list holds the game's three standard sizes and, once the display's size is known, every size from 640x480 up
// to it; any other size plays in the display mode of the largest standard size that fits. A long list scrolls by a
// page. The play mode turns the list on with its display's sizes; otherwise the standard window is shown.

#include "window/display_resolution_list.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "window/options_buttons.h"
#include "window/options_windows.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum display_resolution_list_text {
    TEXT_GROUP = 42,
    TEXT_TITLE = 0,
    TEXT_WINDOWED = 1,
    TEXT_FULLSCREEN = 2,
    TEXT_BACK = 6,
};

enum display_resolution_list_layout {
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
    ROW_MODE = 1,
    ROWS_AROUND_LIST = 2,
    LABEL_DISPLAY = 2,
    LABEL_FOCUSED = 1,
    LABEL_NORMAL = 2,
    LABEL_GROWN = 50,
    STANDARD_RESOLUTIONS = 3,
    LIST_VISIBLE = 9,
    MIN_WIDTH = 640,
    MIN_HEIGHT = 480,
};

static int32_t list_allowed;
static screen_size list_entries[DISPLAY_RESOLUTION_LIST_MAX];
static int32_t list_entry_count;
static generic_button list_buttons[DISPLAY_RESOLUTION_LIST_MAX + ROWS_AROUND_LIST];
static int32_t list_button_count;
static int32_t list_first;
static int32_t list_scrolls;
static screen_size display_limit;
static screen_size custom_size;
static screen_size requested_size;
static int32_t size_requested;

static const screen_size standard_resolutions[STANDARD_RESOLUTIONS] = {
    { 640, 480 },
    { 800, 600 },
    { 1024, 768 },
};

void display_resolution_list_allow(int32_t allowed)
{
    list_allowed = allowed;
}

int32_t display_resolution_list_allowed(void)
{
    return list_allowed;
}

int32_t display_resolution_mode(int32_t width, int32_t height)
{
    for (int32_t i = 0; i < STANDARD_RESOLUTIONS; i++) {
        if (standard_resolutions[i].width == width && standard_resolutions[i].height == height) {
            return DISPLAY_MODE_640X480 + i;
        }
    }
    return 0;
}

void display_resolution_set_limit(int32_t width, int32_t height)
{
    display_limit.width = width;
    display_limit.height = height;
}

int32_t display_resolution_supported(int32_t width, int32_t height)
{
    if (display_resolution_mode(width, height) != 0) {
        return 1;
    }
    return display_limit.width >= MIN_WIDTH && display_limit.height >= MIN_HEIGHT && width >= MIN_WIDTH && height >= MIN_HEIGHT &&
        width <= display_limit.width && height <= display_limit.height;
}

int32_t display_resolution_layout_mode(int32_t width, int32_t height)
{
    int32_t mode = DISPLAY_MODE_640X480;
    for (int32_t i = 0; i < STANDARD_RESOLUTIONS; i++) {
        if (standard_resolutions[i].width <= width && standard_resolutions[i].height <= height) {
            mode = DISPLAY_MODE_640X480 + i;
        }
    }
    return mode;
}

void display_resolution_start_custom(screen_size size)
{
    custom_size.width = 0;
    custom_size.height = 0;
    if (display_resolution_mode(size.width, size.height) == 0) {
        custom_size = size;
    }
}

void display_resolution_request(screen_size size)
{
    requested_size = size;
    size_requested = 1;
}

void display_resolution_take_request(void)
{
    screen_size none = { 0, 0 };
    custom_size = none;
    if (size_requested && display_resolution_mode(requested_size.width, requested_size.height) == 0) {
        custom_size = requested_size;
    }
    size_requested = 0;
}

void display_resolution_apply_custom(void)
{
    if (custom_size.width > 0 && custom_size.height > 0) {
        g_screen_width = custom_size.width;
        g_screen_height = custom_size.height;
    }
}

int32_t display_resolution_custom_active(void)
{
    return custom_size.width > 0 && custom_size.height > 0;
}

void display_resolution_custom_screen(int32_t *width, int32_t *height)
{
    if (custom_size.width > 0 && custom_size.height > 0) {
        *width = custom_size.width;
        *height = custom_size.height;
    }
}

screen_size display_resolution_of_mode(int32_t mode)
{
    int32_t index = mode - DISPLAY_MODE_640X480;
    if (index < 0 || index >= STANDARD_RESOLUTIONS) {
        index = 0;
    }
    return standard_resolutions[index];
}

static int32_t distance(int32_t a, int32_t b)
{
    return a > b ? a - b : b - a;
}

static int32_t clamp(int32_t value, int32_t low, int32_t high)
{
    return value < low ? low : (value > high ? high : value);
}

screen_size display_resolution_nearest(int32_t width, int32_t height)
{
    if (display_limit.width >= MIN_WIDTH && display_limit.height >= MIN_HEIGHT) {
        screen_size clamped = { clamp(width, MIN_WIDTH, display_limit.width), clamp(height, MIN_HEIGHT, display_limit.height) };
        return clamped;
    }
    screen_size nearest = standard_resolutions[0];
    int32_t nearest_distance = distance(width, nearest.width) + distance(height, nearest.height);
    for (int32_t i = 1; i < STANDARD_RESOLUTIONS; i++) {
        int32_t d = distance(width, standard_resolutions[i].width) + distance(height, standard_resolutions[i].height);
        if (d < nearest_distance) {
            nearest = standard_resolutions[i];
            nearest_distance = d;
        }
    }
    return nearest;
}

static int32_t comes_before(screen_size a, screen_size b)
{
    return a.width < b.width || (a.width == b.width && a.height < b.height);
}

static void add_entry(screen_size size)
{
    if (!display_resolution_supported(size.width, size.height) || list_entry_count >= DISPLAY_RESOLUTION_LIST_MAX) {
        return;
    }
    int32_t at = 0;
    while (at < list_entry_count && comes_before(list_entries[at], size)) {
        at++;
    }
    if (at < list_entry_count && list_entries[at].width == size.width && list_entries[at].height == size.height) {
        return;
    }
    for (int32_t i = list_entry_count; i > at; i--) {
        list_entries[i] = list_entries[i - 1];
    }
    list_entries[at] = size;
    list_entry_count++;
}

static void select_resolution(void)
{
    int32_t index = button_parameter;
    if (index < 0 || index >= list_entry_count) {
        return;
    }
    screen_size size = list_entries[index];
    if (display_resolution_mode(size.width, size.height) == 0) {
        display_resolution_request(size);
    }
    button_parameter = display_resolution_layout_mode(size.width, size.height);
    display_options_button_resolution();
}

static void build_buttons(void);

static void scroll_list(void)
{
    int32_t last_first = list_entry_count - LIST_VISIBLE;
    list_first += button_parameter < 0 ? -LIST_VISIBLE : LIST_VISIBLE;
    list_first = clamp(list_first, 0, last_first > 0 ? last_first : 0);
    build_buttons();
    window_redraw_request = 1;
}

static void set_button(int32_t row, button_handler handler, int32_t parameter)
{
    generic_button *button = &list_buttons[row - 1];
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

static int32_t shown_entries(void)
{
    return list_scrolls ? LIST_VISIBLE : list_entry_count;
}

static int32_t first_entry_row(void)
{
    return list_scrolls ? ROW_MODE + 2 : ROW_MODE + 1;
}

static void build_buttons(void)
{
    list_scrolls = list_entry_count > LIST_VISIBLE + ROWS_AROUND_LIST;
    if (!list_scrolls) {
        list_first = 0;
    }
    set_button(ROW_MODE, display_options_button_mode, 1);
    int32_t row = first_entry_row();
    if (list_scrolls) {
        set_button(ROW_MODE + 1, scroll_list, -1);
    }
    for (int32_t i = 0; i < shown_entries(); i++) {
        set_button(row++, select_resolution, list_first + i);
    }
    if (list_scrolls) {
        set_button(row++, scroll_list, 1);
    }
    set_button(row, window_go_city, 1);
    list_button_count = row;
}

void display_resolution_list_set_display_modes(const screen_size *modes, int32_t count)
{
    list_entry_count = 0;
    for (int32_t i = 0; i < STANDARD_RESOLUTIONS; i++) {
        add_entry(standard_resolutions[i]);
    }
    for (int32_t i = 0; i < count; i++) {
        add_entry(modes[i]);
    }
    screen_size current = { g_screen_width, g_screen_height };
    add_entry(current);
    list_first = 0;
    build_buttons();
}

static void build_if_empty(void)
{
    if (list_button_count == 0) {
        display_resolution_list_set_display_modes(0, 0);
    }
}

int32_t display_resolution_list_count(void)
{
    build_if_empty();
    return list_entry_count;
}

screen_size display_resolution_list_entry(int32_t index)
{
    build_if_empty();
    if (index < 0 || index >= list_entry_count) {
        return standard_resolutions[0];
    }
    return list_entries[index];
}

generic_button *display_resolution_list_buttons(void)
{
    build_if_empty();
    return list_buttons;
}

int32_t display_resolution_list_button_count(void)
{
    build_if_empty();
    return list_button_count;
}

static int32_t row_color(int32_t row)
{
    return focus_button_id == row ? COLOR_ORANGE : COLOR_BLUE;
}

static void format_number(char **out, int32_t value)
{
    char digits[DISPLAY_RESOLUTION_TEXT_SIZE];
    int32_t count = 0;
    do {
        digits[count++] = (char) ('0' + value % 10);
        value /= 10;
    } while (value > 0 && count < DISPLAY_RESOLUTION_TEXT_SIZE);
    while (count > 0) {
        *(*out)++ = digits[--count];
    }
}

static void format_resolution(char *text, screen_size size)
{
    char *out = text;
    format_number(&out, size.width);
    *out++ = ' ';
    *out++ = 'x';
    *out++ = ' ';
    format_number(&out, size.height);
    *out = 0;
}

static int32_t is_current(screen_size size)
{
    return size.width == g_screen_width && size.height == g_screen_height;
}

static void draw_row_label(int32_t row)
{
    label_draw_animated_ui(LABEL_DISPLAY, ui_origin_x + ROW_X, ui_origin_y + ROW_LABEL_Y + row * ROW_HEIGHT, ROW_LABEL_WIDTH_BLOCKS, 0,
        focus_button_id == row ? LABEL_FOCUSED : LABEL_NORMAL, ui_origin_x, ui_origin_y);
}

static void draw_row_lang_text(int32_t row, int32_t text, int32_t font)
{
    lang_text_draw_centered_ui(TEXT_GROUP, text, ui_origin_x + ROW_X, ui_origin_y + ROW_TEXT_Y + row * ROW_HEIGHT, ROW_WIDTH, font, row_color(row),
        ui_origin_x, ui_origin_y);
}

static void draw_resolution_row(int32_t row, screen_size size)
{
    char text[DISPLAY_RESOLUTION_TEXT_SIZE * 2];
    format_resolution(text, size);
    int32_t color = row_color(row);
    if (is_current(size) && focus_button_id != row) {
        color = COLOR_RED;
    }
    text_draw_centered((const uint8_t *) text, ui_origin_x + ROW_X, ui_origin_y + ROW_TEXT_Y + row * ROW_HEIGHT, ROW_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), color);
}

static void draw_scroll_row(int32_t row)
{
    text_draw_centered((const uint8_t *) "...", ui_origin_x + ROW_X, ui_origin_y + ROW_TEXT_Y + row * ROW_HEIGHT, ROW_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), row_color(row));
}

void display_resolution_list_draw(void)
{
    build_if_empty();
    int32_t rows = list_button_count;
    int32_t height = rows * ROW_HEIGHT + PANEL_HEIGHT_OUTSIDE_ROWS;
    outer_panel_draw(ui_origin_x + PANEL_X, ui_origin_y + PANEL_Y, PANEL_WIDTH_BLOCKS, (height + PANEL_BLOCK_SIZE - 1) / PANEL_BLOCK_SIZE);
    for (int32_t row = 1; row <= rows; row++) {
        draw_row_label(row);
    }
    lang_text_draw_centered_ui(TEXT_GROUP, TEXT_TITLE, ui_origin_x + ROW_X, ui_origin_y + TITLE_Y, ROW_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    draw_row_lang_text(ROW_MODE, settings_data.fullscreen != 0 ? TEXT_FULLSCREEN : TEXT_WINDOWED, image_group(IMAGE_GROUP_FONT_BLACK));
    int32_t row = first_entry_row();
    if (list_scrolls) {
        draw_scroll_row(ROW_MODE + 1);
    }
    for (int32_t i = 0; i < shown_entries(); i++) {
        draw_resolution_row(row++, list_entries[list_first + i]);
    }
    if (list_scrolls) {
        draw_scroll_row(row);
    }
    draw_row_lang_text(rows, TEXT_BACK, image_group(IMAGE_GROUP_FONT_BLACK));
    graphics_reset_clip_rectangle();
    label_animation[LABEL_SLOT_DISPLAY_OPTIONS] = LABEL_GROWN;
}
