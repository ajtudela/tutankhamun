// Access to the building records: building_get, a monument part's main part and the check that a run of free
// records exists. The record itself is in building/building_types.h.

#ifndef BUILDING_BUILDING_H
#define BUILDING_BUILDING_H

#include "building/building_data.h"
#include "building/building_types.h"
#include <stddef.h>
#include <stdint.h>

static inline building *building_get(int32_t id)
{
    return &city_buildings.records[id];
}

static inline int32_t monument_main_part_id(int32_t id)
{
    return id - building_get(id)->subtype;
}

static inline int32_t building_test_free_records(int32_t count)
{
    int32_t run = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        run = building_get(id)->state == 0 ? run + 1 : 0;
        if (run >= count) {
            return 1;
        }
    }
    return 0;
}

#endif
