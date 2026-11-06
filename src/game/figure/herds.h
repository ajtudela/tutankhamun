// The herd functions of figure/herds.c: the update of every herd formation and the scaring of a herd by a building
// placed within HERD_SCARE_DISTANCE tiles.

#ifndef FIGURE_HERDS_H
#define FIGURE_HERDS_H

#include <stdint.h>

enum herd_timing {
    HERD_SPAWNED_WAIT_TICKS = 10,
    HERD_SCARED_WAIT_TICKS = 50,
};

enum herd_range {
    HERD_SCARE_DISTANCE = 7,
};

void formation_herds_scared_by_building(int32_t x, int32_t y);
void formation_herd_update(void);

#endif
