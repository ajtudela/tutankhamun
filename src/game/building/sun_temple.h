// The sun temple (building/sun_temple.c): its parts, stages, progress report and stone blocks.

#ifndef BUILDING_SUN_TEMPLE_H
#define BUILDING_SUN_TEMPLE_H

#include <stdint.h>

enum sun_temple_parts {
    SUN_TEMPLE_PARTS = 100,
};

enum sun_temple_blocks {
    SUN_TEMPLE_STAGES = 14,
    SUN_TEMPLE_BLOCKS = 4,
};

typedef struct {
    int32_t x;
    int32_t y;
} sun_temple_block_offset;

extern int32_t sun_temple_stage_blocks[SUN_TEMPLE_STAGES];
extern sun_temple_block_offset sun_temple_block_offsets[SUN_TEMPLE_BLOCKS];

enum sun_temple_work_flag {
    SUN_TEMPLE_TEMPLE_DONE = 1,
    SUN_TEMPLE_KIND8_STARTED = 2,
    SUN_TEMPLE_KIND1_DONE = 4,
    SUN_TEMPLE_KIND2_DONE = 8,
    SUN_TEMPLE_KIND8_DONE = 16,
    SUN_TEMPLE_KIND12_DONE = 32,
};

enum sun_temple_stage {
    SUN_TEMPLE_PART_STARTED = 1,
    SUN_TEMPLE_PART_DONE = 2,
    SUN_TEMPLE_LAST_PART_DONE = 4,
    SUN_TEMPLE_STAGE_COMPLETE = 13,
    SUN_TEMPLE_PART_KIND_LAST = 12,
    SUN_TEMPLE_LABORER_WORK_TICKS = 50,
};

enum sun_temple_status {
    SUN_TEMPLE_STATUS_SITE = 1,
    SUN_TEMPLE_STATUS_TEMPLE_STARTED = 2,
    SUN_TEMPLE_STATUS_TEMPLE_RISING = 3,
    SUN_TEMPLE_STATUS_TEMPLE_DONE = 4,
};

enum sun_temple_progress_out {
    SUN_TEMPLE_PROGRESS_STATUS = 0,
    SUN_TEMPLE_PROGRESS_AT_STAGE_0 = 1,
    SUN_TEMPLE_PROGRESS_TEMPLE_LEFT = 2,
    SUN_TEMPLE_PROGRESS_KIND1_LEFT = 3,
    SUN_TEMPLE_PROGRESS_KIND2_LEFT = 4,
    SUN_TEMPLE_PROGRESS_KIND8_LEFT = 5,
    SUN_TEMPLE_PROGRESS_KIND12_LEFT = 6,
    SUN_TEMPLE_PROGRESS_STONEMASONS = 7,
    SUN_TEMPLE_PROGRESS_STONEMASONS_WAITING = 8,
    SUN_TEMPLE_PROGRESS_LABORERS = 9,
    SUN_TEMPLE_PROGRESS_CARPENTERS = 10,
    SUN_TEMPLE_PROGRESS_MATERIAL = 11,
    SUN_TEMPLE_PROGRESS_PERCENT = 12,
    SUN_TEMPLE_PROGRESS_MATERIAL_TOTAL = 13,
    SUN_TEMPLE_PROGRESS_SLOTS = 14,
    SUN_TEMPLE_WORK_UNITS = 213,
    SUN_TEMPLE_MATERIAL_TOTAL = 38000,
    SUN_TEMPLE_BLOCK_IMAGE_A = 13,
    SUN_TEMPLE_BLOCK_IMAGE_B = 14,
};

enum sun_temple_part_kind {
    SUN_TEMPLE_PART_SIDE_ROW = 6,
};

void sun_temple_draw_stage_blocks(int32_t building_id, int32_t x, int32_t y);
int32_t sun_temple_part_size(int32_t building_id);
int32_t sun_temple_get_progress(int32_t building_id, int32_t *out);
int32_t sun_temple_has_free_spot_for_type(int32_t figure_type, int32_t stage);
void sun_temple_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t next_action);
int32_t sun_temple_find_work_for_figure(int32_t figure_id);

void sun_temple_place(int32_t x, int32_t y, int32_t building_id);
void sun_temple_update_image(int32_t building_id);
void sun_temple_reset_work_spots(void);
void sun_temple_draw_spots(int32_t x, int32_t y);

#endif
