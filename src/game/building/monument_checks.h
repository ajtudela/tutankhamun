// Monument checks of the build tool (building/monument_checks.c): the mausoleum and sun temple sites, the
// finished sun temple and the pyramid presets.

#ifndef BUILDING_MONUMENT_CHECKS_H
#define BUILDING_MONUMENT_CHECKS_H

#include <stdbool.h>
#include <stdint.h>

void pyramid_init_preset_parts(int32_t building_id);
bool sun_temple_tile_is_free(int32_t grid_offset);
bool sun_temple_any_completed(void);
bool mausoleum_can_place(int32_t grid_offset);

#endif
