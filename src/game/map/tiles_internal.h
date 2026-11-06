// Shared by the tile modules: the image group table and the terrain image groups (the *_BLOOD groups replace their base
// group where the blood river has passed), the road bits, the context table aliases, the floodplain node test, the
// tile offset and the functions of map/terrain.c.

#ifndef MAP_TILES_INTERNAL_H
#define MAP_TILES_INTERNAL_H

#include "building/building_data.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_road.h"

#include <stdbool.h>
#include <stdint.h>

enum {
    GROUP_TERRAIN_GARDEN = 0x148,
    GROUP_TERRAIN_WALL = 0x197,
    GROUP_TERRAIN_PLAZA = 0x1B5,
    GROUP_TERRAIN_GRASS = 0x1F4,
    GROUP_TERRAIN_TREE = 0x1F5,
    GROUP_TERRAIN_SHORE = 0x1F6,
    GROUP_TERRAIN_FAULT = 0x1F7,
    GROUP_TERRAIN_EMPTY_LAND_ALT = 0x1F8,
    GROUP_TERRAIN_ROCK = 0x1F9,
    GROUP_TERRAIN_CANAL = 0x1FA,
    GROUP_TERRAIN_EMPTY_LAND = 0x1FB,
    GROUP_TERRAIN_REEDS = 0x1FC,
    GROUP_TERRAIN_TREE_GROWING = 0x1FD,
    GROUP_TERRAIN_DUNES = 0x1FE,
    GROUP_TERRAIN_WATER = 0x204,
    GROUP_TERRAIN_ENTRY_EXIT_FLAGS = 0x209,
    GROUP_TERRAIN_FLOODPLAIN = 0x210,
    GROUP_TERRAIN_REEDS_GROWN = 0x211,
    GROUP_TERRAIN_ROAD = 0x212,
    GROUP_TERRAIN_RUBBLE = 0x213,
    GROUP_TERRAIN_FLOODPLAIN_FERTILITY = 0x216,
    GROUP_TERRAIN_GRASS_BY_MOISTURE = 0x21F,
    GROUP_TERRAIN_GRASS_WETTEST_BESIDE_TERRAIN = 0x225,
    GROUP_TERRAIN_GRASS_WETTEST_INNER = 0x226,
    GROUP_TERRAIN_MEADOW_FERTILE_WETTEST = 0x227,
    GROUP_TERRAIN_MEADOW_FERTILE_DRIEST = 0x228,
    GROUP_TERRAIN_GRASS_WETTEST_BESIDE_OTHER = 0x229,
    GROUP_TERRAIN_GRASS_PLACED = 0x231,
    GROUP_TERRAIN_MEADOW_INFERTILE_DRIEST = 0x233,
    GROUP_TERRAIN_ORE_ROCK = 0x21B,
    GROUP_TERRAIN_DIRT_ROAD = 0x21C,
    GROUP_TERRAIN_WATER_SHORE = 0x21D,
    GROUP_TERRAIN_FLOODPLAIN_ROAD = 0x224,
    GROUP_TERRAIN_DEEPWATER = 0x22E,
    GROUP_TERRAIN_SHORE_BLOOD = 0x2CD,
    GROUP_TERRAIN_DEEPWATER_BLOOD = 0x2CE,
    GROUP_TERRAIN_WATER_BLOOD = 0x2CF,
    GROUP_TERRAIN_FLOODPLAIN_BLOOD = 0x2D0,
    GROUP_TERRAIN_REEDS_BLOOD = 0x2D1,
    GROUP_TERRAIN_REEDS_GROWN_BLOOD = 0x2D2,
    GROUP_TERRAIN_WATER_SHORE_BLOOD = 0x2D6,
    GROUP_TERRAIN_ROCK_MONUMENT = 0x2E7,
    GROUP_TERRAIN_BLOOD_FRONT = 0x2EE,
};

#define TERRAIN_ROADS (TERRAIN_ROAD | TERRAIN_SUBMERGED_ROAD)

static inline int32_t floodplain_node_dry(int32_t offset)
{
    const flood_tile *tile = floods_data.tiles[offset];
    return tile == 0 || tile->is_floodplain != 0;
}

static inline int32_t tile_offset(int32_t x, int32_t y)
{
    return map_info.grid_start + y * GRID_SIZE + x;
}

bool map_grid_is_inside(int32_t x, int32_t y);
bool map_grid_is_inside_size(int32_t x, int32_t y, int32_t size);
void map_terrain_and_area(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask);
int32_t map_terrain_ring_is_outside(int32_t x, int32_t y, int32_t size, int32_t distance, int32_t index);
int32_t map_terrain_ring_all_tree_or_rock(int32_t x, int32_t y, int32_t distance);
int32_t map_terrain_any_tile_in_square_without(int32_t x, int32_t y, int32_t radius, uint32_t mask);
int32_t map_terrain_any_tile_in_square_with(int32_t x, int32_t y, int32_t radius, uint32_t mask);
int32_t map_terrain_column_has_two(int32_t x, int32_t y, uint32_t mask);
int32_t map_terrain_row_has_two(int32_t x, int32_t y, uint32_t mask);
void map_terrain_update_deepwater(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_terrain_shrink_deepwater_around(int32_t x, int32_t y);

#endif
