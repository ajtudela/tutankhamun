// The gameplay config tables of the monuments (core/config.h): the work spot animation images, positions and
// scripts, the access tiles, the part layouts of the scripted monuments, Abu Simbel's built parts, the royal
// tomb images, chamber kinds, closed tiles and parts, the sun temple's blocks, the library's part images and the
// material needed by monuments not yet placed.

#include "building/abu_simbel.h"
#include "building/library.h"
#include "building/monument_access.h"
#include "building/monument_anim_images.h"
#include "building/monument_anim_script.h"
#include "building/monument_data.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "core/config.h"
#include <stddef.h>

#define ANIM_FIELD_OFFSET(type, member) ((int32_t) (offsetof(type, member)))
#define anim_images_column(member) (&monument_anim_images[0].member)
#define anim_script_column(member) (&anim_scripts[0].member)
#define ANIM_IMAGES_STRIDE ((int32_t) (sizeof(anim_images) / sizeof(int32_t)))
#define ANIM_SCRIPT_STRIDE ((int32_t) (sizeof(anim_script) / sizeof(int32_t)))

static const config_field monument_anim_image_fields[] = {
    { "group", anim_images_column(image_group), ANIM_IMAGES_STRIDE, 1, 0, 10000, config_anim_image_group_names },
    { "offset", anim_images_column(image_offset), ANIM_IMAGES_STRIDE, 1, 0, 10000, 0 },
    { "frames", anim_images_column(frames), ANIM_IMAGES_STRIDE, 1, 1, 1000, 0 },
    { "frame_step", anim_images_column(frame_step), ANIM_IMAGES_STRIDE, 1, 1, 1000, 0 },
};

static const config_storage monument_anim_image_storage[] = {
    { monument_anim_images, sizeof(monument_anim_images) },
};

const config_table monument_anim_image_table = {
    "monument_anim_images.json",
    "images",
    config_anim_images_names,
    ANIM_IMAGES_COUNT,
    CONFIG_TABLE_COLUMNS(monument_anim_image_fields, monument_anim_image_storage),
};

typedef struct anim_position_table {
    anim_position *positions;
    int32_t count;
} anim_position_table;

#define ANIM_POSITION_TABLE(table) { table, CONFIG_COUNT(table) / ANIM_VIEWS }

static const anim_position_table anim_position_tables[] = {
    ANIM_POSITION_TABLE(sphinx_anim_positions),
    ANIM_POSITION_TABLE(obelisk_small_anim_positions),
    ANIM_POSITION_TABLE(obelisk_large_anim_positions),
    ANIM_POSITION_TABLE(sun_temple_anim_positions),
    ANIM_POSITION_TABLE(library_anim_positions),
    ANIM_POSITION_TABLE(library_anim_positions_a),
    ANIM_POSITION_TABLE(library_anim_positions_b),
    ANIM_POSITION_TABLE(abu_simbel_anim_positions),
    ANIM_POSITION_TABLE(caesareum_anim_positions),
    ANIM_POSITION_TABLE(lighthouse_anim_positions),
    ANIM_POSITION_TABLE(royal_tomb_chamber_anim_positions),
};

static const config_name anim_position_table_names[] = { { "sphinx_anim_positions", 0 }, { "obelisk_small_anim_positions", 1 },
    { "obelisk_large_anim_positions", 2 }, { "sun_temple_anim_positions", 3 }, { "library_anim_positions", 4 }, { "library_anim_positions_a", 5 },
    { "library_anim_positions_b", 6 }, { "abu_simbel_anim_positions", 7 }, { "caesareum_anim_positions", 8 }, { "lighthouse_anim_positions", 9 },
    { "royal_tomb_chamber_anim_positions", 10 }, { 0, 0 } };

static void *anim_position_block(int32_t record, int32_t *count)
{
    *count = anim_position_tables[record].count;
    return anim_position_tables[record].positions;
}

static const config_field anim_view_fields[] = {
    { "x", 0, 0, 1, -10000, 10000, 0, ANIM_FIELD_OFFSET(anim_position, x) },
    { "y", 0, 0, 1, -10000, 10000, 0, ANIM_FIELD_OFFSET(anim_position, y) },
};

static const config_list anim_view_list = {
    "view",
    sizeof(anim_position),
    anim_view_fields,
    CONFIG_COUNT(anim_view_fields),
    0,
    ANIM_VIEWS,
};

