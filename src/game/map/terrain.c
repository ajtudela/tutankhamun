// Terrain queries and passes: the visible diamond, ring and square tests, the neighbours of a tile with some bits and
// without others, the deep water marking (deep water must be at least three tiles wide), the garden and shrub passes
// at load, the terrain bits under a building's rectangle (an active flood object standing in for floodplain), bits
// ORed or ANDed over a square grown by a radius, and the free land tile nearest a point.
//
// The ring outside test picks a building's ring list by size, size 6 rings for sizes above 5, and the deep water pass
// reads the eight neighbours without a bounds check.

#include "map/terrain.h"

#include "building/building_data.h"
#include "building/building_image_group.h"
#include "figure/figure_data.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/ring.h"
#include "map/routing_terrain.h"
#include "map/tiles_internal.h"

#include <stdint.h>

static int32_t is_inside(int32_t x, int32_t y)
{
    if (x <= map_info.diamond_x_center) {
        if (y <= map_info.diamond_y_center) {
            return (x + y) >= (map_info.diamond_x_min + map_info.diamond_y_center);
        }
        return (x - y) >= (map_info.diamond_x_center - map_info.diamond_y_max);
    }
    if (y <= map_info.diamond_y_center) {
        return (x - y) <= (map_info.diamond_x_max - map_info.diamond_y_center);
    }
    return (x + y) <= (map_info.diamond_x_max + 1 + map_info.diamond_y_center);
}

bool map_grid_is_inside(int32_t x, int32_t y)
{
    return is_inside(x, y);
}

static int32_t is_inside_size_args(int32_t x, int32_t y, int32_t size)
{
    if (!is_inside(x, y)) {
        return 0;
    }
    if (x <= map_info.diamond_x_center) {
        if (y <= map_info.diamond_y_center) {
            return (x - size + y) < (map_info.diamond_x_min + map_info.diamond_y_center);
        }
        return (x - y - size) < (map_info.diamond_x_center - map_info.diamond_y_max);
    }
    if (y <= map_info.diamond_y_center) {
        return (x - y + size) > (map_info.diamond_x_max - map_info.diamond_y_center);
    }
    return (x + y + size) > (map_info.diamond_x_max + 1 + map_info.diamond_y_center);
}

bool map_grid_is_inside_size(int32_t x, int32_t y, int32_t size)
{
    return is_inside_size_args(x, y, size);
}

static const ring_tile *ring_list(int32_t size, int32_t distance)
{
    if (size >= 1 && size < RING_BUILDING_SIZE_LARGEST) {
        int32_t ring = (distance >= 1 && distance < RING_AROUND_BUILDING_MAX) ? distance : RING_AROUND_BUILDING_MAX;
        return map_ring_tile_list(size, ring);
    }
    int32_t ring = (distance >= 0 && distance < (RING_AROUND_BUILDING_MAX - 1)) ? (distance + 1) : RING_AROUND_BUILDING_MAX;
    return map_ring_tile_list(RING_BUILDING_SIZE_LARGEST, ring);
}

int32_t map_terrain_ring_is_outside(int32_t building_x, int32_t building_y, int32_t size, int32_t distance, int32_t index)
{
    const ring_tile *tile = &ring_list(size, distance)[index];
    int32_t x = building_x + tile->x;
    int32_t y = building_y + tile->y;
    return x < -1 || y < -1 || x > map_info.grid_width || y > map_info.grid_height;
}

int32_t map_terrain_ring_all_tree_or_rock(int32_t x, int32_t y, int32_t distance)
{
    if (distance <= 0) {
        return 1;
    }
    if (distance > TERRAIN_TREE_OR_ROCK_RING_MAX) {
        distance = TERRAIN_TREE_OR_ROCK_RING_MAX;
    }
    int32_t count = map_ring_tile_count(1, distance);
    const int16_t *offsets = map_ring_grid_offsets(1, distance);
    map_tile_cursor_offset = tile_offset(x, y);
    for (int32_t i = 0; i < count; i++) {
        if (!map_terrain_ring_is_outside(x, y, 1, distance, i) && !(map_grids.terrain[map_tile_cursor_offset + offsets[i]] & (TERRAIN_TREE | TERRAIN_ROCK))) {
            return 0;
        }
    }
    return 1;
}

