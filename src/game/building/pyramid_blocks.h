// Block construction of the pyramid family: the work of stonemasons, bricklayers and carpenters, the blocks, the
// casing, the ramps and the temple of the complex; whether the city can supply what the next block needs.

#ifndef BUILDING_PYRAMID_BLOCKS_H
#define BUILDING_PYRAMID_BLOCKS_H

#include "building/building.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include <stdbool.h>
#include <stdint.h>

static inline int32_t pyramid_part_count_of_size(int32_t size)
{
    return (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_count[size] : pyramid_part_count[size];
}

int32_t pyramid_part_has_active_worker(int32_t building_id);
int32_t pyramid_complex_temple_free_tile(const building *main, int32_t size);
void pyramid_part_place_block(int32_t building_id, int32_t figure_id);
void pyramid_part_add_block_instant(int32_t building_id);
void pyramid_part_build_ramp(int32_t building_id);
void pyramid_carpenter_work_tick(int32_t building_id, int32_t figure_id);
void pyramid_find_frontier_parts(int32_t building_id);
void pyramid_sort_frontier_parts(void);
int32_t pyramid_part_needs_main_material(int32_t building_id);
int32_t pyramid_find_block_task(int32_t figure_id, int32_t is_mason);
int32_t pyramid_find_task_stonemason(int32_t figure_id);
int32_t pyramid_find_task_bricklayer(int32_t figure_id);
int32_t pyramid_find_ramp_task(int32_t figure_id);
void pyramid_check_site_prepared(int32_t building_id);
void pyramid_start_block_phase(int32_t building_id);
bool pyramid_blocks_finished(int32_t building_id);
int32_t pyramid_is_complete(int32_t building_id);
int32_t pyramid_has_ramp_work(int32_t building_id);
void pyramid_build_complex_temple(int32_t building_id, int32_t figure_id, uint8_t next_action);
bool monument_resource_available(int32_t resource, int32_t amount);
bool pyramid_can_build_next(int32_t def, pyramid_progress progress, int32_t main_id);

#endif
