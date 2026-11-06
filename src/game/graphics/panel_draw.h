// The functions of panel_draw.c that other files call.

#ifndef GRAPHICS_PANEL_DRAW_H
#define GRAPHICS_PANEL_DRAW_H

#include <stdint.h>

void outer_panel_draw(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks);
void inner_panel_draw(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks);
void panel_draw_bar(int32_t x, int32_t y, int32_t blocks);
void panel_draw_unbordered(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks);

#endif
