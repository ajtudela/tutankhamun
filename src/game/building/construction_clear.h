// Clearing land (building/construction_clear.c): the clear land tool and its popup callbacks, the answers, the
// part limit and the images a cut canal's neighbours get.

#ifndef BUILDING_CONSTRUCTION_CLEAR_H
#define BUILDING_CONSTRUCTION_CLEAR_H

#include <stdint.h>

enum clear_land_answer {
    CLEAR_LAND_NOT_ASKED = 0,
    CLEAR_LAND_YES = 1,
    CLEAR_LAND_NO = -1,
};

enum clear_land_limit {
    CLEAR_LAND_MAX_PARTS = 525,
    CLEAR_LAND_ROAD_UPDATE_RADIUS = 5,
};

enum canal_neighbour_image {
    CANAL_NEIGHBOUR_END_ABOVE_GAP = 1,
    CANAL_NEIGHBOUR_END_RIGHT_OF_GAP = 2,
    CANAL_NEIGHBOUR_END_BELOW_GAP = 3,
    CANAL_NEIGHBOUR_END_LEFT_OF_GAP = 4,
    CANAL_NEIGHBOUR_STRAIGHT_ALONG_Y = 5,
    CANAL_NEIGHBOUR_STRAIGHT_ALONG_X = 6,
};

void building_construction_clear_land(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
void clear_land_confirm_monuments(void);
void clear_land_confirm_complexes(void);
void clear_land_confirm_forts(void);
void clear_land_confirm_bridges(void);

#endif
