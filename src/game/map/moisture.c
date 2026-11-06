// The moisture map (the grass shades next to water) and its rebuild: the distances to water and to the floodplain and
// marsh, the shade by the climate, and the image context group taken by tiles whose neighbours are wet. Also the
// redraw of the whole map after a pyramid was deleted.

#include "map/moisture.h"

#include "editor/editor_data.h"
#include "editor/editor_terrain_tiles.h"
#include "game/game_internal.h"
#include "map/grid.h"
#include "map/grid_size.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "scenario/scenario_climate.h"

#include <stdint.h>

static int32_t tile_object_active(int32_t offset)
{
    const flood_tile *tile = floods_data.tiles[offset];
    return tile != 0 && tile->is_floodplain == 0;
}

int32_t map_moisture_flood_objects_around(int32_t offset)
{
    if (tile_object_active(offset - GRID_SIZE) && tile_object_active(offset + GRID_SIZE)) {
        return 1;
    }
    if (tile_object_active(offset - 1) && tile_object_active(offset + 1)) {
        return 1;
    }
    return 0;
}

int32_t map_moisture_terrain_around(int32_t offset, uint32_t mask)
{
    if ((map_grids.terrain[offset - GRID_SIZE] & mask) && (map_grids.terrain[offset + GRID_SIZE] & mask)) {
        return 1;
    }
    if ((map_grids.terrain[offset - 1] & mask) && (map_grids.terrain[offset + 1] & mask)) {
        return 1;
    }
    return 0;
}

int32_t map_moisture_or_wet(int32_t offset)
{
    if (map_moisture_terrain_around(offset, TERRAIN_WATER) || map_moisture_flood_objects_around(offset)) {
        return MOISTURE_WET;
    }
    return map_grids.moisture[offset];
}

int32_t map_moisture_neighbour_shade(int32_t offset, int32_t delta)
{
    offset += delta;
    int32_t value = map_moisture_or_wet(offset);
    if (tile_object_active(offset) || (map_grids.terrain[offset] & MOISTURE_TERRAIN_WET)) {
        return MOISTURE_WET;
    }
    return (value >= MOISTURE_CONTEXT_GROUP_BIT) ? 0 : value;
}

int32_t map_moisture_ring_distance(int32_t x, int32_t y, uint32_t mask, int32_t radius)
{
    int32_t result = -1;
    int32_t perimeter = 8;
    int32_t row = map_info.grid_start + (y * GRID_SIZE) - GRID_SIZE;
    int32_t row_y = y - 1;
    for (int32_t r = 1; r < radius; r++) {
        if (result >= 0) {
            return result;
        }
        int32_t side = perimeter / 4;
        int32_t cx = x - y + row_y;
        int32_t cy = row_y;
        int32_t offset = row;
        for (int32_t i = 0; i < perimeter; i++) {
            switch (i / side) {
                case 0:
                    cx++;
                    break;
                case 1:
                    offset += GRID_SIZE;
                    cy++;
                    break;
                case 2:
                    cx--;
                    break;
                case 3:
                    offset -= GRID_SIZE;
                    cy--;
                    break;
                default:
                    break;
            }
            if (cx >= 0 && cx <= map_info.grid_width && cy >= 0 && cy <= map_info.grid_height) {
                uint32_t terrain = map_grids.terrain[offset + cx];
                if ((terrain & mask) && !(terrain & TERRAIN_DIKE)) {
                    result = r;
                    break;
                }
            }
        }
        row -= GRID_SIZE;
        row_y--;
        perimeter += 8;
    }
    return result;
}

int32_t map_moisture_distance_shade(int32_t distance, int32_t span)
{
    int32_t remaining = span - distance;
    int32_t value = (remaining < 0) ? 0 : remaining;
    if (value > span) {
        value = span;
    }
    int32_t divisor = (span < MOISTURE_SHADE_SPAN_MIN) ? MOISTURE_SHADE_SPAN_MIN : span;
    return (int32_t) (int64_t) (((double) value / (double) divisor) * MOISTURE_SHADE_SCALE) - 1;
}

