// Terrain tests of the build tool and its ghosts (map/construction_terrain.c).

#ifndef MAP_CONSTRUCTION_TERRAIN_H
#define MAP_CONSTRUCTION_TERRAIN_H

#include <stdbool.h>
#include <stdint.h>

enum building_ring {
    BUILDING_RING_OFFSETS_PER_SIZE = 24,
};

enum road_paving {
    ROAD_ALWAYS_PAVED_ABOVE_DESIRABILITY = 4,
};

typedef bool (*terrain_test)(int32_t terrain);

int32_t map_terrain_any_tile_in_radius(int32_t x, int32_t y, int32_t size, int32_t radius, terrain_test test);
int32_t map_tiles_road_desirability_paved(int32_t offset);
void memand_grid(void *grid, int32_t size, int32_t mask);
int32_t map_terrain_all_tiles_in_radius_are(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask);
int32_t map_terrain_count_tiles_in_radius_reach(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask, int32_t needed);
int32_t map_terrain_ring_has_wall(int32_t x, int32_t y, int32_t size);
int32_t map_terrain_count_directly_adjacent_with(int32_t offset, uint32_t mask);
int32_t map_terrain_count_diagonally_adjacent_with(int32_t offset, uint32_t mask);
int32_t map_image_context_set_road_neighbours(int32_t offset);
bool map_terrain_roads_in_both_axes(int32_t offset);
int32_t map_terrain_has_adjacent_x_with(int32_t offset, uint32_t mask);
int32_t map_terrain_has_adjacent_y_with(int32_t offset, uint32_t mask);
int32_t map_terrain_all_adjacent_are_water(int32_t offset);

#endif
