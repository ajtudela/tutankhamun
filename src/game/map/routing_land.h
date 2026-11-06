// Land searches by a figure's travel mode and their multi-target variants (map/routing_land.c): a single-target search
// returns 1 when the destination was reached, a multi-target one the 1-based index of the target reached, or 0.

#ifndef MAP_ROUTING_LAND_H
#define MAP_ROUTING_LAND_H

#include <stdint.h>

int32_t map_routing_citizen_can_travel_over_land_prefer_roads(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_animal_can_travel(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_amphibia_can_travel(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_citizen_can_travel_over_road_garden_ext(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_citizen_can_travel_over_land_no_unstaffed_ferry(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_citizen_can_travel_no_ferry(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_citizen_can_travel_over_land_fort(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_can_travel_anywhere(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);

int32_t map_routing_citizen_find_first_of_any(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);
int32_t map_routing_citizen_find_first_of_no_ferry(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);
int32_t map_routing_citizen_find_first_of_prefer_roads(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);
int32_t map_routing_citizen_find_first_of_road_garden(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);
int32_t map_routing_citizen_find_first_of_fort(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count);

#endif
