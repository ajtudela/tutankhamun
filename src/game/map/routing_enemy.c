// Routing of enemies and wall walkers: the weighted enemy search (the closest queued tile first, buildings and walls
// cost more), the plain search through one building, the searches through everything and over walls, their
// multi-target variants, and the wall tile found within a radius of a building.
//
// Search details the enemy paths depend on:
// - the fighting enemy check follows the figure chain without the self-link check of the friendly one;
// - the multi-target searches scan their targets from the last, so with duplicate offsets the highest index wins;
// - the closest-tile scan leaves the chosen offset duplicated in the queue until the head moves on.

#include "map/routing_enemy.h"

#include "figure/figure.h"
#include "figure/figure_action.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"

int32_t map_routing_no_fighting_enemy(int32_t grid_offset)
{
    int32_t id = map_grids.figure_ids[grid_offset];
    if (id > 0) {
        do {
            const figure *f = figure_get(id);
            if (f->is_friendly == 0 && f->action_state == FIGURE_ACTION_ATTACK) {
                return 0;
            }
            id = f->next_figure_id_on_same_tile;
        } while (id != 0);
    }
    return 1;
}

static void start_weighted_search(int32_t src_x, int32_t src_y)
{
    routing_queue_start(src_x, src_y);
    routing_stats_enemy_routes_calculated++;
}

static int32_t dequeue_closest(int32_t *last_min)
{
    int32_t head = routing_queue_head;
    int32_t tail = routing_queue_tail;
    int32_t selected = head;
    int32_t min = routing_grids.distance[routing_grids.queue_items[head]];
    if (min > *last_min) {
        if (head < tail) {
            for (int32_t i = head + 1; i < tail; i++) {
                int32_t d = routing_grids.distance[routing_grids.queue_items[i]];
                if (d < min) {
                    selected = i;
                    min = d;
                    if (d <= *last_min) {
                        break;
                    }
                }
            }
        } else {
            for (int32_t i = head + 1; i < MAX_QUEUE; i++) {
                int32_t d = routing_grids.distance[routing_grids.queue_items[i]];
                if (d < min) {
                    selected = i;
                    min = d;
                    if (d <= *last_min) {
                        break;
                    }
                }
            }
            if (min > *last_min) {
                for (int32_t i = 0; i < tail; i++) {
                    int32_t d = routing_grids.distance[routing_grids.queue_items[i]];
                    if (d < min) {
                        selected = i;
                        min = d;
                    }
                }
            }
        }
        *last_min = min;
    }
    int32_t offset = routing_grids.queue_items[selected];
    if (selected != routing_queue_head) {
        routing_grids.queue_items[selected] = routing_grids.queue_items[routing_queue_head];
    }
    return offset;
}

static int32_t enemy_step_cost(int8_t value)
{
    if (value == ROUTING_NONCITIZEN_WALL) {
        return ENEMY_ROUTE_COST_WALL;
    }
    if (value == ROUTING_NONCITIZEN_BUILDING) {
        return ENEMY_ROUTE_COST_BUILDING;
    }
    return 0;
}

static void expand_weighted(int32_t offset)
{
    int32_t distance = routing_grids.distance[offset] + 1;
    for (int32_t i = 0; i < ROUTING_NEIGHBOUR_COUNT; i++) {
        int32_t next = offset + routing_neighbour_offsets[i];
        if (routing_grids.distance[next] == 0 && map_routing_no_fighting_enemy(next)) {
            int8_t value = routing_grids.land_noncitizen[next];
            if (value >= ROUTING_NONCITIZEN_PASSABLE && value < ROUTING_NONCITIZEN_FORT) {
                routing_enqueue(next, enemy_step_cost(value) + distance);
            }
        }
    }
}

static int32_t only_through_building_id;

