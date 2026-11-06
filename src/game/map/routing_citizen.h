// Citizen routing over land (map/routing_citizen.c), the wall and canal deletion and the canal image helpers.

#ifndef MAP_ROUTING_CITIZEN_H
#define MAP_ROUTING_CITIZEN_H

#include <stdint.h>

enum canal_image {
    CANAL_IMAGE_STRAIGHT_A = 48,
    CANAL_IMAGE_STRAIGHT_B = 50,
    CANAL_IMAGE_STRAIGHT_C = 52,
    CANAL_IMAGE_STRAIGHT_D = 54,
    CANAL_IMAGE_ROAD_CROSSING_A = 84,
    CANAL_IMAGE_ROAD_CROSSING_B = 85,
    CANAL_IMAGE_ROAD_CROSSING_C = 86,
    CANAL_IMAGE_ROAD_CROSSING_D = 87,
    CANAL_IMAGE_FLOODPLAIN_OFFSET = 21,
};

int32_t map_routing_citizen_can_travel_over_land(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_routing_delete_first_wall_or_canal(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);
int32_t map_canal_with_road_image(int32_t image_offset);
int32_t map_canal_floodplain_image(int32_t image_id);
void map_routing_clear_land_citizen(void);

#endif
