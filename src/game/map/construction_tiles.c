// Tiles of the build tool.
//
// The preview marks and site checks of a building's square at the hovered tile, the routing distances of a square
// cleared, wall tiles placed or removed while walls are dragged (with the refund for a replaced wall and the redraw of
// the walls and gatehouses around), and the road images redrawn around a cleared area.

#include "map/construction_tiles.h"

#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "map/building_tiles.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"

#include <stdint.h>

void map_building_tiles_clear_marks(int32_t keep, int32_t x, int32_t y, int32_t size_x, int32_t size_y)
{
    if ((x < 0) || (y < 0) || (x + size_x - 1 >= map_info.grid_width) || (y + size_y - 1 >= map_info.grid_height)) {
        return;
    }
    int32_t base = map_info.grid_start + y * GRID_SIZE + x;
    map_tile_cursor_offset = base;
    for (int32_t row = 0; row < size_y; row++) {
        for (int32_t column = 0; column < size_x; column++) {
            if (keep == 0) {
                routing_grids.distance[building_square_tile_offset(row, column) + base] = 0;
            }
        }
    }
}

void map_building_tiles_mark_square(void)
{
    construction_place_ok = 1;
    int32_t base = map_tile_cursor_offset;
    for (int32_t row = 0; row < building_tiles_size_y; row++) {
        for (int32_t column = 0; column < building_tiles_size_x; column++) {
            map_grids.bitfields[base + building_square_tile_offset(row, column)] |= MAP_BITFIELD_OVERLAY;
        }
    }
}

static int32_t square_reaches_edge_margin(int32_t x, int32_t y, int32_t size)
{
    int32_t last_x = x + size - 1;
    int32_t last_y = y + size - 1;
    int32_t end_x = map_info.grid_width - BUILDING_MAP_EDGE_MARGIN;
    int32_t end_y = map_info.grid_height - BUILDING_MAP_EDGE_MARGIN;
    return (x < BUILDING_MAP_EDGE_MARGIN) || (y < BUILDING_MAP_EDGE_MARGIN) || (last_x >= end_x) || (last_y >= end_y);
}

static void rotate_origin(int32_t size, int32_t *x, int32_t *y)
{
    if (map_info.orientation == DIRECTION_LEFT) {
        *y += 1 - size;
    } else if (map_info.orientation == DIRECTION_BOTTOM) {
        *x += 1 - size;
        *y += 1 - size;
    } else if (map_info.orientation == DIRECTION_RIGHT) {
        *x += 1 - size;
    }
}

void map_building_tiles_mark_construction(int32_t size, int32_t x, int32_t y, uint32_t mask, int32_t fixed, int32_t floodplain_ok)
{
    building_tiles_size_y = size;
    building_tiles_size_x = size;
    if (fixed == 0 && map_info.orientation != 0) {
        rotate_origin(size, &x, &y);
    }
    if (square_reaches_edge_margin(x, y, size)) {
        return;
    }
    int32_t on_floodplain = 0;
    int32_t off_floodplain = 0;
    int32_t base = map_info.grid_start + y * GRID_SIZE + x;
    map_tile_cursor_offset = base;
    for (int32_t row = 0; row < size; row++) {
        for (int32_t column = 0; column < size; column++) {
            int32_t offset = building_square_tile_offset(row, column) + base;
            uint32_t terrain = map_grids.terrain[offset];
            if (terrain & TERRAIN_FLOODPLAIN) {
                if (!(uint8_t) floodplain_ok || off_floodplain) {
                    return;
                }
                on_floodplain = 1;
            } else {
                if (on_floodplain) {
                    return;
                }
                off_floodplain = 1;
            }
            if (terrain & mask & TERRAIN_NOT_CLEAR) {
                return;
            }
            if (map_grids.figure_ids[offset] != 0) {
                return;
            }
        }
    }
    map_building_tiles_mark_square();
}

int32_t map_building_tiles_area_free(int32_t size, int32_t x, int32_t y, uint32_t mask, bool floodplain_ok)
{
    building_tiles_size_y = size;
    building_tiles_size_x = size;
    if (map_info.orientation != 0) {
        rotate_origin(size, &x, &y);
    }
    if ((x < 0) || (y < 0) || (x + size - 1 >= map_info.grid_width) || (y + size - 1 >= map_info.grid_height)) {
        return 0;
    }
    map_tile_cursor_offset = map_info.grid_start + y * GRID_SIZE + x;
    for (int32_t row = 0; row < building_tiles_size_y; row++) {
        for (int32_t column = 0; column < building_tiles_size_x; column++) {
            int32_t offset = building_square_tile_offset(row, column) + map_tile_cursor_offset;
            if (map_grid_is_inside_size(offset % GRID_SIZE, offset / GRID_SIZE, BUILDING_MAP_EDGE_MARGIN)) {
                return 0;
            }
            const flood_tile *node = floods_data.tiles[offset];
            uint32_t terrain = map_grids.terrain[offset] & mask;
            if ((terrain & TERRAIN_FLOODPLAIN) && !floodplain_ok) {
                return 0;
            }
            if (terrain & TERRAIN_NOT_CLEAR) {
                return 0;
            }
            if (node != 0 && node->is_floodplain == 0) {
                return 0;
            }
        }
    }
    return 1;
}

