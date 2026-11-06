// The map grid: tile coordinates and offsets, the eight directions (clockwise from the top of the grid, y - 1) with the
// movement results that follow them, the routing wall orientation (0, 2, 4, 6, not the view orientation), the edge and
// bitfield flags of a tile, and the functions of map/grid.c.

#ifndef MAP_GRID_H
#define MAP_GRID_H

#include "building/building_data.h"
#include "map/grid_size.h"
#include "map/terrain.h"

#include <stdint.h>

enum direction {
    DIRECTION_TOP = 0,
    DIRECTION_TOP_RIGHT = 1,
    DIRECTION_RIGHT = 2,
    DIRECTION_BOTTOM_RIGHT = 3,
    DIRECTION_BOTTOM = 4,
    DIRECTION_BOTTOM_LEFT = 5,
    DIRECTION_LEFT = 6,
    DIRECTION_TOP_LEFT = 7,
    DIRECTION_AT_DESTINATION = 8,
    DIRECTION_REROUTE = 9,
    DIRECTION_LOST = 10,
    DIRECTION_ATTACK = 11,
};

enum {
    DIRECTION_COUNT = 8,
};

enum map_edge_bit {
    MAP_EDGE_COLUMN = 0x07,
    MAP_EDGE_ROW_ONE = 0x08,
    MAP_EDGE_ROW = 0x38,
    MAP_EDGE_DRAW_TILE = 0x40,
};

enum map_bitfield {
    MAP_BITFIELD_SIZE = 0x0F,
    MAP_BITFIELD_OVERLAY = 0x10,
    MAP_BITFIELD_ALTERNATE_TERRAIN = 0x20,
    MAP_BITFIELD_DELETED = 0x40,
    MAP_BITFIELD_PLAZA_OR_FAULT = 0x80,
};

enum alternate_terrain {
    ALTERNATE_GRASS_IMAGES = 8,
};

static inline int32_t map_grid_offset(int32_t x, int32_t y)
{
    return map_info.grid_start + y * GRID_SIZE + x;
}

void map_grid_init(void);
void map_random_init(void);
void map_grids_clear_all(void);
void map_terrain_init_outside_map(void);
int32_t map_grid_offset_is_outside(int32_t offset);
void map_property_init_alternate_terrain(void);

#endif
