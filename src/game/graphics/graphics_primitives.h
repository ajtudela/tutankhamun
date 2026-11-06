// Rectangles, frames, pixel operations, screen blocks and the dashed rectangle of the back buffer.

#ifndef GRAPHICS_GRAPHICS_PRIMITIVES_H
#define GRAPHICS_GRAPHICS_PRIMITIVES_H

#include <stdint.h>

int32_t image_draw_minimap_tile(int32_t image_id, int32_t x, int32_t y);
void graphics_draw_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color);
void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color);
void graphics_draw_raised_rect(int32_t x, int32_t y, int32_t width, int32_t height);
void graphics_draw_inset_rect(int32_t x, int32_t y, int32_t width, int32_t height);
void graphics_reset_clip_rectangle(void);
void graphics_and_pixel(int32_t x, int32_t y, int32_t mask);
void graphics_or_pixel(int32_t x, int32_t y, int32_t bits);
void graphics_draw_dash_pixel(int32_t x, int32_t y);
void graphics_dash_step(void);
void graphics_dash_blink(void);
void graphics_draw_dashed_rect(int32_t x, int32_t y, int32_t width, int32_t height);
void graphics_and_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t mask);
void graphics_grey_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t shift);
uint32_t graphics_draw_pixel_pair(const uint32_t *pixels);
uint32_t graphics_get_pixel_masked(int32_t x, int32_t y, uint32_t mask);
int32_t graphics_point_in_clip(int32_t x, int32_t y);
void graphics_draw_from_buffer(const uint32_t *buffer, int32_t screen_skip, int32_t buffer_skip);
void graphics_save_to_buffer(uint32_t *buffer, int32_t screen_skip, int32_t buffer_skip);

#endif
