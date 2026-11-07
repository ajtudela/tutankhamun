// The isometric footprint drawers and one-tile blitters, and an isometric tile's sizes.

#ifndef GRAPHICS_IMAGE_DRAW_FOOTPRINT_H
#define GRAPHICS_IMAGE_DRAW_FOOTPRINT_H

#include <stdint.h>

enum isometric_tile {
    FOOTPRINT_TILE_WIDTH = 60,
    FOOTPRINT_TILE_HEIGHT = 30,
    FOOTPRINT_TILE_HALF_HEIGHT = 15,
    FOOTPRINT_MAX_TILES = 6,
    FOOTPRINT_ONE_TILE_WIDTH = 58,
    FOOTPRINT_ONE_TILE_HEIGHT = 30,
};

uint32_t image_draw_footprint_tile_masked(const uint32_t *data, int32_t clip_y);
uint32_t image_draw_footprint_tile_left_masked(intptr_t data, int32_t clip_y);
uint32_t image_draw_footprint_tile_right_masked(intptr_t data, int32_t clip_y);
uint32_t image_draw_footprint_tile(const uint32_t *data, int32_t clip_y);
uint32_t image_draw_footprint_tile_left(intptr_t data, int32_t clip_y);
uint32_t image_draw_footprint_tile_right(intptr_t data, int32_t clip_y);
int32_t image_draw_footprint_size1(void);
int32_t image_draw_footprint_size2(void);
int32_t image_draw_footprint_size3(void);
int32_t image_draw_footprint_size4(void);
int32_t image_draw_footprint_size5(void);

#endif
