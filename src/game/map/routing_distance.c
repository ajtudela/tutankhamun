// Distance maps over the whole reachable area and the distance maps of routed construction (roads, canals, walls), in
// the editor's variant and in the game's checked one, with the rules where a road may cross a canal and a canal a
// road, the water lift connections and the extra cost tiles.
//
// The two variants differ in small ways that routed placement depends on:
// - the road source check asks the canal piece twice, and the checked variant's canal check can never reject;
// - the checked variant tests the map edge on the raw grid offset split into x and y;
// - a road may not enter a roadblock from the top neighbour (only that side also tests the canal mode);
// - the water lift values expected from the top and bottom neighbours are not the same in both.

#include "map/routing_distance.h"

#include "building/building.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "graphics/image_collection.h"
#include "map/construction_terrain.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_citizen.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"

#include <string.h>

static int32_t building_type_at(int32_t grid_offset)
{
    return building_get(map_grids.building_ids[grid_offset])->type;
}

static const uint8_t canal_road_piece_axis[ROUTED_CANAL_ROAD_PIECES] = { CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_NONE,
    CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH,
    CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE,
    CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST,
    CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE,
    CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NONE,
    CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_NONE, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH,
    CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST,
    CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH, CANAL_ROAD_AXIS_EAST_WEST, CANAL_ROAD_AXIS_NORTH_SOUTH };

static int32_t view_is_rotated(void)
{
    return map_info.orientation == DIRECTION_RIGHT || map_info.orientation == DIRECTION_LEFT;
}

int32_t map_can_place_road_under_canal(int32_t grid_offset)
{
    int32_t piece = map_grids.images[grid_offset] - image_group(IMAGE_GROUP_CANAL);
    if (piece >= ROUTED_CANAL_ROAD_PIECES) {
        piece -= ROUTED_CANAL_ROAD_PIECES;
    }
    if ((uint32_t) piece > (ROUTED_CANAL_ROAD_PIECES - 1)) {
        return 0;
    }
    int32_t kind = canal_road_piece_axis[piece];
    if (kind == CANAL_ROAD_AXIS_NONE) {
        return 0;
    }
    int32_t check_east_west = kind == CANAL_ROAD_AXIS_EAST_WEST;
    if (view_is_rotated()) {
        check_east_west ^= 1;
    }
    if (check_east_west) {
        if (map_grids.terrain[grid_offset + 1] & TERRAIN_ROAD) {
            return 0;
        }
        if (map_grids.terrain[grid_offset - 1] & TERRAIN_ROAD) {
            return 0;
        }
    } else {
        if (map_grids.terrain[grid_offset - GRID_SIZE] & TERRAIN_ROAD) {
            return 0;
        }
        if (map_grids.terrain[grid_offset + GRID_SIZE] & TERRAIN_ROAD) {
            return 0;
        }
    }
    return 1;
}

int32_t map_can_place_road_on_canal_object(int32_t grid_offset)
{
    if (map_grids.terrain[grid_offset] & TERRAIN_CANAL) {
        return map_can_place_road_under_canal(grid_offset);
    }
    const flood_tile *object = floods_data.tiles[grid_offset];
    if (!object || object->is_floodplain != 0) {
        return 1;
    }
    int32_t piece = map_grids.images[grid_offset] - image_group(IMAGE_GROUP_CANAL_ROAD);
    int32_t axis;
    if (piece == CANAL_IMAGE_STRAIGHT_C || piece == CANAL_IMAGE_STRAIGHT_A) {
        axis = CANAL_ROAD_AXIS_EAST_WEST;
    } else if (piece == CANAL_IMAGE_STRAIGHT_D || piece == CANAL_IMAGE_STRAIGHT_B) {
        axis = CANAL_ROAD_AXIS_NORTH_SOUTH;
    } else {
        return 0;
    }
    if (view_is_rotated()) {
        axis ^= 1;
    }
    if (axis != CANAL_ROAD_AXIS_EAST_WEST) {
        if (map_grids.terrain[grid_offset - GRID_SIZE] & TERRAIN_ROAD) {
            return 0;
        }
        if (routing_grids.distance[grid_offset - GRID_SIZE] > 0) {
            return 0;
        }
        if (map_grids.terrain[grid_offset + GRID_SIZE] & TERRAIN_ROAD) {
            return 0;
        }
        return routing_grids.distance[grid_offset + GRID_SIZE] <= 0;
    } else {
        if (map_grids.terrain[grid_offset + 1] & TERRAIN_ROAD) {
            return 0;
        }
        if (routing_grids.distance[grid_offset + 1] > 0) {
            return 0;
        }
        if (map_grids.terrain[grid_offset - 1] & TERRAIN_ROAD) {
            return 0;
        }
        return routing_grids.distance[grid_offset - 1] <= 0;
    }
}

