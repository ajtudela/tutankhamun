// The service visit grid (map/service_visit.c): the counters of a tile, their increment and decay.

#ifndef MAP_SERVICE_VISIT_H
#define MAP_SERVICE_VISIT_H

#include <stdint.h>

enum service_visit_counter {
    SERVICE_VISIT_COUNTERS = 10,
    SERVICE_VISIT_COUNTER_BITS = 3,
    SERVICE_VISIT_COUNTER_MAX = 7,
    SERVICE_VISIT_SHIFT_MASK = 31,
};

enum service_visit_decay {
    SERVICE_VISIT_DECAY_DAYS = 20,
};

void map_service_visit_increment(int32_t service, int32_t grid_offset);
int32_t map_service_visit_get(int32_t service, int32_t grid_offset);
void map_service_visit_decay(void);

#endif
