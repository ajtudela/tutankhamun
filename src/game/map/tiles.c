// Terrain tile images: water, shore and deep water, marshland, trees and reeds, meadow, floodplain, rubble, earthquake
// faults, rocks, gardens, dunes, plazas and empty land, and the update of the whole map after loading, rotating and
// large changes. A tile's image is set with its terrain bit and the 3 x 3 around it redrawn; the region updates do
// that for every fitting tile of a rectangle. The blood river turns the water red column by column from the right edge.
//
// Side effects other code relies on:
// - the tile loops leave their position in the tile cursor globals, which other functions read;
// - the rock-next-to-monument image clears 16 entries of map_adjacent_tiles;
// - the dunes search for elevation around a dune without using the result;
// - a building on the floodplain refreshes the complex of a pyramid found 20 tiles to the left.

#include "map/tiles.h"

#include "building/building.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/pyramid_level.h"
#include "building/pyramid_parts.h"
#include "figure/figure_data.h"
#include "map/map_data.h"
#include "map/orientation.h"
#include "map/routing_citizen.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "map/vegetation.h"

#include <stdint.h>

static const int32_t neighbour_delta[DIRECTION_COUNT] = { -GRID_SIZE, 1 - GRID_SIZE, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };

typedef struct {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} block;

static block block_around(int32_t x, int32_t y)
{
    block b = { x - 1, y - 1, 3, 3 };
    if (b.x < 0) {
        b.width = x + 2;
        b.x = 0;
    } else if (x + 2 > map_info.grid_width) {
        b.width = map_info.grid_width - b.x;
    }
    if (b.y < 0) {
        b.height = y + 2;
        b.y = 0;
    } else if (y + 2 > map_info.grid_height) {
        b.height = map_info.grid_height - b.y;
    }
    return b;
}

static int32_t is_inside_map(int32_t x, int32_t y)
{
    return map_grid_is_inside(map_info.diamond_x_min + x, map_info.diamond_y_min + y);
}

static void mark_tile_drawn(int32_t offset)
{
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_SIZE;
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
}

static void for_each_tile(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t (*accept)(uint32_t terrain, int32_t offset), void (*fn)(int32_t x, int32_t y))
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
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++) {
            int32_t offset = tile_offset(x, y);
            if (accept(map_grids.terrain[offset], offset)) {
                fn(x, y);
            }
        }
    }
}

static int32_t sweep_has_passed(int32_t x)
{
    return map_sweep_countdown > 0 && (map_sweep_total - map_sweep_countdown) / 2 > GRID_SIZE - x;
}

int32_t map_sweep_has_passed(int32_t x)
{
    return (uint8_t) sweep_has_passed(x);
}

int32_t map_sweep_is_at(int32_t x)
{
    return (uint8_t) (map_sweep_countdown > 0 && (map_sweep_total - map_sweep_countdown) / 2 == GRID_SIZE - x);
}

void map_tiles_set_vegetation_image(int32_t offset)
{
    uint32_t terrain = map_grids.terrain[offset];
    int32_t random = map_grids.random[offset] & 7;
    if (!(terrain & TERRAIN_TREE) || (terrain & TERRAIN_DIKE)) {
        if ((terrain & TERRAIN_MARSH) && !(terrain & TERRAIN_DIKE) && map_terrain_count_neighbours_with(TERRAIN_MARSH, offset) == 8) {
            int32_t blood = map_sweep_has_passed((offset - map_info.grid_start) % GRID_SIZE);
            if (map_grids.vegetation_growth[offset] == VEGETATION_FULLY_GROWN) {
                map_grids.images[offset] = random + image_group(blood ? GROUP_TERRAIN_REEDS_GROWN_BLOOD : GROUP_TERRAIN_REEDS_GROWN);
            } else {
                map_grids.images[offset] = random + image_group(blood ? GROUP_TERRAIN_REEDS_BLOOD : GROUP_TERRAIN_REEDS);
            }
        }
        return;
    }
    int32_t moisture = map_grids.moisture[offset];
    if (moisture > 100) {
        moisture = 100;
    }
    if (map_grids.vegetation_growth[offset] != VEGETATION_FULLY_GROWN) {
        map_grids.images[offset] = image_group(GROUP_TERRAIN_TREE_GROWING) + random + (moisture / 34) * 8;
        return;
    }
    int32_t relative = offset - map_info.grid_start;
    int32_t distance = 3;
    while (!map_terrain_ring_all_tree_or_rock(relative % GRID_SIZE, relative / GRID_SIZE, distance)) {
        distance--;
    }
    int32_t extra;
    if (distance < 2) {
        extra = (moisture / 34) << 4;
    } else {
        extra = 48;
        distance -= 2;
    }
    map_grids.images[offset] = image_group(GROUP_TERRAIN_TREE) + random + distance * 8 + extra;
}

static int32_t water_group(int32_t blood)
{
    return image_group(blood ? GROUP_TERRAIN_WATER_BLOOD : GROUP_TERRAIN_WATER);
}

static int32_t floodplain_shore_side(const uint8_t *flooded, int32_t *count)
{
    int32_t o = map_info.orientation;
    switch (context_group_offset) {
        case 8:
            return flooded[(o - 4) & 7] ? 4 : -1;
        case 12:
            return flooded[(o - 2) & 7] ? 6 : -1;
        case 16:
            return flooded[o & 7] ? 0 : -1;
        case 20:
            return flooded[(o + 2) & 7] ? 2 : -1;
        default:
            break;
    }
    int32_t side;
    switch (context_group_offset) {
        case 24:
            if (flooded[(o - 2) & 7]) {
                side = 6;
            } else if (flooded[(o - 4) & 7]) {
                side = 4;
            } else {
                return -1;
            }
            break;
        case 28:
            if (flooded[(o - 2) & 7]) {
                side = 6;
            } else if (flooded[o & 7]) {
                side = 0;
            } else {
                return -1;
            }
            break;
        case 32:
            if (flooded[(o + 2) & 7]) {
                side = 2;
            } else if (flooded[o & 7]) {
                side = 0;
            } else {
                return -1;
            }
            break;
        case 36:
            if (flooded[(o + 2) & 7]) {
                side = 2;
            } else if (flooded[(o - 4) & 7]) {
                side = 4;
            } else {
                return -1;
            }
            break;
        default:
            return -1;
    }
    int32_t straight_neighbour = map_adjacent_tiles[DIRECTION_LEFT] || map_adjacent_tiles[DIRECTION_BOTTOM] || map_adjacent_tiles[DIRECTION_RIGHT] ||
        map_adjacent_tiles[DIRECTION_TOP];
    if (*count == 2 && straight_neighbour) {
        *count = 4;
    }
    return side;
}

