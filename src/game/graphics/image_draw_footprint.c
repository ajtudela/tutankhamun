// Isometric footprints on the RGB565 back buffer: one tile's diamond, whole or cut at the top, bottom, left
// or right, with or without the colour mask, and footprints of 2..6 tiles each way drawn tile by tile in
// diagonal rows through the clip helpers.
//
// The left-cut tile repeats the previous dword in one row and mixes two pixels in another.

#include "graphics/image_draw_footprint.h"
#include "graphics/graphics_data.h"
#include "graphics/image_draw.h"
#include "graphics/screen_pixels.h"
#include "io/io_data.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum tile_layout {
    TILE_ROW_DWORDS_HALF = 14,
    FOOTPRINT_TILE_BYTES = 1800,
    FOOTPRINT_TILE_HALF_WIDTH = FOOTPRINT_TILE_WIDTH / 2,
};

enum tile_part {
    TILE_WHOLE,
    TILE_CUT_LEFT,
    TILE_CUT_RIGHT,
};

enum left_cut_slip {
    LEFT_CUT_REPEAT_ROW = 15,
    LEFT_CUT_REPEAT_DWORD = 4,
    LEFT_CUT_MIXED_ROW = 25,
    LEFT_CUT_MIXED_DWORD = 3,
};

enum pixel_pair {
    PIXEL_PAIR_LOW_MASK = 0xFFFF,
    PIXEL_PAIR_HIGH_MASK = 0xFFFF0000u,
    PIXEL_PAIR_SHIFT = 16,
};

static int32_t row_from_edge(int32_t row)
{
    return (row < FOOTPRINT_TILE_HALF_HEIGHT) ? row : (FOOTPRINT_TILE_HEIGHT - 1) - row;
}

static uint32_t left_cut_pair(const uint32_t *source, int32_t row, int32_t dword)
{
    if (row == LEFT_CUT_REPEAT_ROW && dword == LEFT_CUT_REPEAT_DWORD) {
        return source[-1];
    }
    if (row == LEFT_CUT_MIXED_ROW && dword == LEFT_CUT_MIXED_DWORD) {
        return (source[-1] & PIXEL_PAIR_HIGH_MASK) | (source[0] & PIXEL_PAIR_LOW_MASK);
    }
    return source[0];
}

static void draw_tile_row(uint8_t *row_address, const uint32_t *row_source, int32_t row, int32_t part, uint32_t mask)
{
    int32_t edge = row_from_edge(row);
    int32_t first_x = TILE_ROW_DWORDS_HALF - edge;
    uint32_t *pairs = (uint32_t *) row_address;
    if (part == TILE_WHOLE) {
        for (int32_t i = 0; i <= 2 * edge; i++) {
            pairs[first_x + i] = row_source[i] & mask;
        }
    } else if (part == TILE_CUT_RIGHT) {
        for (int32_t i = 0; i < edge; i++) {
            pairs[first_x + i] = row_source[i] & mask;
        }
    } else {
        const uint32_t *right_half = &row_source[edge + 1];
        for (int32_t i = 0; i < edge; i++) {
            pairs[i] = left_cut_pair(&right_half[i], row, i) & mask;
        }
    }
}

static uint32_t draw_tile(const uint32_t *data, int32_t clip_y, int32_t part, uint32_t mask)
{
    uint8_t *tile_address = screen_pixel_address(g_draw_x, g_draw_y);
    int32_t first_row = (clip_y == IMAGE_CLIP_TOP) ? FOOTPRINT_TILE_HALF_HEIGHT : 0;
    int32_t end_row = (clip_y == IMAGE_CLIP_BOTTOM) ? FOOTPRINT_TILE_HALF_HEIGHT : FOOTPRINT_TILE_HEIGHT;
    const uint32_t *row_source = data;
    for (int32_t row = 0; row < end_row; row++) {
        if (row >= first_row) {
            draw_tile_row(tile_address + ((row - first_row) * g_screen_pitch), row_source, row, part, mask);
        }
        row_source += (2 * row_from_edge(row)) + 1;
    }
    return 0;
}