int32_t map_moisture_neighbours_wet(int32_t offset)
{
    if (map_moisture_terrain_around(offset, TERRAIN_WATER) || map_moisture_flood_objects_around(offset)) {
        map_grids.moisture[offset] = MOISTURE_WET;
        return 0;
    }
    int32_t moisture = map_grids.moisture[offset];
    if (moisture < MOISTURE_CONTEXT_LOWEST || moisture > MOISTURE_CONTEXT_HIGHEST) {
        return 0;
    }
    for (int32_t direction = 0; direction < DIRECTION_COUNT; direction++) {
        map_adjacent_tiles[direction] = map_moisture_neighbour_shade(offset, map_grid_direction_delta[direction]) == MOISTURE_WET;
    }
    return map_image_context_count_tiles();
}

void map_moisture_update_area(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
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
    int32_t saved_scratch = map_tile_cursor_offset;
    int32_t row_step = map_info.grid_border_size - x1 + map_info.grid_width + x0 - 1;
    int32_t span = MOISTURE_REACH_NORTHERN;
    if (scenario_data.climate_type == CLIMATE_CENTRAL) {
        span = MOISTURE_REACH_CENTRAL;
    } else if (scenario_data.climate_type == CLIMATE_DESERT) {
        span = MOISTURE_REACH_DESERT;
    }
    int32_t offset = map_info.grid_start + (y0 * GRID_SIZE) + x0;
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            uint32_t terrain = map_grids.terrain[offset];
            if (terrain & MOISTURE_TERRAIN_NONE) {
                map_grids.moisture[offset] = 0;
                map_grids.terrain[offset] = terrain & ~TERRAIN_GROUNDWATER;
                continue;
            }
            int32_t water = map_moisture_ring_distance(x, y, TERRAIN_WATER, span);
            int32_t flood = map_moisture_ring_distance(x, y, MOISTURE_TERRAIN_FLOODPLAIN_OR_MARSH, MOISTURE_REACH_FLOODPLAIN);
            int32_t water_shade;
            if (water < 0) {
                if (flood < 0) {
                    map_grids.terrain[offset] &= ~TERRAIN_GROUNDWATER;
                    map_grids.moisture[offset] = 0;
                    continue;
                }
                water_shade = 0;
            } else {
                int32_t unshaded = span - MOISTURE_SHADE_SPAN_MIN;
                int32_t shade_span = span;
                if (water < unshaded) {
                    water = 0;
                } else {
                    water = water - unshaded + 1;
                    shade_span = MOISTURE_SHADE_SPAN_MIN;
                }
                water_shade = map_moisture_distance_shade((water - 1 > 0) ? water - 1 : 0, shade_span);
            }
            int32_t moisture = 0;
            if (flood >= 0) {
                moisture = map_moisture_distance_shade((flood - 1 > 0) ? flood - 1 : 0, MOISTURE_REACH_FLOODPLAIN);
            }
            if (water_shade > moisture) {
                moisture = water_shade;
            }
            if (moisture < 1) {
                moisture = 1;
            }
            map_grids.terrain[offset] |= TERRAIN_GROUNDWATER;
            map_grids.moisture[offset] = (uint8_t) moisture;
        }
        offset += row_step;
    }
    offset = (y0 * GRID_SIZE) + map_info.grid_start + x0;
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            if (!(map_grids.terrain[offset] & TERRAIN_GROUNDWATER) || !map_moisture_neighbours_wet(offset)) {
                continue;
            }
            map_grids.terrain[offset] &= ~TERRAIN_MEADOW;
            int32_t orientation = map_info.orientation;
            map_info.orientation = 0;
            map_image_context_match(context_deepwater, MOISTURE_CONTEXT_COUNT, -1, 0);
            if (context_group_offset == MOISTURE_CONTEXT_GROUP_FULL) {
                map_grids.moisture[offset] = MOISTURE_WET;
            } else {
                map_grids.moisture[offset] = (uint8_t) context_group_offset | MOISTURE_CONTEXT_GROUP_BIT;
            }
            map_info.orientation = orientation;
        }
        offset += row_step;
    }
    map_tile_cursor_offset = saved_scratch;
}

void map_pyramid_redraw(void)
{
    if (map_pyramid_redraw_pending == 0) {
        return;
    }
    editor_fix_all();
    map_moisture_update_area(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    map_terrain_update_deepwater(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    map_pyramid_redraw_pending = 0;
}
