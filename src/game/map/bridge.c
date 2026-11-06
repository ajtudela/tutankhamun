// Bridges and ferry routes over water.
//
// The length and direction of a bridge measured from a shore water tile before the player drags it, the water route
// in front of a ferry landing, the removal (or deletion marking) of a bridge and the count of figures standing on it.
// The results go to the construction globals the bridge and ferry ghosts read.
//
// A ferry route that reaches land is given the maximum length, whatever its real length.

#include "map/bridge.h"

#include "building/building_data.h"
#include "map/construction_terrain.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/water.h"
#include "widget/widget_data.h"

#include <stdint.h>

static int32_t is_shore_water(int32_t offset)
{
    return map_terrain_count_directly_adjacent_with(offset, TERRAIN_WATER) == 3 && map_terrain_count_directly_adjacent_with(offset, TERRAIN_FLOODPLAIN) == 0;
}

static int32_t has_ferry_lane_around(int32_t offset)
{
    for (int32_t direction = 0; direction < DIRECTION_COUNT; direction++) {
        uint8_t route = routing_grids.land_citizen[offset + map_grid_direction_delta[direction]];
        if ((route == ROUTING_CITIZEN_FERRY_STAFFED) || (route == ROUTING_CITIZEN_FERRY_UNSTAFFED)) {
            return 1;
        }
    }
    return 0;
}

void map_bridge_calculate_length_direction(int32_t x, int32_t y)
{
    bridge_end_grid_offset = 0;
    construction_bridge_length = 0;
    construction_bridge_step = 0;
    bridge_direction = 0;
    int32_t offset = map_grid_offset(x, y);
    uint32_t terrain = map_grids.terrain[offset];
    if (!(terrain & TERRAIN_WATER) || (terrain & (TERRAIN_ROAD | TERRAIN_BUILDING))) {
        return;
    }
    if (!is_shore_water(offset)) {
        return;
    }
    if (!((uint8_t) map_grids.terrain[offset - GRID_SIZE] & TERRAIN_WATER)) {
        bridge_direction = DIRECTION_BOTTOM;
        construction_bridge_step = GRID_SIZE;
    } else if (!((uint8_t) map_grids.terrain[offset + 1] & TERRAIN_WATER)) {
        bridge_direction = DIRECTION_LEFT;
        construction_bridge_step = -1;
    } else if (!((uint8_t) map_grids.terrain[offset + GRID_SIZE] & TERRAIN_WATER)) {
        bridge_direction = DIRECTION_TOP;
        construction_bridge_step = -GRID_SIZE;
    } else if (!((uint8_t) map_grids.terrain[offset - 1] & TERRAIN_WATER)) {
        bridge_direction = DIRECTION_RIGHT;
        construction_bridge_step = 1;
    } else {
        return;
    }
    construction_bridge_length = 1;
    int32_t middle = 0;
    while (1) {
        int32_t step = construction_bridge_step;
        offset += step;
        construction_bridge_length++;
        uint32_t ahead = map_grids.terrain[offset + step];
        if (ahead & TERRAIN_TREE) {
            return;
        }
        uint32_t here = map_grids.terrain[offset];
        if ((here & TERRAIN_DEEPWATER) && (here & TERRAIN_ROAD)) {
            return;
        }
        if (has_ferry_lane_around(offset)) {
            return;
        }
        if (!(ahead & TERRAIN_WATER)) {
            break;
        }
        if (ahead & (TERRAIN_ROAD | TERRAIN_BUILDING)) {
            return;
        }
        if (map_terrain_count_diagonally_adjacent_with(offset, TERRAIN_WATER) != 4) {
            return;
        }
        middle++;
        if (middle >= BRIDGE_MAX_OPEN_WATER_TILES) {
            return;
        }
    }
    bridge_end_grid_offset = offset;
    if (!is_shore_water(offset)) {
        bridge_end_grid_offset = 0;
    }
}

void map_ferry_route_measure(int32_t x, int32_t y)
{
    ferry_route_length = 0;
    ferry_route_direction = 0;
    int32_t offset = map_grid_offset(x, y);
    int32_t direction = map_water_orientation_match(x, y) * 2;
    int32_t step;
    switch (direction) {
        case DIRECTION_TOP:
            step = -GRID_SIZE;
            break;
        case DIRECTION_RIGHT:
            step = 1;
            break;
        case DIRECTION_BOTTOM:
            step = GRID_SIZE;
            break;
        case DIRECTION_LEFT:
            step = -1;
            break;
        default:
            return;
    }
    ferry_route_direction = direction;
    int32_t length = 1;
    int32_t water_tiles = 0;
    while (1) {
        uint32_t ahead = map_grids.terrain[offset + 2 * step];
        offset += step;
        length++;
        if (ahead & TERRAIN_TREE) {
            break;
        }
        if (map_grids.sprite[offset + step] != 0) {
            break;
        }
        if (!(ahead & TERRAIN_WATER)) {
            ferry_route_length = FERRY_ROUTE_MAX_LENGTH;
            return;
        }
        if (construction_ferry_state != 0 && (ahead & (TERRAIN_ROAD | TERRAIN_BUILDING))) {
            break;
        }
        water_tiles++;
        if (water_tiles >= FERRY_ROUTE_MAX_LENGTH) {
            break;
        }
    }
    ferry_route_length = length;
}

