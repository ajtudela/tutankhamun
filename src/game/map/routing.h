// Routing entry points of the figures and of construction: the friendly fighting check, the general direction between
// two tiles, the citizen land searches and the path taken from the distance map.

#ifndef MAP_ROUTING_H
#define MAP_ROUTING_H

#include <stdint.h>

int32_t map_routing_no_fighting_friendly(int32_t grid_offset);
int32_t calc_general_direction(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to);
int32_t map_routing_citizen_can_travel_over_land_any(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_citizen_can_travel_over_road_garden(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_get_path(int32_t num_directions, int32_t path_id, int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);

#endif
