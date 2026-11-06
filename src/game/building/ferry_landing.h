// Ferry landings (building/ferry_landing.c): placement states and the ferry queue records.

#ifndef BUILDING_FERRY_LANDING_H
#define BUILDING_FERRY_LANDING_H

#include "building/ferry_queue_types.h"
#include "figure/boats.h"
#include <stdint.h>

enum ferry_placement_state {
    FERRY_PLACEMENT_NONE = 0,
    FERRY_PLACEMENT_FIRST_PLACED = 1,
    FERRY_PLACEMENT_FIRST_EXISTING = 2,
    FERRY_PLACEMENT_PAIR_LINKED = 3,
};

static inline ferry_queue *ferry_queue_get(int32_t index)
{
    return &ferry_queues[index];
}

void construction_ferry_landing_placed(int32_t building_id);
void construction_ferry_cancel(int32_t building_id);
int32_t building_ferry_record_create(int32_t building_id);
void ferry_queues_clear_all(void);
void ferry_paths_clear_all(void);
void building_ferry_records_update(void);

#endif
