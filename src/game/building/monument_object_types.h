// The records of the scripted monument objects (library, Caesareum, lighthouse, Abu Simbel and the four royal tombs),
// one global per type: the base class, the derived part descriptors, and a royal tomb's base followed by its per-part
// arrays sized by its part count.

#ifndef BUILDING_MONUMENT_OBJECT_TYPES_H
#define BUILDING_MONUMENT_OBJECT_TYPES_H

#include "building/monument_tables.h"
#include <stdbool.h>
#include <stdint.h>

struct monument_object {
    const monument_object_vtable *vtable;
    int32_t frame_count;
    int32_t frame;
    monument_part_desc base_part;
    const char *base_scripts[3];
};

enum monument_object_part_counts {
    LIBRARY_PARTS = 25,
    CAESAREUM_PARTS = 9,
    LIGHTHOUSE_PARTS = 2,
    ABU_SIMBEL_PARTS = 16,
    CAESAREUM_SCRIPTS = 20,
    LIGHTHOUSE_SCRIPTS = 10,
};

typedef struct {
    monument_object base;
    monument_part_desc parts[LIBRARY_PARTS];
} library_object_t;

typedef struct {
    monument_object base;
    monument_part_desc parts[CAESAREUM_PARTS];
    const char *scripts[CAESAREUM_SCRIPTS];
} caesareum_object_t;

typedef struct {
    monument_object base;
    monument_part_desc parts[LIGHTHOUSE_PARTS];
    const char *scripts[LIGHTHOUSE_SCRIPTS];
    uint8_t reserved_DC[0x2C];
} lighthouse_object_t;

typedef struct {
    monument_object base;
    monument_part_desc parts[ABU_SIMBEL_PARTS];
    uint8_t reserved_38C[4];
} abu_simbel_object_t;

typedef struct tomb_object tomb_object;

