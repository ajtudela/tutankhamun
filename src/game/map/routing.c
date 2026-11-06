// Routing core: the distance queue and the breadth-first search with a per-mode callback, the single and multi-target
// route queues, the friendly fighting check, the general direction between two tiles, two citizen land searches, and
// the path walked back from the destination over decreasing distances (on a tie towards the source).

#include "map/routing.h"

#include "figure/figure.h"
#include "figure/figure_action.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"

#include <string.h>

const int32_t routing_neighbour_offsets[ROUTING_NEIGHBOUR_COUNT] = { -GRID_SIZE, 1, GRID_SIZE, -1 };
const int32_t routing_direction_dx[DIRECTION_COUNT] = { 0, 1, 1, 1, 0, -1, -1, -1 };
const int32_t routing_direction_dy[DIRECTION_COUNT] = { -1, -1, 0, 1, 1, 1, 0, -1 };

void routing_queue_push(int32_t offset)
{
    routing_grids.queue_items[routing_queue_tail++] = offset;
    if (routing_queue_tail >= MAX_QUEUE) {
        routing_queue_tail = 0;
    }
}

void routing_enqueue(int32_t offset, int32_t distance)
{
    routing_grids.distance[offset] = distance;
    routing_queue_push(offset);
}

void routing_queue_reset(int32_t source_offset)
{
    routing_grids.distance[source_offset] = 1;
    routing_grids.queue_items[0] = source_offset;
    routing_queue_head = 0;
    routing_queue_tail = 1;
}

void routing_store_path(int32_t path_id, int32_t length)
{
    uint8_t *path = &city_figures.route_paths[path_id * MAX_PATH_LENGTH];
    for (int32_t i = 0; i < length; i++) {
        path[i] = routing_grids.direction_path_tmp[length - 1 - i];
    }
}

void routing_clear_distances(void)
{
    memset(routing_grids.distance, 0, GRID_CELLS * sizeof(routing_grids.distance[0]));
}

void routing_queue_set_source(int32_t src_x, int32_t src_y)
{
    routing_source_offset = map_grid_offset(src_x, src_y);
    routing_stats_total_routes_calculated++;
    routing_queue_reset(routing_source_offset);
}

void routing_queue_start(int32_t src_x, int32_t src_y)
{
    routing_clear_distances();
    routing_queue_set_source(src_x, src_y);
}

void routing_queue_advance(void)
{
    if (++routing_queue_head >= MAX_QUEUE) {
        routing_queue_head = 0;
    }
}

int32_t routing_target_index(int32_t offset, const int32_t *offsets, int32_t count)
{
    for (int32_t i = count - 1; i >= 0; i--) {
        if (offsets[i] == offset) {
            return i + 1;
        }
    }
    return -1;
}

void routing_search(int32_t dst_offset, int32_t (*can_enter)(int32_t next, int32_t current))
{
    while (routing_queue_head != routing_queue_tail) {
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        if (offset == dst_offset) {
            break;
        }
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
            int32_t next = offset + routing_neighbour_offsets[i];
            if (routing_grids.distance[next] == 0 && can_enter(next, offset)) {
                routing_enqueue(next, distance);
            }
        }
        routing_queue_advance();
    }
}

void routing_route_queue(int32_t src_x, int32_t src_y, int32_t dst_offset, int32_t (*can_enter)(int32_t next, int32_t current))
{
    routing_queue_start(src_x, src_y);
    routing_search(dst_offset, can_enter);
}

int32_t routing_route_queue_first_of(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count, int32_t (*can_enter)(int32_t next, int32_t current))
{
    routing_queue_start(src_x, src_y);
    while (routing_queue_head != routing_queue_tail) {
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        int32_t target = routing_target_index(offset, offsets, count);
        if (target >= 0) {
            return target;
        }
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
            int32_t next = offset + routing_neighbour_offsets[i];
            if (routing_grids.distance[next] == 0 && can_enter(next, offset)) {
                routing_enqueue(next, distance);
            }
        }
        routing_queue_advance();
    }
    return 0;
}