static int32_t canal_mouth_side(void)
{
    int32_t o = map_info.orientation;
    const int32_t *t = map_adjacent_tiles;
    int32_t side = -1;
    switch (context_group_offset) {
        case 8:
            return t[(o - 4) & 7] ? 3 : -1;
        case 12:
            return t[(o - 2) & 7] ? 0 : -1;
        case 16:
            return t[o & 7] ? 1 : -1;
        case 20:
            return t[(o + 2) & 7] ? 2 : -1;
        default:
            break;
    }
    int32_t left = t[(o - 2) & 7];
    if (left && t[(o - 4) & 7]) {
        side = 7;
    } else if (left && t[o & 7]) {
        side = 4;
    } else if (t[(o + 2) & 7]) {
        if (t[o & 7]) {
            side = 5;
        } else if (t[(o - 4) & 7]) {
            side = 6;
        }
    }
    switch (context_group_offset) {
        case 24:
            if (t[(o - 4) & 7]) {
                side = 11;
            } else if (left) {
                side = 8;
            }
            break;
        case 28:
            if (left) {
                side = 13;
            } else if (t[o & 7]) {
                side = 10;
            }
            break;
        case 32:
            if (t[(o + 2) & 7]) {
                side = 12;
            } else if (t[o & 7]) {
                side = 15;
            }
            break;
        case 36:
            if (t[(o + 2) & 7]) {
                side = 9;
            } else if (t[(o - 4) & 7]) {
                side = 14;
            }
            break;
        default:
            break;
    }
    return side;
}

static int32_t deep_or_shore_water_image(int32_t offset, int32_t x0, int32_t blood)
{
    if (!(map_grids.terrain[offset] & TERRAIN_DEEPWATER)) {
        if (context_group_offset == 0) {
            return water_group(blood) + context_item_offset;
        }
        return image_group(blood ? GROUP_TERRAIN_SHORE_BLOOD : GROUP_TERRAIN_SHORE) + context_item_offset + context_group_offset;
    }
    map_image_context_reset(context_deepwater, 26);
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles(TERRAIN_DEEPWATER);
    map_image_context_invert_tiles();
    map_image_context_match(context_deepwater, 26, map_grids.random[map_tile_cursor_offset], 1);
    if (blood) {
        return image_group(GROUP_TERRAIN_DEEPWATER_BLOOD) + context_item_offset * 15 + context_group_offset;
    }
    if (map_sweep_is_at(x0)) {
        return image_group(GROUP_TERRAIN_BLOOD_FRONT) + (map_grids.random[map_tile_cursor_offset] & 3) + (map_info.orientation / 2) * 4;
    }
    return image_group(GROUP_TERRAIN_DEEPWATER) + context_item_offset * 15 + context_group_offset;
}

static int32_t floodplain_shore_image(int32_t floodplain_tiles, int32_t blood)
{
    uint8_t flooded[8];
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        const flood_tile *node = floods_data.tiles[map_tile_cursor_offset + neighbour_delta[i]];
        flooded[i] = node && node->is_floodplain == 0;
    }
    int32_t count = (floodplain_tiles > 1) ? 4 : (floodplain_tiles * 2);
    int32_t side = floodplain_shore_side(flooded, &count);
    if (side < 0) {
        return -1;
    }
    if (context_group_offset <= 20) {
        count = 0;
    }
    int32_t floodplain = image_group(GROUP_TERRAIN_FLOODPLAIN);
    int32_t image = floodplain + (((side - 4) & 7) + 20) * 3 + count;
    if (map_adjacent_tiles[(map_info.orientation + 1 + side) & 7] || map_adjacent_tiles[(map_info.orientation + 2 + side) & 7]) {
        image++;
    }
    if (blood && image - floodplain > 59 && image - floodplain < 84) {
        image += image_group(GROUP_TERRAIN_FLOODPLAIN_BLOOD) - floodplain - 60;
    }
    return image;
}

static int32_t canal_mouth_or_water_image(int32_t offset, int32_t blood)
{
    int32_t image = water_group(blood) + context_item_offset + context_group_offset;
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles_without(TERRAIN_CANAL, TERRAIN_WATER);
    map_adjacent_tiles[DIRECTION_TOP_LEFT] = map_adjacent_tiles[DIRECTION_BOTTOM_LEFT] = map_adjacent_tiles[DIRECTION_BOTTOM_RIGHT] = map_adjacent_tiles
        [DIRECTION_TOP_RIGHT] = 0;
    if (map_image_context_count_tiles()) {
        int32_t side = canal_mouth_side();
        if (side >= 0) {
            image = image_group(blood ? GROUP_TERRAIN_WATER_SHORE_BLOOD : GROUP_TERRAIN_WATER_SHORE) + side;
        }
    }
    return image;
}

static int32_t water_tile_image(int32_t offset, int32_t x0, int32_t blood)
{
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles(TERRAIN_WATER);
    map_image_context_invert_tiles();
    map_image_context_match(context_water, 48, map_grids.random[offset], 0);
    if (map_grids.terrain[offset] & TERRAIN_FLOODPLAIN) {
        if ((uint32_t) context_group_offset >= 8) {
            int32_t image = water_group(blood) + context_item_offset + context_group_offset;
            if (image >= 0) {
                return image;
            }
        }
        return deep_or_shore_water_image(offset, x0, blood);
    }
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles(TERRAIN_FLOODPLAIN);
    int32_t floodplain_tiles = map_image_context_count_tiles();
    if (floodplain_tiles != 0) {
        int32_t image = floodplain_shore_image(floodplain_tiles, blood);
        if (image >= 0) {
            return image;
        }
        image = canal_mouth_or_water_image(offset, blood);
        if (image >= 0) {
            return image;
        }
    }
    return deep_or_shore_water_image(offset, x0, blood);
}

void map_tiles_set_water_image(int32_t x, int32_t y, int32_t set)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    int32_t offset = tile_offset(x, y);
    if (set == 1) {
        map_grids.terrain[offset] |= TERRAIN_WATER;
    } else {
        map_grids.terrain[offset] &= ~(TERRAIN_WATER | TERRAIN_DEEPWATER);
    }
    block b = block_around(x, y);
    int32_t blood = map_sweep_has_passed(b.x);
    int32_t saved_cursor = map_tile_cursor_offset;
    for (int32_t yy = b.y; yy < b.y + b.height; yy++) {
        for (int32_t xx = b.x; xx < b.x + b.width; xx++) {
            int32_t tile = tile_offset(xx, yy);
            if (!(map_grids.terrain[tile] & TERRAIN_WATER) || (map_grids.terrain[tile] & TERRAIN_BUILDING)) {
                continue;
            }
            int32_t image = water_tile_image(tile, b.x, blood);
            map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
            map_grids.images[tile] = image;
            map_grids.bitfields[tile] &= ~MAP_BITFIELD_SIZE;
        }
    }
    map_tile_cursor_offset = saved_cursor;
}

