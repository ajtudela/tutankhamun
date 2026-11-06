// The service visit grid: per tile ten 3-bit counters, one per service, raised by passing service walkers and decayed
// every 20th day.
//
// A counter is cleared with the mask (7 << shift) ^ 0xFFFF, so any change of a counter also clears bits 16 and up.

#include "map/service_visit.h"

#include "building/building_data.h"
#include "map/map_data.h"

#include <stdint.h>

int32_t service_visit_decay_counter;

static int32_t counter_shift(int32_t service)
{
    return (service * SERVICE_VISIT_COUNTER_BITS) & SERVICE_VISIT_SHIFT_MASK;
}

static uint32_t clear_counter_and_high_half(uint32_t value, int32_t shift)
{
    return ((SERVICE_VISIT_COUNTER_MAX << shift) ^ 0xFFFFu) & value;
}

void map_service_visit_decay(void)
{
    if (++service_visit_decay_counter < SERVICE_VISIT_DECAY_DAYS) {
        return;
    }
    service_visit_decay_counter = 0;
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        if (map_row_x_start[y] >= map_row_x_end[y]) {
            continue;
        }
        uint32_t *tile = &map_service_visit_grid[map_row_start_offset[y]];
        for (int32_t n = map_row_x_end[y] - map_row_x_start[y]; n > 0; n--, tile++) {
            uint32_t value = *tile;
            if (value) {
                for (int32_t shift = 0; shift < (SERVICE_VISIT_COUNTERS * SERVICE_VISIT_COUNTER_BITS); shift += SERVICE_VISIT_COUNTER_BITS) {
                    uint32_t counter = (value >> shift) & SERVICE_VISIT_COUNTER_MAX;
                    if (counter) {
                        value = clear_counter_and_high_half(value, shift) | ((counter - 1) << shift);
                    }
                }
            }
            *tile = value;
        }
    }
}

int32_t map_service_visit_get(int32_t service, int32_t grid_offset)
{
    return (int32_t) ((map_service_visit_grid[grid_offset] >> counter_shift(service)) & SERVICE_VISIT_COUNTER_MAX);
}

void map_service_visit_increment(int32_t service, int32_t grid_offset)
{
    int32_t shift = counter_shift(service);
    uint32_t value = map_service_visit_grid[grid_offset];
    uint32_t counter = (value >> shift) & SERVICE_VISIT_COUNTER_MAX;
    if (counter < SERVICE_VISIT_COUNTER_MAX) {
        map_service_visit_grid[grid_offset] = clear_counter_and_high_half(value, shift) | ((counter + 1) << shift);
    }
}
