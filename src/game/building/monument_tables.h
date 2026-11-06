// The types of the monument tables (the tables are in building/monument_data.h): the work spot animation
// (positions, image runs, sequences, transitions, spots and work spots), the part descriptors and monument
// definitions, the method table of the monument objects, the pyramid family's styles, part kinds, stages and
// layout tables, and the monument eras of a scenario.
//
// pyramid_layout and mastaba_entry keep their tables together as one struct each, because the part layout, the bent
// layer tables and the ramp offsets are read past their ends into the next table.

#ifndef BUILDING_MONUMENT_TABLES_H
#define BUILDING_MONUMENT_TABLES_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PYRAMID_STYLE_BENT = 1,
    PYRAMID_STYLE_MUDBRICK = 2,
    PYRAMID_STYLE_STEPPED = 3,
    PYRAMID_STYLE_PYRAMID = 4,
    PYRAMID_STYLE_MASTABA = 5,
} pyramid_style;

typedef enum {
    PYRAMID_PART_CORNER_TOP = 0,
    PYRAMID_PART_CORNER_LEFT = 1,
    PYRAMID_PART_CORNER_BOTTOM = 2,
    PYRAMID_PART_CORNER_RIGHT = 3,
    PYRAMID_PART_EDGE_TOP_LEFT = 4,
    PYRAMID_PART_EDGE_BOTTOM_LEFT = 5,
    PYRAMID_PART_EDGE_BOTTOM_RIGHT = 6,
    PYRAMID_PART_EDGE_TOP_RIGHT = 7,
    PYRAMID_PART_INNER = 8,
    PYRAMID_PART_MASTABA_MAIN = 9,
} pyramid_part_kind;

enum {
    PYRAMID_VIEW_COUNT = 4,
    PYRAMID_SIZE_COUNT = 5,
    MASTABA_SIZE_COUNT = 3,
    BENT_PYRAMID_SIZE_COUNT = 2,
    PYRAMID_MAX_PARTS = 144,
    MASTABA_MAX_PARTS = 36,
    PYRAMID_MAX_RAMPS = 48,
    PYRAMID_RAMP_KIND_COUNT = 9,
    PYRAMID_ENTRY_POINT_COUNT = 60,
    MASTABA_ENTRY_POINT_COUNT = 16,
    PYRAMID_RING_COUNT = 6,
    PYRAMID_BLOCKS_PER_LAYER = 6,
    BENT_PYRAMID_LAYER_STRIDE = 6,
};

typedef struct {
    int32_t x;
    int32_t y;
} anim_position;

typedef struct {
    int32_t image_group;
    int32_t image_offset;
    int32_t frames;
    int32_t frame_step;
} anim_images;

typedef struct {
    int32_t position_from;
    int32_t position_to;
    int32_t images;
    int32_t last_frame;
    int32_t image_add[4];
    int32_t image_add_reverse[4];
} anim_sequence;

typedef struct {
    int32_t sequence;
    int8_t reverse;
    uint8_t spare_6[3];
} anim_choice;

enum anim_counts {
    ANIM_VIEWS = 4,
    ANIM_TRANSITION_CHOICES = 4,
};

typedef struct {
    int32_t sequence;
    int8_t reverse;
    uint8_t spare_6[3];
    anim_choice next[ANIM_TRANSITION_CHOICES];
} anim_transition;

typedef struct {
    anim_position *positions;
    int32_t position_count;
    anim_images *images;
    int32_t image_count;
    anim_sequence *sequences;
    int32_t sequence_count;
    anim_transition *transitions;
    int32_t transition_count;
    int32_t x_offset;
    int32_t y_offset;
} anim_script;

typedef enum {
    MONUMENT_SPOT_LOOP = 0,
    MONUMENT_SPOT_HOLD = 1,
    MONUMENT_SPOT_FROZEN = 2,
    MONUMENT_SPOT_IDLE = 3,
} monument_spot_mode;

typedef struct {
    int32_t script;
    int32_t sequence;
    int32_t frame;
    int32_t work_remaining;
    int16_t mode;
    int16_t figure_id;
    uint8_t figure_action;
    uint8_t reverse;
    uint8_t spare_7[2];
} monument_spot;

