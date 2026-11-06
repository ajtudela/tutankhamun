// The daily desirability updates (map/desirability.c): from buildings and from terrain.

#ifndef MAP_DESIRABILITY_H
#define MAP_DESIRABILITY_H

#include <stdint.h>

enum desirability_limit {
    DESIRABILITY_MIN = -100,
    DESIRABILITY_MAX = 100,
};

enum desirability_rubble {
    DESIRABILITY_RUBBLE_VALUE = -2,
    DESIRABILITY_RUBBLE_STEP = 1,
    DESIRABILITY_RUBBLE_STEP_SIZE = 1,
    DESIRABILITY_RUBBLE_RANGE = 2,
};

void map_desirability_update_buildings(void);
void map_desirability_update_terrain(void);

#endif