int32_t map_can_place_canal_on_road(int32_t grid_offset)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    if (building_id != 0 && building_get(building_id)->type == BUILDING_ROADBLOCK) {
        return 1;
    }
    int32_t image;
    int32_t base;
    if (map_grids.terrain[grid_offset] & TERRAIN_FLOODPLAIN) {
        image = map_grids.images[grid_offset];
        base = image_group(IMAGE_GROUP_FLOODPLAIN_ROAD);
    } else if (map_grids.bitfields[grid_offset] & MAP_BITFIELD_PLAZA_OR_FAULT) {
        if (map_grids.images[grid_offset] - image_group(IMAGE_GROUP_PLAZA_PLAIN) >= 3) {
            return 0;
        }
        if (map_image_context_set_road_neighbours(grid_offset) > 2) {
            return 0;
        }
        return map_terrain_roads_in_both_axes(grid_offset) ? 0 : 1;
    } else {
        int32_t road_is_paved = map_tiles_road_desirability_paved(grid_offset);
        base = image_group(IMAGE_GROUP_ROAD_PAVED);
        image = map_grids.images[grid_offset];
        if (!road_is_paved) {
            base = image_group(IMAGE_GROUP_ROAD);
        }
    }
    int32_t axis;
    switch ((uint32_t) (image - base)) {
        case 0:
        case 2:
        case 8:
        case 10:
            axis = CANAL_ROAD_AXIS_NORTH_SOUTH;
            break;
        case 1:
        case 3:
        case 9:
        case 11:
            axis = CANAL_ROAD_AXIS_EAST_WEST;
            break;
        default:
            return 0;
    }
    if (view_is_rotated()) {
        axis ^= 1;
    }
    if (axis == CANAL_ROAD_AXIS_NORTH_SOUTH) {
        if (map_grids.terrain[grid_offset - GRID_SIZE] & TERRAIN_CANAL) {
            return 0;
        }
        if (map_grids.terrain[grid_offset + GRID_SIZE] & TERRAIN_CANAL) {
            return 0;
        }
    } else {
        if (map_grids.terrain[grid_offset + 1] & TERRAIN_CANAL) {
            return 0;
        }
        if (map_grids.terrain[grid_offset - 1] & TERRAIN_CANAL) {
            return 0;
        }
    }
    return 1;
}

static int32_t can_enter_road_or_ramp(int32_t next, int32_t current)
{
    (void) current;
    int8_t value = routing_grids.land_citizen[next];
    return value >= ROUTING_CITIZEN_ROAD && value < ROUTING_CITIZEN_FERRY_UNSTAFFED;
}

void map_routing_calculate_distances(int32_t x, int32_t y)
{
    routing_route_queue(x, y, ROUTING_NO_DESTINATION, can_enter_road_or_ramp);
}

static int32_t is_clear_terrain(int32_t next, int32_t current)
{
    (void) current;
    return routing_grids.land_citizen[next] == ROUTING_CITIZEN_CLEAR;
}

int32_t map_routing_calculate_distances_for_wall(int32_t x, int32_t y)
{
    routing_clear_distances();
    routing_source_offset = map_grid_offset(x, y);
    routing_queue_reset(routing_source_offset);
    routing_search(ROUTING_NO_DESTINATION, is_clear_terrain);
    return 1;
}

