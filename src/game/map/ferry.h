// Ferry landing checks of construction (map/ferry.c): a landing at a tile and a water route between two landings.

#ifndef MAP_FERRY_H
#define MAP_FERRY_H

#include <stdint.h>

enum ferry_landing_check {
    FERRY_LANDING_NOT_HERE = 0,
    FERRY_LANDING_HERE = 1,
    FERRY_LANDING_OUTSIDE_MAP = 999,
};

enum ferry_route_path {
    FERRY_ROUTE_PATH_ID = 1001,
};

enum ferry_route_fan {
    FERRY_ROUTE_FAN_HALF_WIDTH_MAX = 5,
};

int32_t map_is_ferry_landing(int32_t grid_offset);
int32_t map_ferry_route_exists(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t size, int32_t direction0, int32_t direction1, int32_t *directions, int32_t *path_length);
int32_t map_ferry_landing_at(int32_t x, int32_t y);

#endif
