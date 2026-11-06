// Bridges placed with the build tool.

#ifndef BUILDING_CONSTRUCTION_BRIDGE_H
#define BUILDING_CONSTRUCTION_BRIDGE_H

#include <stdint.h>

enum bridge_min_length {
    BRIDGE_MIN_LENGTH = 2,
    SHIP_BRIDGE_MIN_LENGTH = 5,
};

void map_bridge_add(int32_t x, int32_t y, int32_t is_ship_bridge);

#endif
