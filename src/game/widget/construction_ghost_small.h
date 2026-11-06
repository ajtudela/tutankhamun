// The functions of construction_ghost_small.c that other files call.

#ifndef WIDGET_CONSTRUCTION_GHOST_SMALL_H
#define WIDGET_CONSTRUCTION_GHOST_SMALL_H

#include <stdint.h>

int32_t city_view_relative_direction(int32_t map_direction);
void city_building_ghost_draw_shore2(int32_t unused, int32_t allow);
void city_building_ghost_draw_shore3(int32_t group);
void city_building_ghost_draw_single(void);

#endif