static const config_field anim_position_element_fields[] = {
    { "views", 0, 0, 1, 0, 0, 0, 0, 0, &anim_view_list },
};

static const config_list anim_position_list = {
    "position",
    sizeof(anim_position) * ANIM_VIEWS,
    anim_position_element_fields,
    CONFIG_COUNT(anim_position_element_fields),
    anim_position_block,
    0,
};

static const config_field monument_anim_position_fields[] = {
    { "positions", 0, 0, 1, 0, 0, 0, 0, 0, &anim_position_list },
};

const config_table monument_anim_position_table = {
    "monument_anim_positions.json",
    "table",
    anim_position_table_names,
    CONFIG_COUNT(anim_position_tables),
    monument_anim_position_fields,
    CONFIG_COUNT(monument_anim_position_fields),
    0,
    0,
};

static void *anim_script_sequences(int32_t record, int32_t *count)
{
    *count = anim_scripts[record].sequence_count;
    return anim_scripts[record].sequences;
}

static void *anim_script_transitions(int32_t record, int32_t *count)
{
    *count = anim_scripts[record].transition_count;
    return anim_scripts[record].transitions;
}

static const config_field anim_sequence_fields[] = {
    { "from", 0, 0, 1, 0, 10000, 0, ANIM_FIELD_OFFSET(anim_sequence, position_from) },
    { "to", 0, 0, 1, 0, 10000, 0, ANIM_FIELD_OFFSET(anim_sequence, position_to) },
    { "images", 0, 0, 1, 0, ANIM_IMAGES_COUNT - 1, config_anim_images_names, ANIM_FIELD_OFFSET(anim_sequence, images) },
    { "last_frame", 0, 0, 1, 0, 100000, 0, ANIM_FIELD_OFFSET(anim_sequence, last_frame) },
    { "image_add", 0, 0, ANIM_VIEWS, -1000, 1000, 0, ANIM_FIELD_OFFSET(anim_sequence, image_add) },
    { "image_add_reverse", 0, 0, ANIM_VIEWS, -1000, 1000, 0, ANIM_FIELD_OFFSET(anim_sequence, image_add_reverse) },
};

static const config_list anim_sequence_list = {
    "sequence",
    sizeof(anim_sequence),
    anim_sequence_fields,
    CONFIG_COUNT(anim_sequence_fields),
    anim_script_sequences,
    0,
};

static const config_field anim_choice_fields[] = {
    { "sequence", 0, 0, 1, -1, 10000, 0, ANIM_FIELD_OFFSET(anim_choice, sequence) },
    { "reverse", 0, 0, 1, 0, 1, 0, ANIM_FIELD_OFFSET(anim_choice, reverse), 1 },
};

static const config_list anim_choice_list = {
    "choice",
    sizeof(anim_choice),
    anim_choice_fields,
    CONFIG_COUNT(anim_choice_fields),
    0,
    4,
};

static const config_field anim_transition_fields[] = {
    { "sequence", 0, 0, 1, -1, 10000, 0, ANIM_FIELD_OFFSET(anim_transition, sequence) },
    { "reverse", 0, 0, 1, 0, 1, 0, ANIM_FIELD_OFFSET(anim_transition, reverse), 1 },
    { "next", 0, 0, 1, 0, 0, 0, ANIM_FIELD_OFFSET(anim_transition, next), 0, &anim_choice_list },
};

static const config_list anim_transition_list = {
    "transition",
    sizeof(anim_transition),
    anim_transition_fields,
    CONFIG_COUNT(anim_transition_fields),
    anim_script_transitions,
    0,
};

static const config_field monument_anim_script_fields[] = {
    { "x_offset", anim_script_column(x_offset), ANIM_SCRIPT_STRIDE, 1, -10000, 10000, 0 },
    { "y_offset", anim_script_column(y_offset), ANIM_SCRIPT_STRIDE, 1, -10000, 10000, 0 },
    { "sequences", 0, 0, 1, 0, 0, 0, 0, 0, &anim_sequence_list },
    { "transitions", 0, 0, 1, 0, 0, 0, 0, 0, &anim_transition_list },
};

static const config_storage monument_anim_script_storage[] = {
    { anim_scripts, sizeof(anim_scripts) },
};

const config_table monument_anim_script_table = {
    "monument_anim_scripts.json",
    "script",
    config_anim_script_names,
    ANIM_SCRIPT_COUNT,
    CONFIG_TABLE_COLUMNS(monument_anim_script_fields, monument_anim_script_storage),
};

