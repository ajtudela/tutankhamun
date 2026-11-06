// Rebuilds of the routing grids from the terrain: water, walls, non-citizen land and citizen land, for the whole map
// or a rectangle, with the ferry lanes laid again after a full citizen rebuild (staffed or not).
//
// A tile with the building bit but no building is reset to grass; the gatehouse value then goes into the non-citizen grid
// while the tile's citizen value stays as it was. A ferry lane's path clears only its first
// FERRY_ROUTE_DIRECTIONS_CLEARED directions before the walk-back writes it.

#include "map/routing_terrain.h"

#include "building/building.h"
#include "building/building_type.h"
#include "core/calc.h"
#include "figure/boats.h"
#include "figure/figure_data.h"
#include "graphics/image_collection.h"
#include "io/io_data.h"
#include "map/bridge.h"
#include "map/ferry.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_citizen.h"
#include "map/routing_water.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/water.h"
#include "widget/widget_data.h"

#include <string.h>

static int32_t is_monument_or_blocking_for_noncitizen(int32_t type)
{
    switch (type) {
        case BUILDING_BURNING_RUIN:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_NATIVE_CROPS:
        case BUILDING_PYRAMID:
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
        case BUILDING_SPHINX:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_PYRAMID_ENTRY_TILE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_CAESAREUM:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_ABU_SIMBEL:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
            return 1;
        default:
            return 0;
    }
}

static void clamp_rect(int32_t *x0, int32_t *y0, int32_t *x1, int32_t *y1)
{
    if (*x0 < 0) {
        *x0 = 0;
    }
    if (*y0 < 0) {
        *y0 = 0;
    }
    if (*x1 >= map_info.grid_width) {
        *x1 = map_info.grid_width - 1;
    }
    if (*y1 >= map_info.grid_height) {
        *y1 = map_info.grid_height - 1;
    }
}

static int32_t next_row_offset(int32_t offset_after_row, int32_t x0, int32_t x1)
{
    return offset_after_row + map_info.grid_border_size + (map_info.grid_width - x1) - 1 + x0;
}

static int8_t water_value(int32_t offset, int32_t x, int32_t y)
{
    const uint32_t *terrain = map_grids.terrain;
    if (!(terrain[offset] & TERRAIN_WATER)) {
        return ROUTING_WATER_LAND;
    }
    if (terrain[offset] & TERRAIN_DIKE) {
        return ROUTING_WATER_BLOCKED;
    }
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        if (!(terrain[offset + map_grid_direction_delta[i]] & TERRAIN_WATER)) {
            return ROUTING_WATER_BLOCKED;
        }
    }
    if (y < 1 || x < 1 || y > map_info.grid_height - 2 || x > map_info.grid_width - 2) {
        return ROUTING_WATER_MAP_EDGE;
    }
    switch (map_grids.sprite[offset]) {
        case BRIDGE_SPRITE_SHIP_SECOND_TILE:
            return ROUTING_WATER_BLOCKED;
        case BRIDGE_SPRITE_LOW_SPAN_ALONG_Y:
        case BRIDGE_SPRITE_LOW_SPAN_ALONG_X:
            return ROUTING_WATER_LOW_BRIDGE;
        default:
            return ROUTING_WATER_OPEN;
    }
}

void map_routing_update_water_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    clamp_rect(&x0, &y0, &x1, &y1);
    int32_t offset = map_grid_offset(x0, y0);
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            routing_grids.water[offset] = water_value(offset, x, y);
        }
        offset = next_row_offset(offset, x0, x1);
    }
}

