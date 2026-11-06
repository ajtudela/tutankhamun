// Shared by figure/route.c and the figure files that walk routes: the travel test type of route.c and the ferry record
// sizes.

#ifndef FIGURE_MOVEMENT_INTERNAL_H
#define FIGURE_MOVEMENT_INTERNAL_H

#include "figure/figure.h"
#include "figure/movement.h"
#include "figure/service.h"
#include "map/routing.h"
#include "map/routing_enemy.h"
#include "map/routing_water.h"
#include <stdint.h>

typedef int32_t (*travel_function)(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y);

enum ferry_records {
    FERRY_PATH_RECORD_COUNT = 50,
    FERRY_PATH_FIGURES = 4,
};

#endif
