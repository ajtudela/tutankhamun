// The desirability grid, rebuilt once a day from every building in use and from gardens, plazas, earthquake faults
// and rubble. Each source adds its model's value over rings around it, changing by a step every few rings, clamped
// to the desirability range.
//
// On a ring partly outside the map the ring tiles still get the value, while the clamp is applied to the source's base
// tile.

#include "map/desirability.h"

#include "building/building.h"
#include "building/building_type.h"
#include "core/calc.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/ring.h"
#include "map/terrain.h"

#include <stdint.h>
#include <string.h>

static void clamp_desirability(int32_t offset)
{
    if (map_grids.desirability[offset] < DESIRABILITY_MIN) {
        map_grids.desirability[offset] = DESIRABILITY_MIN;
    }
    if (map_grids.desirability[offset] > DESIRABILITY_MAX) {
        map_grids.desirability[offset] = DESIRABILITY_MAX;
    }
}

static void add_clamped(int32_t offset, int8_t value)
{
    map_grids.desirability[offset] = (int8_t) (map_grids.desirability[offset] + value);
    clamp_desirability(offset);
}

static void add_clamping_base_tile(int32_t base_offset, int32_t offset, int8_t value)
{
    map_grids.desirability[offset] = (int8_t) (map_grids.desirability[offset] + value);
    clamp_desirability(base_offset);
}

void map_desirability_add_at_distance(int32_t x, int32_t y, int32_t size, int32_t distance, int32_t value)
{
    int32_t partially_outside = 0;
    if (x - distance < -1) {
        partially_outside = 1;
    }
    if (y - distance < -1) {
        partially_outside = 1;
    }
    if (x + size - 1 + distance > map_info.grid_width) {
        partially_outside = 1;
    }
    if (y + size - 1 + distance > map_info.grid_height) {
        partially_outside = 1;
    }
    if (size < 1 || size > RING_BUILDING_SIZE_LARGEST) {
        return;
    }
    int32_t count = map_ring_tile_count(size, distance);
    const int16_t *tile = map_ring_grid_offsets(size, distance);
    int32_t base_offset = map_info.grid_start + (y * GRID_SIZE) + x;
    map_tile_cursor_offset = base_offset;
    if (!partially_outside) {
        for (int32_t i = 0; i < count; i++) {
            add_clamped(base_offset + tile[i], (int8_t) value);
        }
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        if (!map_terrain_ring_is_outside(x, y, size, distance, i)) {
            int32_t cursor = map_tile_cursor_offset;
            add_clamping_base_tile(cursor, cursor + tile[i], (int8_t) value);
        }
    }
}

void map_desirability_add_to_terrain(int32_t x, int32_t y, int32_t size, int32_t value, int32_t step, int32_t step_size, int32_t range)
{
    if (size <= 0) {
        return;
    }
    if (range > RING_AROUND_BUILDING_MAX) {
        range = RING_AROUND_BUILDING_MAX;
    }
    int32_t tiles_within_step = 0;
    int32_t distance = 1;
    while (range > 0) {
        map_desirability_add_at_distance(x, y, size, distance, value);
        distance++;
        tiles_within_step++;
        if (tiles_within_step >= step) {
            value += step_size;
            tiles_within_step = 0;
        }
        range--;
    }
}

static void add_model(int32_t x, int32_t y, int32_t size, int32_t model)
{
    const building_model_record *m = &building_model_table[model];
    map_desirability_add_to_terrain(x, y, size, m->desirability_value, m->desirability_step, m->desirability_step_size, m->desirability_range);
}

void map_desirability_update_buildings(void)
{
    memset(map_grids.desirability, 0, GRID_CELLS);
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE) {
            add_model(b->x, b->y, b->size, b->type);
        }
    }
}

static void add_rubble(int32_t x, int32_t y)
{
    map_desirability_add_to_terrain(x, y, 1, DESIRABILITY_RUBBLE_VALUE, DESIRABILITY_RUBBLE_STEP, DESIRABILITY_RUBBLE_STEP_SIZE, DESIRABILITY_RUBBLE_RANGE);
}

void map_desirability_update_terrain(void)
{
    int32_t grid_offset = map_info.grid_start;
    for (int32_t y = 0; y < map_info.grid_height; y++, grid_offset += map_info.grid_border_size) {
        for (int32_t x = 0; x < map_info.grid_width; x++, grid_offset++) {
            uint32_t terrain = map_grids.terrain[grid_offset];
            if (map_grids.bitfields[grid_offset] & MAP_BITFIELD_PLAZA_OR_FAULT) {
                if (terrain & TERRAIN_ROAD) {
                    add_model(x, y, 1, BUILDING_PLAZA);
                } else if (terrain & TERRAIN_ROCK) {
                    add_model(x, y, 1, BUILDING_HOUSE_CRUDE_HUT);
                } else {
                    map_grids.bitfields[grid_offset] &= (uint8_t) ~MAP_BITFIELD_PLAZA_OR_FAULT;
                }
            } else if (terrain & TERRAIN_GARDEN) {
                add_model(x, y, 1, BUILDING_GARDENS);
            } else if (terrain & TERRAIN_RUBBLE) {
                add_rubble(x, y);
            }
        }
    }
}
