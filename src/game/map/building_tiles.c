// The tiles of buildings on the map, and of multi-tile terrain pieces like large rocks and gardens.
//
// Writing a building's square (terrain, building id, size bits, images, the tile drawn with the whole image), the
// area and shrub fit tests, rubble, the removal of a building's tiles back to grass or water, the top-left tile of a
// building and the tile a sphinx is entered from.
//
// Kept as the game has them:
// - the shrub test reads its tiles from the 6-column square table, so an area wider than 6 tiles wraps into the next row;
// - the shrub test checks the map margin on the raw grid offset split into x and y;
// - removing a building redraws water relative to (x, y), not to the top-left tile.

#include "map/building_tiles.h"

#include "building/building.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/sun_temple.h"
#include "figure/figure_data.h"
#include "map/construction_tiles.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "widget/widget_data.h"

#include <stdint.h>

static int32_t area_inside_map(int32_t x, int32_t y, int32_t size_x, int32_t size_y)
{
    return (x >= 0) && (y >= 0) && (x + size_x - 1 < map_info.grid_width) && (y + size_y - 1 < map_info.grid_height);
}

static int32_t area_inside_edge_margin(int32_t x, int32_t y, int32_t size_x, int32_t size_y)
{
    int32_t end_x = map_info.grid_width - BUILDING_MAP_EDGE_MARGIN;
    int32_t end_y = map_info.grid_height - BUILDING_MAP_EDGE_MARGIN;
    return (x >= BUILDING_MAP_EDGE_MARGIN) && (y >= BUILDING_MAP_EDGE_MARGIN) && (x + size_x - 1 < end_x) && (y + size_y - 1 < end_y);
}

int32_t map_building_tiles_area_matches(int32_t x, int32_t y, int32_t size_x, int32_t size_y, uint32_t terrain, int32_t image_plus_one)
{
    if (!area_inside_map(x, y, size_x, size_y)) {
        return 0;
    }
    map_tile_cursor_offset = tile_offset(x, y);
    for (int32_t dy = 0; dy < size_y; dy++) {
        for (int32_t dx = 0; dx < size_x; dx++) {
            int32_t offset = map_tile_cursor_offset + dy * GRID_SIZE + dx;
            if ((map_grids.terrain[offset] & TERRAIN_NOT_CLEAR) != terrain) {
                return 0;
            }
            if (image_plus_one && map_grids.images[offset] != image_plus_one - 1) {
                return 0;
            }
        }
    }
    return 1;
}

void map_building_tiles_add(int32_t building_id, int32_t x, int32_t y, int32_t size_x, int32_t size_y, int32_t image_id, uint32_t terrain)
{
    int32_t corner_x;
    int32_t corner_y;
    switch (city_view_orientation) {
        case DIRECTION_TOP:
            corner_x = 0;
            corner_y = size_y - 1;
            break;
        case DIRECTION_RIGHT:
            corner_x = 0;
            corner_y = 0;
            break;
        case DIRECTION_BOTTOM:
            corner_x = size_x - 1;
            corner_y = 0;
            break;
        default:
            corner_x = size_x - 1;
            corner_y = size_y - 1;
            break;
    }
    if (!area_inside_map(x, y, size_x, size_y)) {
        return;
    }
    int32_t base = tile_offset(x, y);
    map_tile_cursor_offset = base;
    for (int32_t dy = 0; dy < size_y; dy++) {
        for (int32_t dx = 0; dx < size_x; dx++) {
            int32_t offset = base + dy * GRID_SIZE + dx;
            map_grids.terrain[offset] = (map_grids.terrain[offset] & TERRAIN_KEPT_BY_BUILDING) | terrain;
            map_grids.building_ids[offset] = (int16_t) building_id;
            map_grids.bitfields[offset] &= ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
            if (size_x >= 2 && size_x <= BUILDING_SQUARE_SIDE) {
                map_grids.bitfields[offset] |= (uint8_t) (size_x - 1);
            }
            map_grids.images[offset] = image_id;
            map_grids.edge[offset] = (uint8_t) ((dy * MAP_EDGE_ROW_ONE) + dx);
            if (dx == corner_x && dy == corner_y) {
                map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
            }
        }
    }
    map_tiles_update_empty_land_around(x, y, size_x);
}

