// The ferry landing's ghost and the route fan drawn from a landing.

#ifndef WIDGET_CONSTRUCTION_GHOST_FERRY_H
#define WIDGET_CONSTRUCTION_GHOST_FERRY_H

#include <stdbool.h>
#include <stdint.h>

void city_building_ghost_draw_ferry(void);
bool city_building_ghost_draw_ferry_route(int32_t direction, int32_t x, int32_t y, int32_t px0, int32_t py0, int32_t length);

#endif
