// The routing grids, one record (routing_grids): water drag, wall, non-citizen, water and citizen passability, the
// distance from the route's source, the route queue and the path being built.
//
// Like the map grids, these lie in a fixed order: an offset past the end of one grid lands in the next one.

#ifndef MAP_ROUTING_GRID_TYPES_H
#define MAP_ROUTING_GRID_TYPES_H

#include "map/grid_size.h"

#include <stdint.h>

enum routing_grid_set_size {
    ROUTING_QUEUE_RESERVED_INTS = 250,
    ROUTING_DIRECTION_PATH_BYTES = 500,
};

typedef struct routing_grid_set {
    uint8_t water_drag[GRID_CELLS];
    int8_t walls[GRID_CELLS];
    int8_t land_noncitizen[GRID_CELLS];
    int8_t water[GRID_CELLS];
    int8_t land_citizen[GRID_CELLS];
    int32_t distance[GRID_CELLS];
    int32_t queue_items[GRID_CELLS];
    int32_t reserved_1[ROUTING_QUEUE_RESERVED_INTS];
    uint8_t direction_path_tmp[ROUTING_DIRECTION_PATH_BYTES];
} routing_grid_set;

#endif
