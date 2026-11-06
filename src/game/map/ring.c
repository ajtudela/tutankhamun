// The ring tables, built at start-up: the tiles at distance 1 to 12 around a tile and the six rings around a building
// of size 2 to 6, as (x, y) steps and as grid offsets, and the lookups of a ring at a distance.

#include "map/ring.h"

#include "map/grid_size.h"
#include "map/map_data.h"

#include <stdint.h>

static const ring_tile *building_ring_tiles[RING_BUILDING_SIZES][RING_AROUND_BUILDING_MAX];

static int16_t *const offsets_around_building[RING_BUILDING_SIZES] = {
    ring_offsets_by_size.size2_offsets,
    ring_offsets_by_size.size3_offsets,
    ring_offsets_by_size.size4_offsets,
    ring_offsets_by_size.size5_offsets,
    ring_offsets_by_size.size6_offsets,
};

void map_ring_generate_tiles(void)
{
    for (int32_t distance = 1; distance <= RING_DISTANCE_MAX; distance++) {
        ring_tile *tile = ring_tiles_at_distance(distance);
        for (int32_t y = -distance; y <= distance; y++) {
            for (int32_t x = -distance; x <= distance; x++) {
                if (x == -distance || x == distance || y == -distance || y == distance) {
                    tile->x = (int8_t) x;
                    tile->y = (int8_t) y;
                    tile++;
                }
            }
        }
    }
}

static int16_t *append_grid_offsets(int16_t *offsets, const ring_tile *tiles, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        *offsets++ = (int16_t) (tiles[i].x + (tiles[i].y * GRID_SIZE));
    }
    return offsets;
}

void map_ring_init(void)
{
    map_ring_generate_tiles();
    const ring_tile_table *table = ring_tile_tables();
    int16_t *offsets = ring_offsets;
    for (int32_t distance = 1; distance <= RING_DISTANCE_MAX; distance++) {
        offsets = append_grid_offsets(offsets, ring_tiles_at_distance(distance), table->tile_count[distance]);
    }
    const ring_tile *tiles = table->building_tiles;
    for (int32_t size = 0; size < RING_BUILDING_SIZES; size++) {
        offsets = offsets_around_building[size];
        for (int32_t ring = 0; ring < RING_AROUND_BUILDING_MAX; ring++) {
            int32_t count = table->building_tile_count[size][ring];
            building_ring_tiles[size][ring] = tiles;
            offsets = append_grid_offsets(offsets, tiles, count);
            tiles += count;
        }
    }
}

int32_t map_ring_tile_count(int32_t size, int32_t distance)
{
    const ring_tile_table *table = ring_tile_tables();
    if (size == 1) {
        return table->tile_count[distance];
    }
    return table->building_tile_count[size - RING_BUILDING_SIZE_SMALLEST][distance - 1];
}

const int16_t *map_ring_grid_offsets(int32_t size, int32_t distance)
{
    const ring_tile_table *table = ring_tile_tables();
    if (size == 1) {
        return &ring_offsets[table->tile_start[distance - 1]];
    }
    int32_t building_size = size - RING_BUILDING_SIZE_SMALLEST;
    return &offsets_around_building[building_size][table->building_tile_start[building_size][distance - 1]];
}

const ring_tile *map_ring_tile_list(int32_t size, int32_t distance)
{
    if (size == 1) {
        return ring_tiles_at_distance(distance);
    }
    return building_ring_tiles[size - RING_BUILDING_SIZE_SMALLEST][distance - 1];
}
