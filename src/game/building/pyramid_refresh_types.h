// The pyramid complex refresh queue: the main parts whose complex area is redrawn once per tick.
//
// Quirk kept: the queue's last entry is also the y origin of the frontier part sort, so a fourth queued
// pyramid and a sort overwrite each other.

#ifndef BUILDING_PYRAMID_REFRESH_TYPES_H
#define BUILDING_PYRAMID_REFRESH_TYPES_H

#include <stdint.h>

enum pyramid_complex_refresh {
    PYRAMID_COMPLEX_REFRESH_QUEUE = 4,
};

typedef struct {
    int32_t queue_head[PYRAMID_COMPLEX_REFRESH_QUEUE - 1];
    int32_t sort_origin_y;
} pyramid_refresh_sort_slots;

typedef union {
    int32_t queue[PYRAMID_COMPLEX_REFRESH_QUEUE];
    pyramid_refresh_sort_slots sort;
} pyramid_complex_refresh_slots;

#endif
