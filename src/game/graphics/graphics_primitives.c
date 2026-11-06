// Drawing primitives of the back buffer: filled and outlined rectangles, inset and raised frames, the shaded
// and grey rectangles, single-pixel colour operations, the minimap's two-pixel tile, the clip test and reset, a
// screen block saved to a buffer and drawn back, and the empire map's marching dashed rectangle.
//
// Edge cases of the frames and pixel reads:
// - a frame whose x or y is below 1 starts at 0 without getting shorter;
// - the masked pixel read keeps the high half of y * pitch above the pixel;
// - the dashed rectangle's right edge is clip-tested at x + 2 * width - 1 but drawn at x + width - 1.

#include "graphics/graphics_primitives.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_present.h"
#include "graphics/image_collection.h"
#include "graphics/screen_pixels.h"
#include "window/window_data.h"
#include <stdint.h>

uint32_t pixel_read_result;

enum dash_timing {
    DASH_LENGTH = 4,
    DASH_BLINK_FRAMES = 20,
    DASH_BOTTOM_MARGIN = 32,
};

static void clamp_rect_to_screen(int32_t *x, int32_t *y, int32_t *width, int32_t *height)
{
    if (*x <= 0) {
        *x = 0;
    }
    if (*x + *width >= display_width) {
        *width = display_width - *x;
    }
    if (*y <= 0) {
        *y = 0;
    }
    if (*y + *height >= screen_height) {
        *height = screen_height - *y;
    }
}

void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    clamp_rect_to_screen(&x, &y, &width, &height);
    for (int32_t row = y; row < y + height; row++) {
        graphics_draw_line(x, row, x + width - 1, row, color);
    }
}

void graphics_draw_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color)
{
    clamp_rect_to_screen(&x, &y, &width, &height);
    int32_t right = x + width - 1;
    int32_t bottom = y + height - 1;
    graphics_draw_line(x, y, right, y, color);
    graphics_draw_line(x, bottom, right, bottom, color);
    graphics_draw_line(x, y, x, bottom, color);
    graphics_draw_line(right, y, right, bottom, color);
}

uint32_t graphics_draw_pixel_pair(const uint32_t *pixels)
{
    uint32_t value = *pixels;
    *(uint32_t *) screen_pixel_address(g_draw_x, g_draw_y) = value;
    return value;
}

int32_t image_draw_minimap_tile(int32_t image_id, int32_t x, int32_t y)
{
    const image_collection_object *collection = &image_collection;
    if (image_id <= 0 || image_id >= IMAGE_HEADERS) {
        return image_id;
    }
    if (image_id >= collection->image_count) {
        image_id = 0;
    }
    int32_t data = collection->headers[image_id].data;
    g_data_ptr = data;
    if (data <= 0) {
        return data;
    }
    g_draw_x = x;
    g_draw_y = y;
    return (int32_t) graphics_draw_pixel_pair((const uint32_t *) (uintptr_t) data);
}

void graphics_and_pixel(int32_t x, int32_t y, int32_t mask)
{
    uint8_t *pixel = screen_pixel_address(x, y);
    pixel_write(pixel, pixel_read(pixel) & (uint16_t) mask);
}

uint32_t graphics_get_pixel_masked(int32_t x, int32_t y, uint32_t mask)
{
    uint32_t row_offset = (uint32_t) y * (uint32_t) g_screen_pitch;
    uint32_t value = (row_offset & 0xFFFF0000u) | pixel_read(screen_pixel_address(x, y));
    pixel_read_result = value & mask;
    return pixel_read_result;
}

void graphics_or_pixel(int32_t x, int32_t y, int32_t bits)
{
    uint8_t *pixel = screen_pixel_address(x, y);
    pixel_write(pixel, pixel_read(pixel) | (uint16_t) bits);
}

void graphics_and_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t mask)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    clamp_rect_to_screen(&x, &y, &width, &height);
    int32_t bottom = y + height;
    for (; y < bottom; y++) {
        for (int32_t column = x; column < x + width; column++) {
            graphics_and_pixel(column, y, mask);
        }
    }
}

void graphics_grey_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t shift)
{
    int32_t bottom = y + height;
    for (; y < bottom; y++) {
        for (int32_t column = x; column < x + width; column++) {
            int32_t is_565 = display_pixel_format == DISPLAY_PIXEL_FORMAT_565;
            int32_t red;
            int32_t green;
            if (is_565) {
                red = (int32_t) graphics_get_pixel_masked(column, y, PIXEL_565_RED_MASK) >> PIXEL_565_RED_SHIFT;
                green = (int32_t) graphics_get_pixel_masked(column, y, PIXEL_565_GREEN_MASK) >> PIXEL_565_GREEN_SHIFT;
            } else {
                red = (int32_t) graphics_get_pixel_masked(column, y, PIXEL_555_RED_MASK) >> PIXEL_555_RED_SHIFT;
                green = (int32_t) graphics_get_pixel_masked(column, y, PIXEL_555_GREEN_MASK) >> PIXEL_555_GREEN_SHIFT;
            }
            int32_t blue = (int32_t) graphics_get_pixel_masked(column, y, PIXEL_BLUE_MASK);
            int32_t grey = ((blue + green + red) / 3) >> (shift & 0x1F);
            graphics_and_pixel(column, y, 0);
            if (is_565) {
                graphics_or_pixel(column, y, grey << PIXEL_565_RED_SHIFT);
                graphics_or_pixel(column, y, grey << PIXEL_565_GREEN_SHIFT);
            } else {
                graphics_or_pixel(column, y, grey << PIXEL_555_RED_SHIFT);
                graphics_or_pixel(column, y, grey << PIXEL_555_GREEN_SHIFT);
            }
            graphics_or_pixel(column, y, grey);
        }
    }
}

