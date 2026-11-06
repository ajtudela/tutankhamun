// The map's grids, one record (map_grids) with GRID_CELLS entries per grid: images, vegetation, moisture, fertility,
// road networks, desirability, terrain, bitfields, figure and building ids, edges and the rest.
//
// The grids lie in a fixed order, so a tile offset past the end of one grid reads the start of the next.

#ifndef MAP_MAP_GRID_TYPES_H
#define MAP_MAP_GRID_TYPES_H

#include "map/grid_size.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct map_grid_set {
    uint32_t images_backup[GRID_CELLS];
    uint8_t vegetation_growth[GRID_CELLS];
    uint8_t moisture[GRID_CELLS];
    uint8_t soil_fertility[GRID_CELLS];
    uint8_t road_segment[GRID_CELLS];
    uint8_t road_network[GRID_CELLS];
    uint8_t soldier_strength[GRID_CELLS];
    uint8_t rubble_building_type[GRID_CELLS];
    uint16_t building_damage[GRID_CELLS];
    uint8_t elevation[GRID_CELLS];
    int8_t desirability[GRID_CELLS];
    uint8_t random[GRID_CELLS];
    uint8_t sprite[GRID_CELLS];
    uint8_t canals[GRID_CELLS];
    uint32_t terrain[GRID_CELLS];
    uint8_t bitfields[GRID_CELLS];
    int16_t figure_ids[GRID_CELLS];
    int16_t building_ids[GRID_CELLS];
    uint8_t edge[GRID_CELLS];
    int32_t images[GRID_CELLS];
} map_grid_set;

static_assert(sizeof(map_grid_set) == 1663488, "map_grid_set is a savegame record of 1663488 bytes");
static_assert(offsetof(map_grid_set, vegetation_growth) == 0x32C40 && offsetof(map_grid_set, rubble_building_type) == 0x7EEA0 &&
                  offsetof(map_grid_set, building_damage) == 0x8B9B0 && offsetof(map_grid_set, elevation) == 0xA4FD0 &&
                  offsetof(map_grid_set, canals) == 0xD7C10 && offsetof(map_grid_set, terrain) == 0xE4720 && offsetof(map_grid_set, bitfields) == 0x117360 &&
                  offsetof(map_grid_set, figure_ids) == 0x123E70 && offsetof(map_grid_set, building_ids) == 0x13D490 &&
                  offsetof(map_grid_set, edge) == 0x156AB0 && offsetof(map_grid_set, images) == 0x1635C0, "map_grid_set is the savegame's layout");

#endif
