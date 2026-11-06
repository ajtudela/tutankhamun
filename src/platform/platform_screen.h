// The game's 16-bit screen buffer handed to the display, with its width and height.

#ifndef PLATFORM_PLATFORM_SCREEN_H
#define PLATFORM_PLATFORM_SCREEN_H

#include <stdint.h>

void platform_screen_set_external(uint16_t *buffer, int32_t width, int32_t height);
void platform_screen_clear_external(void);
uint16_t *platform_screen_buffer(void);
int32_t platform_screen_width(void);
int32_t platform_screen_height(void);
int32_t platform_screen_is_565(void);
static inline uint32_t platform_rgb565_to_argb8888(uint16_t pixel)
{
    uint32_t r = (uint32_t) ((pixel >> 8) & 0xf8);
    uint32_t g = (uint32_t) ((pixel >> 3) & 0xfc);
    uint32_t b = (uint32_t) ((pixel << 3) & 0xf8);
    return 0xff000000u | (r << 16) | (g << 8) | b;
}
static inline uint32_t platform_rgb555_to_argb8888(uint16_t pixel)
{
    uint32_t r = (uint32_t) ((pixel >> 7) & 0xf8);
    uint32_t g = (uint32_t) ((pixel >> 2) & 0xf8);
    uint32_t b = (uint32_t) ((pixel << 3) & 0xf8);
    return 0xff000000u | (r << 16) | (g << 8) | b;
}
#endif
