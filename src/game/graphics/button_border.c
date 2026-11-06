// Button borders, the animated labels of the message and advisor windows (a label opens with an animation
// counter per label id) and the dialog text helpers: centred, prefixed and lang texts, years and numbers drawn at
// a button rectangle plus the dialog offset.
//
// A border is drawn in UI blocks of the bordered button images; the last row and column move back by what the
// block grid overshoots. A prefixed text is built in string_number_buffer.

#include "graphics/button_border.h"
#include "core/text.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/grid.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum border_piece {
    BORDER_TOP_LEFT,
    BORDER_TOP,
    BORDER_TOP_RIGHT,
    BORDER_RIGHT,
    BORDER_BOTTOM_RIGHT,
    BORDER_BOTTOM,
    BORDER_BOTTOM_LEFT,
    BORDER_LEFT,
    BORDER_INSIDE = -1,
    BORDER_FOCUSED_IMAGES = 8,
};

enum label_animation_value {
    SMALL_LABEL_ANIMATION_GROWING = 20,
    SMALL_LABEL_ANIMATION_BLOCKS_OFFSET = 18,
    LABEL_REDRAW_FRAMES = 100,
};

enum label_images {
    LABEL_PIECES_PER_STYLE = 3,
    SMALL_LABEL_IMAGES = 40,
};

enum text_prefix_limit {
    TEXT_PREFIX_MAX_CHARS = 0xFF,
};

static const int32_t border_pieces[PIECE_PARTS][PIECE_PARTS] = {
    [PIECE_FIRST] = { BORDER_TOP_LEFT, BORDER_TOP, BORDER_TOP_RIGHT },
    [PIECE_MIDDLE] = { BORDER_LEFT, BORDER_INSIDE, BORDER_RIGHT },
    [PIECE_LAST] = { BORDER_BOTTOM_LEFT, BORDER_BOTTOM, BORDER_BOTTOM_RIGHT },
};

static int32_t blocks_for(int32_t pixels)
{
    int32_t blocks = pixels / UI_BLOCK_SIZE;
    if ((pixels % UI_BLOCK_SIZE) != 0) {
        blocks++;
    }
    return blocks;
}

void button_border_draw(int32_t x, int32_t y, int32_t width, int32_t height, int32_t has_focus)
{
    int32_t width_blocks = blocks_for(width);
    int32_t height_blocks = blocks_for(height);
    int32_t last_x_offset = (width_blocks * UI_BLOCK_SIZE) - width;
    int32_t last_y_offset = (height_blocks * UI_BLOCK_SIZE) - height;
    int32_t base = image_group(IMAGE_GROUP_BORDERED_BUTTON);
    if (has_focus) {
        base += BORDER_FOCUSED_IMAGES;
    }
    for (int32_t row = 0; row < height_blocks; row++) {
        int32_t row_part = piece_part(row, height_blocks);
        for (int32_t column = 0; column < width_blocks; column++) {
            int32_t column_part = piece_part(column, width_blocks);
            int32_t draw_x = (column * UI_BLOCK_SIZE) + x;
            int32_t draw_y = (row * UI_BLOCK_SIZE) + y;
            g_draw_x = draw_x;
            g_draw_y = draw_y;
            int32_t piece = border_pieces[row_part][column_part];
            if (piece == BORDER_INSIDE) {
                continue;
            }
            if (column_part == PIECE_LAST) {
                draw_x -= last_x_offset;
            }
            if (row_part == PIECE_LAST) {
                draw_y -= last_y_offset;
            }
            image_draw(base + piece, draw_x, draw_y);
        }
    }
}

static void draw_label_blocks(int32_t first_image, int32_t x, int32_t y, int32_t blocks)
{
    for (int32_t i = 0; i < blocks; i++) {
        int32_t draw_x = (i * UI_BLOCK_SIZE) + x;
        g_draw_y = y;
        g_draw_x = draw_x;
        g_last_image_id = first_image + piece_part(i, blocks);
        image_draw(g_last_image_id, draw_x, y);
    }
}

static int32_t centre_label(int32_t blocks, int32_t width_blocks, int32_t *x)
{
    if (blocks > width_blocks) {
        return width_blocks;
    }
    *x += ((width_blocks - blocks) * UI_BLOCK_SIZE) / 2;
    return blocks;
}

void large_label_draw_animated(int32_t id, int32_t x, int32_t y, int32_t width_blocks, int32_t type)
{
    int32_t animation = label_animation[id];
    if (animation < LABEL_ANIMATION_OPENING) {
        return;
    }
    if (animation < LABEL_ANIMATION_GROWING) {
        const int32_t *frame = &large_label_frames[(animation - LABEL_ANIMATION_OPENING) * LARGE_LABEL_FRAME_FIELDS];
        int32_t draw_y = frame[LARGE_LABEL_FRAME_Y] + y;
        int32_t draw_x = (((width_blocks - 2) * UI_BLOCK_SIZE) / 2) + frame[LARGE_LABEL_FRAME_X] + x;
        g_draw_x = draw_x;
        g_draw_y = draw_y;
        int32_t image = frame[LARGE_LABEL_FRAME_IMAGE] + LABEL_OPENING_IMAGES + (type * LABEL_OPENING_IMAGES_PER_STYLE) + image_group(IMAGE_GROUP_PANEL_BUTTON);
        image_draw(image, draw_x, draw_y);
        return;
    }
    int32_t blocks = centre_label(animation - LABEL_ANIMATION_BLOCKS_OFFSET, width_blocks, &x);
    draw_label_blocks(image_group(IMAGE_GROUP_PANEL_BUTTON) + (type * LABEL_PIECES_PER_STYLE), x, y, blocks);
}

