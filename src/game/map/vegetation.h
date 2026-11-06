// Vegetation (map/vegetation.c): the daily regrowth and the reach search of the gatherers.

#ifndef MAP_VEGETATION_H
#define MAP_VEGETATION_H

#include <stdint.h>

enum vegetation_growth {
    VEGETATION_FULLY_GROWN = 0xFF,
};

enum vegetation_growth_random {
    VEGETATION_RANDOM_GROWTH_RANGE = 10,
};

enum vegetation_growth_percentage {
    VEGETATION_GROWTH_NORMAL_PERCENT = 100,
    VEGETATION_GROWTH_OSIRIS_PERCENT = 125,
};

enum vegetation_candidates {
    VEGETATION_CANDIDATES_MAX = 3999,
};

void map_vegetation_growth_update(void);
int32_t map_vegetation_find_reachable(int32_t x, int32_t y, uint32_t mask, int32_t x0, int32_t y0, int32_t x1, int32_t y1);
int32_t map_vegetation_find_reachable_all(int32_t x, int32_t y, uint32_t mask);

#endif