static_assert(sizeof(monument_spot) == 24, "monument_spot is a savegame record of 24 bytes");

typedef struct {
    int32_t stage;
    int32_t figure_type;
    monument_spot *spot;
    int32_t initial_work;
} monument_work_spot;

typedef struct {
    int32_t dx;
    int32_t dy;
    int32_t size;
    int32_t kind;
    int32_t grid_delta;
    int32_t view_x[4];
    int32_t view_y[4];
} monument_part_desc;

typedef struct {
    int32_t part;
    int32_t grid_offset;
} pyramid_preset_part;

typedef struct {
    const pyramid_preset_part *parts;
    int32_t count;
} pyramid_preset_list;

enum pyramid_preset_size {
    PYRAMID_PRESET_PARTS = 4,
    PYRAMID_PRESET_PARTS_LARGEST = 12,
};

typedef struct {
    int32_t reserved_0;
    pyramid_preset_part sizes[PYRAMID_SIZE_COUNT - 1][PYRAMID_PRESET_PARTS];
    pyramid_preset_part largest[PYRAMID_PRESET_PARTS_LARGEST];
} pyramid_preset_part_table;

typedef struct {
    pyramid_preset_list lists[PYRAMID_SIZE_COUNT];
    pyramid_preset_part mastaba_parts[MASTABA_SIZE_COUNT][PYRAMID_PRESET_PARTS];
} pyramid_preset_table;

typedef enum {
    MONUMENT_DEF_KIND_PYRAMID_FAMILY = 1,
    MONUMENT_DEF_KIND_OTHER = 2,
} monument_def_kind;

typedef enum {
    MONUMENT_DEF_SPHINX = 1,
    MONUMENT_DEF_OBELISK = 2,
    MONUMENT_DEF_SUN_TEMPLE = 3,
    MONUMENT_DEF_MAUSOLEUM = 4,
    MONUMENT_DEF_LIGHTHOUSE = 5,
    MONUMENT_DEF_LIBRARY = 6,
    MONUMENT_DEF_CAESAREUM = 7,
    MONUMENT_DEF_COLOSSI = 8,
    MONUMENT_DEF_TEMPLE_OF_LUXOR = 9,
    MONUMENT_DEF_SMALL_ROYAL_TOMB = 10,
    MONUMENT_DEF_MEDIUM_ROYAL_TOMB = 11,
    MONUMENT_DEF_LARGE_ROYAL_TOMB = 12,
    MONUMENT_DEF_GRAND_ROYAL_TOMB = 13,
    MONUMENT_DEF_ABU_SIMBEL = 14,
} monument_def_subkind;

typedef enum {
    PYRAMID_STAGE_MAIN_MATERIAL = 0,
    PYRAMID_STAGE_CASING = 1,
    PYRAMID_STAGE_SITE_PREPARATION = 2,
    PYRAMID_STAGE_PRESET = 3,
} pyramid_part_stage;

enum monument_era {
    MONUMENT_ERA_NONE = 0,
    MONUMENT_ERA_PYRAMIDS = 1,
    MONUMENT_ERA_VALLEY_OF_THE_KINGS = 2,
    MONUMENT_ERA_ALEXANDRIA = 3,
    MONUMENT_ERA_ABU_SIMBEL = 4,
    MONUMENT_ERA_COUNT = 5,
};

enum monument_def_counts {
    MONUMENT_DEF_RESOURCES = 3,
};

typedef struct {
    int16_t kind;
    int16_t subkind;
    int16_t size;
    int16_t resources[MONUMENT_DEF_RESOURCES];
    int16_t weight;
    int16_t flag;
} monument_def;

typedef char monument_script_name[12];

typedef struct monument_object monument_object;