static const config_name monument_access_names[] = { { "small_royal_tomb", MONUMENT_ACCESS_SMALL_ROYAL_TOMB },
    { "medium_royal_tomb", MONUMENT_ACCESS_MEDIUM_ROYAL_TOMB }, { "large_royal_tomb", MONUMENT_ACCESS_LARGE_ROYAL_TOMB },
    { "grand_royal_tomb", MONUMENT_ACCESS_GRAND_ROYAL_TOMB }, { "sun_temple", MONUMENT_ACCESS_SUN_TEMPLE }, { 0, 0 } };

static void *monument_access_block(int32_t record, int32_t *count)
{
    *count = monument_access_tables[record].count;
    return monument_access_tables[record].offsets;
}

static const config_field monument_access_offset_fields[] = {
    { "dx", 0, 0, 1, -10, 10, 0, ANIM_FIELD_OFFSET(monument_access_offset, dx), 1 },
    { "dy", 0, 0, 1, -10, 10, 0, ANIM_FIELD_OFFSET(monument_access_offset, dy), 1 },
};

static const config_list monument_access_part_list = {
    "part",
    sizeof(monument_access_offset),
    monument_access_offset_fields,
    CONFIG_COUNT(monument_access_offset_fields),
    monument_access_block,
    0,
};

static const config_field monument_access_fields[] = {
    { "parts", 0, 0, 1, 0, 0, 0, 0, 0, &monument_access_part_list },
};

const config_table monument_access_config = {
    "monument_access_tiles.json",
    "monument",
    monument_access_names,
    MONUMENT_ACCESS_KIND_COUNT,
    monument_access_fields,
    CONFIG_COUNT(monument_access_fields),
    0,
    0,
};

static const config_name monument_part_layout_names[] = { { "library", MONUMENT_LAYOUT_LIBRARY }, { "caesareum", MONUMENT_LAYOUT_CAESAREUM },
    { "lighthouse", MONUMENT_LAYOUT_LIGHTHOUSE }, { "abu_simbel", MONUMENT_LAYOUT_ABU_SIMBEL }, { 0, 0 } };

static void *monument_part_layout_block(int32_t record, int32_t *count)
{
    *count = monument_part_layouts[record].count;
    return monument_part_layouts[record].parts;
}

static const config_field monument_part_init_fields[] = {
    { "dx", 0, 0, 1, -20, 20, 0, ANIM_FIELD_OFFSET(monument_part_init, dx), 1 },
    { "dy", 0, 0, 1, -20, 20, 0, ANIM_FIELD_OFFSET(monument_part_init, dy), 1 },
    { "size", 0, 0, 1, 1, 6, 0, ANIM_FIELD_OFFSET(monument_part_init, size), 1 },
    { "kind", 0, 0, 1, 0, 100, 0, ANIM_FIELD_OFFSET(monument_part_init, kind), 1 },
};

static const config_list monument_part_layout_list = {
    "part",
    sizeof(monument_part_init),
    monument_part_init_fields,
    CONFIG_COUNT(monument_part_init_fields),
    monument_part_layout_block,
    0,
};

static const config_field monument_part_layout_fields[] = {
    { "parts", 0, 0, 1, 0, 0, 0, 0, 0, &monument_part_layout_list },
};

const config_table monument_part_layout_config = {
    "monument_part_layouts.json",
    "monument",
    monument_part_layout_names,
    MONUMENT_LAYOUT_COUNT,
    monument_part_layout_fields,
    CONFIG_COUNT(monument_part_layout_fields),
    0,
    0,
};

static const config_name abu_simbel_built_part_names[] = { { "abu_simbel", 0 }, { 0, 0 } };

static void *abu_simbel_built_part_block(int32_t record, int32_t *count)
{
    *count = ABU_SIMBEL_BUILT_PARTS;
    return abu_simbel_built_part_offsets;
}

static const config_field abu_simbel_built_part_offset_fields[] = {
    { "view0_x", 0, 0, 1, -2000, 2000, 0, ANIM_FIELD_OFFSET(abu_simbel_built_part_offset, view0_x) },
    { "view0_y", 0, 0, 1, -2000, 2000, 0, ANIM_FIELD_OFFSET(abu_simbel_built_part_offset, view0_y) },
    { "view3_x", 0, 0, 1, -2000, 2000, 0, ANIM_FIELD_OFFSET(abu_simbel_built_part_offset, view3_x) },
    { "view3_y", 0, 0, 1, -2000, 2000, 0, ANIM_FIELD_OFFSET(abu_simbel_built_part_offset, view3_y) },
};