static int32_t any_tile_in_square(int32_t x, int32_t y, int32_t radius, uint32_t mask, int32_t has_bits)
{
    for (int32_t yy = y + 1 - radius; yy <= y - 1 + radius; yy++) {
        for (int32_t xx = x + 1 - radius; xx <= x - 1 + radius; xx++) {
            if (xx >= 0 && yy >= 0 && xx < map_info.grid_width && yy < map_info.grid_height && ((map_grids.terrain[tile_offset(xx, yy)] & mask) != 0) == has_bits) {
                return 1;
            }
        }
    }
    return 0;
}

int32_t map_terrain_any_tile_in_square_without(int32_t x, int32_t y, int32_t radius, uint32_t mask)
{
    return any_tile_in_square(x, y, radius, mask, 0);
}

int32_t map_terrain_any_tile_in_square_with(int32_t x, int32_t y, int32_t radius, uint32_t mask)
{
    return any_tile_in_square(x, y, radius, mask, 1);
}

int32_t map_terrain_column_has_two(int32_t x, int32_t y, uint32_t mask)
{
    int32_t count = 0;
    for (int32_t yy = y - 1; yy <= y + 1; yy++) {
        if (yy >= 0 && yy < map_info.grid_height && (map_grids.terrain[tile_offset(x, yy)] & mask)) {
            count++;
        }
    }
    return count > 1;
}

int32_t map_terrain_row_has_two(int32_t x, int32_t y, uint32_t mask)
{
    int32_t count = 0;
    for (int32_t xx = x - 1; xx <= x + 1; xx++) {
        if (xx >= 0 && xx < map_info.grid_width && (map_grids.terrain[tile_offset(xx, y)] & mask)) {
            count++;
        }
    }
    return count > 1;
}

static int32_t deepwater_is_too_thin(int32_t x, int32_t y, int32_t offset)
{
    if (!map_terrain_column_has_two(x, y, TERRAIN_DEEPWATER) || !map_terrain_row_has_two(x, y, TERRAIN_DEEPWATER)) {
        return 1;
    }
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles(TERRAIN_DEEPWATER);
    int32_t found = 0;
    int32_t run = -1;
    int32_t limit = 2 * DIRECTION_COUNT;
    for (int32_t i = 0; i < limit; i++) {
        if (run >= DIRECTION_COUNT) {
            break;
        }
        if (map_adjacent_tiles[i % DIRECTION_COUNT]) {
            if (run >= 0) {
                run++;
            }
        } else {
            if (run < 0) {
                limit = i + DIRECTION_COUNT + 2;
            } else if (run == DEEPWATER_THIN_RUN) {
                found = 1;
                i = limit;
            }
            run = 0;
        }
    }
    return found;
}

void map_terrain_shrink_deepwater_around(int32_t x, int32_t y)
{
    int32_t x0 = (x - 1 < 0) ? 0 : x - 1;
    int32_t x1 = (x + 1 <= map_info.grid_width) ? x + 1 : map_info.grid_width;
    int32_t y0 = (y - 1 < 0) ? 0 : y - 1;
    int32_t y1 = (y + 1 <= map_info.grid_height) ? y + 1 : map_info.grid_height;
    for (int32_t yy = y0; yy <= y1; yy++) {
        for (int32_t xx = x0; xx <= x1; xx++) {
            int32_t offset = tile_offset(xx, yy);
            if ((map_grids.terrain[offset] & TERRAIN_DEEPWATER) && deepwater_is_too_thin(xx, yy, offset)) {
                map_grids.terrain[offset] &= ~TERRAIN_DEEPWATER;
                map_terrain_shrink_deepwater_around(xx, yy);
            }
        }
    }
}

