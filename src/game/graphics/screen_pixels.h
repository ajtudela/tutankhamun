// The back buffer's pixels and pitch, the address of a pixel, the channel masks and shifts of the 565 and 555
// formats, and the transparent key of raw images.

#ifndef GRAPHICS_SCREEN_PIXELS_H
#define GRAPHICS_SCREEN_PIXELS_H

#include "building/building_data.h"
#include "graphics/graphics_data.h"
#include <stdint.h>

enum pixel_channel {
    PIXEL_565_RED_MASK = 0xF800,
    PIXEL_565_GREEN_MASK = 0x7E0,
    PIXEL_565_RED_SHIFT = 11,
    PIXEL_565_GREEN_SHIFT = 6,
    PIXEL_555_RED_MASK = 0x7C00,
    PIXEL_555_GREEN_MASK = 0x3E0,
    PIXEL_555_RED_SHIFT = 10,
    PIXEL_555_GREEN_SHIFT = 5,
    PIXEL_BLUE_MASK = 0x1F,
    PIXEL_TRANSPARENT_KEY = 0xF81F,
};

static inline uint8_t *screen_pixel_address(int32_t x, int32_t y)
{
    return (uint8_t *) screen_rgb565_ptr + (x * 2) + (int32_t) ((uint32_t) y * (uint32_t) g_screen_pitch);
}

static inline uint16_t pixel_read(const uint8_t *pixel)
{
    return *(const uint16_t *) pixel;
}

static inline void pixel_write(uint8_t *pixel, uint16_t color)
{
    *(uint16_t *) pixel = color;
}

#endif
