// The scripted monument objects (building/monument_object.c): one global object per monument type with a method
// table; the base class holds the animation frame count and frame and one part descriptor. Also the part
// layouts and the material laborer, who dies when no storage yard has the resource.

#ifndef BUILDING_MONUMENT_OBJECT_H
#define BUILDING_MONUMENT_OBJECT_H

#include "building/building_create.h"
#include "building/building_figure_helpers.h"
#include "building/monument_internal.h"
#include "building/monument_object_types.h"
#include "building/monument_spots.h"
#include "building/monument_tables.h"
#include "figure/figure_type.h"
#include "game/undo_build.h"
#include "map/tiles.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { MATERIAL_NO_WORK = 0, MATERIAL_ASSIGNED = 1, MATERIAL_PARTNER_BUSY = 2 };
enum { MONUMENT_MATERIAL_PER_LOAD = 400 };
enum monument_work_flag { MONUMENT_WORK_COMPLETE = 1 };
int32_t monument_check_material_laborer(int32_t mason_id, int32_t figure_id, int32_t storage_building, int32_t resource, int32_t carry_action, bool assign);

void monument_object_init(monument_object *self);

typedef struct {
    int8_t dx, dy, size, kind;
} monument_part_init;
void monument_object_init_parts(monument_part_desc *parts, const monument_part_init *init, int32_t count);

typedef struct {
    monument_part_init *parts;
    int32_t count;
} monument_part_layout;

enum monument_part_layout_kind {
    MONUMENT_LAYOUT_LIBRARY,
    MONUMENT_LAYOUT_CAESAREUM,
    MONUMENT_LAYOUT_LIGHTHOUSE,
    MONUMENT_LAYOUT_ABU_SIMBEL,
    MONUMENT_LAYOUT_COUNT,
};

extern const monument_part_layout monument_part_layouts[MONUMENT_LAYOUT_COUNT];

bool monument_object_can_place(monument_object *self, int32_t grid_offset, const monument_part_desc *descs, int32_t count);
void monument_object_draw_placement(monument_object *self, const monument_part_desc *descs, int32_t count);
bool monument_object_has_incomplete(void *self, int32_t type, int32_t count);
bool monument_object_anim_tick(monument_object *self);
int32_t monument_object_no_progress(monument_object *self, int32_t building_id, int32_t *out);
bool monument_object_tile_is_free(monument_object *self, uint32_t terrain, int32_t part);
void monument_object_update_image_parts(monument_object *self, int32_t building_id, const monument_part_desc *parts);
void monument_part_desc_rotate_n(monument_part_desc *descs, int32_t count);
void library_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t done_action);
void monument_part_desc_rotate(monument_part_desc *self);

void abu_simbel_reset_work_spots(void);
void caesareum_reset_work_spots(void);
void library_reset_work_spots(void);
void lighthouse_reset_work_spots(void);

#endif
