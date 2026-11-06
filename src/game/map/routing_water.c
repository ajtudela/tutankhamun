// Water routing: the distance maps of boats, flotsam and ferry lanes, and the walk-backs of water paths (boats and
// flotsam, warships, and the four-direction one of the ferry lanes). None of them count into the route statistics.
//
// Boats near the map edge are slowed because a dequeued edge tile is queued again up to four times; the ferry check
// builds 24 neighbour offsets per tile but tests only the first eight.

#include "map/routing_water.h"

#include "building/building_data.h"
#include "core/random.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"

#include <string.h>

int32_t map_routing_calculate_distances_water_boat(int32_t x, int32_t y)
{
    int32_t iterations = 0;
    routing_clear_distances();
    routing_source_offset = map_grid_offset(x, y);
    if (routing_grids.water[routing_source_offset] == ROUTING_WATER_LAND) {
        return 0;
    }
    memset(routing_grids.water_drag, 0, GRID_CELLS);
    routing_queue_reset(routing_source_offset);
    do {
        if (++iterations > WATER_ROUTE_MAX_STEPS) {
            return 1;
        }
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        int32_t drag_queued = 0;
        if (routing_grids.water[offset] == ROUTING_WATER_MAP_EDGE) {
            int8_t drag = (int8_t) routing_grids.water_drag[offset];
            routing_grids.water_drag[offset] = (uint8_t) (drag + 1);
            drag_queued = drag < WATER_ROUTE_EDGE_REQUEUES;
        }
        if (drag_queued) {
            routing_queue_push(offset);
        } else {
            int32_t distance = routing_grids.distance[offset] + 1;
            for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
                int32_t next = offset + routing_neighbour_offsets[i];
                int8_t value = routing_grids.water[next];
                if (routing_grids.distance[next] == 0 && value != ROUTING_WATER_BLOCKED && value != ROUTING_WATER_LOW_BRIDGE) {
                    routing_enqueue(next, (value == ROUTING_WATER_MAP_EDGE) ? (distance + WATER_ROUTE_EDGE_EXTRA_DISTANCE) : distance);
                }
            }
        }
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return 1;
}

int32_t map_routing_calculate_distances_water_flotsam(int32_t x, int32_t y)
{
    static const int32_t flotsam_neighbours[DIRECTION_COUNT] = { -GRID_SIZE, 1, GRID_SIZE, -1, -GRID_SIZE + 1, GRID_SIZE + 1, GRID_SIZE - 1, -GRID_SIZE - 1 };
    int32_t iterations = 0;
    routing_clear_distances();
    routing_source_offset = map_grid_offset(x, y);
    if (routing_grids.water[routing_source_offset] == ROUTING_WATER_BLOCKED) {
        return 0;
    }
    routing_queue_reset(routing_source_offset);
    do {
        if (++iterations > WATER_ROUTE_MAX_STEPS) {
            return 1;
        }
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
            int32_t next = offset + flotsam_neighbours[i];
            if (routing_grids.distance[next] == 0 && routing_grids.water[next] != ROUTING_WATER_BLOCKED) {
                routing_enqueue(next, distance);
            }
        }
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return 1;
}

static int32_t is_clear_ferry_tile(int32_t next, int32_t dst_offset)
{
    if (routing_grids.water[next] == ROUTING_WATER_BLOCKED || (map_grids.terrain[next] & TERRAIN_FLOODPLAIN) || (map_grids.terrain[next] & TERRAIN_ROAD)) {
        return 0;
    }
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        int32_t tile = next + map_grid_direction_delta[i];
        if (map_grids.terrain[tile] & TERRAIN_ROAD) {
            return 0;
        }
        if (tile == routing_source_offset || tile == dst_offset) {
            break;
        }
        if (routing_grids.water[tile] == ROUTING_WATER_BLOCKED || (map_grids.terrain[tile] & TERRAIN_FLOODPLAIN)) {
            return 0;
        }
    }
    return 1;
}

int32_t map_routing_calculate_distances_water_ferry(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    int32_t iterations = 0;
    routing_clear_distances();
    routing_source_offset = map_grid_offset(src_x, src_y);
    int32_t dst_offset = map_grid_offset(dst_x, dst_y);
    if (routing_grids.water[routing_source_offset] == ROUTING_WATER_BLOCKED) {
        return 0;
    }
    routing_queue_reset(routing_source_offset);
    do {
        if (++iterations > WATER_ROUTE_MAX_STEPS) {
            return 1;
        }
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
            int32_t next = offset + routing_neighbour_offsets[i];
            if (routing_grids.distance[next] == 0 && (next == routing_source_offset || next == dst_offset || is_clear_ferry_tile(next, dst_offset))) {
                routing_enqueue(next, distance);
            }
        }
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return 1;
}

static int32_t is_floodplain(int32_t offset)
{
    return (map_grids.terrain[offset] & TERRAIN_FLOODPLAIN) != 0;
}