static const config_list abu_simbel_built_part_list = {
    "part",
    sizeof(abu_simbel_built_part_offset),
    abu_simbel_built_part_offset_fields,
    CONFIG_COUNT(abu_simbel_built_part_offset_fields),
    abu_simbel_built_part_block,
    0,
};

static const config_field abu_simbel_built_part_fields[] = {
    { "parts", 0, 0, 1, 0, 0, 0, 0, 0, &abu_simbel_built_part_list },
};

const config_table abu_simbel_built_part_config = {
    "abu_simbel_built_parts.json",
    "monument",
    abu_simbel_built_part_names,
    1,
    abu_simbel_built_part_fields,
    CONFIG_COUNT(abu_simbel_built_part_fields),
    0,
    0,
};

static const config_name royal_tomb_image_names[] = { { "small_royal_tomb", ROYAL_TOMB_SMALL }, { "medium_royal_tomb", ROYAL_TOMB_MEDIUM },
    { "large_royal_tomb", ROYAL_TOMB_LARGE }, { "grand_royal_tomb", ROYAL_TOMB_GRAND }, { 0, 0 } };

#define royal_tomb_image_column(member) (&royal_tomb_image_sets[0].member)
#define ROYAL_TOMB_IMAGE_STRIDE ((int32_t) (sizeof(royal_tomb_image_set) / sizeof(int32_t)))

static void *royal_tomb_chamber_block(int32_t record, int32_t *count)
{
    *count = royal_tomb_image_sets[record].chamber_count;
    return royal_tomb_image_sets[record].chambers;
}

static const config_field royal_tomb_chamber_fields[] = {
    { "kind", 0, 0, 1, 0, 100, 0, ANIM_FIELD_OFFSET(royal_tomb_chamber_image, kind) },
    { "group", 0, 0, 1, 0, 10000, 0, ANIM_FIELD_OFFSET(royal_tomb_chamber_image, group) },
};

static const config_list royal_tomb_chamber_list = {
    "chamber",
    sizeof(royal_tomb_chamber_image),
    royal_tomb_chamber_fields,
    CONFIG_COUNT(royal_tomb_chamber_fields),
    royal_tomb_chamber_block,
    0,
};

static const config_field royal_tomb_image_fields[] = {
    { "last_part_closed", royal_tomb_image_column(last_part_closed), ROYAL_TOMB_IMAGE_STRIDE, 1, 0, 1, 0 },
    { "sealing_adds_stage", royal_tomb_image_column(sealing_adds_stage), ROYAL_TOMB_IMAGE_STRIDE, 1, 0, 1, 0 },
    { "sealing_group", royal_tomb_image_column(sealing_group), ROYAL_TOMB_IMAGE_STRIDE, 1, 0, 10000, 0 },
    { "entrance_groups", royal_tomb_image_column(entrance_groups[0]), ROYAL_TOMB_IMAGE_STRIDE, ROYAL_TOMB_ENTRANCE_KINDS, 0, 10000, 0 },
    { "chambers", 0, 0, 1, 0, 0, 0, 0, 0, &royal_tomb_chamber_list },
};

static const config_storage royal_tomb_image_storage[] = {
    { royal_tomb_image_sets, sizeof(royal_tomb_image_sets) },
};

const config_table royal_tomb_image_config = {
    "royal_tomb_images.json",
    "tomb",
    royal_tomb_image_names,
    ROYAL_TOMB_SIZES,
    CONFIG_TABLE_COLUMNS(royal_tomb_image_fields, royal_tomb_image_storage),
};

#define ROYAL_TOMB_CHAMBER_KIND_STRIDE ((int32_t) (sizeof(royal_tomb_chamber_kind_rule) / sizeof(int32_t)))

static const config_field royal_tomb_chamber_kind_fields[] = {
    { "spot_table", &royal_tomb_chamber_kinds[0].spot_table, ROYAL_TOMB_CHAMBER_KIND_STRIDE, 1, -1, ROYAL_TOMB_SPOT_TABLES - 1, 0 },
    { "closed_tiles", &royal_tomb_chamber_kinds[0].closed_tiles, ROYAL_TOMB_CHAMBER_KIND_STRIDE, 1, -2, ROYAL_TOMB_CLOSED_TILE_GROUPS - 1, 0 },
};