static int32_t is_water_tile(uint32_t terrain, int32_t offset)
{
    (void) offset;
    return (terrain & TERRAIN_WATER) && !(terrain & TERRAIN_BUILDING);
}

static void redraw_water(int32_t x, int32_t y)
{
    map_tiles_set_water_image(x, y, 1);
}

void map_tiles_update_region_water(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t update_deepwater)
{
    if ((char) update_deepwater) {
        map_terrain_update_deepwater(x0 - 5, y0 - 5, x1 + 5, y1 + 5);
    }
    for_each_tile(x0, y0, x1, y1, is_water_tile, redraw_water);
}

void map_tiles_set_marshland_image(int32_t x, int32_t y, int32_t set)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    map_tile_cursor_offset = tile_offset(x, y);
    if (set == 1) {
        map_grids.terrain[map_tile_cursor_offset] |= TERRAIN_MARSH;
    } else {
        map_grids.terrain[map_tile_cursor_offset] &= ~TERRAIN_MARSH;
    }
    block b = block_around(x, y);
    int32_t blood = map_sweep_has_passed(b.x);
    map_tile_cursor_offset = tile_offset(b.x, b.y);
    for (map_tile_cursor_y = b.y; map_tile_cursor_y < b.y + b.height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = b.x; map_tile_cursor_x < b.x + b.width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            if (!(map_grids.terrain[map_tile_cursor_offset] & TERRAIN_MARSH) || (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_BUILDING)) {
                continue;
            }
            map_image_context_set_tiles(TERRAIN_MARSH);
            map_image_context_invert_tiles();
            map_image_context_match(context_water, 48, map_grids.random[map_tile_cursor_offset], 0);
            int32_t offset = map_tile_cursor_offset;
            map_grids.images[offset] = image_group(blood ? GROUP_TERRAIN_REEDS_BLOOD : GROUP_TERRAIN_REEDS) + context_item_offset + context_group_offset;
            mark_tile_drawn(offset);
            map_tiles_set_vegetation_image(offset);
        }
        map_tile_cursor_offset += GRID_SIZE - b.width;
    }
}

static int32_t is_marshland_tile(uint32_t terrain, int32_t offset)
{
    (void) offset;
    return (terrain & TERRAIN_MARSH) && !(terrain & TERRAIN_BUILDING);
}

static void redraw_marshland(int32_t x, int32_t y)
{
    map_tiles_set_marshland_image(x, y, 1);
}

void map_tiles_update_region_marshland(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    for_each_tile(x0, y0, x1, y1, is_marshland_tile, redraw_marshland);
}

void map_tiles_set_tree_image(int32_t x, int32_t y, int32_t set)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    int32_t offset = tile_offset(x, y);
    if (set == 1) {
        map_grids.terrain[offset] |= TERRAIN_TREE;
    } else {
        map_grids.terrain[offset] &= ~TERRAIN_TREE;
    }
    block b = block_around(x, y);
    map_tile_cursor_y = b.y;
    map_tile_cursor_offset = tile_offset(b.x, b.y);
    for (; map_tile_cursor_y < b.y + b.height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = b.x; map_tile_cursor_x < b.x + b.width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if ((terrain & TERRAIN_TREE) && !(terrain & (TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP))) {
                map_tiles_set_vegetation_image(map_tile_cursor_offset);
                mark_tile_drawn(map_tile_cursor_offset);
                map_grids.canals[map_tile_cursor_offset] = 0;
            }
        }
        map_tile_cursor_offset += GRID_SIZE - b.width;
    }
}

static int32_t is_tree_tile(uint32_t terrain, int32_t offset)
{
    (void) offset;
    return (terrain & TERRAIN_TREE) && !(terrain & (TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP));
}

static void redraw_tree(int32_t x, int32_t y)
{
    map_tiles_set_tree_image(x, y, 1);
}

void map_tiles_update_region_trees(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    for_each_tile(x0, y0, x1, y1, is_tree_tile, redraw_tree);
}

static int32_t is_drawn_meadow(uint32_t terrain)
{
    return (terrain & TERRAIN_MEADOW) && !(terrain & (TERRAIN_CANAL | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) && !(terrain & TERRAIN_ROAD) &&
        (!(terrain & TERRAIN_BUILDING) || (terrain & TERRAIN_MONUMENT)) && !(terrain & TERRAIN_GARDEN) && !(terrain & TERRAIN_RUBBLE);
}

void map_tiles_set_meadow_image(int32_t x, int32_t y, int32_t set)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    int32_t offset = tile_offset(x, y);
    if (set == 1) {
        map_grids.terrain[offset] |= TERRAIN_MEADOW;
    } else {
        map_grids.terrain[offset] &= ~TERRAIN_MEADOW;
    }
    block b = block_around(x, y);
    map_tile_cursor_y = b.y;
    map_tile_cursor_offset = tile_offset(b.x, b.y);
    for (; map_tile_cursor_y < b.y + b.height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = b.x; map_tile_cursor_x < b.x + b.width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            int32_t tile = map_tile_cursor_offset;
            if (!is_drawn_meadow(map_grids.terrain[tile])) {
                continue;
            }
            int32_t moisture = map_grids.moisture[tile];
            if (moisture > 100) {
                moisture = 100;
            }
            int32_t fertility = (int32_t) ((map_grids.soil_fertility[tile] - 1) * 0.010101010101010102 * 3.0);
            int32_t wetness = (int32_t) ((moisture - 1) / 96.0 * 12.0);
            int32_t group = GROUP_TERRAIN_FLOODPLAIN_FERTILITY;
            if (fertility == 0) {
                if (wetness == 0) {
                    wetness = map_grids.random[tile] & 7;
                    group = GROUP_TERRAIN_MEADOW_INFERTILE_DRIEST;
                }
            } else if (fertility == 2) {
                if (wetness == 0) {
                    fertility = 0;
                    wetness = map_grids.random[tile] & 7;
                    group = GROUP_TERRAIN_MEADOW_FERTILE_DRIEST;
                } else if (wetness == 11) {
                    fertility = 0;
                    wetness = map_grids.random[tile] & 7;
                    group = GROUP_TERRAIN_MEADOW_FERTILE_WETTEST;
                }
            }
            map_grids.images[tile] = image_group(group) + fertility * 12 + wetness;
            mark_tile_drawn(tile);
            map_grids.canals[tile] = 0;
        }
        map_tile_cursor_offset += GRID_SIZE - b.width;
    }
}

