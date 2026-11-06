// The pyramid family's style of the scenario and whether a part still needs its ramp.

#ifndef BUILDING_PYRAMID_H
#define BUILDING_PYRAMID_H

#include <stdint.h>

enum pyramid_ramp_need {
    PYRAMID_RAMP_NONE = 0,
    PYRAMID_RAMP_FIRST = 1,
    PYRAMID_RAMP_SECOND = 2,
};

int32_t scenario_get_pyramid_style(void);
int32_t pyramid_part_needs_ramp(int32_t building_id);

#endif