static const config_storage royal_tomb_chamber_kind_storage[] = {
    { royal_tomb_chamber_kinds, sizeof(royal_tomb_chamber_kinds) },
};

const config_table royal_tomb_chamber_kind_config = {
    "royal_tomb_chamber_kinds.json",
    "kind",
    0,
    ROYAL_TOMB_CHAMBER_KINDS,
    CONFIG_TABLE_COLUMNS(royal_tomb_chamber_kind_fields, royal_tomb_chamber_kind_storage),
};

static void *royal_tomb_closed_tile_block(int32_t record, int32_t *count)
{
    *count = ROYAL_TOMB_ANIM_KEYS;
    return royal_tomb_closed_tiles[record];
}

static const config_field royal_tomb_closed_tile_fields[] = {
    { "tiles", 0, 0, ROYAL_TOMB_CLOSED_TILES_MAX, -1, 35, 0, ANIM_FIELD_OFFSET(royal_tomb_closed_tile_list, tiles), 1 },
};

static const config_list royal_tomb_closed_tile_list_of_keys = {
    "anim_key",
    sizeof(royal_tomb_closed_tile_list),
    royal_tomb_closed_tile_fields,
    CONFIG_COUNT(royal_tomb_closed_tile_fields),
    royal_tomb_closed_tile_block,
    0,
};

static const config_field royal_tomb_closed_tile_group_fields[] = {
    { "anim_keys", 0, 0, 1, 0, 0, 0, 0, 0, &royal_tomb_closed_tile_list_of_keys },
};

const config_table royal_tomb_closed_tile_config = {
    "royal_tomb_closed_tiles.json",
    "group",
    0,
    ROYAL_TOMB_CLOSED_TILE_GROUPS,
    royal_tomb_closed_tile_group_fields,
    CONFIG_COUNT(royal_tomb_closed_tile_group_fields),
    0,
    0,
};

static const config_field sun_temple_stage_block_fields[] = {
    { "blocks", sun_temple_stage_blocks, 1, 1, 0, SUN_TEMPLE_BLOCKS, 0 },
};

static const config_storage sun_temple_stage_block_storage[] = {
    { sun_temple_stage_blocks, sizeof(sun_temple_stage_blocks) },
};

const config_table sun_temple_stage_block_config = {
    "sun_temple_stage_blocks.json",
    "stage",
    0,
    SUN_TEMPLE_STAGES,
    CONFIG_TABLE_COLUMNS(sun_temple_stage_block_fields, sun_temple_stage_block_storage),
};

#define SUN_TEMPLE_BLOCK_STRIDE ((int32_t) (sizeof(sun_temple_block_offset) / sizeof(int32_t)))

static const config_field sun_temple_block_offset_fields[] = {
    { "x", &sun_temple_block_offsets[0].x, SUN_TEMPLE_BLOCK_STRIDE, 1, -1000, 1000, 0 },
    { "y", &sun_temple_block_offsets[0].y, SUN_TEMPLE_BLOCK_STRIDE, 1, -1000, 1000, 0 },
};

static const config_storage sun_temple_block_offset_storage[] = {
    { sun_temple_block_offsets, sizeof(sun_temple_block_offsets) },
};

const config_table sun_temple_block_offset_config = {
    "sun_temple_block_offsets.json",
    "block",
    0,
    SUN_TEMPLE_BLOCKS,
    CONFIG_TABLE_COLUMNS(sun_temple_block_offset_fields, sun_temple_block_offset_storage),
};

