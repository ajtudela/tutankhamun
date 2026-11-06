// The royal tombs (building/royal_tomb.c): sizes, chamber kinds, stages, the chamber and closed tile tables, the
// part images and part tables, the progress report and the tomb object methods.

#ifndef BUILDING_ROYAL_TOMB_H
#define BUILDING_ROYAL_TOMB_H

#include "building/monument_object_types.h"
#include "building/monument_tables.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    SMALL_TOMB_CHAMBERS = 9,
    MEDIUM_TOMB_CHAMBERS = 10,
    LARGE_TOMB_CHAMBERS = 21,
    GRAND_TOMB_CHAMBERS = 31,
};

enum royal_tomb_spot_tables {
    ROYAL_TOMB_SPOT_TABLES = 25,
    ROYAL_TOMB_WORKER_GO_TO_PART = 9,
};

enum royal_tomb_chamber_tables {
    ROYAL_TOMB_CHAMBER_KINDS = 30,
    ROYAL_TOMB_CLOSED_TILE_GROUPS = 8,
    ROYAL_TOMB_ANIM_KEYS = 4,
    ROYAL_TOMB_CLOSED_TILES_MAX = 16,
};

typedef struct {
    int32_t spot_table;
    int32_t closed_tiles;
} royal_tomb_chamber_kind_rule;

typedef struct {
    int8_t tiles[ROYAL_TOMB_CLOSED_TILES_MAX];
} royal_tomb_closed_tile_list;

extern royal_tomb_chamber_kind_rule royal_tomb_chamber_kinds[ROYAL_TOMB_CHAMBER_KINDS];
extern royal_tomb_closed_tile_list royal_tomb_closed_tiles[ROYAL_TOMB_CLOSED_TILE_GROUPS][ROYAL_TOMB_ANIM_KEYS];

enum royal_tomb_size {
    ROYAL_TOMB_SMALL,
    ROYAL_TOMB_MEDIUM,
    ROYAL_TOMB_LARGE,
    ROYAL_TOMB_GRAND,
    ROYAL_TOMB_SIZES,
};

enum royal_tomb_entrance {
    ROYAL_TOMB_ENTRANCE_A,
    ROYAL_TOMB_ENTRANCE_B,
    ROYAL_TOMB_ENTRANCE_C,
    ROYAL_TOMB_ENTRANCE_KINDS,
};

enum royal_tomb_chamber_kind {
    TOMB_CHAMBER_SEALING = 17,
    TOMB_CHAMBER_ENTRANCE_A = 18,
    TOMB_CHAMBER_ENTRANCE_B = 27,
    TOMB_CHAMBER_ENTRANCE_C = 28,
};

enum royal_tomb_stage {
    TOMB_STAGE_CLOSED = 0,
    TOMB_STAGE_STONEMASONS = 1,
    TOMB_STAGE_ARTISANS_LAST = 3,
    TOMB_STAGE_OPENS_NEIGHBOURS = 3,
    TOMB_STAGE_DONE = 4,
    TOMB_STAGE_FIRST_BUILT_IMAGE = 3,
    TOMB_STAGE_LAST_BUILT_IMAGE = 5,
};

enum royal_tomb_closed_tile_rule {
    TOMB_CLOSED_TILES_NONE = -1,
    TOMB_CLOSED_TILES_FIRST_ONLY = -2,
};

enum royal_tomb_progress_out {
    TOMB_PROGRESS_STONEMASONS = 1,
    TOMB_PROGRESS_LABORERS = 2,
    TOMB_PROGRESS_ARTISANS = 4,
    TOMB_PROGRESS_PERCENT = 5,
    TOMB_PROGRESS_STOCK = 6,
};

enum royal_tomb_images {
    IMAGE_GROUP_ROYAL_TOMB_CLOSED_PART = 805,
    TOMB_VIEW_IMAGES = 4,
    TOMB_IDLE_GROUP_DISTANCE = 1,
};

typedef struct {
    int32_t kind;
    int32_t group;
} royal_tomb_chamber_image;

typedef struct {
    int32_t last_part_closed;
    int32_t sealing_adds_stage;
    int32_t sealing_group;
    int32_t entrance_groups[ROYAL_TOMB_ENTRANCE_KINDS];
    royal_tomb_chamber_image *chambers;
    int32_t chamber_count;
} royal_tomb_image_set;

extern royal_tomb_image_set royal_tomb_image_sets[ROYAL_TOMB_SIZES];

enum {
    ROYAL_TOMB_CLOSED_VARIANT_RANDOM = -1,
    ROYAL_TOMB_CLOSED_VARIANTS = 5,
    ROYAL_TOMB_PART_NEIGHBOURS_LISTED = 2,
};

typedef struct {
    int8_t dx;
    int8_t dy;
    int8_t size;
    int8_t anim_key;
    int8_t chamber_kind;
    int8_t neighbours[ROYAL_TOMB_PART_NEIGHBOURS_LISTED];
    int32_t work;
    int8_t completed_image;
    int8_t closed_image_variant;
} royal_tomb_part_init;

typedef struct {
    royal_tomb_part_init *parts;
    int32_t count;
} royal_tomb_part_table;

extern royal_tomb_part_table royal_tomb_part_tables[ROYAL_TOMB_SIZES];

static inline royal_tomb_record *royal_tomb_record_of(tomb_object *self)
{
    return (royal_tomb_record *) self;
}

typedef struct {
    int32_t count;
    int32_t *anim_key;
    int32_t *chamber_kind;
    int32_t (*neighbours)[TOMB_NEIGHBOURS];
    int32_t (*work)[TOMB_WORK_STAGES];
    uint8_t *completed_image;
    uint8_t *closed_image_variant;
    monument_part_desc *descs;
} royal_tomb_parts;

