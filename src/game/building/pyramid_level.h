// Site preparation and the god speed-up of the pyramid family (building/pyramid_level.c).

#ifndef BUILDING_PYRAMID_LEVEL_H
#define BUILDING_PYRAMID_LEVEL_H

#include "building/pyramid_level_types.h"
#include <stdbool.h>
#include <stdint.h>

enum pyramid_level_stage {
    PYRAMID_STAGE_LEVELED = 4,
    PYRAMID_STAGE_LEVEL_1 = 5,
    PYRAMID_STAGE_LEVEL_2 = 6,
    PYRAMID_STAGE_LEVEL_3 = 7,
    PYRAMID_STAGE_LEVEL_4 = 8,
    PYRAMID_STAGE_LEVEL_5 = 9,
    PYRAMID_STAGE_LEVEL_6 = 10,
    PYRAMID_STAGE_LEVEL_TOP = 11,
};

enum pyramid_level_quadrants {
    PYRAMID_PART_QUADRANTS = 4,
    PYRAMID_PREP_ALL_QUADRANTS = 15,
};

enum pyramid_level_metric {
    PYRAMID_LEVEL_NEAR_MAIN_PART = 0,
    PYRAMID_LEVEL_NEAR_REFERENCE_PART = 1,
    PYRAMID_LEVEL_FAR_FROM_REFERENCE_PART = 2,
    PYRAMID_LEVEL_FAR_METRIC_BASE = 1000,
    PYRAMID_LEVEL_NO_METRIC = 99999,
};

enum pyramid_level_neighbours {
    PYRAMID_LEVEL_ALL_NEIGHBOURS = 0xFF,
    PYRAMID_LEVEL_EDGE_NEIGHBOURS = 0x55,
    PYRAMID_LEVEL_STAGE_1_VARIANTS = 8,
    PYRAMID_LEVEL_TILE_WORK_TICKS = 50,
};

void pyramid_part_update_level_images(int32_t building_id, int32_t size);
bool pyramid_level_tile_set_image(int32_t building_id, int32_t grid_offset, int32_t stage, bool redraw);
int32_t pyramid_level_stage_next(int32_t stage);
bool pyramid_laborer_take_level_tile(int32_t figure_id);
bool pyramid_laborer_level_work(int32_t figure_id);
bool pyramid_find_level_tile(int32_t main_id, int32_t *x, int32_t *y, int32_t check_workers);
int32_t pyramid_level_tile_done(int32_t x, int32_t y);
bool pyramid_level_one_tile_instant(int32_t main_id);
int32_t pyramid_tile_effective_stage(int32_t grid_offset);
int32_t pyramid_level_tile_work_time(int32_t grid_offset);
void pyramid_speedup_start(int32_t multiplier);
int32_t pyramid_speedup_level(void);
void pyramid_speedup_tick(void);

#endif