void map_building_tiles_add_without_corner(int32_t building_id, int32_t x, int32_t y, int32_t size_x, int32_t size_y, int32_t image_id, uint32_t terrain)
{
    if (!area_inside_map(x, y, size_x, size_y)) {
        return;
    }
    int32_t base = tile_offset(x, y);
    map_tile_cursor_offset = base;
    int32_t image = image_id;
    for (int32_t dy = 0; dy < size_y; dy++) {
        for (int32_t dx = 0; dx < size_x; dx++) {
            int32_t offset = base + dy * GRID_SIZE + dx;
            map_grids.bitfields[offset] &= ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
            map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
            map_grids.terrain[offset] = (map_grids.terrain[offset] & TERRAIN_KEPT_BY_BUILDING) | terrain;
            map_grids.building_ids[offset] = (int16_t) building_id;
            map_grids.images[offset] = image++;
        }
    }
}

int32_t map_building_tiles_shrub_fits(int32_t x, int32_t y, int32_t size_x, int32_t size_y, uint32_t mask, int32_t image_plus_one, int32_t floodplain_ok)
{
    if (!area_inside_edge_margin(x, y, size_x, size_y)) {
        return 0;
    }
    int32_t seen_floodplain = 0;
    int32_t seen_dry = 0;
    map_tile_cursor_offset = tile_offset(x, y);
    for (int32_t dy = 0; dy < size_y; dy++) {
        for (int32_t dx = 0; dx < size_x; dx++) {
            int32_t offset = map_tile_cursor_offset + building_square_tile_offset(dy, dx);
            if (map_grid_is_inside_size(offset % GRID_SIZE, offset / GRID_SIZE, BUILDING_MAP_EDGE_MARGIN)) {
                return 0;
            }
            uint32_t terrain = map_grids.terrain[offset] & (mask | TERRAIN_MONUMENT | TERRAIN_FLOODPLAIN);
            if (!floodplain_node_dry(offset)) {
                return 0;
            }
            if (terrain & TERRAIN_FLOODPLAIN) {
                if (seen_dry) {
                    return 0;
                }
                seen_floodplain = 1;
            } else {
                if (seen_floodplain) {
                    return 0;
                }
                seen_dry = 1;
            }
            terrain &= (char) floodplain_ok ? (TERRAIN_NOT_CLEAR | TERRAIN_MONUMENT) : (TERRAIN_NOT_EMPTY | TERRAIN_MONUMENT);
            if (terrain || map_grids.figure_ids[offset] != 0) {
                return 0;
            }
            if (image_plus_one && map_grids.images[offset] != image_plus_one - 1) {
                return 0;
            }
        }
    }
    return 1;
}

static void clear_building_tile(int32_t offset)
{
    map_grids.building_ids[offset] = 0;
    map_grids.bitfields[offset] &= ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
    map_grids.building_damage[offset] = 0;
    map_grids.sprite[offset] = 0;
}

void map_building_tiles_set_rubble(int32_t building_id, int32_t x, int32_t y, int32_t size_x, int32_t size_y)
{
    if (!area_inside_map(x, y, size_x, size_y)) {
        return;
    }
    const building *b = building_get(building_id);
    for (int32_t dy = 0; dy < size_y; dy++) {
        for (int32_t dx = 0; dx < size_x; dx++) {
            int32_t offset = tile_offset(x + dx, y + dy);
            if (map_grids.building_ids[offset] != building_id) {
                continue;
            }
            if (building_id != 0 && b->type != BUILDING_BURNING_RUIN) {
                map_grids.rubble_building_type[offset] = (uint8_t) b->type;
            }
            map_grids.canals[offset] = 0;
            clear_building_tile(offset);
            map_grids.edge[offset] = MAP_EDGE_DRAW_TILE;
            uint32_t terrain = map_grids.terrain[offset];
            if (!(terrain & TERRAIN_WATER)) {
                map_grids.terrain[offset] = (terrain & TERRAIN_KEPT_BY_BUILDING) | TERRAIN_RUBBLE;
                map_grids.images[offset] = (map_grids.random[offset] & 7) + image_group(GROUP_TERRAIN_RUBBLE);
            } else {
                map_grids.terrain[offset] = TERRAIN_WATER;
                map_tiles_set_water_image(x + dx, y + dy, 1);
            }
        }
    }
}

