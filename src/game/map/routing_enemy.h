// Routing of enemies and wall walkers (map/routing_enemy.c): single and multi-target searches over land, through
// everything and over walls, and the wall tile within a radius.

#ifndef MAP_ROUTING_ENEMY_H
#define MAP_ROUTING_ENEMY_H

#include <stdint.h>

enum enemy_route_cost {
    ENEMY_ROUTE_COST_BUILDING = 10,
    ENEMY_ROUTE_COST_WALL = 40,
};

int32_t map_routing_no_fighting_enemy(int32_t grid_offset);
int32_t map_routing_noncitizen_can_travel_over_land(
    int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t only_through_building_id, int32_t max_tiles);
int32_t map_routing_noncitizen_can_travel_through_everything(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_can_travel_over_walls(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);

int32_t map_routing_walls_find_first_of(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);
int32_t map_routing_noncitizen_find_first_of(
    int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count, int32_t only_through_building_id, int32_t max_tiles);
int32_t map_routing_noncitizen_find_first_of_through_everything(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);
int32_t map_routing_wall_tile_in_radius(int32_t x, int32_t y, int32_t size, int32_t radius);
int32_t map_routing_wall_within_radius(int32_t x, int32_t y, int32_t size, int32_t max_radius);

#endif
