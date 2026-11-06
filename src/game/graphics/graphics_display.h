// The display set-up, the present and the screen fade; the pixel formats and display modes.

#ifndef GRAPHICS_GRAPHICS_DISPLAY_H
#define GRAPHICS_GRAPHICS_DISPLAY_H

#include <stdint.h>

enum display_pixel_format {
    DISPLAY_PIXEL_FORMAT_555 = 0x555,
    DISPLAY_PIXEL_FORMAT_556 = 0x556,
    DISPLAY_PIXEL_FORMAT_565 = 0x565,
    DISPLAY_PIXEL_FORMAT_655 = 0x655,
};

enum display_mode {
    DISPLAY_MODE_RGB555 = 0,
    DISPLAY_MODE_RGB556 = 1,
    DISPLAY_MODE_RGB565 = 2,
    DISPLAY_MODE_RGB655 = 3,
};

int32_t graphics_clear_surfaces(void);
int32_t graphics_screen_resize(void);
int32_t graphics_display_shutdown(int32_t release_display);
int32_t graphics_begin(void);
int32_t graphics_present_frame(void);
int32_t calc_bound(int32_t value, int32_t min, int32_t max);

#endif
