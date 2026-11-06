// The records of the clear land tool kept in globals: the answers of its confirmation popups and the rectangle
// being cleared, kept for the clear that runs again after an answer.

#ifndef BUILDING_CLEAR_LAND_TYPES_H
#define BUILDING_CLEAR_LAND_TYPES_H

#include <stdint.h>

typedef struct clear_land_confirmations {
    int32_t complexes;
    int32_t monuments;
    int32_t bridges;
    int32_t forts;
    int32_t reserved_1[2];
} clear_land_confirmations;

typedef struct clear_land_rectangle {
    int32_t y_end;
    int32_t x_end;
    int32_t y_start;
    int32_t x_start;
} clear_land_rectangle;

#endif