int32_t map_routing_get_path_on_water(int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t is_flotsam)
{
    int32_t random_value = random_generator_state.random1_7bit & 3;
    int32_t offset = map_grid_offset(dst_x, dst_y);
    routing_source_offset = offset;
    if (routing_grids.distance[offset] <= 0) {
        return 0;
    }
    int32_t length = 0;
    int32_t last_direction = -1;
    while (routing_grids.distance[offset] > 1) {
        int32_t best_distance = routing_grids.distance[offset];
        int32_t best_floodplain = is_floodplain(offset);
        int32_t tile_value = (is_flotsam == 1) ? (map_grids.random[offset] & 3) : random_value;
        calc_general_direction(dst_x, dst_y, src_x, src_y);
        int32_t best_direction = -1;
        for (int32_t direction = DIRECTION_TOP; direction < DIRECTION_COUNT; direction++) {
            if (direction == last_direction) {
                continue;
            }
            int32_t next = offset + map_grid_direction_delta[direction];
            int32_t distance = routing_grids.distance[next];
            int32_t floodplain = is_floodplain(next);
            if (distance == 0) {
                continue;
            }
            if (distance < best_distance || (distance == best_distance && ((best_floodplain && !floodplain) || random_value == tile_value))) {
                best_distance = distance;
                best_direction = direction;
                best_floodplain = floodplain;
            }
        }
        if (best_direction == ROUTING_WATER_BLOCKED) {
            return 0;
        }
        offset += map_grid_direction_delta[best_direction];
        last_direction = routing_opposite_direction(best_direction);
        routing_grids.direction_path_tmp[length++] = (uint8_t) last_direction;
        if (length >= MAX_PATH_LENGTH) {
            return 0;
        }
    }
    routing_store_path(path_id, length);
    return length;
}

int32_t map_routing_get_path_warship(int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t straight_length)
{
    int32_t offset = map_grid_offset(dst_x, dst_y);
    routing_source_offset = offset;
    if (routing_grids.distance[offset] <= 0) {
        return 0;
    }
    int32_t length = 0;
    int32_t last_direction = -1;
    int32_t x = dst_x;
    int32_t y = dst_y;
    while (routing_grids.distance[offset] > 1) {
        int32_t best_distance = routing_grids.distance[offset];
        int32_t straight = length <= straight_length;
        int32_t general_direction = straight ? calc_general_direction(dst_x, dst_y, src_x, src_y) : calc_general_direction(x, y, src_x, src_y);
        general_direction &= 0xFF;
        int32_t best_direction = -1;
        for (int32_t direction = DIRECTION_TOP; direction < DIRECTION_COUNT; direction++) {
            if (direction == last_direction) {
                continue;
            }
            int32_t distance = routing_grids.distance[offset + map_grid_direction_delta[direction]];
            if (distance == 0) {
                continue;
            }
            int32_t take;
            if (straight) {
                take = distance <= best_distance && direction == general_direction;
            } else {
                take = distance < best_distance ||
                    (distance == best_distance && (direction == general_direction || best_direction == ROUTING_WATER_BLOCKED));
            }
            if (take) {
                best_distance = distance;
                best_direction = direction;
            }
        }
        if (best_direction == ROUTING_WATER_BLOCKED) {
            return 0;
        }
        x += routing_direction_dx[best_direction];
        y += routing_direction_dy[best_direction];
        offset += map_grid_direction_delta[best_direction];
        last_direction = routing_opposite_direction(best_direction);
        routing_grids.direction_path_tmp[length++] = (uint8_t) last_direction;
        if (length >= MAX_PATH_LENGTH) {
            return 0;
        }
    }
    routing_store_path(path_id, length);
    return length;
}

int32_t map_routing_get_path_on_water_4dir(int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t is_flotsam, int32_t *directions)
{
    int32_t random_value = random_generator_state.random1_7bit & 3;
    int32_t offset = map_grid_offset(dst_x, dst_y);
    routing_source_offset = offset;
    if (routing_grids.distance[offset] <= 0) {
        return 0;
    }
    int32_t length = 0;
    int32_t last_direction = -1;
    int32_t x = dst_x;
    int32_t y = dst_y;
    while (routing_grids.distance[offset] > 1) {
        int32_t best_distance = routing_grids.distance[offset];
        int32_t tile_value = (is_flotsam == 1) ? (map_grids.random[offset] & 3) : random_value;
        calc_general_direction(x, y, src_x, src_y);
        int32_t best_direction = -1;
        for (int32_t direction = DIRECTION_TOP; direction < DIRECTION_COUNT; direction += 2) {
            if (direction == last_direction) {
                continue;
            }
            int32_t distance = routing_grids.distance[offset + map_grid_direction_delta[direction]];
            if (distance == 0) {
                continue;
            }
            if (distance < best_distance || (distance == best_distance && random_value == tile_value)) {
                best_distance = distance;
                best_direction = direction;
            }
        }
        if (best_direction == ROUTING_WATER_BLOCKED) {
            return 0;
        }
        x += routing_direction_dx[best_direction];
        y += routing_direction_dy[best_direction];
        offset += map_grid_direction_delta[best_direction];
        last_direction = routing_opposite_direction(best_direction);
        routing_grids.direction_path_tmp[length++] = (uint8_t) last_direction;
        if (length >= MAX_PATH_LENGTH) {
            return 0;
        }
    }
    if (path_id < MAX_PATHS) {
        routing_store_path(path_id, length);
    } else if (directions && length > 0) {
        for (int32_t i = 0; i < length; i++) {
            directions[i] = routing_grids.direction_path_tmp[length - 1 - i];
        }
    }
    return length;
}