static uint32_t color_mask_pairs(void)
{
    return (uint32_t) city_view.color_mask | ((uint32_t) city_view.color_mask << PIXEL_PAIR_SHIFT);
}

static const uint32_t *tile_data(intptr_t data)
{
    return (const uint32_t *) (uintptr_t) data;
}

uint32_t image_draw_footprint_tile_masked(const uint32_t *data, int32_t clip_y)
{
    return draw_tile(data, clip_y, TILE_WHOLE, color_mask_pairs());
}

uint32_t image_draw_footprint_tile_left_masked(intptr_t data, int32_t clip_y)
{
    return draw_tile(tile_data(data), clip_y, TILE_CUT_LEFT, color_mask_pairs());
}

uint32_t image_draw_footprint_tile_right_masked(intptr_t data, int32_t clip_y)
{
    return draw_tile(tile_data(data), clip_y, TILE_CUT_RIGHT, color_mask_pairs());
}

uint32_t image_draw_footprint_tile(const uint32_t *data, int32_t clip_y)
{
    if (city_view.color_mask != 0) {
        return image_draw_footprint_tile_masked(data, clip_y);
    }
    return draw_tile(data, clip_y, TILE_WHOLE, ~0u);
}

uint32_t image_draw_footprint_tile_left(intptr_t data, int32_t clip_y)
{
    if (city_view.color_mask != 0) {
        return image_draw_footprint_tile_left_masked(data, clip_y);
    }
    return draw_tile(tile_data(data), clip_y, TILE_CUT_LEFT, ~0u);
}

uint32_t image_draw_footprint_tile_right(intptr_t data, int32_t clip_y)
{
    if (city_view.color_mask != 0) {
        return image_draw_footprint_tile_right_masked(data, clip_y);
    }
    return draw_tile(tile_data(data), clip_y, TILE_CUT_RIGHT, ~0u);
}

static void draw_clipped_tile(intptr_t data)
{
    graphics_clip_horizontal();
    graphics_clip_vertical();
    if (g_clip_y == IMAGE_CLIP_INVISIBLE) {
        return;
    }
    if (g_clip_x == IMAGE_CLIP_LEFT) {
        image_draw_footprint_tile_left(data, g_clip_y);
    } else if (g_clip_x == IMAGE_CLIP_RIGHT) {
        image_draw_footprint_tile_right(data, g_clip_y);
    } else {
        image_draw_footprint_tile(tile_data(data), g_clip_y);
    }
}

static int32_t draw_footprint(int32_t tiles)
{
    int32_t x = g_draw_x;
    int32_t y = g_draw_y;
    g_img_width = FOOTPRINT_ONE_TILE_WIDTH;
    g_img_height = FOOTPRINT_ONE_TILE_HEIGHT;
    int32_t tile = 0;
    for (int32_t diagonal = 0; diagonal < (2 * tiles) - 1; diagonal++) {
        int32_t count = (diagonal < tiles) ? diagonal + 1 : ((2 * tiles) - 1) - diagonal;
        for (int32_t i = 0; i < count; i++) {
            g_draw_x = x + (((2 * i) - (count - 1)) * FOOTPRINT_TILE_HALF_WIDTH);
            g_draw_y = y + (diagonal * FOOTPRINT_TILE_HALF_HEIGHT);
            draw_clipped_tile(g_data_ptr + (tile * FOOTPRINT_TILE_BYTES) + g_footprint_data_off);
            tile++;
        }
    }
    return FOOTPRINT_ONE_TILE_WIDTH;
}

int32_t image_draw_footprint_size1(void)
{
    return draw_footprint(2);
}

int32_t image_draw_footprint_size2(void)
{
    return draw_footprint(3);
}

int32_t image_draw_footprint_size3(void)
{
    return draw_footprint(4);
}

int32_t image_draw_footprint_size4(void)
{
    return draw_footprint(5);
}

int32_t image_draw_footprint_size5(void)
{
    return draw_footprint(6);
}
