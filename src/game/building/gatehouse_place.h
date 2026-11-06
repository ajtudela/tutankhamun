// Gatehouses on the build tool (building/gatehouse_place.c) and the dock site check.

#ifndef BUILDING_GATEHOUSE_PLACE_H
#define BUILDING_GATEHOUSE_PLACE_H

#include "building/gatehouse_types.h"
#include <stdbool.h>
#include <stdint.h>

enum gatehouse_image {
    GATEHOUSE_TOWER_PLACEHOLDER_GROUP = 430,
};

enum dock_site {
    DOCK_SIZE = 3,
};

void building_gatehouse_mark_construction(int32_t x, int32_t y, int32_t orientation);
int32_t building_dock_is_connected_to_open_water(int32_t x, int32_t y);
bool building_gatehouse_site_fits(int32_t origin_x, int32_t origin_y, int32_t orientation);
void building_gatehouse_draw_flat_tiles(int32_t x0, int32_t y0, int32_t orientation);
void city_building_ghost_draw_gatehouse(void);

#endif