int32_t map_building_tiles_find_top_left(int32_t x, int32_t y)
{
    int32_t offset = tile_offset(x, y);
    int32_t size_code = map_grids.bitfields[offset] & MAP_BITFIELD_SIZE;
    int32_t size = ((size_code >= 1) && (size_code < BUILDING_SQUARE_SIDE)) ? (size_code + 1) : 1;
    building_tiles_size_x = size;
    building_tiles_size_y = size;
    for (int32_t i = 0; i < size && (map_grids.edge[offset] & MAP_EDGE_COLUMN); i++) {
        offset--;
    }
    for (int32_t i = 0; i < building_tiles_size_y; i++) {
        if (!(map_grids.edge[offset] & MAP_EDGE_ROW)) {
            break;
        }
        offset -= GRID_SIZE;
    }
    return offset;
}

void map_building_tiles_remove(int32_t building_id, int32_t x, int32_t y)
{
    if (x < 0 || y < 0 || x >= map_info.grid_width || y >= map_info.grid_height) {
        return;
    }
    const building *b = building_get(building_id);
    int32_t base;
    if (b->type == BUILDING_PYRAMID) {
        base = b->grid_offset;
        building_tiles_size_x = 2;
        building_tiles_size_y = 2;
    } else {
        base = map_building_tiles_find_top_left(x, y);
    }
    if (map_grids.terrain[base] == TERRAIN_ROCK) {
        return;
    }
    if (building_is_farm(building_id)) {
        building_tiles_size_x = 3;
        building_tiles_size_y = 3;
    } else if (b->type == BUILDING_SUN_TEMPLE) {
        building_tiles_size_y = sun_temple_part_size(building_id);
        building_tiles_size_x = building_tiles_size_y;
    }
    uint32_t keep = TERRAIN_KEPT_BY_BUILDING | TERRAIN_WATER;
    if (b->type == BUILDING_ROADBLOCK) {
        keep |= map_grids.terrain[base] & (TERRAIN_CANAL | TERRAIN_ROAD);
    }
    for (int32_t dy = 0; dy < building_tiles_size_y; dy++) {
        for (int32_t dx = 0; dx < building_tiles_size_x; dx++) {
            int32_t offset = base + dy * GRID_SIZE + dx;
            if (building_id != 0) {
                if (map_grids.building_ids[offset] != building_id) {
                    continue;
                }
                if (b->type != BUILDING_BURNING_RUIN) {
                    map_grids.rubble_building_type[offset] = (uint8_t) b->type;
                }
            }
            clear_building_tile(offset);
            map_grids.canals[offset] = 0;
            map_grids.edge[offset] = (map_grids.edge[offset] & ~(MAP_EDGE_ROW | MAP_EDGE_COLUMN)) | MAP_EDGE_DRAW_TILE;
            uint32_t terrain = map_grids.terrain[offset];
            if (!(terrain & TERRAIN_WATER) || (terrain & TERRAIN_FLOODPLAIN)) {
                map_grids.images[offset] = (map_grids.random[offset] & 7) + image_group(GROUP_TERRAIN_GRASS);
                map_grids.terrain[offset] = terrain & keep;
            } else {
                map_grids.terrain[offset] = TERRAIN_WATER;
                map_tiles_set_water_image(dx + x, dy + y, 1);
            }
        }
    }
    map_tiles_update_region_empty_land(x, y, building_tiles_size_x + x, building_tiles_size_y + y, 0, 0);
    map_tiles_update_empty_land_around(x, y, building_tiles_size_x);
    map_tiles_update_region_meadow(x, y, building_tiles_size_x + x, building_tiles_size_y + y);
    map_tiles_update_region_floodplain(x, y, building_tiles_size_x + x, building_tiles_size_y + y);
    map_tiles_update_region_water(x, y, building_tiles_size_x + x, building_tiles_size_y + y, 0);
    map_tiles_update_region_marshland(x, y, building_tiles_size_x + x, building_tiles_size_y + y);
    map_tiles_update_region_rubble(x, y, building_tiles_size_x + x, building_tiles_size_y + y);
}

int32_t map_sphinx_access_tile(int32_t building_x, int32_t building_y)
{
    int32_t x = building_x + SPHINX_ACCESS_DX;
    int32_t y = building_y + SPHINX_ACCESS_DY;
    map_result_y = y;
    map_result_x = x;
    return map_info.grid_start + y * GRID_SIZE + x;
}
