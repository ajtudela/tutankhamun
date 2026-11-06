// Terrain tests of the build tool and its ghosts.
//
// Neighbour counts for a terrain bit, the road neighbours of a tile for the road images, whether a road tile is drawn
// paved, walls on the ring around a building, the "any / all / enough tiles" tests on a building's square grown by a
// radius, and a byte AND over a whole grid (used to clear the preview marks).

#include "map/construction_terrain.h"

#include "building/building.h"
#include "building/building_type.h"
#include "figure/figure_data.h"
#include "map/map_data.h"
#include "map/tiles_internal.h"

#include <stdbool.h>
#include <stdint.h>

int32_t map_terrain_count_directly_adjacent_with(int32_t offset, uint32_t mask)
{
    int32_t count = ((map_grids.terrain[offset - GRID_SIZE] & mask) != 0);
    if (map_grids.terrain[offset + 1] & mask) {
        count++;
    }
    if (map_grids.terrain[offset + GRID_SIZE] & mask) {
        count++;
    }
    if (map_grids.terrain[offset - 1] & mask) {
        count++;
    }
    return count;
}

int32_t map_terrain_count_diagonally_adjacent_with(int32_t offset, uint32_t mask)
{
    int32_t count = ((map_grids.terrain[offset - GRID_SIZE + 1] & mask) != 0);
    if (map_grids.terrain[offset + GRID_SIZE + 1] & mask) {
        count++;
    }
    if (map_grids.terrain[offset + GRID_SIZE - 1] & mask) {
        count++;
    }
    if (map_grids.terrain[offset - GRID_SIZE - 1] & mask) {
        count++;
    }
    return count;
}

int32_t map_terrain_has_adjacent_x_with(int32_t offset, uint32_t mask)
{
    if (map_grids.terrain[offset + 1] & mask) {
        return 1;
    }
    return (map_grids.terrain[offset - 1] & mask) != 0;
}

int32_t map_terrain_has_adjacent_y_with(int32_t offset, uint32_t mask)
{
    if (map_grids.terrain[offset - GRID_SIZE] & mask) {
        return 1;
    }
    return (map_grids.terrain[offset + GRID_SIZE] & mask) != 0;
}

int32_t map_terrain_all_adjacent_are_water(int32_t offset)
{
    if (!((uint8_t) map_grids.terrain[offset - GRID_SIZE] & TERRAIN_WATER)) {
        return 0;
    }
    if (!((uint8_t) map_grids.terrain[offset + 1] & TERRAIN_WATER)) {
        return 0;
    }
    if (!((uint8_t) map_grids.terrain[offset + GRID_SIZE] & TERRAIN_WATER)) {
        return 0;
    }
    return ((uint8_t) map_grids.terrain[offset - 1] & TERRAIN_WATER) != 0;
}

bool map_terrain_roads_in_both_axes(int32_t offset)
{
    if ((map_grids.terrain[offset - GRID_SIZE] & TERRAIN_ROADS) || (map_grids.terrain[offset + GRID_SIZE] & TERRAIN_ROADS)) {
        if ((map_grids.terrain[offset + 1] & TERRAIN_ROADS) || (map_grids.terrain[offset - 1] & TERRAIN_ROADS)) {
            return 1;
        }
    }
    return 0;
}

static int32_t road_building_at(int32_t offset)
{
    int32_t type = city_buildings.records[map_grids.building_ids[offset]].type;
    return (type == BUILDING_ROADBLOCK) || (type == BUILDING_FERRY_LANDING);
}

int32_t map_image_context_set_road_neighbours(int32_t offset)
{
    uint32_t north = map_grids.terrain[offset - GRID_SIZE];
    uint32_t east = map_grids.terrain[offset + 1];
    uint32_t south = map_grids.terrain[offset + GRID_SIZE];
    uint32_t west = map_grids.terrain[offset - 1];
    map_adjacent_tiles[DIRECTION_TOP] = north & TERRAIN_ROADS;
    map_adjacent_tiles[DIRECTION_RIGHT] = east & TERRAIN_ROADS;
    map_adjacent_tiles[DIRECTION_BOTTOM] = south & TERRAIN_ROADS;
    map_adjacent_tiles[DIRECTION_LEFT] = west & TERRAIN_ROADS;
    if ((north & TERRAIN_BUILDING) && road_building_at(offset - GRID_SIZE)) {
        map_adjacent_tiles[DIRECTION_TOP] = 1;
    }
    if ((east & TERRAIN_BUILDING) && road_building_at(offset + 1)) {
        map_adjacent_tiles[DIRECTION_RIGHT] = 1;
    }
    if ((south & TERRAIN_BUILDING) && road_building_at(offset + GRID_SIZE)) {
        map_adjacent_tiles[DIRECTION_BOTTOM] = 1;
    }
    if ((west & TERRAIN_BUILDING) && road_building_at(offset - 1)) {
        map_adjacent_tiles[DIRECTION_LEFT] = 1;
    }
    int32_t count = (map_adjacent_tiles[DIRECTION_TOP] != 0);
    if (map_adjacent_tiles[DIRECTION_RIGHT] != 0) {
        count++;
    }
    if (map_adjacent_tiles[DIRECTION_BOTTOM] != 0) {
        count++;
    }
    if (map_adjacent_tiles[DIRECTION_LEFT] != 0) {
        count++;
    }
    return count;
}

