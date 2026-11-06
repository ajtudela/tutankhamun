// The tiles of a building on the map (map/building_tiles.c): adding and removing them, rubble, the area and shrub
// tests, the top-left tile of a building and the access tile of a sphinx.

#ifndef MAP_BUILDING_TILES_H
#define MAP_BUILDING_TILES_H

#include "map/map_data.h"

#include <stdint.h>

enum building_square {
    BUILDING_SQUARE_SIDE = 6,
};

enum sphinx_access {
    SPHINX_ACCESS_DX = 9,
    SPHINX_ACCESS_DY = 6,
};

static inline int32_t building_square_tile_offset(int32_t row, int32_t column)
{
    return building_tile_offsets.square[(row * BUILDING_SQUARE_SIDE) + column].grid_offset;
}

int32_t map_sphinx_access_tile(int32_t building_x, int32_t building_y);
void map_building_tiles_add(int32_t building_id, int32_t x, int32_t y, int32_t size_x, int32_t size_y, int32_t image_id, uint32_t terrain);
void map_building_tiles_remove(int32_t building_id, int32_t x, int32_t y);
void map_building_tiles_set_rubble(int32_t building_id, int32_t x, int32_t y, int32_t size_x, int32_t size_y);
int32_t map_building_tiles_area_matches(int32_t x, int32_t y, int32_t size_x, int32_t size_y, uint32_t terrain, int32_t image_plus_one);
void map_building_tiles_add_without_corner(int32_t building_id, int32_t x, int32_t y, int32_t size_x, int32_t size_y, int32_t image_id, uint32_t terrain);
int32_t map_building_tiles_shrub_fits(int32_t x, int32_t y, int32_t size_x, int32_t size_y, uint32_t mask, int32_t image_plus_one, int32_t floodplain_ok);
int32_t map_building_tiles_find_top_left(int32_t x, int32_t y);

#endif
