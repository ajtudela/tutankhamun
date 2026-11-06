// The block panel and the panel kinds with a look of their own.

#ifndef GRAPHICS_PANEL_BLOCKS_H
#define GRAPHICS_PANEL_BLOCKS_H

#include <stdint.h>

enum panel_blocks_kind {
    PANEL_BLOCKS_DEBUG = 6,
    PANEL_BLOCKS_RED = 8,
};

void panel_draw_blocks(int32_t kind, int32_t x, int32_t y, int32_t width, int32_t height, int32_t style);

#endif
