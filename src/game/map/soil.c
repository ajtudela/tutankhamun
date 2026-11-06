// Soil fertility of the floodplain for a rectangle: from the depth of a tile's floodplain object, or from the share of
// floodplain in the rings around a tile without one; then the meadow, floodplain, road and water images of the
// rectangle are redrawn. Also the share of a terrain in the rings around a tile.
//
// The ring share with radius 0 truncates 0 / 0 and returns 0x80000000.

#include "map/soil.h"

#include "building/building_data.h"
#include "core/crt_compat.h"
#include "flood/flood_types.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/ring.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"

#include <stdint.h>

static const double floodplain_image_steps_per_percent = 0.11;

int32_t map_terrain_ring_share(int32_t x, int32_t y, int32_t radius, uint32_t mask)
{
    double total = 0.0;
    double score = 0.0;
    if (radius != 0) {
        const ring_tile_table *rings = ring_tile_tables();
        int32_t centre = (y * GRID_SIZE) + x;
        for (int32_t ring = radius; ring != 0; ring--) {
            double weight = (1.0 / (double) ring) * 100.0;
            int32_t first_tile = rings->tile_start[ring - 1];
            int32_t count = rings->tile_count[ring];
            map_tile_cursor_offset = centre + map_info.grid_start;
            for (int32_t i = 0; i < count; i++) {
                if (map_terrain_ring_is_outside(x, y, 1, ring, i) != 0) {
                    continue;
                }
                total = weight + total;
                if (map_grids.terrain[ring_offsets[first_tile + i] + map_tile_cursor_offset] & mask) {
                    score = weight + score;
                } else {
                    score = score - weight;
                }
            }
        }
        if (score < 0.0) {
            score = 0.0;
        }
    }
    return crt_ftol_double((score / total) * 100.0);
}

int32_t map_floodplain_ring_share(int32_t x, int32_t y, int32_t radius)
{
    return map_terrain_ring_share(x, y, radius, TERRAIN_MEADOW);
}

void map_soil_fertility_update_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool update_meadow)
{
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 >= map_info.grid_width) {
        x1 = map_info.grid_width - 1;
    }
    if (y1 >= map_info.grid_height) {
        y1 = map_info.grid_height - 1;
    }
    int32_t row_skip = map_info.grid_width - x1 + x0 - 1;
    int32_t offset = map_info.grid_start + (y0 * GRID_SIZE) + x0;
    map_tile_cursor_offset = offset;
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++) {
            const flood_tile *object = floods_data.tiles[offset];
            int32_t value = 0;
            if (object) {
                if (object->is_floodplain) {
                    int32_t depth = object->list->depth;
                    value = crt_ftol_double(((double) depth * (1.0 / FLOOD_LIST_COUNT)) * SOIL_FERTILITY_MAX);
                }
            } else if (map_grids.terrain[offset] & TERRAIN_MEADOW) {
                int32_t share = map_floodplain_ring_share(x, y, RING_DISTANCE_FLOODPLAIN_SHARE);
                int32_t step = crt_ftol_double((double) share * floodplain_image_steps_per_percent);
                map_grids.images[offset] = image_group(GROUP_TERRAIN_FLOODPLAIN_FERTILITY) + step;
                value = crt_ftol_double(((double) (step + 1) * (1.0 / FLOODPLAIN_FERTILITY_IMAGES)) * SOIL_FERTILITY_MAX);
            }
            map_grids.soil_fertility[offset] = (uint8_t) value;
            offset++;
            map_tile_cursor_offset = offset;
        }
        offset += map_info.grid_border_size + row_skip;
        map_tile_cursor_offset = offset;
    }
    if (update_meadow) {
        map_tiles_update_region_meadow(x0, y0, x1, y1);
    }
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_roads(x0, y0, x1, y1);
    map_tiles_update_region_water(x0, y0, x1, y1, 1);
}