static inline royal_tomb_parts royal_tomb_parts_of(tomb_object *self, enum royal_tomb_size size)
{
    royal_tomb_record *tomb = royal_tomb_record_of(self);
    royal_tomb_parts parts;
    switch (size) {
        case ROYAL_TOMB_MEDIUM:
            parts.count = MEDIUM_TOMB_PARTS;
            parts.anim_key = tomb->medium.anim_key;
            parts.chamber_kind = tomb->medium.chamber_kind;
            parts.neighbours = tomb->medium.neighbours;
            parts.work = tomb->medium.work;
            parts.completed_image = tomb->medium.completed_image;
            parts.closed_image_variant = tomb->medium.closed_image_variant;
            parts.descs = tomb->medium.descs;
            break;
        case ROYAL_TOMB_LARGE:
            parts.count = LARGE_TOMB_PARTS;
            parts.anim_key = tomb->large.anim_key;
            parts.chamber_kind = tomb->large.chamber_kind;
            parts.neighbours = tomb->large.neighbours;
            parts.work = tomb->large.work;
            parts.completed_image = tomb->large.completed_image;
            parts.closed_image_variant = tomb->large.closed_image_variant;
            parts.descs = tomb->large.descs;
            break;
        case ROYAL_TOMB_GRAND:
            parts.count = GRAND_TOMB_PARTS;
            parts.anim_key = tomb->grand.anim_key;
            parts.chamber_kind = tomb->grand.chamber_kind;
            parts.neighbours = tomb->grand.neighbours;
            parts.work = tomb->grand.work;
            parts.completed_image = tomb->grand.completed_image;
            parts.closed_image_variant = tomb->grand.closed_image_variant;
            parts.descs = tomb->grand.descs;
            break;
        case ROYAL_TOMB_SMALL:
        default:
            parts.count = SMALL_TOMB_PARTS;
            parts.anim_key = tomb->small.anim_key;
            parts.chamber_kind = tomb->small.chamber_kind;
            parts.neighbours = tomb->small.neighbours;
            parts.work = tomb->small.work;
            parts.completed_image = tomb->small.completed_image;
            parts.closed_image_variant = tomb->small.closed_image_variant;
            parts.descs = tomb->small.descs;
            break;
    }
    return parts;
}

int32_t small_royal_tomb_chamber_count(tomb_object *self);
int32_t small_royal_tomb_part_count(tomb_object *self);
int32_t grand_royal_tomb_chamber_count(tomb_object *self);
int32_t grand_royal_tomb_part_count(tomb_object *self);
int32_t large_royal_tomb_chamber_count(tomb_object *self);
int32_t large_royal_tomb_part_count(tomb_object *self);
int32_t medium_royal_tomb_chamber_count(tomb_object *self);
int32_t medium_royal_tomb_part_count(tomb_object *self);
int32_t royal_tomb_no_progress(tomb_object *self, int32_t building_id, int32_t *out);
bool royal_tomb_anim_tick(tomb_object *self);
bool royal_tomb_part_worker_active(tomb_object *self, int32_t building_id);
bool royal_tomb_terrain_allowed(tomb_object *self, uint32_t terrain, int32_t kind);
void royal_tombs_update_all_images(void);
bool small_royal_tomb_has_incomplete(tomb_object *self);
bool medium_royal_tomb_has_incomplete(tomb_object *self);
bool large_royal_tomb_has_incomplete(tomb_object *self);
bool grand_royal_tomb_has_incomplete(tomb_object *self);
void royal_tomb_update_image(tomb_object *self, int32_t building_id);
int32_t small_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t medium_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t large_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t grand_royal_tomb_find_work_for_figure(tomb_object *self, int32_t x, int32_t y, int32_t figure_id);
int32_t royal_tomb_sized_check_work_for_figure(tomb_object *self, int32_t building_id, int32_t figure_id, bool assign);
void royal_tomb_update_part_images(tomb_object *self, int32_t building_id);
void royal_tomb_figure_work_tick(tomb_object *self, int32_t building_id, int32_t figure_id, uint8_t next_action);
void royal_tomb_part_check_stage_done(int32_t building_id);
void small_royal_tomb_part_advance(tomb_object *self, int32_t building_id);
void medium_royal_tomb_part_advance(tomb_object *self, int32_t building_id);
void large_royal_tomb_part_advance(tomb_object *self, int32_t building_id);
void grand_royal_tomb_part_advance(tomb_object *self, int32_t building_id);
void royal_tomb_draw_spots(int32_t x, int32_t y, int32_t building_id);
int32_t royal_tomb_get_progress(tomb_object *self, int32_t building_id, int32_t chambers, int32_t *out);
int32_t royal_tomb_progress(int32_t type, int32_t building_id, int32_t *out);
tomb_object *royal_tomb_object_for_type(int32_t type);
int32_t royal_tomb_chambers_of_type(int32_t type);
int32_t royal_tomb_check_work_for_figure(tomb_object *self, int32_t building_id, int32_t figure_id, bool assign);
int32_t royal_tomb_part_find_spot(int32_t building_id, int32_t figure_id);
int32_t royal_tomb_spot_table_of(int32_t chamber_kind);
void royal_tomb_set_completed(tomb_object *self, int32_t building_id);
bool royal_tomb_is_idle(tomb_object *self, int32_t building_id);
void royal_tomb_part_advance_stage(tomb_object *self, int32_t building_id, int32_t part_count);

void grand_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
void large_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
void medium_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
void royal_tomb_reset_work_spots(void);
void small_royal_tomb_place(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);

#endif
