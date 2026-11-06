// Bridges and ferry routes (map/bridge.c): measuring a bridge and a ferry route, removing a bridge and counting the
// figures on it.

#ifndef MAP_BRIDGE_H
#define MAP_BRIDGE_H

#include <stdint.h>

enum bridge_sprite {
    BRIDGE_SPRITE_NONE = 0,
    BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT = 1,
    BRIDGE_SPRITE_LOW_SPAN_ALONG_X = 2,
    BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT = 3,
    BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM = 4,
    BRIDGE_SPRITE_LOW_SPAN_ALONG_Y = 5,
    BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP = 6,
    BRIDGE_SPRITE_SHIP_RAMP_DOWN_BOTTOM = 7,
    BRIDGE_SPRITE_SHIP_RAMP_DOWN_LEFT = 8,
    BRIDGE_SPRITE_SHIP_RAMP_DOWN_TOP = 9,
    BRIDGE_SPRITE_SHIP_RAMP_DOWN_RIGHT = 10,
    BRIDGE_SPRITE_SHIP_SPAN_ALONG_Y = 11,
    BRIDGE_SPRITE_SHIP_SPAN_ALONG_X = 12,
    BRIDGE_SPRITE_SHIP_SECOND_TILE = 13,
    BRIDGE_SPRITE_SHIP_PILLAR_ALONG_Y = 14,
    BRIDGE_SPRITE_SHIP_PILLAR_ALONG_X = 15,
};

enum bridge_limit {
    BRIDGE_MAX_OPEN_WATER_TILES = 4,
};

enum ferry_route_limit {
    FERRY_ROUTE_MAX_LENGTH = 40,
};

int32_t map_bridge_count_figures(int32_t offset);
void map_bridge_remove(int32_t offset, int32_t mark_deleted);
void map_bridge_calculate_length_direction(int32_t x, int32_t y);
void map_ferry_route_measure(int32_t x, int32_t y);

#endif