static int32_t is_meadow_tile(uint32_t terrain, int32_t offset)
{
    (void) offset;
    return (terrain & TERRAIN_MEADOW) && !(terrain & (TERRAIN_RUBBLE | TERRAIN_CANAL | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) && !(terrain & TERRAIN_ROAD) &&
        (!(terrain & TERRAIN_BUILDING) || (terrain & TERRAIN_MONUMENT)) && !(terrain & TERRAIN_GARDEN);
}

static void redraw_meadow(int32_t x, int32_t y)
{
    map_tiles_set_meadow_image(x, y, 1);
}

void map_tiles_update_region_meadow(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    for_each_tile(x0, y0, x1, y1, is_meadow_tile, redraw_meadow);
}

void map_tiles_set_floodplain_image(int32_t x, int32_t y, int32_t set)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    map_tile_cursor_offset = tile_offset(x, y);
    if (set == 1) {
        map_grids.terrain[map_tile_cursor_offset] |= TERRAIN_FLOODPLAIN;
    } else {
        map_grids.terrain[map_tile_cursor_offset] &= ~TERRAIN_FLOODPLAIN;
    }
    block b = block_around(x, y);
    int32_t blood = map_sweep_has_passed(b.x);
    map_tile_cursor_offset = tile_offset(b.x, b.y);
    for (map_tile_cursor_y = b.y; map_tile_cursor_y < b.y + b.height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = b.x; map_tile_cursor_x < b.x + b.width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if (!(terrain & TERRAIN_FLOODPLAIN) || (terrain & (TERRAIN_BUILDING | TERRAIN_ROAD | TERRAIN_CANAL))) {
                continue;
            }
            map_image_context_set_tiles(TERRAIN_FLOODPLAIN);
            map_image_context_invert_tiles();
            if (floodplain_node_dry(map_tile_cursor_offset)) {
                map_image_context_or_tiles(TERRAIN_WATER, 1);
                map_adjacent_tiles[DIRECTION_TOP_LEFT] = map_adjacent_tiles[DIRECTION_BOTTOM_LEFT] = map_adjacent_tiles
                    [DIRECTION_BOTTOM_RIGHT] = map_adjacent_tiles[DIRECTION_TOP_RIGHT] = 0;
                map_image_context_match_floodsystem(context_floodsystem, 24);
            } else {
                map_image_context_match(context_floodsystem, 24, 0, 1);
            }
            int32_t tile = map_tile_cursor_offset;
            context_group_offset = 0;
            int32_t floodplain = image_group(GROUP_TERRAIN_FLOODPLAIN);
            int32_t image = floodplain + context_item_offset;
            const flood_tile *node = floods_data.tiles[tile];
            if (node) {
                int32_t node_offset = node->grid_offset;
                int32_t fertility = (int32_t) (map_grids.soil_fertility[node_offset] * 0.01 * 8.0);
                const uint8_t *stage = *(uint8_t *const *) node;
                image += *(const int32_t *) (stage + 4) + ((fertility & ~1) | (map_grids.random[node_offset] & 1)) * 6;
            }
            if (blood && image - floodplain >= 60 && image - floodplain <= 83) {
                image += image_group(GROUP_TERRAIN_FLOODPLAIN_BLOOD) - floodplain - 60;
            }
            map_grids.bitfields[tile] &= ~MAP_BITFIELD_SIZE;
            map_grids.images[tile] = image;
            map_grids.edge[tile] |= MAP_EDGE_DRAW_TILE;
            map_grids.canals[tile] = 0;
        }
        map_tile_cursor_offset += GRID_SIZE - b.width;
    }
}

static int32_t is_floodplain_tile(uint32_t terrain, int32_t offset)
{
    (void) offset;
    return (terrain & TERRAIN_FLOODPLAIN) && !(terrain & TERRAIN_BUILDING);
}

static void redraw_floodplain(int32_t x, int32_t y)
{
    map_tiles_set_floodplain_image(x, y, 1);
}

void map_tiles_update_region_floodplain(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    for_each_tile(x0, y0, x1, y1, is_floodplain_tile, redraw_floodplain);
}

void map_tiles_set_rubble_image(int32_t x, int32_t y)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    int32_t offset = tile_offset(x, y);
    map_tile_cursor_offset = offset;
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_SIZE;
    map_grids.canals[offset] = 0;
    map_grids.images[offset] = image_group(GROUP_TERRAIN_RUBBLE) + (map_grids.random[offset] & 7);
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
    map_grids.terrain[offset] |= TERRAIN_RUBBLE;
}

static int32_t is_rubble_tile(uint32_t terrain, int32_t offset)
{
    (void) offset;
    return (terrain & TERRAIN_RUBBLE) && !(terrain & (TERRAIN_CANAL | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) &&
        !(terrain & (TERRAIN_ROAD | TERRAIN_GARDEN | TERRAIN_BUILDING));
}

static void redraw_rubble(int32_t x, int32_t y)
{
    map_tiles_set_rubble_image(x, y);
}

void map_tiles_update_region_rubble(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    for_each_tile(x0, y0, x1, y1, is_rubble_tile, redraw_rubble);
}

void map_tiles_set_fault_image(int32_t x, int32_t y, int32_t set)
{
    if (!is_inside_map(x, y)) {
        return;
    }
    int32_t offset = tile_offset(x, y);
    if (set == 1) {
        map_grids.terrain[offset] |= TERRAIN_ROCK;
        map_grids.bitfields[offset] |= MAP_BITFIELD_PLAZA_OR_FAULT;
    } else {
        map_grids.terrain[offset] &= ~TERRAIN_ROCKS;
        map_grids.bitfields[offset] &= ~MAP_BITFIELD_PLAZA_OR_FAULT;
    }
    block b = block_around(x, y);
    map_tile_cursor_y = b.y;
    map_tile_cursor_offset = tile_offset(b.x, b.y);
    for (; map_tile_cursor_y < b.y + b.height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = b.x; map_tile_cursor_x < b.x + b.width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            if (!(map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROCK) || !(map_grids.bitfields[map_tile_cursor_offset] & MAP_BITFIELD_PLAZA_OR_FAULT)) {
                continue;
            }
            map_image_context_set_tiles_fault();
            map_image_context_normalize_tiles();
            int32_t image = image_group(GROUP_TERRAIN_FAULT);
            if (map_image_context_match(context_fault, 17, -1, 0)) {
                image += context_item_offset + context_group_offset;
            }
            map_grids.images[map_tile_cursor_offset] = image;
            mark_tile_drawn(map_tile_cursor_offset);
        }
        map_tile_cursor_offset += GRID_SIZE - b.width;
    }
}

