// The refresh of the screen and the line primitives (graphics/graphics_present.c).

#ifndef GRAPHICS_GRAPHICS_PRESENT_H
#define GRAPHICS_GRAPHICS_PRESENT_H

#include <stdint.h>

void graphics_refresh_screen(int32_t force_rate);
void graphics_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color);
int32_t graphics_millis_since_last(void);
void graphics_draw_line_setup(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void graphics_draw_line_step(int32_t walk_y);
void graphics_draw_pixel_clipped(int32_t x, int32_t y, int32_t color);

#endif