int32_t map_routing_no_fighting_friendly(int32_t grid_offset)
{
    int32_t id = map_grids.figure_ids[grid_offset];
    if (id <= 0) {
        return 1;
    }
    for (;;) {
        const figure *f = figure_get(id);
        if (f->is_friendly == 1 && f->action_state == FIGURE_ACTION_ATTACK) {
            return 0;
        }
        int32_t next = f->next_figure_id_on_same_tile;
        if (next == id || next == 0) {
            return 1;
        }
        id = next;
    }
}

static int32_t is_blocked_rubble(int8_t value, int32_t offset)
{
    return value == ROUTING_CITIZEN_BLOCKED && (map_grids.terrain[offset] & TERRAIN_RUBBLE);
}

static int32_t can_travel_over_land_any(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    int32_t walkable = value >= ROUTING_CITIZEN_ROAD || value == ROUTING_CITIZEN_AQUEDUCT || is_blocked_rubble(value, next);
    return walkable && value < ROUTING_CITIZEN_MONUMENT;
}

static int32_t can_travel_over_road_garden(int32_t next, int32_t current)
{
    (void) current;
    int8_t value = routing_grids.land_citizen[next];
    return value >= ROUTING_CITIZEN_ROAD && value < ROUTING_CITIZEN_FERRY_UNSTAFFED;
}

int32_t map_routing_citizen_can_travel_over_land_any(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    int32_t dst_offset = map_grid_offset(dst_x, dst_y);
    routing_route_queue(src_x, src_y, dst_offset, can_travel_over_land_any);
    return routing_grids.distance[dst_offset] != 0;
}

int32_t map_routing_citizen_can_travel_over_road_garden(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    int32_t dst_offset = map_grid_offset(dst_x, dst_y);
    routing_route_queue(src_x, src_y, dst_offset, can_travel_over_road_garden);
    return routing_grids.distance[dst_offset] != 0;
}

int32_t calc_general_direction(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to)
{
    if (x_to < x_from) {
        if (y_to < y_from) {
            return DIRECTION_TOP_LEFT;
        } else if (y_from == y_to) {
            return DIRECTION_LEFT;
        } else if (y_from < y_to) {
            return DIRECTION_BOTTOM_LEFT;
        }
    } else if (x_from == x_to) {
        if (y_to < y_from) {
            return DIRECTION_TOP;
        } else if (y_from < y_to) {
            return DIRECTION_BOTTOM;
        }
    } else if (x_from < x_to) {
        if (y_to < y_from) {
            return DIRECTION_TOP_RIGHT;
        } else if (y_from == y_to) {
            return DIRECTION_RIGHT;
        } else if (y_from < y_to) {
            return DIRECTION_BOTTOM_RIGHT;
        }
    }
    return DIRECTION_AT_DESTINATION;
}

int32_t map_routing_get_path(int32_t num_directions, int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    int32_t offset = map_grid_offset(dst_x, dst_y);
    routing_source_offset = offset;
    if (routing_grids.distance[offset] <= 0) {
        return 0;
    }
    int32_t step = (num_directions == DIRECTION_COUNT) ? 1 : 2;
    int32_t last_direction = -1;
    int32_t x = dst_x, y = dst_y;
    for (int32_t length = 0; length < MAX_PATH_LENGTH; length++) {
        if (routing_grids.distance[offset] < 2) {
            routing_store_path(path_id, length);
            return length;
        }
        int32_t general_direction = calc_general_direction(x, y, src_x, src_y) & 0xFF;
        int32_t best_direction = -1;
        int32_t best_distance = routing_grids.distance[offset];
        for (int32_t direction = DIRECTION_TOP; direction < DIRECTION_COUNT; direction += step) {
            if (direction == last_direction) {
                continue;
            }
            int32_t distance = routing_grids.distance[offset + map_grid_direction_delta[direction]];
            if (distance == 0) {
                continue;
            }
            if (distance < best_distance || (distance == best_distance && (direction == general_direction || best_direction == -1))) {
                best_direction = direction;
                best_distance = distance;
            }
        }
        if (best_direction == -1) {
            return 0;
        }
        x += routing_direction_dx[best_direction];
        y += routing_direction_dy[best_direction];
        offset += map_grid_direction_delta[best_direction];
        last_direction = routing_opposite_direction(best_direction);
        routing_grids.direction_path_tmp[length] = (uint8_t) last_direction;
    }
    return 0;
}