typedef bool (*monument_can_place_fn)(monument_object *self, int32_t grid_offset);
typedef void (*monument_draw_placement_fn)(monument_object *self);
typedef bool (*monument_has_incomplete_fn)(monument_object *self);
typedef int32_t (*monument_get_progress_fn)(monument_object *self, int32_t building_id, int32_t *out);
typedef bool (*monument_tile_is_free_fn)(monument_object *self, uint32_t terrain, int32_t part);
typedef void (*monument_place_fn)(monument_object *self, int32_t x, int32_t y, int32_t building_id, bool no_slot);
typedef void (*monument_update_image_fn)(monument_object *self, int32_t building_id);
typedef int32_t (*monument_get_image_fn)(monument_object *self, int32_t building_id, int32_t *flags);
typedef bool (*monument_get_work_tile_fn)(monument_object *self, int32_t building_id, int32_t *x, int32_t *y);
typedef bool (*monument_part_has_active_worker_fn)(monument_object *self, int32_t building_id);
typedef int32_t (*monument_find_work_for_figure_fn)(monument_object *self, int32_t x, int32_t y, int32_t figure_id);
typedef bool (*monument_check_work_for_figure_fn)(
    monument_object *self, int32_t building_id, int32_t figure_id, bool assign);
typedef void (*monument_figure_work_tick_fn)(monument_object *self, int32_t building_id, int32_t figure_id, uint8_t done_action);
typedef bool (*monument_anim_tick_fn)(monument_object *self);

typedef struct {
    monument_can_place_fn can_place;
    monument_draw_placement_fn draw_placement;
    monument_has_incomplete_fn has_incomplete;
    monument_get_progress_fn get_progress;
    monument_tile_is_free_fn tile_is_free;
    monument_place_fn place;
    monument_update_image_fn update_image;
    monument_get_image_fn get_image;
    monument_get_work_tile_fn get_work_tile;
    monument_part_has_active_worker_fn part_has_active_worker;
    monument_find_work_for_figure_fn find_work_for_figure;
    monument_check_work_for_figure_fn check_work_for_figure;
    monument_figure_work_tick_fn figure_work_tick;
    monument_anim_tick_fn anim_tick;
} monument_object_vtable;

typedef struct {
    int32_t kind;
    int32_t layers;
} pyramid_part_layout_entry;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t offset_index;
    int32_t spare_8;
} pyramid_part_tile;

typedef struct {
    int32_t x;
    int32_t y;
} pyramid_tile_offset;

typedef struct {
    int32_t x;
    int32_t y;
} pyramid_pixel_offset;

typedef struct {
    int32_t part;
    int32_t ramp1_height;
    int32_t ramp2_height;
    int32_t ramp1_linked_part;
    int32_t ramp2_linked_part;
} pyramid_ramp;

typedef struct {
    int32_t part;
    int32_t height;
} pyramid_entry_point;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t x_with_ramp;
    int32_t y_with_ramp;
} stepped_ramp_offset;

typedef struct {
    int32_t down_first;
    int32_t down_last;
    int32_t up_first;
    int32_t up_last;
} pyramid_part_ring;

typedef struct {
    int32_t first;
    int32_t second;
} pyramid_paired_parts;

typedef struct {
    pyramid_part_layout_entry pyramid[PYRAMID_SIZE_COUNT][PYRAMID_MAX_PARTS];
    pyramid_part_layout_entry mastaba[MASTABA_SIZE_COUNT][MASTABA_MAX_PARTS];
    uint8_t bent_half_layer[32];
    int32_t bent_pixel_heights[30];
    int32_t size_being_placed;
    pyramid_part_tile pyramid_tiles[PYRAMID_SIZE_COUNT][PYRAMID_VIEW_COUNT][PYRAMID_MAX_PARTS];
    pyramid_part_tile mastaba_tiles[MASTABA_SIZE_COUNT][PYRAMID_VIEW_COUNT][MASTABA_MAX_PARTS];
    int32_t unused_tail;
} pyramid_layout_tables;

typedef struct {
    pyramid_entry_point entry_points[MASTABA_SIZE_COUNT][MASTABA_ENTRY_POINT_COUNT];
    pyramid_pixel_offset ramp_offsets[PYRAMID_RAMP_KIND_COUNT][PYRAMID_BLOCKS_PER_LAYER];
} mastaba_entry_tables;

#endif