int32_t map_tiles_road_desirability_paved(int32_t offset)
{
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & TERRAIN_FLOODPLAIN) {
        return 0;
    }
    int8_t desirability = (int8_t) map_grids.desirability[offset];
    if (desirability > ROAD_ALWAYS_PAVED_ABOVE_DESIRABILITY) {
        return 1;
    }
    if (desirability <= 0) {
        return 0;
    }
    return (terrain & TERRAIN_WELL_RANGE) != 0;
}

int32_t map_terrain_ring_has_wall(int32_t x, int32_t y, int32_t size)
{
    int32_t base = map_grid_offset(x, y);
    const int32_t *ring = &building_tile_offsets.rings[size * BUILDING_RING_OFFSETS_PER_SIZE];
    for (int32_t i = 0; (i < BUILDING_RING_OFFSETS_PER_SIZE) && (ring[i] != 0); i++) {
        if (map_grids.terrain[base + ring[i]] & TERRAIN_WALL) {
            return 1;
        }
    }
    return 0;
}

typedef struct {
    int32_t x_min, y_min, x_max, y_max;
} square;

static square grown_square(int32_t x, int32_t y, int32_t size, int32_t radius)
{
    square s;
    s.x_min = x - radius;
    s.x_max = x + size - 1 + radius;
    s.y_min = y - radius;
    s.y_max = y + size - 1 + radius;
    if (s.x_min < 0) {
        s.x_min = 0;
    }
    if (s.y_min < 0) {
        s.y_min = 0;
    }
    if (s.x_max >= map_info.grid_width) {
        s.x_max = map_info.grid_width - 1;
    }
    if (s.y_max >= map_info.grid_height) {
        s.y_max = map_info.grid_height - 1;
    }
    return s;
}

int32_t map_terrain_any_tile_in_radius(int32_t x, int32_t y, int32_t size, int32_t radius, terrain_test test)
{
    square s = grown_square(x, y, size, radius);
    int32_t offset = map_grid_offset(s.x_min, s.y_min);
    for (map_result_y = s.y_min; map_result_y <= s.y_max; map_result_y++) {
        for (map_result_x = s.x_min; map_result_x <= s.x_max; map_result_x++) {
            if (test(map_grids.terrain[offset])) {
                return 1;
            }
            offset++;
        }
        offset += s.x_min - s.x_max + (GRID_SIZE - 1);
    }
    return 0;
}

int32_t map_terrain_all_tiles_in_radius_are(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask)
{
    square s = grown_square(x, y, size, radius);
    int32_t offset = map_grid_offset(s.x_min, s.y_min);
    for (map_result_y = s.y_min; map_result_y <= s.y_max; map_result_y++) {
        for (map_result_x = s.x_min; map_result_x <= s.x_max; map_result_x++) {
            if ((map_grids.terrain[offset] & mask) == 0) {
                return 0;
            }
            offset++;
        }
        offset += s.x_min - s.x_max + (GRID_SIZE - 1);
    }
    return 1;
}

int32_t map_terrain_count_tiles_in_radius_reach(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask, int32_t needed)
{
    square s = grown_square(x, y, size, radius);
    int32_t offset = map_grid_offset(s.x_min, s.y_min);
    for (map_result_y = s.y_min; map_result_y <= s.y_max; map_result_y++) {
        for (map_result_x = s.x_min; map_result_x <= s.x_max; map_result_x++) {
            if (map_grids.terrain[offset] & mask) {
                needed--;
            }
            offset++;
        }
        offset += s.x_min - s.x_max + (GRID_SIZE - 1);
    }
    return needed <= 0;
}

void memand_grid(void *grid, int32_t size, int32_t mask)
{
    uint8_t *byte = grid;
    uint32_t mask4 = ((uint8_t) mask) * 0x01010101u;
    while (size >= 64) {
        uint32_t *dwords = (uint32_t *) byte;
        for (int32_t i = 0; i < 16; i++) {
            dwords[i] &= mask4;
        }
        byte += 64;
        size -= 64;
    }
    while (size > 0) {
        *byte++ &= (uint8_t) mask;
        size--;
    }
}