static int32_t is_fault_tile(uint32_t terrain, int32_t offset)
{
    return (terrain & TERRAIN_ROCK) && (map_grids.bitfields[offset] & MAP_BITFIELD_PLAZA_OR_FAULT);
}

static void redraw_fault(int32_t x, int32_t y)
{
    map_tiles_set_fault_image(x, y, 1);
}

void map_tiles_update_region_faults(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    for_each_tile(x0, y0, x1, y1, is_fault_tile, redraw_fault);
}

static int32_t is_fault_or_raised(int32_t offset)
{
    return (map_grids.bitfields[offset] & MAP_BITFIELD_PLAZA_OR_FAULT) || (map_grids.terrain[offset] & (TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP));
}

static int32_t monument_neighbour_blocks(int32_t building_id)
{
    const building *b = building_get(building_id);
    int32_t type = b->type;
    if (type == BUILDING_ABU_SIMBEL) {
        return 1;
    }
    if (type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB) {
        const building *main_part = building_get(building_id - b->subtype);
        return b->data.royal_tomb.chamber_kind != 18 && main_part->data.royal_tomb.completed == 1;
    }
    return 0;
}

static int32_t rock_monument_turned(int32_t first_image, int32_t variants, int32_t turns, int32_t turn)
{
    int32_t view_turn = map_info.orientation / 2;
    return first_image + (variants * ((turn + view_turn) % turns));
}

int32_t map_tiles_rock_monument_image(int32_t offset)
{
    int32_t rock[8];
    for (int32_t i = 0; i < 16; i++) {
        map_adjacent_tiles[i] = 0;
    }
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles_all(TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT);
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        rock[i] = map_adjacent_tiles[i];
    }
    map_image_context_set_tiles_all(TERRAIN_DIKE);
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] = map_adjacent_tiles[i] ? 0 : rock[i];
    }
    int32_t cursor = map_tile_cursor_offset;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        int32_t building_id = map_grids.building_ids[neighbour_delta[i] + cursor];
        if (building_id > 0 && monument_neighbour_blocks(building_id)) {
            map_adjacent_tiles[i] = 1;
        }
    }
    int32_t count = 0;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        if (map_adjacent_tiles[i] == 1) {
            count++;
        }
    }
    const int32_t *t = map_adjacent_tiles;
    uint32_t random = map_grids.random[offset];
    uint32_t corner_variant = random % ROCK_MONUMENT_CORNER_VARIANTS;
    uint32_t side_variant = random % ROCK_MONUMENT_SIDE_VARIANTS;
    if (count == 8) {
        return (random % ROCK_MONUMENT_FULL_VARIANTS) + ROCK_MONUMENT_IMAGE_FULL;
    }
    if (count == 7) {
        if (!t[DIRECTION_TOP_RIGHT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_INNER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 0) + corner_variant;
        }
        if (!t[DIRECTION_TOP_LEFT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_INNER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 1) + corner_variant;
        }
        if (!t[DIRECTION_BOTTOM_LEFT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_INNER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 2) + corner_variant;
        }
        if (!t[DIRECTION_BOTTOM_RIGHT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_INNER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 3) + corner_variant;
        }
    } else if (count == 6) {
        if (!t[DIRECTION_TOP_LEFT] && !t[DIRECTION_BOTTOM_RIGHT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_TWO_CORNERS, ROCK_MONUMENT_CORNER_VARIANTS, 2, 0) + corner_variant;
        }
        if (!t[DIRECTION_TOP_RIGHT] && !t[DIRECTION_BOTTOM_LEFT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_TWO_CORNERS, ROCK_MONUMENT_CORNER_VARIANTS, 2, 1) + corner_variant;
        }
    }
    if (t[DIRECTION_LEFT] && t[DIRECTION_TOP_LEFT] && t[DIRECTION_TOP] && t[DIRECTION_TOP_RIGHT] && t[DIRECTION_RIGHT] && count < 8) {
        return rock_monument_turned(ROCK_MONUMENT_IMAGE_SIDE, ROCK_MONUMENT_SIDE_VARIANTS, VIEW_TURNS, 0) + side_variant;
    }
    if (t[DIRECTION_BOTTOM] && t[DIRECTION_BOTTOM_LEFT] && t[DIRECTION_LEFT] && t[DIRECTION_TOP_LEFT] && t[DIRECTION_TOP] && count < 8) {
        return rock_monument_turned(ROCK_MONUMENT_IMAGE_SIDE, ROCK_MONUMENT_SIDE_VARIANTS, VIEW_TURNS, 1) + side_variant;
    }
    if (t[DIRECTION_RIGHT] && t[DIRECTION_BOTTOM_RIGHT] && t[DIRECTION_BOTTOM] && t[DIRECTION_BOTTOM_LEFT] && t[DIRECTION_LEFT] && count < 8) {
        return rock_monument_turned(ROCK_MONUMENT_IMAGE_SIDE, ROCK_MONUMENT_SIDE_VARIANTS, VIEW_TURNS, 2) + side_variant;
    }
    if (t[DIRECTION_TOP] && t[DIRECTION_TOP_RIGHT]) {
        if (t[DIRECTION_RIGHT] && t[DIRECTION_BOTTOM_RIGHT] && t[DIRECTION_BOTTOM] && count < 8) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_SIDE, ROCK_MONUMENT_SIDE_VARIANTS, VIEW_TURNS, 3) + side_variant;
        }
        if (t[DIRECTION_TOP_RIGHT] && t[DIRECTION_RIGHT]) {
            return rock_monument_turned(ROCK_MONUMENT_IMAGE_OUTER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 0) + corner_variant;
        }
    }
    if (t[DIRECTION_LEFT] && t[DIRECTION_TOP_LEFT] && t[DIRECTION_TOP]) {
        return rock_monument_turned(ROCK_MONUMENT_IMAGE_OUTER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 1) + corner_variant;
    }
    if (t[DIRECTION_BOTTOM] && t[DIRECTION_BOTTOM_LEFT] && t[DIRECTION_LEFT]) {
        return rock_monument_turned(ROCK_MONUMENT_IMAGE_OUTER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 2) + corner_variant;
    }
    if (t[DIRECTION_RIGHT] && t[DIRECTION_BOTTOM_RIGHT] && t[DIRECTION_BOTTOM]) {
        return rock_monument_turned(ROCK_MONUMENT_IMAGE_OUTER_CORNER, ROCK_MONUMENT_CORNER_VARIANTS, VIEW_TURNS, 3) + corner_variant;
    }
    return -1;
}

