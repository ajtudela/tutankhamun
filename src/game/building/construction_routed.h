// Routed construction of roads and canals and the kinds it places, and the canal between two water lifts.

#ifndef BUILDING_CONSTRUCTION_ROUTED_H
#define BUILDING_CONSTRUCTION_ROUTED_H

#include "map/terrain.h"
#include <stdint.h>

enum routed_building {
    ROUTED_BUILDING_ROAD = 0,
    ROUTED_BUILDING_PATH_ONLY = 1,
    ROUTED_BUILDING_COUNT_ONLY = 2,
    ROUTED_BUILDING_CANAL = 3,
};

enum routed_terrain {
    ROUTED_TERRAIN_NOT_ALLOWED = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_RUBBLE |
        TERRAIN_WALL | TERRAIN_MARSH | TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_DUNE | TERRAIN_DEEPWATER | TERRAIN_ROCK_NEAR_MONUMENT,
};

enum routed_limit {
    ROUTED_DIRECTION_CHOICES = 4,
    ROUTED_MAX_TILES = 400,
    ROUTED_COST_NONE = 10000,
};

enum water_lift_end {
    WATER_LIFT_END_BLOCKED = -1,
    WATER_LIFT_END_NONE = 0,
    WATER_LIFT_END_NEW = 1,
    WATER_LIFT_END_EXISTING = 2,
};

enum water_lift_connector {
    WATER_LIFT_SIZE = 3,
    WATER_LIFT_CANAL_ENDS = 4,
    WATER_LIFT_NEW_PAIR_MIN_LENGTH = 3,
    WATER_LIFT_SITE_TERRAIN = 0xFFFF,
};

int32_t place_routed_building(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type);
void building_construction_place_road(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
void building_construction_place_canal(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
int32_t construction_is_water_lift(int32_t offset);
int32_t building_construction_place_canal_for_connector(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);

#endif
