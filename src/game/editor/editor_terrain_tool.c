// The scenario editor's terrain tools: one use of the tool on a tile (a special point, or the brush over its
// diamond of 1 to 41 tiles), the image updates of the region the brush touched, and the smoothing of the elevation.
// Each special point has its own terrain rule (land away from the edge, water, the animals' terrain, deep water's
// edge).

#include "editor/editor_terrain_tool.h"
#include "building/building_delete.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_new_map_tiles.h"
#include "editor/editor_terrain_tiles.h"
#include "figure/animals.h"
#include "figure/natives.h"
#include "game/game_data.h"
#include "map/building_tiles.h"
#include "map/construction_terrain.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/moisture.h"
#include "map/soil.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"
#include "scenario/scenario_climate.h"
#include <stdbool.h>
#include <stdint.h>

#define EDITOR_POINT_BLOCKING_TERRAIN (TERRAIN_NOT_CLEAR & ~TERRAIN_ROAD)
#define EDITOR_BRUSH_REDRAW_TERRAIN (TERRAIN_WATER | TERRAIN_FLOODPLAIN | TERRAIN_MARSH)

enum editor_point_limits {
    EDITOR_POINT_EDGE_DISTANCE = 4,
    EDITOR_LAND_INVASION_POINTS = 8,
};

enum editor_brush_limits {
    EDITOR_BRUSH_LARGEST = 4,
    EDITOR_ELEVATION_MAX = 5,
    EDITOR_SMOOTH_PASSES = 4,
};

static int32_t brush_size_clamped(void)
{
    int32_t size = editor_brush_size;
    return (size >= 0 && size < EDITOR_BRUSH_LARGEST) ? size : EDITOR_BRUSH_LARGEST;
}

static int32_t brush_tile_count(void)
{
    if (editor_tool_kind == EDITOR_TOOL_ROAD || construction_building_type == BUILDING_EDITOR_ACCESS_RAMP) {
        return 1;
    }
    int32_t radius = brush_size_clamped();
    return (2 * radius * (radius + 1)) + 1;
}

static int32_t brush_update_radius(void)
{
    if (editor_tool_kind == EDITOR_TOOL_ROAD) {
        return 1;
    }
    return brush_size_clamped() + 2;
}

static int32_t moisture_reach(void)
{
    if (scenario_data.climate_type == CLIMATE_CENTRAL) {
        return MOISTURE_REACH_CENTRAL;
    }
    if (scenario_data.climate_type == CLIMATE_DESERT) {
        return MOISTURE_REACH_DESERT;
    }
    return MOISTURE_REACH_NORTHERN;
}

static int32_t point_away_from_edge(int32_t x, int32_t y)
{
    return map_grid_is_inside_size(map_info.diamond_x_min + x, map_info.diamond_y_min + y, EDITOR_POINT_EDGE_DISTANCE);
}

static void brush_set_tile(int32_t type, int32_t x, int32_t y)
{
    switch (type) {
        case BUILDING_EDITOR_GRASS:
            editor_tile_clear(x, y);
            break;
        case BUILDING_EDITOR_TREES:
            editor_tile_set_tree(x, y);
            break;
        case BUILDING_EDITOR_WATER:
            editor_tile_set_water(x, y);
            break;
        case BUILDING_ORDINARY_ROCK:
            editor_tile_set_rock_kind(x, y, TERRAIN_ROCK);
            break;
        case BUILDING_CLIFF:
            editor_tile_set_rock_kind(x, y, TERRAIN_ROCK_NEAR_MONUMENT | TERRAIN_ROCK);
            break;
        case BUILDING_ORE_BEARING_ROCK:
            editor_tile_set_rock_kind(x, y, TERRAIN_ORE | TERRAIN_ROCK);
            break;
        case BUILDING_EDITOR_GEM_ROCK:
            editor_tile_set_rock_kind(x, y, TERRAIN_GEM_ROCK | TERRAIN_ROCK);
            break;
        case BUILDING_EDITOR_FINISHING_ROCK:
            editor_tile_set_rock_kind(x, y, TERRAIN_GEM_ROCK | TERRAIN_ORE | TERRAIN_ROCK);
            break;
        case BUILDING_SAND_DUNES:
            editor_tile_set_dune(x, y);
            break;
        case BUILDING_EDITOR_MEADOW:
            editor_tile_set_meadow(x, y);
            break;
        case BUILDING_MARSHLAND:
            editor_tile_set_marsh(x, y);
            break;
        case BUILDING_EDITOR_FLOODPLAIN:
            editor_tile_set_floodplain(x, y);
            break;
        case BUILDING_EDITOR_CANAL:
            editor_tile_set_canal(x, y);
            break;
        case BUILDING_EDITOR_ROAD:
            editor_tile_set_road(x, y);
            break;
        case BUILDING_EDITOR_RAISE_LAND:
            editor_tile_raise(x, y);
            break;
        case BUILDING_EDITOR_LOWER_LAND:
            editor_tile_lower(x, y);
            break;
        default:
            break;
    }
}