static const config_name library_image_names[] = {
    { "LIBRARY_IMAGE_BLANK", LIBRARY_IMAGE_BLANK },
    { "LIBRARY_IMAGE_LEVELED", LIBRARY_IMAGE_LEVELED },
    { "LIBRARY_IMAGE_FRONT", LIBRARY_IMAGE_FRONT },
    { "LIBRARY_IMAGE_MAIN_A", LIBRARY_IMAGE_MAIN_A },
    { "LIBRARY_IMAGE_MAIN_B", LIBRARY_IMAGE_MAIN_B },
    { "LIBRARY_IMAGE_MAIN_C", LIBRARY_IMAGE_MAIN_C },
    { "LIBRARY_IMAGE_C4", LIBRARY_IMAGE_C4 },
    { "LIBRARY_IMAGE_CC", LIBRARY_IMAGE_CC },
    { "LIBRARY_IMAGE_DC", LIBRARY_IMAGE_DC },
    { "LIBRARY_IMAGE_E4", LIBRARY_IMAGE_E4 },
    { "LIBRARY_IMAGE_EC", LIBRARY_IMAGE_EC },
    { "LIBRARY_IMAGE_F4", LIBRARY_IMAGE_F4 },
    { "LIBRARY_IMAGE_FC", LIBRARY_IMAGE_FC },
    { "LIBRARY_IMAGE_04", LIBRARY_IMAGE_04 },
    { "LIBRARY_IMAGE_0C", LIBRARY_IMAGE_0C },
    { "LIBRARY_IMAGE_14", LIBRARY_IMAGE_14 },
    { "LIBRARY_IMAGE_1C", LIBRARY_IMAGE_1C },
    { "LIBRARY_IMAGE_24", LIBRARY_IMAGE_24 },
    { "LIBRARY_IMAGE_2C", LIBRARY_IMAGE_2C },
    { "LIBRARY_IMAGE_34", LIBRARY_IMAGE_34 },
    { "LIBRARY_IMAGE_3C", LIBRARY_IMAGE_3C },
    { 0, 0 },
};

static const config_name library_script_names[] = {
    { "LIBRARY_SCRIPT_NONE", LIBRARY_SCRIPT_NONE },
    { "LIBRARY_SCRIPT_MAIN_A1", LIBRARY_SCRIPT_MAIN_A1 },
    { "LIBRARY_SCRIPT_MAIN_A2", LIBRARY_SCRIPT_MAIN_A2 },
    { "LIBRARY_SCRIPT_MAIN_A3", LIBRARY_SCRIPT_MAIN_A3 },
    { "LIBRARY_SCRIPT_MAIN_A4", LIBRARY_SCRIPT_MAIN_A4 },
    { "LIBRARY_SCRIPT_MAIN_A5", LIBRARY_SCRIPT_MAIN_A5 },
    { "LIBRARY_SCRIPT_MAIN_B1", LIBRARY_SCRIPT_MAIN_B1 },
    { "LIBRARY_SCRIPT_MAIN_B2", LIBRARY_SCRIPT_MAIN_B2 },
    { "LIBRARY_SCRIPT_MAIN_B3", LIBRARY_SCRIPT_MAIN_B3 },
    { "LIBRARY_SCRIPT_MAIN_B4", LIBRARY_SCRIPT_MAIN_B4 },
    { "LIBRARY_SCRIPT_MAIN_B5", LIBRARY_SCRIPT_MAIN_B5 },
    { "LIBRARY_SCRIPT_MAIN_C1", LIBRARY_SCRIPT_MAIN_C1 },
    { "LIBRARY_SCRIPT_MAIN_C2", LIBRARY_SCRIPT_MAIN_C2 },
    { "LIBRARY_SCRIPT_MAIN_C3", LIBRARY_SCRIPT_MAIN_C3 },
    { "LIBRARY_SCRIPT_MAIN_C4", LIBRARY_SCRIPT_MAIN_C4 },
    { "LIBRARY_SCRIPT_MAIN_C5", LIBRARY_SCRIPT_MAIN_C5 },
    { "LIBRARY_SCRIPT_EXTRA", LIBRARY_SCRIPT_EXTRA },
    { "LIBRARY_SCRIPT_EXTRA2", LIBRARY_SCRIPT_EXTRA2 },
    { 0, 0 },
};

#define LIBRARY_STEP_STRIDE ((int32_t) (sizeof(library_part_image_step) / sizeof(int32_t)))

static const config_field library_part_image_fields[] = {
    { "kind", &library_part_image_steps[0].kind, LIBRARY_STEP_STRIDE, 1, 0, LIBRARY_PART_KINDS - 1, 0 },
    { "until_stage", &library_part_image_steps[0].until_stage, LIBRARY_STEP_STRIDE, 1, 1, LIBRARY_STAGE_AFTER_COMPLETE, 0 },
    { "north_script", &library_part_image_steps[0].north.script, LIBRARY_STEP_STRIDE, 1, 0, LIBRARY_SCRIPTS - 1, library_script_names },
    { "north_image", &library_part_image_steps[0].north.image, LIBRARY_STEP_STRIDE, 1, 0, LIBRARY_IMAGES - 1, library_image_names },
    { "north_next_in_odd_views", &library_part_image_steps[0].north.next_in_odd_views, LIBRARY_STEP_STRIDE, 1, 0, 1, 0 },
    { "south_script", &library_part_image_steps[0].south.script, LIBRARY_STEP_STRIDE, 1, 0, LIBRARY_SCRIPTS - 1, library_script_names },
    { "south_image", &library_part_image_steps[0].south.image, LIBRARY_STEP_STRIDE, 1, 0, LIBRARY_IMAGES - 1, library_image_names },
    { "south_next_in_odd_views", &library_part_image_steps[0].south.next_in_odd_views, LIBRARY_STEP_STRIDE, 1, 0, 1, 0 },
};

