// The Caesareum (building/caesareum.c): its stages, part kinds, materials and the methods of its monument object.

#ifndef BUILDING_CAESAREUM_H
#define BUILDING_CAESAREUM_H

#include "building/monument_object.h"
#include "building/monument_progress_types.h"
#include <stdbool.h>
#include <stdint.h>

enum caesareum_stage {
    CAESAREUM_STAGE_GRANITE = 18,
    CAESAREUM_STAGE_COMPLETE = 19,
};

enum caesareum_part_kind {
    CAESAREUM_PART_KIND_WORKED = 0,
};

enum caesareum_material {
    CAESAREUM_MATERIAL_LOADS = 38,
};

void caesareum_object_init(caesareum_object_t *self);
bool caesareum_can_place(monument_object *self, int32_t grid_offset);
bool caesareum_check_work_for_figure(monument_object *self, int32_t building_id, int32_t figure_id, bool assign);
void caesareum_figure_work_tick(monument_object *self, int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t caesareum_find_work_for_figure(monument_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t caesareum_get_image(monument_object *self, int32_t building_id, int32_t *flags);
bool caesareum_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool caesareum_has_incomplete(monument_object *self);
bool caesareum_part_has_active_worker(monument_object *self, int32_t building_id);
void caesareum_place(monument_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
void caesareum_update_image(monument_object *self, int32_t building_id);
int32_t caesareum_get_progress(caesareum_object_t *self, int32_t building_id, scripted_monument_info_progress *out);

#endif