void map_terrain_update_deepwater(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t saved_cursor = map_tile_cursor_offset;
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
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++) {
            uint32_t *terrain = &map_grids.terrain[tile_offset(x, y)];
            if ((*terrain & (TERRAIN_DIKE | TERRAIN_WATER)) == TERRAIN_WATER) {
                if (!map_terrain_any_tile_in_square_without(x, y, DEEPWATER_CHECK_RADIUS, TERRAIN_WATER) &&
                    !map_terrain_any_tile_in_square_with(x, y, DEEPWATER_CHECK_RADIUS, TERRAIN_FLOODPLAIN)) {
                    *terrain |= TERRAIN_DEEPWATER;
                } else {
                    *terrain &= ~TERRAIN_DEEPWATER;
                }
            }
        }
    }
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++) {
            if (map_grids.terrain[tile_offset(x, y)] & TERRAIN_DEEPWATER) {
                map_terrain_shrink_deepwater_around(x, y);
            }
        }
    }
    map_tile_cursor_offset = saved_cursor;
}

int32_t map_terrain_neighbor_count(uint32_t have, uint32_t lack, int32_t offset)
{
    int32_t count = 0;
    for (int32_t direction = 0; direction < DIRECTION_COUNT; direction++) {
        uint32_t terrain = map_grids.terrain[offset + map_grid_direction_delta[direction]];
        if ((have & terrain) != 0 && (lack & terrain) == 0) {
            count++;
        }
    }
    return count;
}

void map_terrain_mark_gardens(void)
{
    int32_t first_garden_image = image_group(IMAGE_GROUP_GARDENS);
    int32_t height = map_info.grid_height;
    for (int32_t y = 0; y < height; y++) {
        height = map_info.grid_height;
        if (y >= height) {
            break;
        }
        if (map_row_x_start[y] >= map_row_x_end[y]) {
            continue;
        }
        int32_t offset = map_row_start_offset[y];
        for (int32_t n = map_row_x_end[y] - map_row_x_start[y]; n > 0; n--, offset++) {
            int32_t image = map_grids.images[offset];
            if (image >= first_garden_image && image < first_garden_image + GARDEN_IMAGES) {
                map_grids.terrain[offset] |= TERRAIN_GARDEN;
                map_grids.bitfields[offset] &= (uint8_t) ~MAP_BITFIELD_PLAZA_OR_FAULT;
                map_grids.canals[offset] = 0;
            }
        }
    }
}

void map_terrain_shrub_to_tree(void)
{
    int32_t height = map_info.grid_height;
    for (int32_t y = 0; y < height; y++) {
        if (map_row_x_start[y] >= map_row_x_end[y]) {
            continue;
        }
        uint32_t *tile = map_grids.terrain + map_row_start_offset[y];
        for (int32_t n = map_row_x_end[y] - map_row_x_start[y]; n > 0; n--, tile++) {
            if (*tile & TERRAIN_SHRUB) {
                *tile = (*tile & ~(uint32_t) TERRAIN_SHRUB) | TERRAIN_TREE;
            }
        }
    }
}

bool map_terrain_has_floodplain(int32_t x, int32_t y, int32_t size_x, int32_t size_y)
{
    return map_terrain_has_bits(x, y, size_x, size_y, TERRAIN_FLOODPLAIN);
}

static void terrain_area(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask, int32_t do_or)
{
    int32_t x0 = x - radius;
    int32_t y0 = y - radius;
    int32_t x1 = x + size + radius - 1;
    int32_t y1 = y + size + radius - 1;
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
    int32_t offset = map_info.grid_start + (y0 * GRID_SIZE) + x0;
    int32_t row_adjust = x0 - x1 + (GRID_SIZE - 1);
    map_result_y = y0;
    map_tile_cursor_offset = offset;
    if (y0 > y1) {
        return;
    }
    int32_t rows = y1 - y0 + 1;
    map_result_y = rows + y0;
    do {
        map_result_x = x0;
        if (x0 <= x1) {
            uint32_t *tile = map_grids.terrain + offset;
            int32_t count = x1 - x0 + 1;
            offset += count;
            map_result_x = count + x0;
            do {
                if (do_or) {
                    *tile |= mask;
                } else {
                    *tile &= mask;
                }
                tile++;
            } while (--count);
        }
        offset += row_adjust;
        rows--;
    } while (rows);
    map_tile_cursor_offset = offset;
}

