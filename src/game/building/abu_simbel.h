// Abu Simbel (building/abu_simbel.c): its stages, part kinds, image groups and built-part flags, the screen
// offsets of the built parts and the methods of its monument object.

#ifndef BUILDING_ABU_SIMBEL_H
#define BUILDING_ABU_SIMBEL_H

#include "building/monument_object.h"
#include "building/monument_progress_types.h"
#include <stdbool.h>
#include <stdint.h>

enum abu_simbel_stage {
    ABU_SIMBEL_STAGE_COMPLETE = 56,
};

enum abu_simbel_part_kind {
    ABU_SIMBEL_PART_KIND_ON_OPEN_GROUND = 9,
    ABU_SIMBEL_PART_KINDS_WORKED = 12,
};

enum abu_simbel_image_group {
    IMAGE_GROUP_ABU_SIMBEL_PART10_SIDE = 812,
    IMAGE_GROUP_ABU_SIMBEL_PART10_FRONT = 813,
    IMAGE_GROUP_ABU_SIMBEL_PART7_SIDE = 814,
    IMAGE_GROUP_ABU_SIMBEL_PART7_FRONT = 815,
    IMAGE_GROUP_ABU_SIMBEL_PART8 = 816,
    IMAGE_GROUP_ABU_SIMBEL_PART9 = 817,
    IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_1 = 818,
    IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_0 = 819,
    IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_3 = 820,
    IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_2 = 821,
    IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_0 = 822,
    IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_3 = 823,
    IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_1 = 824,
    IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_2 = 825,
};

enum abu_simbel_built_parts {
    ABU_SIMBEL_BUILT_PART_GROUPS = 4,
    ABU_SIMBEL_BUILT_PARTS_PER_GROUP = 3,
    ABU_SIMBEL_BUILT_PARTS = 12,
    ABU_SIMBEL_BUILT_PART_A_GROUPS = 2,
};

typedef struct {
    int32_t view0_x;
    int32_t view0_y;
    int32_t view3_x;
    int32_t view3_y;
} abu_simbel_built_part_offset;

extern abu_simbel_built_part_offset abu_simbel_built_part_offsets[ABU_SIMBEL_BUILT_PARTS];

void abu_simbel_draw_built_parts(abu_simbel_object_t *self, int32_t building_id, int32_t x, int32_t y);
void abu_simbel_object_init(abu_simbel_object_t *self);
bool abu_simbel_can_place(monument_object *self, int32_t grid_offset);
void abu_simbel_figure_work_tick(monument_object *self, int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t abu_simbel_find_work_for_figure(monument_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t abu_simbel_find_work_for_figure_with_wait(
    abu_simbel_object_t *self, int32_t x, int32_t y, int32_t figure_id, int32_t carpenter_waits);
bool abu_simbel_has_incomplete(monument_object *self);
void abu_simbel_place(monument_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
void abu_simbel_update_image(monument_object *self, int32_t building_id);
bool abu_simbel_tile_is_free(monument_object *self, uint32_t terrain, int32_t part);
bool abu_simbel_check_work_for_figure(
    abu_simbel_object_t *self, int32_t building_id, int32_t figure_id, bool assign, int32_t carpenter_waits);
int32_t abu_simbel_get_progress(abu_simbel_object_t *self, int32_t building_id, abu_simbel_info_progress *out);

#endif