static bool brush_fix_tile(uint32_t terrain, int32_t x, int32_t y)
{
    if (terrain & TERRAIN_FLOODPLAIN) {
        return editor_floodplain_spread(x, y);
    }
    if (terrain & EDITOR_FLOODPLAIN_BLOCKING_TERRAIN) {
        return editor_floodplain_shrink(x, y);
    }
    if (terrain & TERRAIN_WATER) {
        return false;
    }
    return editor_floodplain_fill(x, y);
}

static int32_t set_special_point(int32_t x, int32_t y)
{
    uint32_t blocking = map_grids.terrain[map_tile_cursor_offset] & EDITOR_POINT_BLOCKING_TERRAIN;
    switch (construction_building_type) {
        case BUILDING_EDITOR_EARTHQUAKE:
            if (blocking == 0) {
                scenario_data.earthquake_y = (int16_t) y;
                scenario_data.earthquake_x = (int16_t) x;
            }
            return 1;
        case BUILDING_EDITOR_ENTRY_POINT:
            if (blocking == 0 && point_away_from_edge(x, y)) {
                scenario_data.entry_point_y = (int16_t) y;
                scenario_data.entry_point_x = (int16_t) x;
            }
            return 1;
        case BUILDING_EDITOR_EXIT_POINT:
            if (blocking == 0 && point_away_from_edge(x, y)) {
                scenario_data.exit_point_y = (int16_t) y;
                scenario_data.exit_point_x = (int16_t) x;
            }
            return 1;
        case BUILDING_EDITOR_INVASION_POINT: {
            int32_t number = editor_invasion_point_number;
            int32_t allowed = (number > EDITOR_LAND_INVASION_POINTS) ? (blocking & TERRAIN_WATER) != 0 : blocking == 0;
            if (allowed && point_away_from_edge(x, y)) {
                scenario_data.invasion_point_x[number - 1] = (int16_t) x;
                scenario_data.invasion_point_y[number - 1] = (int16_t) y;
            }
            return 1;
        }
        case BUILDING_EDITOR_FISHING_WATERS:
            if (blocking & TERRAIN_WATER) {
                int32_t number = editor_fishing_point_number;
                scenario_data.fishing_point_x[number - 1] = (int16_t) x;
                scenario_data.fishing_point_y[number - 1] = (int16_t) y;
            }
            return 1;
        case BUILDING_EDITOR_KILLER_POINT:
            if (animal_terrain_allowed_climate(map_grids.terrain[map_info.hover_offset], 0)) {
                int32_t number = editor_herd_point_number;
                scenario_data.predator_herd_x[number - 1] = (int16_t) x;
                scenario_data.predator_herd_y[number - 1] = (int16_t) y;
            }
            return 1;
        case BUILDING_EDITOR_PREY:
            if (animal_terrain_allowed_climate(map_grids.terrain[map_info.hover_offset], 1)) {
                int32_t number = editor_herd_point_number;
                scenario_data.prey_herd_x[number - 1] = x;
                scenario_data.prey_herd_y[number - 1] = y;
            }
            return 1;
        case BUILDING_DISEMBARK_POINT:
            if ((blocking & TERRAIN_WATER) && editor_tile_deep_water_edge(x, y)) {
                int32_t number = editor_landing_point_number;
                scenario_data.invasion_landing_x[number - 1] = x;
                scenario_data.invasion_landing_y[number - 1] = y;
            }
            return 1;
        case BUILDING_EDITOR_RIVER_IN:
            if (point_away_from_edge(x, y) && map_terrain_all_adjacent_are_water(map_tile_cursor_offset)) {
                scenario_data.river_entry_x = (int16_t) x;
                scenario_data.river_entry_y = (int16_t) y;
                figure_create_flotsam((int16_t) x, (int16_t) y, 1);
            }
            return 1;
        case BUILDING_EDITOR_RIVER_OUT:
            if (point_away_from_edge(x, y) && map_terrain_all_adjacent_are_water(map_tile_cursor_offset)) {
                scenario_data.river_exit_y = (int16_t) y;
                scenario_data.river_exit_x = (int16_t) x;
            }
            return 1;
        default:
            return 0;
    }
}

