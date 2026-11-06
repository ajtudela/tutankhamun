// The Library of Alexandria (building/library.c): its stages, work, progress report, part images and the image
// steps per part kind, and the methods of its monument object.

#ifndef BUILDING_LIBRARY_H
#define BUILDING_LIBRARY_H

#include "building/monument_object.h"
#include "building/monument_progress_types.h"
#include <stdbool.h>
#include <stdint.h>

enum library_stage {
    LIBRARY_STAGE_STARTED = 1,
    LIBRARY_STAGE_PREPARED = 2,
    LIBRARY_STAGE_BUILDING = 3,
    LIBRARY_STAGE_COPPER_FIRST = 16,
    LIBRARY_STAGE_COPPER_END = 20,
    LIBRARY_STAGE_COMPLETE = 21,
};

enum library_work {
    LIBRARY_FRAME_COUNT = 2,
    LIBRARY_LABORER_WORK_TICKS = 100,
    LIBRARY_MATERIAL_LOADS = 36,
    LIBRARY_WORK_TILE_DX = 3,
    LIBRARY_WORK_TILE_DY = 11,
    LIBRARY_SPOT_DESTINATION_OFFSET = 15,
    LIBRARY_VIEW_SOUTH_FIRST = 2,
};

enum library_part_image {
    LIBRARY_IMAGE_BLANK,
    LIBRARY_IMAGE_LEVELED,
    LIBRARY_IMAGE_FRONT,
    LIBRARY_IMAGE_MAIN_A,
    LIBRARY_IMAGE_MAIN_B,
    LIBRARY_IMAGE_MAIN_C,
    LIBRARY_IMAGE_C4,
    LIBRARY_IMAGE_CC,
    LIBRARY_IMAGE_DC,
    LIBRARY_IMAGE_E4,
    LIBRARY_IMAGE_EC,
    LIBRARY_IMAGE_F4,
    LIBRARY_IMAGE_FC,
    LIBRARY_IMAGE_04,
    LIBRARY_IMAGE_0C,
    LIBRARY_IMAGE_14,
    LIBRARY_IMAGE_1C,
    LIBRARY_IMAGE_24,
    LIBRARY_IMAGE_2C,
    LIBRARY_IMAGE_34,
    LIBRARY_IMAGE_3C,
    LIBRARY_IMAGES,
};

enum library_script {
    LIBRARY_SCRIPT_NONE,
    LIBRARY_SCRIPT_MAIN_A1,
    LIBRARY_SCRIPT_MAIN_A2,
    LIBRARY_SCRIPT_MAIN_A3,
    LIBRARY_SCRIPT_MAIN_A4,
    LIBRARY_SCRIPT_MAIN_A5,
    LIBRARY_SCRIPT_MAIN_B1,
    LIBRARY_SCRIPT_MAIN_B2,
    LIBRARY_SCRIPT_MAIN_B3,
    LIBRARY_SCRIPT_MAIN_B4,
    LIBRARY_SCRIPT_MAIN_B5,
    LIBRARY_SCRIPT_MAIN_C1,
    LIBRARY_SCRIPT_MAIN_C2,
    LIBRARY_SCRIPT_MAIN_C3,
    LIBRARY_SCRIPT_MAIN_C4,
    LIBRARY_SCRIPT_MAIN_C5,
    LIBRARY_SCRIPT_EXTRA,
    LIBRARY_SCRIPT_EXTRA2,
    LIBRARY_SCRIPTS,
};

enum library_script_slot {
    LIBRARY_SCRIPT_SLOT_A = 20,
    LIBRARY_SCRIPT_SLOT_B = 21,
    LIBRARY_SCRIPT_SLOT_C = 22,
    LIBRARY_SCRIPT_SLOT_EXTRA = 23,
};

enum library_part_images {
    LIBRARY_PART_KINDS = 25,
    LIBRARY_PART_IMAGE_STEPS = 126,
    LIBRARY_STAGE_AFTER_COMPLETE = LIBRARY_STAGE_COMPLETE + 1,
};

typedef struct {
    int32_t script;
    int32_t image;
    int32_t next_in_odd_views;
} library_view_image;

typedef struct {
    int32_t kind;
    int32_t until_stage;
    library_view_image north;
    library_view_image south;
} library_part_image_step;

extern library_part_image_step library_part_image_steps[LIBRARY_PART_IMAGE_STEPS];

void library_object_init(library_object_t *self);
bool library_can_place(monument_object *self, int32_t grid_offset);
bool library_check_work_for_figure(monument_object *self, int32_t building_id, int32_t figure_id, bool assign);
void library_draw_placement(monument_object *self);
void library_figure_work_tick(monument_object *self, int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t library_find_work_for_figure(monument_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t library_get_image(monument_object *self, int32_t building_id, int32_t *flags);
bool library_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool library_has_incomplete(monument_object *self);
bool library_part_has_active_worker(monument_object *self, int32_t building_id);
void library_place(monument_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
void library_update_image(monument_object *self, int32_t building_id);
int32_t library_get_progress(library_object_t *self, int32_t building_id, library_info_report *out);

#endif