void map_routing_update_water(void)
{
    memset(routing_grids.water, ROUTING_WATER_BLOCKED, GRID_CELLS);
    map_routing_update_water_rect(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
}

static int8_t wall_value(int32_t offset)
{
    const uint32_t *terrain = map_grids.terrain;
    if (!(terrain[offset] & TERRAIN_WALL)) {
        return (terrain[offset] & TERRAIN_GATEHOUSE) ? ROUTING_WALL_PASSABLE : ROUTING_WALL_BLOCKED;
    }
    int32_t row_delta, diagonal_delta, column_delta;
    switch (city_view_orientation) {
        case DIRECTION_TOP:
            row_delta = GRID_SIZE;
            diagonal_delta = GRID_SIZE + 1;
            column_delta = 1;
            break;
        case DIRECTION_RIGHT:
            row_delta = GRID_SIZE;
            diagonal_delta = GRID_SIZE - 1;
            column_delta = -1;
            break;
        case DIRECTION_BOTTOM:
            row_delta = -GRID_SIZE;
            diagonal_delta = -GRID_SIZE - 1;
            column_delta = -1;
            break;
        case DIRECTION_LEFT:
            row_delta = -GRID_SIZE;
            diagonal_delta = -GRID_SIZE + 1;
            column_delta = 1;
            break;
        default:
            return ROUTING_WALL_BLOCKED;
    }
    int32_t count = 0;
    const uint32_t wall_or_gate = TERRAIN_WALL | TERRAIN_GATEHOUSE;
    count += (terrain[offset + row_delta] & wall_or_gate) != 0;
    count += (terrain[offset + diagonal_delta] & wall_or_gate) != 0;
    count += (terrain[offset + column_delta] & wall_or_gate) != 0;
    return (count == 3) ? ROUTING_WALL_PASSABLE : ROUTING_WALL_BLOCKED;
}

void map_routing_update_walls(void)
{
    memset(routing_grids.walls, ROUTING_WALL_BLOCKED, GRID_CELLS);
    int32_t offset = map_info.grid_start;
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        for (int32_t x = 0; x < map_info.grid_width; x++, offset++) {
            routing_grids.walls[offset] = wall_value(offset);
        }
        offset += map_info.grid_border_size;
    }
}

static int8_t noncitizen_building_value(int32_t building_id)
{
    int32_t type = building_get(building_id)->type;
    if (type == BUILDING_STORAGE_YARD) {
        return ROUTING_NONCITIZEN_BUILDING;
    }
    if (type == BUILDING_GATEHOUSE_UP) {
        return ROUTING_NONCITIZEN_GATEHOUSE;
    }
    if (is_monument_or_blocking_for_noncitizen(type)) {
        return ROUTING_NONCITIZEN_BLOCKED;
    }
    if (type == BUILDING_FORT) {
        return ROUTING_NONCITIZEN_FORT;
    }
    if (type == BUILDING_FORT_GROUND) {
        return ROUTING_NONCITIZEN_PASSABLE;
    }
    if (type == BUILDING_FERRY_LANDING) {
        return ROUTING_NONCITIZEN_BLOCKED;
    }
    return ROUTING_NONCITIZEN_BUILDING;
}

static int8_t noncitizen_value(int32_t offset)
{
    uint32_t terrain = map_grids.terrain[offset];
    if ((terrain & TERRAIN_DIKE) || (terrain & (TERRAIN_FLOODPLAIN | TERRAIN_WATER)) == (TERRAIN_FLOODPLAIN | TERRAIN_WATER)) {
        return ROUTING_NONCITIZEN_BLOCKED;
    }
    if (terrain & TERRAIN_GATEHOUSE) {
        return ROUTING_NONCITIZEN_GATEHOUSE;
    }
    if (terrain & (TERRAIN_ROAD | TERRAIN_WALKABLE_BUILDING)) {
        return ROUTING_NONCITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_GARDEN) {
        return ROUTING_NONCITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_ACCESS_RAMP) {
        return ROUTING_NONCITIZEN_ACCESS_RAMP;
    }
    if (terrain & TERRAIN_BUILDING) {
        return noncitizen_building_value(map_grids.building_ids[offset]);
    }
    if (terrain & TERRAIN_CANAL) {
        return ROUTING_NONCITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_WALL) {
        return ROUTING_NONCITIZEN_WALL;
    }
    if (terrain & (TERRAIN_TREE | TERRAIN_SHRUB | TERRAIN_MARSH | TERRAIN_DUNE)) {
        return ROUTING_NONCITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_RUBBLE) {
        return ROUTING_NONCITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_NOT_CLEAR) {
        return ROUTING_NONCITIZEN_BLOCKED;
    }
    return ROUTING_NONCITIZEN_PASSABLE;
}

void map_routing_update_land_noncitizen_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    clamp_rect(&x0, &y0, &x1, &y1);
    int32_t offset = map_grid_offset(x0, y0);
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            routing_grids.land_noncitizen[offset] = noncitizen_value(offset);
        }
        offset = next_row_offset(offset, x0, x1);
    }
}