static bool brush_paint(int32_t x, int32_t y)
{
    int32_t count = brush_tile_count();
    bool changed = false;
    for (int32_t i = 0; i < count; i++) {
        int32_t tx = editor_brush_tiles[i * 2] + x;
        int32_t ty = editor_brush_tiles[(i * 2) + 1] + y;
        if (tx < 0 || tx >= map_info.grid_width || ty < 0 || ty >= map_info.grid_height) {
            continue;
        }
        int32_t tile = map_grid_offset(tx, ty);
        int32_t building_id = (int16_t) map_grids.building_ids[tile];
        if (building_id != 0) {
            map_building_tiles_remove(building_id, tx, ty);
            building_delete(building_id);
        }
        uint32_t before = map_grids.terrain[tile];
        brush_set_tile(construction_building_type, tx, ty);
        changed |= brush_fix_tile(map_grids.terrain[tile], tx, ty);
        if ((before ^ map_grids.terrain[tile]) & EDITOR_BRUSH_REDRAW_TERRAIN) {
            changed |= 1;
        }
    }
    return changed;
}

static void brush_update_region(int32_t x, int32_t y)
{
    int32_t radius = brush_update_radius();
    int32_t x0 = x - radius;
    int32_t y0 = y - radius;
    int32_t x1 = x + radius;
    int32_t y1 = y + radius;
    switch (construction_building_type) {
        case BUILDING_EDITOR_GRASS:
            editor_region_clear(x0, y0, x1, y1);
            break;
        case BUILDING_EDITOR_TREES:
            editor_region_trees(x0, y0, x1, y1);
            break;
        case BUILDING_EDITOR_WATER:
            editor_region_water(x0, y0, x1, y1);
            break;
        case BUILDING_PLATEAU:
            editor_region_shrub(x0, y0, x1, y1);
            break;
        case BUILDING_ORDINARY_ROCK:
        case BUILDING_CLIFF:
        case BUILDING_ORE_BEARING_ROCK:
        case BUILDING_EDITOR_GEM_ROCK:
        case BUILDING_EDITOR_FINISHING_ROCK:
            editor_region_rock_kind(x0, y0, x1, y1);
            break;
        case BUILDING_SAND_DUNES:
            editor_region_dune(x0, y0, x1, y1);
            break;
        case BUILDING_MARSHLAND:
            editor_region_marsh(x0, y0, x1, y1);
            break;
        case BUILDING_EDITOR_MEADOW:
            map_soil_fertility_update_rect(x0 - 1, y0 - 1, x1 + 1, y1 + 1, 1);
            editor_region_meadow(x0 - 4, y0 - 4, x1 + 4, y1 + 4);
            break;
        case BUILDING_EDITOR_FLOODPLAIN:
            editor_region_floodplain(x0, y0, x1, y1);
            break;
        case BUILDING_EDITOR_CANAL:
            editor_region_canal(x0, y0, x1, y1);
            break;
        case BUILDING_EDITOR_ROAD:
            editor_region_road(x0, y0, x1, y1);
            break;
        case BUILDING_EDITOR_RAISE_LAND:
        case BUILDING_EDITOR_LOWER_LAND:
        case BUILDING_EDITOR_ACCESS_RAMP:
            editor_region_elevation(x0, y0, x1, y1);
            break;
        default:
            break;
    }
}

void editor_tool_update_use(int32_t x, int32_t y)
{
    map_tile_cursor_offset = map_grid_offset(x, y);
    if (set_special_point(x, y)) {
        return;
    }
    if (!brush_paint(x, y)) {
        brush_update_region(x, y);
        return;
    }
    if (key_ctrl != 0) {
        int32_t radius = moisture_reach() + 1;
        map_moisture_update_area(x - radius, y - radius, x + radius, y + radius);
        editor_update_tiles_around(x, y, radius);
    } else {
        map_pyramid_redraw_pending = 1;
    }
}

static void reset_contexts(void)
{
    map_image_context_reset(context_water, 48);
    map_image_context_reset(context_water_alt, 48);
    map_image_context_reset(context_floodsystem, 24);
    map_image_context_reset(context_fault, 17);
}

static void update_ground(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    map_tiles_update_region_water(x0, y0, x1, y1, 1);
    map_tiles_update_region_marshland(x0, y0, x1, y1);
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_faults(x0, y0, x1, y1);
}