void graphics_reset_clip_rectangle(void)
{
    g_clip_x1 = g_screen_width;
    g_clip_x0 = 0;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}

void graphics_draw_from_buffer(const uint32_t *buffer, int32_t screen_skip, int32_t buffer_skip)
{
    uint32_t *screen = (uint32_t *) screen_pixel_address(g_draw_x, g_draw_y);
    int32_t rows = g_img_height;
    do {
        for (int32_t n = g_img_width * 2; n != 0; n--) {
            *screen++ = *buffer++;
        }
        screen = (uint32_t *) ((uint8_t *) screen + screen_skip);
        buffer = (const uint32_t *) ((const uint8_t *) buffer + buffer_skip);
        rows--;
    } while (rows > 0);
}

void graphics_save_to_buffer(uint32_t *buffer, int32_t screen_skip, int32_t buffer_skip)
{
    const uint32_t *screen = (const uint32_t *) screen_pixel_address(g_draw_x, g_draw_y);
    int32_t rows = g_img_height;
    do {
        for (int32_t n = g_img_width * 2; n != 0; n--) {
            *buffer++ = *screen++;
        }
        screen = (const uint32_t *) ((const uint8_t *) screen + screen_skip);
        buffer = (uint32_t *) ((uint8_t *) buffer + buffer_skip);
        rows--;
    } while (rows > 0);
}

void graphics_draw_dash_pixel(int32_t x, int32_t y)
{
    if (x >= 0 && x < display_width && y >= 0 && y < screen_height - DASH_BOTTOM_MARGIN) {
        pixel_write(screen_pixel_address(x, y), (uint16_t) dash_color);
    }
}

int32_t graphics_point_in_clip(int32_t x, int32_t y)
{
    if (x < g_clip_x0 || x > g_clip_x1 || y < g_clip_y0) {
        return 0;
    }
    return y <= g_clip_y1;
}

static int32_t next_dash_color(int32_t color)
{
    if (color == COLOR_RED) {
        return COLOR_BLACK;
    }
    return (color != COLOR_BLACK) ? COLOR_RED : COLOR_WHITE;
}

void graphics_dash_blink(void)
{
    dash_blink_frame++;
    if (dash_blink_frame >= DASH_BLINK_FRAMES) {
        dash_blink_frame = 0;
        dash_base_color = next_dash_color(dash_base_color);
    }
}

void graphics_dash_step(void)
{
    dash_step++;
    if (dash_step >= DASH_LENGTH) {
        dash_step = 0;
        dash_color = next_dash_color(dash_color);
    }
}

static void draw_dash_if_in_clip(int32_t test_x, int32_t x, int32_t y)
{
    if (graphics_point_in_clip(test_x, y)) {
        graphics_draw_dash_pixel(x, y);
    }
    graphics_dash_step();
}

void graphics_draw_dashed_rect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    dash_step = 0;
    int32_t x_end = x + width;
    dash_color = dash_base_color;
    for (int32_t column = x; column < x_end; column++) {
        draw_dash_if_in_clip(column, column, y);
    }
    int32_t bottom_column = x;
    int32_t bottom = y + height - 1;
    for (; bottom_column < x_end; bottom_column++) {
        draw_dash_if_in_clip(bottom_column, bottom_column, bottom);
    }
    int32_t y_end = y + height;
    for (int32_t row = y; row < y_end; row++) {
        draw_dash_if_in_clip(x, x, row);
    }
    int32_t right_edge_test_x = width + bottom_column - 1;
    for (int32_t row = y; row < y_end; row++) {
        draw_dash_if_in_clip(right_edge_test_x, x_end - 1, row);
    }
}

static void draw_frame(int32_t x, int32_t y, int32_t width, int32_t height, int32_t top_left_color, int32_t bottom_right_color)
{
    if (x < 1) {
        x = 0;
    } else if (x + width >= display_width) {
        width = display_width - x;
    }
    if (y < 1) {
        y = 0;
    } else if (y + height >= screen_height) {
        height = screen_height - y;
    }
    int32_t right = x - 1 + width;
    graphics_draw_line(x, y, right, y, top_left_color);
    int32_t bottom = y - 1 + height;
    graphics_draw_line(right, y, right, bottom, bottom_right_color);
    graphics_draw_line(x, bottom, right, bottom, bottom_right_color);
    graphics_draw_line(x, y, x, bottom, top_left_color);
}

void graphics_draw_inset_rect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    draw_frame(x, y, width, height, COLOR_GREY, COLOR_WHITE);
}

void graphics_draw_raised_rect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    draw_frame(x, y, width, height, COLOR_WHITE, COLOR_GREY);
}
