// The records of the map tables that hold more than one list: the tile offsets of a building's square followed by the
// rings around a building, and the river-side tiles of a water lift per orientation.

#ifndef MAP_MAP_DATA_TYPES_H
#define MAP_MAP_DATA_TYPES_H

#include <stdint.h>

enum building_tile_offset_counts {
    BUILDING_SQUARE_TILES = 36,
    BUILDING_RING_OFFSETS = 186,
    WATER_LIFT_ORIENTATIONS = 4,
};

typedef struct building_square_tile {
    int32_t grid_offset;
    int32_t packed_xy;
} building_square_tile;

typedef struct building_tile_offset_tables {
    building_square_tile square[BUILDING_SQUARE_TILES];
    int32_t rings[BUILDING_RING_OFFSETS];
} building_tile_offset_tables;

typedef struct water_lift_side_tables {
    int32_t side_a[WATER_LIFT_ORIENTATIONS];
    int32_t side_b[WATER_LIFT_ORIENTATIONS];
    int32_t unused[WATER_LIFT_ORIENTATIONS];
} water_lift_side_tables;

#endif