static int32_t is_plain_rock(int32_t offset)
{
    return (map_grids.terrain[offset] & TERRAIN_ROCK) && !is_fault_or_raised(offset);
}

static void place_rock(int32_t x, int32_t y, int32_t offset, uint32_t terrain, int32_t group)
{
    if (map_building_tiles_area_matches(x, y, 3, 3, terrain, 1)) {
        map_building_tiles_add(0, x, y, 3, 3, (map_grids.random[offset] & 1) + 12 + group, terrain);
    } else if (map_building_tiles_area_matches(x, y, 2, 2, terrain, 1)) {
        map_building_tiles_add(0, x, y, 2, 2, (map_grids.random[offset] & 3) + 8 + group, terrain);
    } else {
        map_grids.images[offset] = (map_grids.random[offset] & 7) + group;
    }
}

void map_tiles_update_all_rocks(void)
{
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            if (is_plain_rock(offset)) {
                map_grids.images[offset] = 0;
                mark_tile_drawn(offset);
            }
        }
    }
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            uint32_t terrain = map_grids.terrain[offset];
            if (!is_plain_rock(offset) || map_grids.images[offset] != 0) {
                continue;
            }
            if ((terrain & (TERRAIN_ORE | TERRAIN_ROCK)) == (TERRAIN_ORE | TERRAIN_ROCK)) {
                place_rock(x, y, offset, TERRAIN_ORE | TERRAIN_ROCK, image_group(GROUP_TERRAIN_ORE_ROCK));
            } else if ((terrain & (TERRAIN_ROCK_NEAR_MONUMENT | TERRAIN_ROCK)) == (TERRAIN_ROCK_NEAR_MONUMENT | TERRAIN_ROCK)) {
                int32_t group = image_group(GROUP_TERRAIN_ROCK_MONUMENT);
                int32_t image = map_tiles_rock_monument_image(offset);
                if (image == -1) {
                    map_grids.images[offset] = (map_grids.random[offset] & 7) + image_group(GROUP_TERRAIN_ROCK);
                } else {
                    map_grids.images[offset] = image + group;
                }
            } else {
                place_rock(x, y, offset, TERRAIN_ROCK, image_group(GROUP_TERRAIN_ROCK));
            }
        }
    }
}

static int32_t is_garden(int32_t offset)
{
    return (map_grids.terrain[offset] & TERRAIN_GARDEN) && !(map_grids.terrain[offset] & (TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP));
}

void map_tiles_update_all_gardens(void)
{
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            if (is_garden(offset)) {
                map_grids.images[offset] = 0;
                mark_tile_drawn(offset);
            }
        }
    }
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            if (!is_garden(offset) || map_grids.images[offset] != 0) {
                continue;
            }
            int32_t group = image_group(GROUP_TERRAIN_GARDEN);
            if (map_building_tiles_area_matches(x, y, 3, 3, TERRAIN_GARDEN, 1)) {
                map_building_tiles_add(0, x, y, 3, 3, group + 7, TERRAIN_GARDEN);
            } else if (map_building_tiles_area_matches(x, y, 2, 2, TERRAIN_GARDEN, 1)) {
                int32_t random = map_grids.random[offset] & 3;
                map_building_tiles_add(0, x, y, 2, 2, group + (random < 2 ? 6 : random < 3 ? 5 : 4), TERRAIN_GARDEN);
            } else {
                map_grids.images[offset] = group + (x & 1) + (y & 1) * 2;
            }
        }
    }
}

static int32_t is_plain_dune(int32_t offset)
{
    return (map_grids.terrain[offset] & TERRAIN_DUNE) && !is_fault_or_raised(offset);
}

int32_t map_terrain_exists_tile_in_radius_with_type(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask)
{
    int32_t x0 = x - radius;
    int32_t x1 = x + size - 1 + radius;
    int32_t y1 = y + size - 1 + radius;
    map_result_y = y - radius;
    if (x0 < 0) {
        x0 = 0;
    }
    if (map_result_y < 0) {
        map_result_y = 0;
    }
    if (x1 >= map_info.grid_width) {
        x1 = map_info.grid_width - 1;
    }
    if (y1 >= map_info.grid_height) {
        y1 = map_info.grid_height - 1;
    }
    int32_t offset = tile_offset(x0, map_result_y);
    for (; map_result_y <= y1; map_result_y++) {
        for (map_result_x = x0; map_result_x <= x1; map_result_x++, offset++) {
            if (map_grids.terrain[offset] & mask) {
                return 1;
            }
        }
        offset += x0 - x1 + (GRID_SIZE - 1);
    }
    return 0;
}

void map_tiles_update_all_dunes(void)
{
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            if (is_plain_dune(offset)) {
                map_grids.images[offset] = 0;
                mark_tile_drawn(offset);
            }
        }
    }
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t offset = map_row_start_offset[y];
        for (int32_t x = map_row_x_start[y]; x < map_row_x_end[y]; x++, offset++) {
            if (!is_plain_dune(offset) || map_grids.images[offset] != 0) {
                continue;
            }
            int32_t group = image_group(GROUP_TERRAIN_DUNES);
            if (map_building_tiles_area_matches(x, y, 3, 3, TERRAIN_DUNE, 1)) {
                map_terrain_exists_tile_in_radius_with_type(x, y, 3, 4, TERRAIN_ELEVATION);
                map_building_tiles_add(0, x, y, 3, 3, (map_grids.random[offset] & 1) + 12 + group, TERRAIN_DUNE);
            } else if (map_building_tiles_area_matches(x, y, 2, 2, TERRAIN_DUNE, 1)) {
                map_terrain_exists_tile_in_radius_with_type(x, y, 2, 4, TERRAIN_ELEVATION);
                map_building_tiles_add(0, x, y, 2, 2, (map_grids.random[offset] & 3) + 8 + group, TERRAIN_DUNE);
            } else {
                map_terrain_exists_tile_in_radius_with_type(x, y, 1, 4, TERRAIN_ELEVATION);
                map_grids.images[offset] = (map_grids.random[offset] & 7) + group;
            }
        }
    }
}

