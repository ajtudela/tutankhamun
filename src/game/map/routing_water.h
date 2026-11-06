// Water routing (map/routing_water.c): boat, flotsam and ferry lane distances and the water path walk-backs.

#ifndef MAP_ROUTING_WATER_H
#define MAP_ROUTING_WATER_H

#include <stdint.h>

enum water_route_rule {
    WATER_ROUTE_MAX_STEPS = 50000,
    WATER_ROUTE_EDGE_EXTRA_DISTANCE = 4,
    WATER_ROUTE_EDGE_REQUEUES = 4,
};

int32_t map_routing_calculate_distances_water_boat(int32_t x, int32_t y);
int32_t map_routing_calculate_distances_water_flotsam(int32_t x, int32_t y);
int32_t map_routing_calculate_distances_water_ferry(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_get_path_on_water(int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t is_flotsam);
int32_t map_routing_get_path_warship(int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t straight_length);
int32_t map_routing_get_path_on_water_4dir(
    int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t is_flotsam, int32_t *directions);

#endif
