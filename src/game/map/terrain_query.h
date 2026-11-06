// Terrain queries (map/terrain.c): terrain in a square and floodplain under an area.

#ifndef MAP_TERRAIN_QUERY_H
#define MAP_TERRAIN_QUERY_H

#include <stdbool.h>
#include <stdint.h>

bool map_terrain_has_floodplain(int32_t x, int32_t y, int32_t size_x, int32_t size_y);
int32_t map_terrain_any_tile_in_square_with(int32_t x, int32_t y, int32_t radius, uint32_t mask);

#endif