typedef bool (*tomb_can_place_fn)(tomb_object *self, int32_t grid_offset);
typedef void (*tomb_draw_placement_fn)(tomb_object *self);
typedef bool (*tomb_has_incomplete_fn)(tomb_object *self);
typedef int32_t (*tomb_get_progress_fn)(tomb_object *self, int32_t building_id, int32_t *out);
typedef bool (*tomb_tile_is_free_fn)(tomb_object *self, uint32_t terrain, int32_t part);
typedef void (*tomb_place_fn)(tomb_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
typedef void (*tomb_update_image_fn)(tomb_object *self, int32_t building_id);
typedef int32_t (*tomb_get_image_fn)(tomb_object *self, int32_t building_id, int32_t *flags);
typedef bool (*tomb_get_work_tile_fn)(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y);
typedef bool (*tomb_part_has_active_worker_fn)(tomb_object *self, int32_t building_id);
typedef int32_t (*tomb_find_work_for_figure_fn)(tomb_object *self, int32_t x, int32_t y, int32_t figure_id);
typedef int32_t (*tomb_check_work_for_figure_fn)(tomb_object *self, int32_t building_id, int32_t figure_id, bool assign);
typedef void (*tomb_figure_work_tick_fn)(tomb_object *self, int32_t building_id, int32_t figure_id, uint8_t next_action);
typedef bool (*tomb_anim_tick_fn)(tomb_object *self);
typedef int32_t (*tomb_count_fn)(tomb_object *self);

typedef struct {
    tomb_can_place_fn can_place;
    tomb_draw_placement_fn draw_placement;
    tomb_has_incomplete_fn has_incomplete;
    tomb_get_progress_fn get_progress;
    tomb_tile_is_free_fn tile_is_free;
    tomb_place_fn place;
    tomb_update_image_fn update_image;
    tomb_get_image_fn get_image;
    tomb_get_work_tile_fn get_work_tile;
    tomb_part_has_active_worker_fn part_has_active_worker;
    tomb_find_work_for_figure_fn find_work_for_figure;
    tomb_check_work_for_figure_fn check_work_for_figure;
    tomb_figure_work_tick_fn figure_work_tick;
    tomb_anim_tick_fn anim_tick;
    tomb_count_fn chamber_count;
    tomb_count_fn part_count;
} tomb_object_vtable;

struct tomb_object {
    const tomb_object_vtable *vtable;
    int32_t frame_count;
    int32_t anim_frame;
    monument_part_desc base_part;
    const char *base_scripts[3];
    uint8_t material_requested;
    uint8_t spare_9[3];
    int32_t last_part;
    int32_t idle;
};

enum {
    SMALL_TOMB_PARTS = 167,
    MEDIUM_TOMB_PARTS = 158,
    LARGE_TOMB_PARTS = 437,
    GRAND_TOMB_PARTS = 513,
};

enum royal_tomb_part_data {
    TOMB_NEIGHBOURS = 6,
    TOMB_NO_NEIGHBOUR = -1,
    TOMB_WORK_STAGES = 3,
    TOMB_LOAD = 100,
    TOMB_STOCK_REFILL_BELOW = 400,
    TOMB_STOCK_FULL = 400,
    TOMB_LAST_ANIM_KEY = 3,
    TOMB_SPOT_TABLE_SHARING_WORK = 13,
    TOMB_SPOT_TABLE_WORK_SOURCE = 2,
    ROYAL_TOMB_OVERLAY_SHOWS_BUILDINGS = 51,
};

typedef struct {
    tomb_object base;
    int32_t anim_key[SMALL_TOMB_PARTS];
    int32_t chamber_kind[SMALL_TOMB_PARTS];
    int32_t neighbours[SMALL_TOMB_PARTS][TOMB_NEIGHBOURS];
    int32_t work[SMALL_TOMB_PARTS][TOMB_WORK_STAGES];
    uint8_t completed_image[SMALL_TOMB_PARTS];
    uint8_t closed_image_variant[SMALL_TOMB_PARTS];
    monument_part_desc descs[SMALL_TOMB_PARTS];
} small_royal_tomb_record;

typedef struct {
    tomb_object base;
    int32_t anim_key[MEDIUM_TOMB_PARTS];
    int32_t chamber_kind[MEDIUM_TOMB_PARTS];
    int32_t neighbours[MEDIUM_TOMB_PARTS][TOMB_NEIGHBOURS];
    int32_t work[MEDIUM_TOMB_PARTS][TOMB_WORK_STAGES];
    uint8_t completed_image[MEDIUM_TOMB_PARTS];
    uint8_t closed_image_variant[MEDIUM_TOMB_PARTS];
    monument_part_desc descs[MEDIUM_TOMB_PARTS];
    uint8_t reserved_3CD4[4];
} medium_royal_tomb_record;

typedef struct {
    tomb_object base;
    int32_t anim_key[LARGE_TOMB_PARTS];
    int32_t chamber_kind[LARGE_TOMB_PARTS];
    int32_t neighbours[LARGE_TOMB_PARTS][TOMB_NEIGHBOURS];
    int32_t work[LARGE_TOMB_PARTS][TOMB_WORK_STAGES];
    uint8_t completed_image[LARGE_TOMB_PARTS];
    uint8_t closed_image_variant[LARGE_TOMB_PARTS];
    monument_part_desc descs[LARGE_TOMB_PARTS];
    uint8_t reserved_A7A4[4];
} large_royal_tomb_record;

typedef struct {
    tomb_object base;
    int32_t anim_key[GRAND_TOMB_PARTS];
    int32_t chamber_kind[GRAND_TOMB_PARTS];
    int32_t neighbours[GRAND_TOMB_PARTS][TOMB_NEIGHBOURS];
    int32_t work[GRAND_TOMB_PARTS][TOMB_WORK_STAGES];
    uint8_t completed_image[GRAND_TOMB_PARTS];
    uint8_t closed_image_variant[GRAND_TOMB_PARTS];
    monument_part_desc descs[GRAND_TOMB_PARTS];
    uint8_t reserved_C4BC[4];
} grand_royal_tomb_record;

typedef union {
    tomb_object base;
    small_royal_tomb_record small;
    medium_royal_tomb_record medium;
    large_royal_tomb_record large;
    grand_royal_tomb_record grand;
} royal_tomb_record;

#endif
