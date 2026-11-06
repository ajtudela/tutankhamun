// Distance maps over the whole reachable area and of routed construction (roads, canals, walls), and the checks where
// a road may cross a canal (map/routing_distance.c).

#ifndef MAP_ROUTING_DISTANCE_H
#define MAP_ROUTING_DISTANCE_H

#include <stdint.h>

enum {
    ROUTED_MODE_ROAD = 0,
    ROUTED_MODE_CANAL = 1,
};

enum canal_road_axis {
    CANAL_ROAD_AXIS_EAST_WEST = 0,
    CANAL_ROAD_AXIS_NORTH_SOUTH = 1,
    CANAL_ROAD_AXIS_NONE = 2,
};

enum routed_rule {
    ROUTED_CANAL_ROAD_PIECES = 48,
    ROUTED_MAP_EDGE_MARGIN = 3,
    ROUTED_EXTRA_COST_DISTANCE = 4,
    ROUTED_DISTANCE_REJECTED = -1,
};

void map_routing_calculate_distances(int32_t x, int32_t y);
int32_t map_routing_calculate_distances_for_wall(int32_t x, int32_t y);
int32_t map_routing_calculate_distances_for_building(int32_t x, int32_t y, int32_t mode);
int32_t map_routing_calculate_distances_for_building_checked(int32_t x, int32_t y, int32_t mode);

int32_t map_can_place_road_under_canal(int32_t grid_offset);
int32_t map_can_place_road_on_canal_object(int32_t grid_offset);
int32_t map_can_place_canal_on_road(int32_t grid_offset);

#endif
