// Shared by the construction ghosts: image drawing (plain, isometric top, colour-masked), the R key that
// rotates a ghost, the ghost colours and the footprint tables.

#ifndef WIDGET_CONSTRUCTION_GHOST_H
#define WIDGET_CONSTRUCTION_GHOST_H

#include "building/building_data.h"
#include "building/pyramid_images.h"
#include "graphics/image_blit_mask.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "platform/platform_input.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum ghost_color {
    GHOST_COLOR_FREE = 0x1FE3,
    GHOST_COLOR_BLOCKED = 0xF863,
};

enum ghost_footprint {
    GHOST_FOOTPRINT_TILES_MAX = 36,
    GHOST_TREASURY_BROKE = -5000,
    GHOST_ROTATE_KEY = 'R',
    GHOST_KEY_WENT_DOWN = 0x8001,
};

static inline int32_t ghost_rotate_key_went_down(void)
{
    return platform_key_state(GHOST_ROTATE_KEY) == (int16_t) GHOST_KEY_WENT_DOWN;
}

static inline void ghost_image_draw(int32_t image_id, int32_t x, int32_t y)
{
    image_draw(image_id, x, y);
}

static inline void ghost_image_draw_top(int32_t image_id, int32_t x, int32_t y)
{
    image_draw_isometric_top(image_id, x, y);
}

static inline void ghost_image_draw_masked(int32_t image_id, int32_t x, int32_t y, int32_t color)
{
    image_draw_masked(image_id, x, y, color);
}

static inline int32_t ghost_tiles_of_size(int32_t size)
{
    return tile_count_for_size[size];
}

static inline int32_t ghost_view_tile_offset(int32_t view, int32_t tile)
{
    return tile_grid_offsets_base.grid_offsets[view][tile];
}

static inline int32_t ghost_flat_tile_dx(int32_t tile)
{
    return construction_ghost_tile_offsets[tile].x;
}

static inline int32_t ghost_flat_tile_dy(int32_t tile)
{
    return construction_ghost_tile_offsets[tile].y;
}

#endif
