// Config tables of the building animations:
// - building_anim_offsets.json: the pixel offset of each named animation from its building's sprite origin;
// - shipwright_, fishing_wharf_, dock_ and water_lift_anim_offsets.json: the offsets per view or image;
// - granary_pile_spots.json and granary_pile_rules.json: the pile spots and the amounts that give a pile.

#include "core/config.h"
#include "widget/city_building_anims.h"

enum building_anim_config_limit {
    BUILDING_ANIM_OFFSET_LIMIT = 1000,
    GRANARY_PILE_AMOUNT_LIMIT = 100000,
};

enum { BUILDING_ANIM_OFFSET_STRIDE = sizeof(building_anim_offset) / sizeof(int32_t) };

static const config_name building_anim_names[] = {
    { "mine", BUILDING_ANIM_MINE },
    { "quarry_first", BUILDING_ANIM_QUARRY_FIRST },
    { "quarry_second", BUILDING_ANIM_QUARRY_SECOND },
    { "bandstand_gate", BUILDING_ANIM_BANDSTAND_GATE },
    { "bandstand_stage", BUILDING_ANIM_BANDSTAND_STAGE },
    { "conservatory", BUILDING_ANIM_CONSERVATORY },
    { "booth", BUILDING_ANIM_BOOTH },
    { "pavilion", BUILDING_ANIM_PAVILION },
    { "dance_school", BUILDING_ANIM_DANCE_SCHOOL },
    { "granary", BUILDING_ANIM_GRANARY },
    { "storage_yard", BUILDING_ANIM_STORAGE_YARD },
    { "storage_yard_roof", BUILDING_ANIM_STORAGE_YARD_ROOF },
    { "fort_flag", BUILDING_ANIM_FORT_FLAG },
    { "house_sick_flag", BUILDING_ANIM_HOUSE_SICK_FLAG },
    { 0, 0 },
};

static const config_field building_anim_offset_fields[] = {
    { "dx", &building_anim_offsets[0].dx, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
    { "dy", &building_anim_offsets[0].dy, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
};

static const config_storage building_anim_offset_storage[] = {
    { building_anim_offsets, sizeof(building_anim_offsets) },
};

const config_table building_anim_offset_table = {
    "building_anim_offsets.json",
    "anim",
    building_anim_names,
    BUILDING_ANIMS,
    CONFIG_TABLE_COLUMNS(building_anim_offset_fields, building_anim_offset_storage),
};

static const config_field shipwright_anim_offset_fields[] = {
    { "dx", &shipwright_anim_offsets[0].dx, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
    { "dy", &shipwright_anim_offsets[0].dy, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
};

static const config_storage shipwright_anim_offset_storage[] = {
    { shipwright_anim_offsets, sizeof(shipwright_anim_offsets) },
};

const config_table shipwright_anim_offset_table = {
    "shipwright_anim_offsets.json",
    "view",
    0,
    SHIPWRIGHT_ANIM_VIEWS,
    CONFIG_TABLE_COLUMNS(shipwright_anim_offset_fields, shipwright_anim_offset_storage),
};

static const config_field fishing_wharf_anim_offset_fields[] = {
    { "dx", &fishing_wharf_anim_offsets[0].dx, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
    { "dy", &fishing_wharf_anim_offsets[0].dy, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
};

static const config_storage fishing_wharf_anim_offset_storage[] = {
    { fishing_wharf_anim_offsets, sizeof(fishing_wharf_anim_offsets) },
};

const config_table fishing_wharf_anim_offset_table = {
    "fishing_wharf_anim_offsets.json",
    "image",
    0,
    FISHING_WHARF_ANIM_IMAGES,
    CONFIG_TABLE_COLUMNS(fishing_wharf_anim_offset_fields, fishing_wharf_anim_offset_storage),
};

static const config_field dock_anim_offset_fields[] = {
    { "dx", &dock_anim_offsets[0].dx, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
    { "dy", &dock_anim_offsets[0].dy, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
};

static const config_storage dock_anim_offset_storage[] = {
    { dock_anim_offsets, sizeof(dock_anim_offsets) },
};

const config_table dock_anim_offset_table = {
    "dock_anim_offsets.json",
    "image",
    0,
    DOCK_ANIM_IMAGES,
    CONFIG_TABLE_COLUMNS(dock_anim_offset_fields, dock_anim_offset_storage),
};

static const config_field water_lift_anim_offset_fields[] = {
    { "dx", &water_lift_anim_offsets[0].dx, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
    { "dy", &water_lift_anim_offsets[0].dy, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
};

static const config_storage water_lift_anim_offset_storage[] = {
    { water_lift_anim_offsets, sizeof(water_lift_anim_offsets) },
};

const config_table water_lift_anim_offset_table = {
    "water_lift_anim_offsets.json",
    "view",
    0,
    WATER_LIFT_ANIM_VIEWS,
    CONFIG_TABLE_COLUMNS(water_lift_anim_offset_fields, water_lift_anim_offset_storage),
};

static const config_field granary_pile_spot_fields[] = {
    { "dx", &granary_pile_spots[0].dx, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
    { "dy", &granary_pile_spots[0].dy, BUILDING_ANIM_OFFSET_STRIDE, 1, -BUILDING_ANIM_OFFSET_LIMIT, BUILDING_ANIM_OFFSET_LIMIT, 0 },
};

static const config_storage granary_pile_spot_storage[] = {
    { granary_pile_spots, sizeof(granary_pile_spots) },
};

const config_table granary_pile_spot_table = {
    "granary_pile_spots.json",
    "spot",
    0,
    GRANARY_PILE_SPOTS,
    CONFIG_TABLE_COLUMNS(granary_pile_spot_fields, granary_pile_spot_storage),
};

static const config_name granary_pile_rule_names[] = {
    { "first_amount", GRANARY_PILE_RULE_FIRST_AMOUNT },
    { "amount_step", GRANARY_PILE_RULE_AMOUNT_STEP },
    { "full_amount", GRANARY_PILE_RULE_FULL_AMOUNT },
    { "max_piles", GRANARY_PILE_RULE_MAX_PILES },
    { 0, 0 },
};

static const config_field granary_pile_rule_fields[] = {
    { "value", granary_pile_rules, 1, 1, 1, GRANARY_PILE_AMOUNT_LIMIT, 0 },
};

static const config_storage granary_pile_rule_storage[] = {
    { granary_pile_rules, sizeof(granary_pile_rules) },
};

const config_table granary_pile_rule_table = {
    "granary_pile_rules.json",
    "rule",
    granary_pile_rule_names,
    GRANARY_PILE_RULES,
    CONFIG_TABLE_COLUMNS(granary_pile_rule_fields, granary_pile_rule_storage),
};