static int32_t building_source_allowed(int32_t offset, int32_t mode)
{
    uint32_t terrain = map_grids.terrain[offset];
    if ((terrain & TERRAIN_NOT_CLEAR_FOR_ROUTED) || routing_grids.land_citizen[offset] == ROUTING_CITIZEN_BLOCKED ||
        ((terrain & TERRAIN_CANAL) && !map_can_place_road_under_canal(offset))) {
        if (mode == ROUTED_MODE_ROAD) {
            return 0;
        }
        if (!(terrain & TERRAIN_CANAL)) {
            if (!(terrain & TERRAIN_BUILDING)) {
                return 0;
            }
            int32_t type = building_type_at(offset);
            if (type != BUILDING_WATER_LIFT && type != BUILDING_ROADBLOCK) {
                return 0;
            }
        }
    }
    if (routing_grids.land_citizen[offset] == ROUTING_CITIZEN_PASSABLE) {
        return 0;
    }
    if ((terrain & TERRAIN_CANAL) && mode == ROUTED_MODE_ROAD && !map_can_place_road_under_canal(offset)) {
        return 0;
    }
    if (routing_grids.land_citizen[offset] == ROUTING_CITIZEN_CANAL_ON_FLOODPLAIN) {
        if (mode != ROUTED_MODE_ROAD || !map_can_place_road_on_canal_object(offset)) {
            return 0;
        }
    }
    if ((terrain & TERRAIN_ROAD) && mode != ROUTED_MODE_ROAD && !map_can_place_canal_on_road(offset)) {
        return 0;
    }
    return 1;
}

static int32_t building_source_allowed_checked(int32_t offset, int32_t x, int32_t y, int32_t mode)
{
    int8_t citizen = routing_grids.land_citizen[offset];
    uint32_t terrain = map_grids.terrain[offset];
    if (citizen == ROUTING_CITIZEN_BLOCKED || (terrain & TERRAIN_NOT_CLEAR_FOR_ROUTED)) {
        if (mode == ROUTED_MODE_ROAD) {
            return 0;
        }
        if (!(terrain & TERRAIN_CANAL)) {
            if (!(terrain & TERRAIN_BUILDING)) {
                return 0;
            }
            int32_t type = building_type_at(offset);
            if (type != BUILDING_WATER_LIFT && type != BUILDING_ROADBLOCK) {
                return 0;
            }
        }
    }
    if (citizen == ROUTING_CITIZEN_PASSABLE) {
        return 0;
    }
    if ((terrain & TERRAIN_CANAL) && map_can_place_road_under_canal(offset) && mode == ROUTED_MODE_ROAD && !map_can_place_road_under_canal(offset)) {
        return 0;
    }
    if (routing_grids.land_citizen[offset] == ROUTING_CITIZEN_CANAL_ON_FLOODPLAIN) {
        if (mode != ROUTED_MODE_ROAD || !map_can_place_road_on_canal_object(offset)) {
            return 0;
        }
    }
    if ((terrain & TERRAIN_ROAD) && mode != ROUTED_MODE_ROAD && !map_can_place_canal_on_road(offset)) {
        return 0;
    }
    if (map_grid_is_inside_size(x + map_info.diamond_x_min, y + map_info.diamond_y_min, ROUTED_MAP_EDGE_MARGIN)) {
        return 0;
    }
    return 1;
}

static int32_t building_tile_allowed(int32_t next, int8_t citizen, int32_t connector_value, int32_t mode, int32_t checked, int32_t direction)
{
    if (checked && building_type_at(next) == BUILDING_ROADBLOCK && (direction != ROUTING_NEIGHBOUR_TOP || mode == ROUTED_MODE_CANAL)) {
        return 1;
    }
    return citizen == connector_value && mode == ROUTED_MODE_CANAL;
}