void editor_region_clear(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    map_tiles_update_region_water(x0, y0, x1, y1, 1);
    map_tiles_update_region_marshland(x0, y0, x1, y1);
    map_tiles_update_region_faults(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_region_empty_land(x0, y0, x1, y1, 0, 0);
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_roads(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    map_tiles_update_region_canal_images(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    map_tiles_update_region_meadow(x0, y0, x1, y1);
}

void editor_region_trees(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_trees(x0, y0, x1, y1);
}

void editor_region_water(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_roads(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    map_tiles_update_region_canal_images(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
}

void editor_region_shrub(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
}

void editor_region_rock_kind(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
}

void editor_region_dune(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_all_dunes();
}

void editor_region_meadow(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_region_meadow(x0, y0, x1, y1);
}

void editor_region_marsh(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    map_tiles_update_region_water(x0, y0, x1, y1, 1);
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_faults(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_region_marshland(x0, y0, x1, y1);
}

void editor_region_floodplain(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    map_tiles_update_region_empty_land(x0, y0, x1, y1, 0, 0);
    update_ground(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_region_roads(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    map_tiles_update_region_canal_images(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
}

void editor_region_road(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    update_ground(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_all_roads();
}

void editor_region_canal(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    reset_contexts();
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_water(x0 - 1, y0 - 1, x1 + 1, y1 + 1, 1);
    map_tiles_update_region_marshland(x0, y0, x1, y1);
    map_tiles_update_region_faults(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_all_canals();
}

void editor_region_elevation(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    map_image_context_reset(context_water, 48);
    map_image_context_reset(context_water_alt, 48);
    map_image_context_reset(context_floodsystem, 24);
    map_image_context_reset(context_aqueduct, 14);
    map_image_context_reset(context_fault, 17);
    map_tiles_update_region_water(x0, y0, x1, y1, 1);
    map_tiles_update_region_marshland(x0, y0, x1, y1);
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_trees(x0, y0, x1, y1);
    map_tiles_update_region_meadow(x0, y0, x1, y1);
    map_tiles_update_region_faults(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_empty_land(x0, y0, x1, y1, 0, 0);
}

int32_t editor_tile_raise(int32_t x, int32_t y)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    uint8_t elevation = map_grids.elevation[offset];
    if (elevation >= EDITOR_ELEVATION_MAX || elevation != construction_start_elevation) {
        return offset;
    }
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & (TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
        return offset;
    }
    map_grids.bitfields[offset] &= (uint8_t) ~MAP_BITFIELD_SIZE;
    map_grids.terrain[offset] = terrain & ~(TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_DEEPWATER);
    map_grids.elevation[offset] = (uint8_t) (elevation + 1);
    return offset;
}

int32_t editor_tile_lower(int32_t x, int32_t y)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & TERRAIN_ACCESS_RAMP) {
        map_grids.terrain[offset] = (terrain & ~TERRAIN_ACCESS_RAMP) | TERRAIN_ELEVATION;
        map_grids.bitfields[offset] &= (uint8_t) ~MAP_BITFIELD_SIZE;
        map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
    }
    uint8_t elevation = map_grids.elevation[offset];
    if (elevation == 0) {
        map_grids.terrain[offset] &= ~TERRAIN_ELEVATION;
        return offset;
    }
    if (elevation == construction_start_elevation) {
        map_grids.terrain[offset] &= ~(TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP);
        map_grids.elevation[offset] = (uint8_t) (elevation - 1);
    }
    return offset;
}

static int32_t neighbour_is_lower(int32_t neighbour, int32_t level)
{
    return map_grids.elevation[neighbour] < level && !map_grid_offset_is_outside(neighbour);
}

void editor_elevation_smooth_pass(void)
{
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        int32_t x_start = map_row_x_start[y];
        int32_t x_end = map_row_x_end[y];
        int32_t offset = map_row_start_offset[y];
        map_tile_cursor_offset = offset;
        if (x_start >= x_end) {
            continue;
        }
        for (int32_t n = x_end - x_start; n > 0; n--) {
            uint8_t elevation = map_grids.elevation[offset];
            if (elevation > 0) {
                int32_t lower = elevation - 1;
                if (neighbour_is_lower(offset - GRID_SIZE, lower) || neighbour_is_lower(offset + 1, lower) || neighbour_is_lower(offset + GRID_SIZE, lower) ||
                    neighbour_is_lower(offset - 1, lower)) {
                    map_grids.elevation[offset]--;
                }
            }
            offset++;
            map_tile_cursor_offset = offset;
        }
    }
}

void editor_elevation_smooth_all(void)
{
    for (int32_t pass = 0; pass < EDITOR_SMOOTH_PASSES; pass++) {
        editor_elevation_smooth_pass();
    }
    map_tiles_update_all_rocks();
    map_tiles_update_region_empty_land(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0, 0);
    map_tiles_update_region_meadow(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
    map_tiles_update_region_floodplain(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
}
