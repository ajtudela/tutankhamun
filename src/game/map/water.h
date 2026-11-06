// Water queries of the map (map/water.c).

#ifndef MAP_WATER_H
#define MAP_WATER_H

#include "map/terrain.h"

#include <stdbool.h>
#include <stdint.h>

enum water_orientation_check {
    WATER_ORIENTATION_NO_MATCH = 999,
};

enum water_shore_terrain {
    WATER_SHORE_WATER_OR_FLOODPLAIN = TERRAIN_WATER | TERRAIN_FLOODPLAIN,
    WATER_SHORE_BLOCKING = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_RUBBLE | TERRAIN_MARSH |
        TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_DUNE | TERRAIN_DEEPWATER | TERRAIN_ROCK_NEAR_MONUMENT,
};

enum water_search {
    SHIPWRECK_SEARCH_RADIUS = 5,
    OPEN_WATER_DISTANCE = 2,
    BUILDING_NEAR_WATER_SIZE = 3,
    WATER_TILE_MIN_WATER_AROUND = 8,
    WATER_FREE_TILE_MAX_RADIUS = 227,
};

int32_t map_water_landing_point(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t *out_x, int32_t *out_y);
int32_t map_water_get_wharf_for_new_fishing_boat(int32_t figure_id);
int32_t map_water_find_shipwreck_tile(int32_t figure_id);
int32_t map_water_determine_orientation_size2(int32_t x, int32_t y, int32_t adjust, int32_t allow);
int32_t map_water_determine_orientation_size3(int32_t x, int32_t y, int32_t adjust);
int32_t map_terrain_is_adjacent_to_water(int32_t x, int32_t y, int32_t size);
int32_t map_terrain_is_adjacent_to_open_water(int32_t x, int32_t y, int32_t size);
int32_t map_water_tile_next_to(int32_t x, int32_t y, int32_t size);
int32_t map_water_orientation_match(int32_t x, int32_t y);
bool building_near_water(int32_t building_id);
bool map_water_find_free_tile(int32_t *x, int32_t *y);

#endif