static void clamp_square(int32_t x0, int32_t y0, int32_t width, int32_t height, int32_t *out_x, int32_t *out_y, int32_t *out_w, int32_t *out_h)
{
    if (x0 < 0) {
        width += x0;
        x0 = 0;
    } else if (x0 + width > map_info.grid_width) {
        width = map_info.grid_width - x0;
    }
    if (y0 < 0) {
        height += y0;
        y0 = 0;
    } else if (y0 + height > map_info.grid_height) {
        height = map_info.grid_height - y0;
    }
    *out_x = x0;
    *out_y = y0;
    *out_w = width;
    *out_h = height;
}

static void redraw_wall_at_cursor(void)
{
    map_image_context_set_tiles(TERRAIN_WALL);
    map_image_context_invert_tiles();
    map_image_context_match(context_wall, 48, -1, 0);
    int32_t offset = map_tile_cursor_offset;
    map_grids.images[offset] = image_group(GROUP_TERRAIN_WALL) + context_item_offset + context_group_offset;
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_SIZE;
    if (map_image_context_set_tiles_gatehouse()) {
        map_image_context_add_tiles_wall();
        if (map_image_context_match(context_wall_gate, 15, -1, 0)) {
            map_grids.images[map_tile_cursor_offset] = image_group(GROUP_TERRAIN_WALL) + context_item_offset + context_group_offset;
        }
    }
    map_tiles_update_empty_land_around(map_tile_cursor_x, map_tile_cursor_y, 1);
}

void map_tiles_set_wall(int32_t x, int32_t y, int32_t set, uint32_t bits)
{
    if (!map_grid_is_inside(map_info.diamond_x_min + x, map_info.diamond_y_min + y)) {
        return;
    }
    int32_t start = map_info.grid_start;
    int32_t offset = start + y * GRID_SIZE + x;
    if (set == 1) {
        uint32_t terrain = map_grids.terrain[offset];
        uint32_t kind = terrain & TERRAIN_WALL_KINDS;
        if (kind != bits) {
            int32_t refund;
            if (kind == TERRAIN_WALL) {
                refund = building_model_get(BUILDING_MUD_WALL)->cost / 2;
            } else if (kind == (TERRAIN_WALL | TERRAIN_BRICK_WALL) && bits == (TERRAIN_WALL | TERRAIN_STRONG_WALL)) {
                refund = building_model_get(BUILDING_BRICK_WALL)->cost / 2;
            } else {
                refund = 0;
            }
            routed_building_items_placed++;
            construction_cost_refund += refund;
        }
        map_grids.terrain[offset] = (terrain & ~TERRAIN_WALL_KINDS) | bits;
    } else {
        map_grids.terrain[offset] &= ~TERRAIN_WALL_KINDS;
    }
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_OVERLAY;
    int32_t x0, y0, width, height;
    clamp_square(x - 1, y - 1, 3, 3, &x0, &y0, &width, &height);
    int32_t y_end = height + y0;
    map_tile_cursor_y = y0;
    map_tile_cursor_offset = start + y0 * GRID_SIZE + x0;
    while (map_tile_cursor_y < y_end) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x < width + x0; map_tile_cursor_x++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if ((terrain & TERRAIN_WALL) && !(terrain & TERRAIN_BUILDING)) {
                redraw_wall_at_cursor();
            }
            map_tile_cursor_offset++;
        }
        map_tile_cursor_y++;
        map_tile_cursor_offset += GRID_SIZE - width;
    }
}

void map_tiles_update_roads_around(int32_t x, int32_t y, int32_t size)
{
    int32_t x0, y0, width, height;
    clamp_square(x - 1, y - 1, size, size, &x0, &y0, &width, &height);
    int32_t y_end = height + y0;
    map_tile_cursor_y = y0;
    map_tile_cursor_offset = map_info.grid_start + y0 * GRID_SIZE + x0;
    while (map_tile_cursor_y < y_end) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x < width + x0; map_tile_cursor_x++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if ((terrain & TERRAIN_ROAD) && !(terrain & (TERRAIN_WATER | TERRAIN_BUILDING))) {
                map_image_context_set_tiles(TERRAIN_ROADS);
                map_image_context_add_gatehouse_tiles();
                map_tiles_set_road_image();
            }
            map_tile_cursor_offset++;
        }
        map_tile_cursor_y++;
        map_tile_cursor_offset += GRID_SIZE - width;
    }
}
