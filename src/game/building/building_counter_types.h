// The types of the building counters: the active buildings per type (the venue slots count venues by the show they
// give) and the block of ten building counters cleared at the start of a game.
//
// Nine of the ten counters are never read; they keep their place in the save.

#ifndef BUILDING_BUILDING_COUNTER_TYPES_H
#define BUILDING_BUILDING_COUNTER_TYPES_H

#include "building/building_type.h"
#include <stdint.h>

typedef int32_t building_type_counts[BUILDING_TYPE_COUNT];

typedef struct building_counter_block {
    int32_t unread_reset_a[2];
    int32_t unread_a[3];
    int32_t unread_reset_b[2];
    int32_t unread_b[2];
    int32_t figure_flash_tick;
} building_counter_block;

#endif