static int32_t bridge_step_towards_start(int32_t offset)
{
    uint32_t along_y = 0;
    uint32_t along_x = 0;
    if ((map_grids.terrain[offset - GRID_SIZE] & TERRAIN_WATER) && map_grids.sprite[offset - GRID_SIZE] != 0) {
        along_y = 1;
    }
    if ((map_grids.terrain[offset - 2 * GRID_SIZE] & TERRAIN_WATER) && map_grids.sprite[offset - 2 * GRID_SIZE] != 0) {
        along_y++;
    }
    if ((map_grids.terrain[offset + GRID_SIZE] & TERRAIN_WATER) && map_grids.sprite[offset + GRID_SIZE] != 0) {
        along_y++;
    }
    if ((map_grids.terrain[offset + 2 * GRID_SIZE] & TERRAIN_WATER) && map_grids.sprite[offset + 2 * GRID_SIZE] != 0) {
        along_y++;
    }
    if ((map_grids.terrain[offset + 1] & TERRAIN_WATER) && map_grids.sprite[offset + 1] != 0) {
        along_x = 1;
    }
    if ((map_grids.terrain[offset + 2] & TERRAIN_WATER) && map_grids.sprite[offset + 2] != 0) {
        along_x++;
    }
    if ((map_grids.terrain[offset - 1] & TERRAIN_WATER) && map_grids.sprite[offset - 1] != 0) {
        along_x++;
    }
    if ((map_grids.terrain[offset - 2] & TERRAIN_WATER) && map_grids.sprite[offset - 2] != 0) {
        along_x++;
    }
    return (along_y < along_x) ? -1 : -GRID_SIZE;
}

static int32_t bridge_first_tile(int32_t offset, int32_t back)
{
    while ((map_grids.terrain[offset + back] & TERRAIN_WATER) && map_grids.sprite[offset + back] != 0) {
        offset += back;
    }
    return offset;
}

void map_bridge_remove(int32_t offset, int32_t mark_deleted)
{
    if (!((uint8_t) map_grids.terrain[offset] & TERRAIN_WATER) || map_grids.sprite[offset] == 0) {
        return;
    }
    int32_t back = bridge_step_towards_start(offset);
    int32_t forward = -back;
    offset = bridge_first_tile(offset, back);
    if (mark_deleted == 0) {
        uint32_t terrain = map_grids.terrain[offset];
        map_grids.sprite[offset] = 0;
        map_grids.terrain[offset] = terrain & ~TERRAIN_ROAD;
    } else {
        map_grids.bitfields[offset] |= MAP_BITFIELD_DELETED;
    }
    while (map_grids.terrain[offset + forward] & TERRAIN_WATER) {
        if (map_grids.sprite[offset + forward] == 0) {
            return;
        }
        offset += forward;
        if (mark_deleted == 0) {
            map_grids.terrain[offset] &= ~TERRAIN_ROAD;
            map_grids.sprite[offset] = 0;
        } else {
            map_grids.bitfields[offset] |= MAP_BITFIELD_DELETED;
        }
    }
}

int32_t map_bridge_count_figures(int32_t offset)
{
    if (!((uint8_t) map_grids.terrain[offset] & TERRAIN_WATER)) {
        return 0;
    }
    if (map_grids.sprite[offset] == 0) {
        return 0;
    }
    int32_t back = bridge_step_towards_start(offset);
    int32_t forward = -back;
    offset = bridge_first_tile(offset, back);
    int32_t count = (map_grids.figure_ids[offset] != 0);
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_DELETED;
    while (map_grids.terrain[offset + forward] & TERRAIN_WATER) {
        if (map_grids.sprite[offset + forward] == 0) {
            return count;
        }
        offset += forward;
        if (map_grids.figure_ids[offset] != 0) {
            count++;
        }
        map_grids.bitfields[offset] &= ~MAP_BITFIELD_DELETED;
    }
    return count;
}
