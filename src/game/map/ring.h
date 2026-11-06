// The ring tables (map/ring.c): building them and the rings at a distance around a tile or a building, with typed views
// of the ring data of map/map_data.h.

#ifndef MAP_RING_H
#define MAP_RING_H

#include "map/map_data.h"
#include "map/ring_types.h"

#include <stdint.h>

static inline const ring_tile_table *ring_tile_tables(void)
{
    return &ring_tile_count;
}

static inline ring_tile *ring_tiles_at_distance(int32_t distance)
{
    return ring_tiles_by_distance[distance - 1];
}

void map_ring_generate_tiles(void);
void map_ring_init(void);
int32_t map_ring_tile_count(int32_t size, int32_t distance);
const int16_t *map_ring_grid_offsets(int32_t size, int32_t distance);
const ring_tile *map_ring_tile_list(int32_t size, int32_t distance);

#endif
