// The moisture map (map/moisture.c): its rebuild over an area, its helpers and the redraw after a pyramid.

#ifndef MAP_MOISTURE_H
#define MAP_MOISTURE_H

#include "map/terrain.h"

#include <stdint.h>

enum moisture_terrain {
    MOISTURE_TERRAIN_WET = TERRAIN_WATER | TERRAIN_MARSH,
    MOISTURE_TERRAIN_FLOODPLAIN_OR_MARSH = TERRAIN_FLOODPLAIN | TERRAIN_MARSH,
    MOISTURE_TERRAIN_NONE = TERRAIN_WATER | TERRAIN_FLOODPLAIN | TERRAIN_MARSH,
};

enum moisture_value {
    MOISTURE_WET = 100,
    MOISTURE_CONTEXT_GROUP_BIT = 0x80,
};

enum moisture_context_range {
    MOISTURE_CONTEXT_LOWEST = 41,
    MOISTURE_CONTEXT_HIGHEST = 90,
    MOISTURE_CONTEXT_COUNT = 26,
    MOISTURE_CONTEXT_GROUP_FULL = 14,
};

enum moisture_reach {
    MOISTURE_REACH_CENTRAL = 46,
    MOISTURE_REACH_NORTHERN = 16,
    MOISTURE_REACH_DESERT = 8,
    MOISTURE_REACH_FLOODPLAIN = 8,
};

enum moisture_shade {
    MOISTURE_SHADE_SPAN_MIN = 12,
    MOISTURE_SHADE_SCALE = 96,
};

void map_pyramid_redraw(void);
int32_t map_moisture_flood_objects_around(int32_t offset);
int32_t map_moisture_terrain_around(int32_t offset, uint32_t mask);
int32_t map_moisture_or_wet(int32_t offset);
int32_t map_moisture_neighbour_shade(int32_t offset, int32_t delta);
int32_t map_moisture_ring_distance(int32_t x, int32_t y, uint32_t mask, int32_t radius);
int32_t map_moisture_distance_shade(int32_t distance, int32_t span);
int32_t map_moisture_neighbours_wet(int32_t offset);
void map_moisture_update_area(int32_t x0, int32_t y0, int32_t x1, int32_t y1);

#endif
