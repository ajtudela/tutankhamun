// The lighthouse (building/lighthouse.c): its stages, part kinds, work tile and the methods of its monument
// object.

#ifndef BUILDING_LIGHTHOUSE_H
#define BUILDING_LIGHTHOUSE_H

#include "building/monument_object.h"
#include "building/monument_progress_types.h"
#include <stdbool.h>
#include <stdint.h>

enum lighthouse_stage {
    LIGHTHOUSE_STAGE_COMPLETE = 9,
};

enum lighthouse_part_kind {
    LIGHTHOUSE_PART_TOWER = 0,
    LIGHTHOUSE_PART_LAND = 1,
};

enum lighthouse_work_tile {
    LIGHTHOUSE_WORK_TILE_DX = 2,
    LIGHTHOUSE_WORK_TILE_DY = 6,
};

void lighthouse_object_init(lighthouse_object_t *self);
bool lighthouse_can_place(monument_object *self, int32_t grid_offset);
bool lighthouse_check_work_for_figure(monument_object *self, int32_t building_id, int32_t figure_id, bool assign);
void lighthouse_figure_work_tick(monument_object *self, int32_t building_id, int32_t figure_id, uint8_t done_action);
int32_t lighthouse_find_work_for_figure(monument_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t lighthouse_get_image(monument_object *self, int32_t building_id, int32_t *flags);
bool lighthouse_get_work_tile(monument_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool lighthouse_has_incomplete(monument_object *self);
bool lighthouse_part_has_active_worker(monument_object *self, int32_t building_id);
void lighthouse_place(monument_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
bool lighthouse_tile_is_free(monument_object *self, uint32_t terrain, int32_t part);
void lighthouse_update_image(monument_object *self, int32_t building_id);
int32_t lighthouse_get_progress(lighthouse_object_t *self, int32_t building_id, scripted_monument_info_progress *out);

#endif