static void visit_building_neighbour(int32_t next, int32_t distance, int32_t mode, int32_t connector_value, int32_t checked, int32_t direction)
{
    if (routing_grids.distance[next] != 0) {
        return;
    }
    int32_t blocked = 0;
    if ((map_grids.terrain[next] & TERRAIN_CANAL) && map_can_place_road_under_canal(next)) {
        if (mode == ROUTED_MODE_ROAD && !map_can_place_road_under_canal(next)) {
            routing_grids.distance[next] = ROUTED_DISTANCE_REJECTED;
            blocked = 1;
        }
    } else {
        int8_t citizen = routing_grids.land_citizen[next];
        if (citizen == ROUTING_CITIZEN_CANAL_ON_FLOODPLAIN) {
            if (mode != ROUTED_MODE_ROAD) {
                blocked = 1;
            } else if (!map_can_place_road_on_canal_object(next)) {
                routing_grids.distance[next] = ROUTED_DISTANCE_REJECTED;
                blocked = 1;
            }
        } else if (citizen == ROUTING_CITIZEN_PASSABLE || citizen == ROUTING_CITIZEN_BLOCKED || (map_grids.terrain[next] & TERRAIN_NOT_CLEAR_FOR_ROUTED)) {
            blocked = 1;
        } else if ((map_grids.terrain[next] & TERRAIN_BUILDING) && !building_tile_allowed(next, citizen, connector_value, mode, checked, direction)) {
            blocked = 1;
        }
    }
    if ((map_grids.terrain[next] & TERRAIN_ROAD) && mode != ROUTED_MODE_ROAD && !map_can_place_canal_on_road(next)) {
        routing_grids.distance[next] = ROUTED_DISTANCE_REJECTED;
        return;
    }
    if (blocked) {
        return;
    }
    if (checked && map_grid_is_inside_size(next % GRID_SIZE, next / GRID_SIZE, ROUTED_MAP_EDGE_MARGIN)) {
        return;
    }
    routing_enqueue(next, (routing_grids.land_citizen[next] == ROUTING_CITIZEN_EXTRA_COST) ? (distance + ROUTED_EXTRA_COST_DISTANCE) : distance);
}

static const int32_t connector_values[ROUTING_NEIGHBOUR_COUNT] = { ROUTING_CITIZEN_WATER_LIFT_EVEN, ROUTING_CITIZEN_WATER_LIFT_ODD,
    ROUTING_CITIZEN_WATER_LIFT_ODD, ROUTING_CITIZEN_WATER_LIFT_ODD };
static const int32_t connector_values_checked[ROUTING_NEIGHBOUR_COUNT] = { ROUTING_CITIZEN_WATER_LIFT_EVEN, ROUTING_CITIZEN_WATER_LIFT_ODD,
    ROUTING_CITIZEN_WATER_LIFT_EVEN, ROUTING_CITIZEN_WATER_LIFT_ODD };

static int32_t calculate_distances_for_building_common(int32_t x, int32_t y, int32_t mode, int32_t checked)
{
    routing_clear_distances();
    routing_source_offset = map_grid_offset(x, y);
    int32_t allowed = checked ? building_source_allowed_checked(routing_source_offset, x, y, mode) : building_source_allowed(routing_source_offset, mode);
    if (!allowed) {
        return 0;
    }
    routing_stats_total_routes_calculated++;
    memset(routing_grids.water_drag, 0, GRID_CELLS);
    routing_queue_reset(routing_source_offset);
    const int32_t *connectors = checked ? connector_values_checked : connector_values;
    do {
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
            visit_building_neighbour(offset + routing_neighbour_offsets[i], distance, mode, connectors[i], checked, i);
        }
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return 1;
}

int32_t map_routing_calculate_distances_for_building(int32_t x, int32_t y, int32_t mode)
{
    return calculate_distances_for_building_common(x, y, mode, 0);
}

int32_t map_routing_calculate_distances_for_building_checked(int32_t x, int32_t y, int32_t mode)
{
    return calculate_distances_for_building_common(x, y, mode, 1);
}