static int32_t can_travel_through_building(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_enemy(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_noncitizen[next];
    int32_t is_open = value == ROUTING_NONCITIZEN_PASSABLE || value == ROUTING_NONCITIZEN_ACCESS_RAMP;
    return is_open || (value == ROUTING_NONCITIZEN_BUILDING && map_grids.building_ids[next] == only_through_building_id);
}

static int32_t can_travel_through_everything(int32_t next, int32_t current)
{
    (void) current;
    return routing_grids.land_noncitizen[next] >= ROUTING_NONCITIZEN_PASSABLE;
}

static int32_t can_travel_over_walls(int32_t next, int32_t current)
{
    (void) current;
    int8_t value = routing_grids.walls[next];
    return value >= ROUTING_WALL_PASSABLE && value < ROUTING_WALL_ACCEPTED_BELOW;
}

static int32_t route_weighted(int32_t dst_offset, int32_t max_tiles)
{
    int32_t last_min = 0;
    int32_t tiles = 0;
    do {
        int32_t offset = dequeue_closest(&last_min);
        if (offset == dst_offset || ++tiles > max_tiles) {
            break;
        }
        expand_weighted(offset);
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return routing_grids.distance[dst_offset] != 0;
}

static int32_t route_weighted_first_of(const int32_t *offsets, int32_t count, int32_t max_tiles)
{
    int32_t last_min = 0;
    int32_t tiles = 0;
    do {
        int32_t offset = dequeue_closest(&last_min);
        int32_t target = routing_target_index(offset, offsets, count);
        if (target >= 0) {
            return target;
        }
        if (++tiles > max_tiles) {
            return 0;
        }
        expand_weighted(offset);
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return 0;
}

static int32_t travel(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t (*can_enter)(int32_t next, int32_t current))
{
    int32_t dst_offset = map_grid_offset(dst_x, dst_y);
    routing_route_queue(src_x, src_y, dst_offset, can_enter);
    return routing_grids.distance[dst_offset] != 0;
}

int32_t map_routing_noncitizen_can_travel_over_land(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t building_id, int32_t max_tiles)
{
    if (building_id == 0) {
        start_weighted_search(src_x, src_y);
        return route_weighted(map_grid_offset(dst_x, dst_y), max_tiles);
    }
    only_through_building_id = building_id;
    int32_t reached = travel(src_x, src_y, dst_x, dst_y, can_travel_through_building);
    routing_stats_enemy_routes_calculated++;
    return reached;
}

int32_t map_routing_noncitizen_find_first_of(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count, int32_t building_id, int32_t max_tiles)
{
    if (building_id == 0) {
        start_weighted_search(src_x, src_y);
        return route_weighted_first_of(offsets, count, max_tiles);
    }
    only_through_building_id = building_id;
    routing_stats_enemy_routes_calculated++;
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_travel_through_building);
}

int32_t map_routing_noncitizen_can_travel_through_everything(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_travel_through_everything);
}

int32_t map_routing_can_travel_over_walls(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_travel_over_walls);
}

int32_t map_routing_walls_find_first_of(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_travel_over_walls);
}

int32_t map_routing_noncitizen_find_first_of_through_everything(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_travel_through_everything);
}

int32_t map_routing_wall_tile_in_radius(int32_t x, int32_t y, int32_t size, int32_t radius)
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
    int32_t offset = map_grid_offset(x0, y0);
    int32_t row_adjust = x0 - x1 + (GRID_SIZE - 1);
    map_result_y = y0;
    map_tile_cursor_offset = offset;
    if (y0 > y1) {
        return 0;
    }
    int32_t ycur = y0;
    for (;;) {
        int32_t xcur = x0;
        map_result_x = xcur;
        if (x0 <= x1) {
            for (;;) {
                if (routing_grids.walls[offset] == ROUTING_WALL_PASSABLE) {
                    return 1;
                }
                xcur++;
                offset++;
                map_result_x = xcur;
                map_tile_cursor_offset = offset;
                if (xcur > x1) {
                    break;
                }
            }
        }
        ycur++;
        offset += row_adjust;
        map_result_y = ycur;
        map_tile_cursor_offset = offset;
        if (ycur > y1) {
            return 0;
        }
    }
}

int32_t map_routing_wall_within_radius(int32_t x, int32_t y, int32_t size, int32_t max_radius)
{
    if (max_radius > 0) {
        for (int32_t radius = 1; radius <= max_radius; radius++) {
            if (map_routing_wall_tile_in_radius(x, y, size, radius)) {
                return 1;
            }
        }
    }
    return 0;
}