void map_terrain_or_area(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask)
{
    terrain_area(x, y, size, radius, mask, 1);
}

void map_terrain_and_area(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask)
{
    terrain_area(x, y, size, radius, mask, 0);
}

bool map_terrain_has_bits(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t mask)
{
    int32_t origin = map_info.grid_start + (y * GRID_SIZE) + x;
    map_tile_cursor_offset = origin;
    for (int32_t row = 0; row < height; row++) {
        for (int32_t col = 0; col < width; col++) {
            int32_t tile = origin + building_square_tile_offset(row, col);
            if ((mask & TERRAIN_FLOODPLAIN) != 0) {
                const flood_tile *object = floods_data.tiles[tile];
                if (object != 0 && object->is_floodplain == 0) {
                    return true;
                }
            }
            if ((map_grids.terrain[tile] & mask) != 0) {
                return true;
            }
        }
    }
    return false;
}

static int32_t land_tile_free(int32_t t)
{
    int8_t r = routing_grids.land_citizen[t];
    return r >= ROUTING_CITIZEN_ROAD && r <= ROUTING_CITIZEN_CLEAR && !(r & 1) && map_grids.building_ids[t] == 0;
}

bool map_find_nearest_free_land_tile(int32_t *x, int32_t *y)
{
    if (land_tile_free(map_info.grid_start + *y * GRID_SIZE + *x)) {
        return 1;
    }
    int32_t y_lo = *y - 1 + map_info.diamond_y_min;
    int32_t y_hi = *y + 1 + map_info.diamond_y_min;
    int32_t x_hi = *x + 1 + map_info.diamond_x_min;
    int32_t x_lo = *x - 1 + map_info.diamond_x_min;
    for (int32_t radius = 1; radius <= NEAREST_FREE_LAND_MAX_RADIUS; radius++) {
        int32_t x_start = x_lo < map_info.diamond_x_min ? map_info.diamond_x_min : x_lo;
        int32_t x_end = x_hi > map_info.diamond_x_max ? map_info.diamond_x_max : x_hi;
        int32_t y_start = y_lo < map_info.diamond_y_min ? map_info.diamond_y_min : y_lo;
        int32_t y_end = y_hi > map_info.diamond_y_max ? map_info.diamond_y_max : y_hi;
        int32_t fx = 0;
        int32_t fy = 0;
        int32_t found = 0;
        if (y_lo >= map_info.diamond_y_min) {
            for (int32_t xx = x_start; xx <= x_end && !found; xx++) {
                if (land_tile_free(xx + y_start * GRID_SIZE)) {
                    fx = xx, fy = y_start, found = 1;
                }
            }
        }
        if (!found && y_hi <= map_info.diamond_y_max) {
            for (int32_t xx = x_start; xx <= x_end && !found; xx++) {
                if (land_tile_free(xx + y_end * GRID_SIZE)) {
                    fx = xx, fy = y_end, found = 1;
                }
            }
        }
        if (!found && x_lo >= map_info.diamond_x_min) {
            for (int32_t yy = y_start; yy <= y_end && !found; yy++) {
                if (land_tile_free(x_start + yy * GRID_SIZE)) {
                    fx = x_start, fy = yy, found = 1;
                }
            }
        }
        if (!found && x_hi <= map_info.diamond_x_max) {
            for (int32_t yy = y_start; yy <= y_end && !found; yy++) {
                if (land_tile_free(x_end + yy * GRID_SIZE)) {
                    fx = x_end, fy = yy, found = 1;
                }
            }
        }
        if (found) {
            *x = fx - map_info.diamond_x_min;
            *y = fy - map_info.diamond_y_min;
            return 1;
        }
        x_hi++;
        y_hi++;
        x_lo--;
        y_lo--;
    }
    return 0;
}