int32_t label_animation_click(void)
{
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = LABEL_REDRAW_FRAMES;
    return 1;
}

void label_draw_animated(int32_t id, int32_t x, int32_t y, int32_t width_blocks, int32_t force, int32_t type)
{
    int32_t blocks = width_blocks;
    if (force != 1) {
        int32_t animation = label_animation[id];
        if (animation < SMALL_LABEL_ANIMATION_GROWING) {
            return;
        }
        blocks = centre_label(animation - SMALL_LABEL_ANIMATION_BLOCKS_OFFSET, width_blocks, &x);
    }
    draw_label_blocks(image_group(IMAGE_GROUP_PANEL_BUTTON) + SMALL_LABEL_IMAGES + (type * LABEL_PIECES_PER_STYLE), x, y, blocks);
}

void button_border_draw_rect(const int16_t *rect, int32_t has_focus, int32_t x, int32_t y)
{
    button_border_draw(rect[0] + ui_origin_x + x, rect[1] + ui_origin_y + y, rect[2] - rect[0], rect[3] - rect[1], has_focus);
}

void lang_text_draw_centered_rect(int32_t group, int32_t index, const int16_t *rect, int32_t font, int32_t color, int32_t x, int32_t y)
{
    lang_text_draw_centered(group, index, rect[0] + ui_origin_x + x, rect[1] + ui_origin_y + y, rect[2] - rect[0], font, color);
}

void lang_text_draw_rect_ui(
    int32_t group, int32_t index, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy, int32_t offset_x, int32_t offset_y)
{
    lang_text_draw_ui(group, index, rect[0] + ui_origin_x + dx, rect[1] + ui_origin_y + dy, font, color, offset_x, offset_y);
}

void text_draw_number_rect(int32_t value, int32_t prefix, const char *postfix, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy)
{
    text_draw_number(value, prefix, postfix, rect[0] + ui_origin_x + dx, rect[1] + ui_origin_y + dy, font, color);
}

void text_draw_in_rect(const char *text, const int16_t *rect, int32_t font, int32_t color, int32_t x, int32_t y)
{
    text_draw((const uint8_t *) text, rect[0] + ui_origin_x + x, rect[1] + ui_origin_y + y, font, color);
}

static int32_t copy_limited(char *buffer, int32_t n, const char *source)
{
    while (n < TEXT_PREFIX_MAX_CHARS) {
        buffer[n++] = *source;
        if (*++source == 0) {
            break;
        }
    }
    return n;
}

void text_draw_prefixed_centered(const char *prefix, const char *text, int32_t x, int32_t y, int32_t width, int32_t font, int32_t color)
{
    char *buffer = string_number_buffer;
    int32_t n = 0;
    if (prefix) {
        if (*prefix) {
            n = copy_limited(buffer, n, prefix);
        }
        buffer[n++] = ' ';
    }
    if (text && *text) {
        n = copy_limited(buffer, n, text);
    }
    buffer[n] = 0;
    text_draw_centered((const uint8_t *) buffer, x, y, width, font, color);
}

void text_draw_number_prefixed_centered(
    int32_t value, const char *prefix, const char *postfix, int32_t x, int32_t y, int32_t width, int32_t font, int32_t color)
{
    char *buffer = string_number_buffer;
    int32_t prefix_length = (int32_t) strlen(prefix);
    int32_t n = string_from_int(value, prefix_length + 1, 0);
    memcpy(buffer, prefix, (size_t) prefix_length);
    buffer[prefix_length] = ' ';
    buffer[n] = ' ';
    n++;
    for (; *postfix; postfix++) {
        buffer[n++] = *postfix;
    }
    buffer[n] = 0;
    text_draw_centered((const uint8_t *) buffer, x, y, width, font, color);
}

void lang_text_draw_at_rect(int32_t group, int32_t index, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy)
{
    lang_text_draw(group, index, rect[0] + ui_origin_x + dx, rect[1] + ui_origin_y + dy, font, color);
}

void lang_text_draw_year_at_rect(int32_t bc_spacing, int32_t year, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy)
{
    lang_text_draw_year(bc_spacing, year, rect[0] + ui_origin_x + dx, rect[1] + ui_origin_y + dy, font, color);
}

void text_draw_number_at_rect(int32_t value, int32_t prefix, const char *postfix, const int16_t *rect, int32_t font, int32_t color, int32_t dx, int32_t dy)
{
    text_draw_number_centered(
        value, prefix, postfix, rect[0] + ui_origin_x + dx, rect[1] + ui_origin_y + dy, (int32_t) rect[2] - (int32_t) rect[0], font, color);
}