static const config_storage library_part_image_storage[] = {
    { library_part_image_steps, sizeof(library_part_image_steps) },
};

const config_table library_part_image_config = {
    "library_part_images.json",
    "step",
    0,
    LIBRARY_PART_IMAGE_STEPS,
    CONFIG_TABLE_COLUMNS(library_part_image_fields, library_part_image_storage),
};

static void *royal_tomb_part_block(int32_t record, int32_t *count)
{
    *count = royal_tomb_part_tables[record].count;
    return royal_tomb_part_tables[record].parts;
}

static const config_field royal_tomb_part_fields[] = {
    { "dx", 0, 0, 1, -30, 30, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, dx), 1 },
    { "dy", 0, 0, 1, -30, 30, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, dy), 1 },
    { "size", 0, 0, 1, 1, 6, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, size), 1 },
    { "anim_key", 0, 0, 1, 0, ROYAL_TOMB_ANIM_KEYS - 1, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, anim_key), 1 },
    { "chamber_kind", 0, 0, 1, 0, ROYAL_TOMB_CHAMBER_KINDS - 1, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, chamber_kind), 1 },
    { "neighbours", 0, 0, ROYAL_TOMB_PART_NEIGHBOURS_LISTED, -1, GRAND_TOMB_PARTS - 1, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, neighbours), 1 },
    { "work", 0, 0, 1, 0, 100000, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, work) },
    { "completed_image", 0, 0, 1, 0, 1, 0, ANIM_FIELD_OFFSET(royal_tomb_part_init, completed_image), 1 },
    { "closed_image_variant", 0, 0, 1, ROYAL_TOMB_CLOSED_VARIANT_RANDOM, ROYAL_TOMB_CLOSED_VARIANTS - 1, 0,
        ANIM_FIELD_OFFSET(royal_tomb_part_init, closed_image_variant), 1 },
};

static const config_list royal_tomb_part_list = {
    "part",
    sizeof(royal_tomb_part_init),
    royal_tomb_part_fields,
    CONFIG_COUNT(royal_tomb_part_fields),
    royal_tomb_part_block,
    0,
};

static const config_field royal_tomb_part_table_fields[] = {
    { "parts", 0, 0, 1, 0, 0, 0, 0, 0, &royal_tomb_part_list },
};

const config_table royal_tomb_part_config = {
    "royal_tomb_parts.json",
    "tomb",
    royal_tomb_image_names,
    ROYAL_TOMB_SIZES,
    royal_tomb_part_table_fields,
    CONFIG_COUNT(royal_tomb_part_table_fields),
    0,
    0,
};

#define MONUMENT_UNPLACED_NEED_STRIDE ((int32_t) (sizeof(monument_unplaced_need) / sizeof(int32_t)))

static const config_field monument_unplaced_need_fields[] = {
    { "resource", &monument_unplaced_needs[0].resource, MONUMENT_UNPLACED_NEED_STRIDE, 1, 0, RESOURCE_MAX - 1, config_resource_names },
    { "amount", &monument_unplaced_needs[0].amount, MONUMENT_UNPLACED_NEED_STRIDE, 1, 0, 10000000, 0 },
    { "limestone", &monument_unplaced_needs[0].limestone, MONUMENT_UNPLACED_NEED_STRIDE, 1, 0, 10000000, 0 },
};

static const config_storage monument_unplaced_need_storage[] = {
    { monument_unplaced_needs, sizeof(monument_unplaced_needs) },
};

const config_table monument_unplaced_need_config = {
    "monument_unplaced_needs.json",
    "definition",
    config_monument_def_names,
    MONUMENT_DEFINITIONS,
    CONFIG_TABLE_COLUMNS(monument_unplaced_need_fields, monument_unplaced_need_storage),
};