static int32_t is_plaza(int32_t offset)
{
    return (map_grids.terrain[offset] & TERRAIN_ROAD) && (map_grids.bitfields[offset] & MAP_BITFIELD_PLAZA_OR_FAULT);
}

bool map_plaza_square_fits(int32_t offset)
{
    static const int32_t deltas[] = { 1, GRID_SIZE, GRID_SIZE + 1 };
    int32_t score = 0;
    for (int32_t i = 0; i < 3; i++) {
        int32_t tile = offset + deltas[i];
        uint32_t terrain = map_grids.terrain[tile];
        if (terrain & TERRAIN_ROAD) {
            score++;
        }
        if ((map_grids.bitfields[tile] & MAP_BITFIELD_PLAZA_OR_FAULT) == 0) {
            score--;
        }
        if (terrain & TERRAIN_WATER) {
            score--;
        }
        if (terrain & TERRAIN_BUILDING) {
            score--;
        }
        if (terrain & TERRAIN_CANAL) {
            score--;
        }
        if (map_grids.images[tile] != 0) {
            score--;
        }
    }
    return score == 3;
}

void map_tiles_update_all_plazas(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to, int32_t clear)
{
    int32_t x0 = x_from <= x_to ? x_from : x_to;
    int32_t x1 = x_from <= x_to ? x_to : x_from;
    int32_t y0 = y_from <= y_to ? y_from : y_to;
    int32_t y1 = y_from <= y_to ? y_to : y_from;
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
    if (clear) {
        for (int32_t y = y0; y <= y1; y++) {
            for (int32_t x = x0; x <= x1; x++) {
                int32_t offset = tile_offset(x, y);
                int32_t building_id = map_grids.building_ids[offset];
                if (is_plaza(offset) && (building_id == 0 || building_get(building_id)->type != BUILDING_ROADBLOCK)) {
                    map_grids.images[offset] = 0;
                    mark_tile_drawn(offset);
                }
            }
        }
    }
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++) {
            int32_t offset = tile_offset(x, y);
            if (!(map_grids.terrain[offset] & TERRAIN_ROAD) || map_grids.images[offset] != 0 || !(map_grids.bitfields[offset] & MAP_BITFIELD_PLAZA_OR_FAULT)) {
                continue;
            }
            int32_t group = image_group(GROUP_TERRAIN_PLAZA);
            int32_t random = map_grids.random[offset];
            if (map_plaza_square_fits(offset)) {
                map_building_tiles_add(0, x, y, 2, 2, group + 6 + (random & 1), TERRAIN_ROAD);
            } else {
                if ((x & 1) && (y & 1)) {
                    map_grids.images[offset] = group + 1;
                } else if (!(x & 1) && !(y & 1)) {
                    map_grids.images[offset] = group;
                } else {
                    map_grids.images[offset] = group + 2;
                }
                map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
            }
        }
    }
}

static int32_t any_neighbour_not_empty(int32_t offset)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        if (map_grids.terrain[offset + neighbour_delta[i]] & TERRAIN_NOT_EMPTY) {
            return 1;
        }
    }
    return 0;
}

static int32_t monument_hides_land(int32_t building_id, int32_t offset)
{
    const building *b = building_get(building_id);
    int32_t type = b->type;
    int32_t main_id = building_id - b->subtype;
    const building *main_part = building_get(main_id);
    switch (type) {
        case BUILDING_PYRAMID: {
            int32_t stage = pyramid_tile_effective_stage(offset);
            return stage > 10 || (stage > 2 && pyramid_level_tile_set_image(building_id, offset, stage, 0));
        }
        case BUILDING_SUN_TEMPLE:
        case BUILDING_CAESAREUM:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
        case BUILDING_LIGHTHOUSE:
            return b->data.monument.stage != 0 || main_part->data.monument.stage != 0;
        case BUILDING_ABU_SIMBEL:
            return main_part->data.abu_simbel.stage > 0;
        case BUILDING_ALEXANDRIA_LIBRARY: {
            int32_t least = 21;
            for (int32_t part = main_id; part < main_id + LIBRARY_PARTS; part++) {
                int32_t stage = building_get(part)->data.monument.stage;
                if (stage < least) {
                    least = stage;
                }
            }
            return least > 0;
        }
        default: {
            int32_t found = 0;
            for (int32_t left = offset - 20; left > map_info.grid_start; left -= 20) {
                found = map_grids.building_ids[left];
                if (found || !(map_grids.terrain[left] & TERRAIN_MONUMENT)) {
                    break;
                }
            }
            if (found && building_get(found)->type == BUILDING_PYRAMID) {
                pyramid_complex_queue_refresh(found);
            }
            return 0;
        }
    }
}

static int32_t grass_moisture_level(int32_t offset)
{
    int32_t moisture = map_grids.moisture[offset];
    if (moisture > 100) {
        moisture = 100;
    }
    return (int32_t) (moisture * (12.0 / 96.0));
}

static void set_empty_land_image(int32_t offset)
{
    int32_t group;
    int32_t image;
    int32_t saved_cursor = map_tile_cursor_offset;
    map_tile_cursor_offset = offset;
    map_image_context_set_tiles(TERRAIN_FLOODPLAIN);
    map_tile_cursor_offset = saved_cursor;
    if (map_image_context_count_tiles()) {
        map_image_context_match(context_deepwater, 26, -1, 0);
        group = GROUP_TERRAIN_FLOODPLAIN;
        image = context_group_offset + 48;
    } else {
        int32_t moisture = map_grids.moisture[offset];
        int32_t level;
        if (moisture > 100) {
            level = moisture;
            if (!(level & 8)) {
                level = (level - map_info.orientation) & 7;
            } else {
                level = ((level - map_info.orientation / 2) & 3) + (level & 8);
            }
            level = (level & 0xFF) | 0x80;
        } else {
            level = (int32_t) (moisture * (12.0 / 96.0));
        }
        uint32_t random = map_grids.random[offset];
        if (level < 11) {
            group = GROUP_TERRAIN_GRASS_BY_MOISTURE;
            image = level + (random % 3) * 12;
        } else if (level < 0x80) {
            if (!(map_grids.terrain[offset] & TERRAIN_MONUMENT) && any_neighbour_not_empty(offset)) {
                group = GROUP_TERRAIN_GRASS_WETTEST_BESIDE_TERRAIN;
            } else {
                group = GROUP_TERRAIN_GRASS_WETTEST_INNER;
                for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
                    int32_t neighbour = offset + neighbour_delta[i];
                    uint32_t terrain = map_grids.terrain[neighbour];
                    if (grass_moisture_level(neighbour) != 11 || (terrain & (TERRAIN_FLOODPLAIN | TERRAIN_MEADOW | TERRAIN_TREE)) ||
                        (!(terrain & TERRAIN_MONUMENT) && any_neighbour_not_empty(neighbour))) {
                        group = GROUP_TERRAIN_GRASS_WETTEST_BESIDE_OTHER;
                        break;
                    }
                }
            }
            image = random % 12;
        } else {
            group = GROUP_TERRAIN_GRASS_PLACED;
            image = level & 0x7F;
        }
    }
    if (map_grids.terrain[offset] & TERRAIN_ROAD) {
        image = map_canal_with_road_image(image);
    }
    map_grids.images[offset] = image_group(group) + image;
}