void map_routing_update_land_noncitizen(void)
{
    memset(routing_grids.land_noncitizen, ROUTING_NONCITIZEN_BLOCKED, GRID_CELLS);
    map_routing_update_land_noncitizen_rect(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
}

static int8_t water_lift_value(const building *b)
{
    int32_t view_relative_orientation = b->data.oriented.orientation - (map_info.orientation / 2);
    return (view_relative_orientation & 1) ? ROUTING_CITIZEN_WATER_LIFT_ODD : ROUTING_CITIZEN_WATER_LIFT_EVEN;
}

static void reset_building_tile_without_building(int32_t offset, uint32_t terrain, int32_t grass_base)
{
    map_grids.terrain[offset] = terrain & ~TERRAIN_BUILDING;
    routing_grids.land_noncitizen[offset] = ROUTING_NONCITIZEN_GATEHOUSE;
    map_grids.images[offset] = (map_grids.random[offset] & (ALTERNATE_GRASS_IMAGES - 1)) + grass_base;
    map_grids.edge[offset] = MAP_EDGE_DRAW_TILE;
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_SIZE;
    buildings_without_id_cleared++;
}

static int8_t citizen_building_value(int32_t offset, uint32_t terrain, int32_t grass_base)
{
    int32_t building_id = map_grids.building_ids[offset];
    if (building_id == 0) {
        reset_building_tile_without_building(offset, terrain, grass_base);
        return routing_grids.land_citizen[offset];
    }
    const building *b = building_get(building_id);
    switch (b->type) {
        case BUILDING_ROADBLOCK:
        case BUILDING_PYRAMID_ENTRY_TILE:
        case BUILDING_FERRY_LANDING:
            return ROUTING_CITIZEN_ROAD;
        case BUILDING_FORT_GROUND:
            return ROUTING_CITIZEN_FORT_GROUND;
        case BUILDING_FESTIVAL_SQUARE:
            return ROUTING_CITIZEN_CLEAR;
        case BUILDING_WATER_LIFT:
            return water_lift_value(b);
        default:
            return ROUTING_CITIZEN_BLOCKED;
    }
}

static int32_t is_straight_canal_image(int32_t image)
{
    return image == CANAL_IMAGE_STRAIGHT_A || image == CANAL_IMAGE_STRAIGHT_B || image == CANAL_IMAGE_STRAIGHT_C || image == CANAL_IMAGE_STRAIGHT_D;
}

static int8_t citizen_value(int32_t offset, int32_t grass_base)
{
    uint32_t terrain = map_grids.terrain[offset];
    if ((terrain & TERRAIN_DIKE) || (terrain & (TERRAIN_FLOODPLAIN | TERRAIN_WATER)) == (TERRAIN_FLOODPLAIN | TERRAIN_WATER)) {
        return ROUTING_CITIZEN_BLOCKED;
    }
    if (terrain & (TERRAIN_ROAD | TERRAIN_WALKABLE_BUILDING)) {
        return ROUTING_CITIZEN_ROAD;
    }
    if (terrain & TERRAIN_ACCESS_RAMP) {
        return ROUTING_CITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_MONUMENT) {
        return ROUTING_CITIZEN_MONUMENT;
    }
    if (terrain & (TERRAIN_BUILDING | TERRAIN_GATEHOUSE)) {
        return citizen_building_value(offset, terrain, grass_base);
    }
    if (terrain & TERRAIN_CANAL) {
        return ROUTING_CITIZEN_CLEAR;
    }
    const flood_tile *object = floods_data.tiles[offset];
    if (object && object->is_floodplain == 0) {
        int32_t image = map_grids.images[offset] - image_group(IMAGE_GROUP_CANAL_ROAD);
        if (image < CANAL_IMAGE_STRAIGHT_A) {
            return ROUTING_CITIZEN_CLEAR;
        }
        return is_straight_canal_image(image) ? ROUTING_CITIZEN_CANAL_ON_FLOODPLAIN : ROUTING_CITIZEN_BLOCKED;
    }
    if (terrain & TERRAIN_FLOODPLAIN) {
        return ROUTING_CITIZEN_CLEAR;
    }
    if (terrain & (TERRAIN_TREE | TERRAIN_SHRUB | TERRAIN_MARSH | TERRAIN_DUNE)) {
        return ROUTING_CITIZEN_CLEAR;
    }
    if (terrain & TERRAIN_RUBBLE) {
        return ROUTING_CITIZEN_PASSABLE;
    }
    if (terrain & TERRAIN_NOT_EMPTY) {
        return ROUTING_CITIZEN_BLOCKED;
    }
    return ROUTING_CITIZEN_CLEAR;
}

void map_routing_set_ferry_route(int32_t building_id, bool enable, int32_t type)
{
    building *landing = building_get(building_id);
    int32_t other_id = landing->linked_building_id;
    if (other_id <= 0) {
        return;
    }
    int32_t x = landing->x;
    int32_t y = landing->y;
    map_water_orientation_match(x, y);
    int32_t directions[FERRY_ROUTE_DIRECTIONS];
    for (int32_t i = 0; i < FERRY_ROUTE_DIRECTIONS_CLEARED; i++) {
        directions[i] = -1;
    }
    const building *other = building_get(other_id);
    int32_t other_x = other->x;
    int32_t other_y = other->y;
    int32_t size = landing->size;
    int32_t saved_x = map_result_x;
    int32_t saved_y = map_result_y;
    map_water_tile_next_to(x, y, size);
    int32_t src_x = map_result_x;
    int32_t src_y = map_result_y;
    map_water_tile_next_to(other_x, other_y, size);
    int32_t dst_x = map_result_x;
    int32_t dst_y = map_result_y;
    map_routing_calculate_distances_water_ferry(src_x, src_y, dst_x, dst_y);
    int32_t length = map_routing_get_path_on_water_4dir(FERRY_ROUTE_PATH_ID, src_x, src_y, dst_x, dst_y, 0, directions);
    map_result_x = saved_x;
    map_result_y = saved_y;
    if (!length) {
        return;
    }
    int32_t row = map_info.grid_start + (src_y * GRID_SIZE);
    int32_t col = src_x;
    for (int32_t i = -1; i < length; i++) {
        if (i >= 0) {
            switch (directions[i]) {
                case DIRECTION_TOP:
                    row -= GRID_SIZE;
                    break;
                case DIRECTION_RIGHT:
                    col++;
                    break;
                case DIRECTION_BOTTOM:
                    row += GRID_SIZE;
                    break;
                default:
                    col--;
                    break;
            }
        }
        int32_t grid_offset = row + col;
        if (enable) {
            map_grids.terrain[grid_offset] |= TERRAIN_ROAD;
            routing_grids.land_citizen[grid_offset] = (int8_t) (type == FERRY_ROUTE_OPEN ? ROUTING_CITIZEN_FERRY_STAFFED : ROUTING_CITIZEN_FERRY_UNSTAFFED);
        } else {
            map_grids.terrain[grid_offset] &= ~TERRAIN_ROAD;
            routing_grids.land_citizen[grid_offset] = ROUTING_CITIZEN_BLOCKED;
        }
    }
}

static void update_ferry_routes(void)
{
    for (int32_t id = 0; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->type != BUILDING_FERRY_LANDING || (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_CREATED)) {
            continue;
        }
        if (b->num_workers == 0 || building_get(b->linked_building_id)->num_workers < 1) {
            map_routing_set_ferry_route(id, 1, ROUTING_CITIZEN_FERRY_UNSTAFFED);
            ferry_queues_release(id);
        } else {
            map_routing_set_ferry_route(id, 1, ROUTING_CITIZEN_FERRY_STAFFED);
        }
    }
}

static int32_t is_ferry_road_on_open_water(int32_t offset)
{
    uint32_t water_road_or_floodplain = map_grids.terrain[offset] & (TERRAIN_FLOODPLAIN | TERRAIN_ROAD | TERRAIN_WATER);
    return water_road_or_floodplain == (TERRAIN_ROAD | TERRAIN_WATER) && map_grids.sprite[offset] == 0;
}

void map_routing_update_land_citizen_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t rebuild_ferries)
{
    clamp_rect(&x0, &y0, &x1, &y1);
    int32_t offset = map_grid_offset(x0, y0);
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            if ((char) rebuild_ferries && is_ferry_road_on_open_water(offset)) {
                map_grids.terrain[offset] &= ~TERRAIN_ROAD;
            }
            routing_grids.land_citizen[offset] = citizen_value(offset, image_group(IMAGE_GROUP_GRASS));
        }
        offset += x0 - x1 + (GRID_SIZE - 1);
    }
    if ((char) rebuild_ferries) {
        update_ferry_routes();
    }
}

void map_routing_update_land_citizen(void)
{
    memset(routing_grids.land_citizen, ROUTING_CITIZEN_BLOCKED, GRID_CELLS);
    map_routing_update_land_citizen_rect(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 1);
}
