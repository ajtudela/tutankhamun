// The terrain bits of a map tile (map_grids.terrain): one bit per kind of ground or of what stands on it, and the
// combined masks several modules use. A wall tile has TERRAIN_WALL alone (mud), with TERRAIN_BRICK_WALL or with
// TERRAIN_STRONG_WALL. Defines rather than an enum, so the masks keep the unsigned type of their literals.

#ifndef MAP_TERRAIN_H
#define MAP_TERRAIN_H

#include <stdbool.h>
#include <stdint.h>
#define TERRAIN_NONE 0
#define TERRAIN_TREE 0x1
#define TERRAIN_ROCK 0x2
#define TERRAIN_WATER 0x4
#define TERRAIN_BUILDING 0x8
#define TERRAIN_SHRUB 0x10
#define TERRAIN_GARDEN 0x20
#define TERRAIN_ROAD 0x40
#define TERRAIN_GROUNDWATER 0x80
#define TERRAIN_CANAL 0x100
#define TERRAIN_ELEVATION 0x200
#define TERRAIN_ACCESS_RAMP 0x400
#define TERRAIN_MEADOW 0x800
#define TERRAIN_RUBBLE 0x1000
#define TERRAIN_WELL_RANGE 0x2000
#define TERRAIN_WALL 0x4000
#define TERRAIN_GATEHOUSE 0x8000
#define TERRAIN_FLOODPLAIN 0x10000
#define TERRAIN_FERRY_ROUTE 0x20000
#define TERRAIN_MARSH 0x40000
#define TERRAIN_DIKE 0x80000
#define TERRAIN_ORE 0x100000
#define TERRAIN_GEM_ROCK 0x200000
#define TERRAIN_BRICK_WALL 0x400000
#define TERRAIN_STRONG_WALL 0x800000
#define TERRAIN_IRRIGATION_RANGE 0x1000000
#define TERRAIN_DUNE 0x2000000
#define TERRAIN_DEEPWATER 0x4000000
#define TERRAIN_SUBMERGED_ROAD 0x8000000
#define TERRAIN_MONUMENT 0x10000000
#define TERRAIN_ROCK_NEAR_MONUMENT 0x20000000
#define TERRAIN_WALKABLE_BUILDING 0x40000000
#define TERRAIN_SHORE 0x80000000

#define TERRAIN_KEPT_BY_BUILDING 0x03052E80
#define TERRAIN_NOT_CLEAR_FOR_ROUTED (TERRAIN_TREE | TERRAIN_SHRUB | TERRAIN_MARSH | TERRAIN_DUNE | TERRAIN_MONUMENT)
#define TERRAIN_WALL_CANAL_OR_GARDEN (TERRAIN_WALL | TERRAIN_CANAL | TERRAIN_GARDEN)
#define TERRAIN_ROCKS (TERRAIN_ROCK | TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_ROCK_NEAR_MONUMENT)
#define TERRAIN_WALL_KINDS (TERRAIN_WALL | TERRAIN_BRICK_WALL | TERRAIN_STRONG_WALL)
#define TERRAIN_NOT_CLEAR 0xEEFED77F
#define TERRAIN_NOT_EMPTY 0xEEFFD77F
#define TERRAIN_BLOCKS_MONUMENT 0xFEFFD76F
#define TERRAIN_BLOCKS_MONUMENT_SITE (TERRAIN_BLOCKS_MONUMENT & ~TERRAIN_TREE)
#define TERRAIN_BLOCKS_MONUMENT_RAMP (TERRAIN_BLOCKS_MONUMENT_SITE & ~TERRAIN_ROAD)
enum terrain_ring_check {
    TERRAIN_TREE_OR_ROCK_RING_MAX = 11,
};

enum terrain_deepwater {
    DEEPWATER_CHECK_RADIUS = 2,
    DEEPWATER_THIN_RUN = 2,
};

enum terrain_garden {
    GARDEN_IMAGES = 8,
};

enum terrain_free_land {
    NEAREST_FREE_LAND_MAX_RADIUS = 227,
};

int32_t map_terrain_ring_is_outside(int32_t building_x, int32_t building_y, int32_t size, int32_t distance, int32_t index);
bool map_terrain_has_bits(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t mask);
void map_terrain_update_deepwater(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
bool map_grid_is_inside_size(int32_t x, int32_t y, int32_t size);
bool map_terrain_has_floodplain(int32_t x, int32_t y, int32_t size_x, int32_t size_y);
int32_t map_terrain_neighbor_count(uint32_t have, uint32_t lack, int32_t offset);
void map_terrain_shrub_to_tree(void);
void map_terrain_mark_gardens(void);
void map_terrain_or_area(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask);
void map_terrain_and_area(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask);
int32_t map_terrain_any_tile_in_square_with(int32_t x, int32_t y, int32_t radius, uint32_t mask);
bool map_grid_is_inside(int32_t x, int32_t y);
bool map_find_nearest_free_land_tile(int32_t *x, int32_t *y);

#endif