static uint32_t land_terrain(int32_t offset, int32_t *building_id, uint32_t *building_terrain)
{
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & TERRAIN_MONUMENT) {
        *building_id = map_grids.building_ids[offset];
        *building_terrain = TERRAIN_MONUMENT | TERRAIN_BUILDING;
        return terrain & ~(TERRAIN_MONUMENT | TERRAIN_BUILDING);
    }
    *building_id = 0;
    *building_terrain = 0;
    return terrain;
}

void map_tiles_update_region_empty_land(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t keep_drawn, int32_t exact)
{
    keep_drawn = (char) keep_drawn;
    if (!(char) exact) {
        x0--;
        x1++;
        y0--;
        y1++;
    }
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
            int32_t offset = tile_offset(x, y);
            uint32_t terrain = map_grids.terrain[offset];
            if (terrain & TERRAIN_MONUMENT) {
                int32_t building_id = map_grids.building_ids[offset];
                terrain &= ~(TERRAIN_MONUMENT | TERRAIN_BUILDING);
                if (monument_hides_land(building_id, offset)) {
                    continue;
                }
            }
            if (floodplain_node_dry(offset) ? (terrain & TERRAIN_NOT_EMPTY) : (terrain & TERRAIN_BUILDING)) {
                continue;
            }
            if (!(terrain & TERRAIN_GROUNDWATER)) {
                if (!keep_drawn) {
                    map_grids.images[offset] = 0;
                    mark_tile_drawn(offset);
                }
            } else if (!(terrain & (TERRAIN_TREE | TERRAIN_ROCKS | TERRAIN_MEADOW | TERRAIN_FLOODPLAIN | TERRAIN_DUNE))) {
                set_empty_land_image(offset);
            }
        }
    }
    if (!keep_drawn) {
        for (int32_t y = y0; y <= y1; y++) {
            for (int32_t x = x0; x <= x1; x++) {
                int32_t offset = tile_offset(x, y);
                int32_t building_id;
                uint32_t building_terrain;
                uint32_t terrain = land_terrain(offset, &building_id, &building_terrain);
                if ((terrain & TERRAIN_NOT_CLEAR) || map_grids.images[offset] != 0 || (map_grids.random[offset] & 0xF0)) {
                    continue;
                }
                int32_t group = (map_grids.bitfields[offset] & MAP_BITFIELD_ALTERNATE_TERRAIN) ? GROUP_TERRAIN_EMPTY_LAND_ALT : GROUP_TERRAIN_EMPTY_LAND;
                map_building_tiles_add_without_corner(building_id, x, y, 1, 1, (map_grids.random[offset] & 7) + image_group(group), building_terrain);
            }
        }
        for (int32_t y = y0; y <= y1; y++) {
            for (int32_t x = x0; x <= x1; x++) {
                int32_t offset = tile_offset(x, y);
                int32_t building_id;
                uint32_t building_terrain;
                uint32_t terrain = land_terrain(offset, &building_id, &building_terrain);
                if ((terrain & TERRAIN_NOT_CLEAR) || map_grids.images[offset] != 0) {
                    continue;
                }
                int32_t group = image_group((map_grids.bitfields[offset] & MAP_BITFIELD_ALTERNATE_TERRAIN) ? GROUP_TERRAIN_EMPTY_LAND_ALT : GROUP_TERRAIN_EMPTY_LAND);
                int32_t random = map_grids.random[offset];
                if (map_building_tiles_shrub_fits(x, y, 4, 4, 0xFFFF, 1, 1)) {
                    map_building_tiles_add_without_corner(building_id, x, y, 4, 4, group + 42, building_terrain);
                } else if (map_building_tiles_shrub_fits(x, y, 3, 3, 0xFFFF, 1, 1)) {
                    map_building_tiles_add_without_corner(building_id, x, y, 3, 3, (random & 1) * 9 + 24 + group, building_terrain);
                } else if (map_building_tiles_shrub_fits(x, y, 2, 2, 0xFFFF, 1, 1)) {
                    map_building_tiles_add_without_corner(building_id, x, y, 2, 2, group + 8 + (random & 3) * 4, building_terrain);
                } else {
                    map_building_tiles_add_without_corner(building_id, x, y, 1, 1, (random & 7) + group, building_terrain);
                }
            }
        }
    }
    pyramid_complex_process_refresh();
}

void map_tiles_update_empty_land_around(int32_t x, int32_t y, int32_t size)
{
    int32_t margin = EMPTY_LAND_REDRAW_MARGIN;
    map_tiles_update_region_empty_land(x - margin, y - margin, x + margin + size, y + margin + size, 1, 0);
}

void map_tiles_update_all(void)
{
    int32_t x1 = map_info.grid_width - 1;
    int32_t y1 = map_info.grid_height - 1;
    map_tiles_update_region_faults(0, 0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_trees(0, 0, x1, y1);
    map_tiles_update_all_gardens();
    if (!editor_active) {
        map_tiles_update_entry_exit_flags(0);
    }
    map_tiles_update_region_empty_land(0, 0, x1, y1, 0, 0);
    map_tiles_update_region_floodplain(0, 0, x1, y1);
    map_tiles_update_region_meadow(0, 0, x1, y1);
    map_tiles_update_region_marshland(0, 0, x1, y1);
    map_tiles_update_all_dunes();
    map_tiles_update_region_floodplain(0, 0, x1, y1);
    map_tiles_update_region_water(0, 0, x1, y1, 0);
    map_tiles_update_region_rubble(0, 0, x1, y1);
    map_tiles_update_all_roads();
    map_tiles_update_all_plazas(0, 0, x1, y1, 1);
    map_tiles_update_all_walls();
    map_tiles_update_region_canals(0, 0, x1, y1, 0);
}
