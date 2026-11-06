// The mausoleum (building/mausoleum.c): its parts, workers, progress and the tile where its workers enter.

#ifndef BUILDING_MAUSOLEUM_H
#define BUILDING_MAUSOLEUM_H

#include "building/monument_object.h"
#include "building/monument_progress_types.h"
#include "map/terrain.h"
#include <stdbool.h>
#include <stdint.h>

enum mausoleum_parts {
    MAUSOLEUM_PARTS = 116,
    MAUSOLEUM_BLOCK_KINDS = 4,
    MAUSOLEUM_WORK_UNITS = 288,
};

enum mausoleum_part_stage {
    MAUSOLEUM_GROUND_UNTOUCHED = 0,
    MAUSOLEUM_GROUND_LEVELED = 1,
    MAUSOLEUM_GROUND_FINISHED = 2,
    MAUSOLEUM_BLOCK_SUBTILES = 16,
    MAUSOLEUM_BLOCK_LOW_DONE_MASK = 0xFF,
    MAUSOLEUM_BLOCK_FULL = 0xFFFF,
    MAUSOLEUM_PAIR_LOW_SUBTILES = 0x3,
    MAUSOLEUM_PAIR_HIGH_SUBTILES = 0x300,
    MAUSOLEUM_WORK_TICKS = 50,
};

enum mausoleum_part_kind {
    MAUSOLEUM_KIND_MASK = 0xFF,
    MAUSOLEUM_KIND_GROUND_FIRST = 4,
    MAUSOLEUM_KIND_BLOCK_MASK = 0xF00,
    MAUSOLEUM_KIND_BLOCK_0 = 0x100,
    MAUSOLEUM_KIND_BLOCK_1 = 0x200,
    MAUSOLEUM_KIND_BLOCK_2 = 0x400,
    MAUSOLEUM_KIND_BLOCK_3 = 0x800,
    MAUSOLEUM_KIND_PAIR_MASK = 0xF000,
    MAUSOLEUM_KIND_PAIR_0 = 0x1000,
    MAUSOLEUM_KIND_PAIR_1 = 0x2000,
    MAUSOLEUM_KIND_PAIR_2 = 0x4000,
    MAUSOLEUM_KIND_PAIR_3 = 0x8000,
    MAUSOLEUM_KIND_WALKWAY = 0x10000,
};

enum mausoleum_layout {
    MAUSOLEUM_BOX_WIDTH = 22,
    MAUSOLEUM_BOX_DEPTH = 8,
    MAUSOLEUM_ENTRY_DX = 22,
    MAUSOLEUM_ENTRY_DY = 4,
    MAUSOLEUM_SAVED_ORIENTATION_SHIFT = 13,
};

enum mausoleum_status {
    MAUSOLEUM_STATUS_SITE = 1,
    MAUSOLEUM_STATUS_STARTED = 2,
    MAUSOLEUM_STATUS_BLOCKS_RISING = 3,
};

enum mausoleum_ground_image {
    MAUSOLEUM_GROUND_IMAGE_SIDES = 4,
    MAUSOLEUM_GROUND_IMAGE_ENDS = 8,
};

enum mausoleum_terrain {
    MAUSOLEUM_TERRAIN_CLEARED_ON_WALKWAY = TERRAIN_TREE | TERRAIN_MEADOW | TERRAIN_WELL_RANGE | TERRAIN_FLOODPLAIN | TERRAIN_IRRIGATION_RANGE,
    MAUSOLEUM_TERRAIN_CLEARED = MAUSOLEUM_TERRAIN_CLEARED_ON_WALKWAY | TERRAIN_GROUNDWATER,
};

enum mausoleum_main_flags {
    MAUSOLEUM_BLOCKS_LOW_DONE = 1,
    MAUSOLEUM_BLOCKS_FULL = 2,
    MAUSOLEUM_GROUND_STARTED = 4,
    MAUSOLEUM_GROUND_DONE = 8,
    MAUSOLEUM_BLOCK_FINISHED_FIRST = 0x10,
    MAUSOLEUM_BLOCKS_FINISHED = 0xF0,
    MAUSOLEUM_MAIN_FLAGS_ALL = 0xFF,
};

enum mausoleum_image_group {
    MAUSOLEUM_IMAGE_GROUP_BLOCK_FIRST = 564,
    MAUSOLEUM_IMAGE_GROUP_BLOCK_DONE_FIRST = 786,
};

int32_t mausoleum_get_progress(int32_t building_id, mausoleum_info_progress *out);
void mausoleum_get_part_desc(monument_part_desc *out, int32_t part, int32_t view, int32_t orientation);
bool mausoleum_part_has_active_worker(int32_t building_id);
bool mausoleum_check_work_for_figure(int32_t building_id, int32_t figure_id, bool assign);
void mausoleum_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t mausoleum_find_work_for_figure(int32_t figure_id);
void mausoleum_update_image(int32_t building_id);
void mausoleum_part_update_position(int32_t building_id);
int32_t mausoleum_get_image(int32_t building_id, int32_t *flags);
void mausoleum_get_work_tile(int32_t building_id, int32_t *x, int32_t *y);
void mausoleum_place(int32_t x, int32_t y, int32_t building_id, bool no_slot);
void mausoleum_get_entry_tile(int32_t building_id, int32_t *x, int32_t *y);

#endif
