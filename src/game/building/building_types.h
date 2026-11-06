// The building record with its type-specific data, one union member per kind of building, the building model record,
// and city_buildings: the building records followed by the bazaar slots' nearest storage ids.
//
// A monument part's subtype is its part index, so the main part's id is the part's id minus its subtype; the
// parts are chained by their previous and next part ids.

#ifndef BUILDING_BUILDING_TYPES_H
#define BUILDING_BUILDING_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_BUILDINGS 4000

enum building_state {
    BUILDING_STATE_UNUSED = 0,
    BUILDING_STATE_IN_USE = 1,
    BUILDING_STATE_UNDO = 2,
    BUILDING_STATE_CREATED = 3,
    BUILDING_STATE_RUBBLE = 4,
    BUILDING_STATE_DELETED_BY_GAME = 5,
    BUILDING_STATE_DELETED = 6,
};

enum house_sickness {
    HOUSE_NOT_SICK = 0,
    HOUSE_SICK_DISEASE = 1,
    HOUSE_SICK_FROGS = 2,
};

typedef struct {
    int32_t cost;
    int32_t desirability_value;
    int32_t desirability_step;
    int32_t desirability_step_size;
    int32_t desirability_range;
    int32_t laborers;
    int32_t fire_risk_increment;
    int32_t damage_risk_increment;
    int32_t walker_cost;
    int32_t service_reduction;
} building_model_record;

static_assert(sizeof(building_model_record) == 0x28, "building_model_record is a model file row of 40 bytes");
static_assert(offsetof(building_model_record, service_reduction) == 0x24, "building_model_record is the model file's layout");

typedef struct {
    uint8_t size_variant;
    uint8_t stage;
    uint8_t blocks;
    uint8_t layer;
    uint8_t target_layers;
    uint8_t ramp1_height;
    uint8_t ramp2_height;
    uint8_t prep_mask;
    int16_t ramp_image_x1;
    int16_t ramp_image_y1;
    int16_t ramp_image_x2;
    int16_t ramp_image_y2;
    int16_t worker_figure_id;
    uint8_t reserved_1[2];
    uint32_t ramp_image_id;
    uint32_t ramp_link_building_id;
    int32_t ramp_link_draw_x;
    int32_t ramp_link_draw_y;
    int32_t ramp_link_x;
    int32_t ramp_link_y;
    uint8_t completion_shown;
    uint8_t reserved_2[55];
} building_pyramid;

typedef struct {
    int32_t city_slot;
    int32_t stage;
    int32_t worker_figure_id;
    int32_t is_main_part;
    int32_t work_flags;
    uint8_t completion_shown;
    uint8_t reserved_1[79];
} building_monument;

typedef struct {
    uint8_t reserved_1;
    uint8_t progress;
    uint8_t reserved_2[42];
    uint8_t completion_shown;
    uint8_t reserved_3[55];
} building_obelisk;

typedef struct {
    int32_t stage;
    int32_t monument_stage;
    int32_t worker_figure_id;
    uint8_t completion_shown;
    uint8_t built_parts[12];
    uint8_t reserved_1[75];
} building_abu_simbel;

typedef struct {
    int32_t city_slot;
    int32_t stage;
    int32_t worker_figure_id;
    int32_t is_main_part;
    int32_t work_flags;
    int32_t origin_x;
    int32_t origin_y;
    int32_t orientation;
    uint8_t completion_shown;
    uint8_t reserved_1[67];
} building_mausoleum;

typedef struct {
    int32_t city_slot;
    int32_t completed;
    int32_t worker_figure_id;
    int32_t stage;
    int32_t can_take_workers;
    int32_t chamber_kind;
    int32_t neighbour_offsets[6];
    int32_t remaining_work[3];
    int32_t anim_key;
    uint8_t completed_image;
    uint8_t reserved_1[3];
    int32_t stock;
    uint8_t closed_image_variant;
    uint8_t completion_shown;
    uint8_t reserved_2[26];
} building_royal_tomb;

typedef struct {
    int16_t inventory[12];
    uint8_t juggler;
    uint8_t musician;
    uint8_t physician;
    uint8_t dancer;
    uint8_t courthouse;
    uint8_t senet;
    uint8_t school;
    uint8_t library;
    uint8_t reserved_1;
    uint8_t dentist;
    uint8_t apothecary;
    uint8_t water_supply;
    uint8_t mortuary;
    uint8_t temple[5];
    uint8_t no_space_to_expand;
    uint8_t food_types;
    int8_t entertainment;
    int8_t education;
    int8_t health;
    int8_t num_gods;
    uint8_t devolve_delay;
    uint8_t evolve_text_id;
    uint8_t zoo;
    uint8_t is_sick;
    uint8_t tax_bonus;
    uint8_t padding_1[3];
    int32_t tax_income;
    uint8_t reserved_2[40];
} building_house;

typedef struct {
    int16_t reserved_1;
    int16_t inventory[12];
    uint8_t reserved_2[48];
    int16_t good_demands[4];
    uint8_t reserved_3[6];
    uint8_t fetch_inventory_id;
    uint8_t reserved_4[7];
    int16_t second_trader_id;
    int16_t second_buyer_id;
} building_market;

enum building_dock {
    BUILDING_DOCK_DOCKERS = 3,
};

enum market_good_demand {
    MARKET_DEMAND_POTTERY = 0,
    MARKET_DEMAND_JEWELRY = 1,
    MARKET_DEMAND_LINEN = 2,
    MARKET_DEMAND_BEER = 3,
};

typedef struct {
    int16_t reserved_1;
    int16_t resource_stored[36];
    uint8_t reserved_2[24];
    int16_t second_cart_id;
} building_granary;

typedef struct {
    uint8_t reserved_1[98];
    int16_t second_cart_id;
} building_storage_yard;

typedef struct {
    int16_t progress;
    uint8_t reserved_1[6];
    int32_t reserved_2;
    uint8_t reserved_3[4];
    int16_t fish_caught;
    uint8_t reserved_4[64];
    uint8_t worker_count;
    uint8_t labor_days_left[3];
    uint8_t orientation;
    uint8_t reserved_5[2];
    uint8_t curse_days_left;
    uint8_t reserved_6[6];
    int16_t worker_figure_id;
    int16_t second_cart_id;
} building_industry;

typedef struct {
    int16_t progress;
    uint8_t reserved_1[84];
    uint8_t orientation;
    uint8_t reserved_2[9];
    uint16_t boat_order;
    int16_t reserved_3;
} building_shipyard;

typedef struct {
    int16_t image_variant;
    uint8_t reserved_1[98];
} building_native;

typedef struct {
    uint8_t reserved_1[74];
    int16_t juggler_id;
    int16_t musician_id;
    int16_t dancer_id;
    int16_t reserved_2;
    uint8_t num_shows;
    uint8_t show_days_left[3];
    uint8_t reserved_3[14];
} building_entertainment;

typedef struct {
    int16_t unloading_docker_id;
    uint8_t reserved_1[81];
    uint8_t ship_wait_days;
    uint8_t reserved_2[2];
    uint8_t orientation;
    uint8_t reserved_3[3];
    int16_t docker_ids[BUILDING_DOCK_DOCKERS];
    int16_t ship_id;
    int16_t docked_boat_id;
} building_dock;

typedef struct {
    uint8_t reserved_1[85];
    uint8_t upgrades;
    uint8_t orientation;
    uint8_t reserved_2[13];
} building_temple_complex;

typedef struct {
    uint8_t reserved_1[86];
    uint8_t orientation;
    uint8_t reserved_2[13];
} building_oriented;

typedef struct {
    uint8_t reserved_1[74];
    int16_t aqueduct_links[4];
    uint8_t reserved_2[18];
} building_water_lift;

typedef struct {
    uint8_t state;
    uint8_t player;
    uint8_t reserved_id;
    uint8_t size;
    uint8_t house_is_merged;
    uint8_t house_size;
    int16_t x;
    int16_t y;
    uint8_t padding_2[2];
    int32_t grid_offset;
    int16_t type;
    int16_t subtype;
    uint8_t road_network_id;
    uint8_t padding_3;
    uint16_t created_sequence;
    int16_t houses_covered;
    int16_t percentage_houses_covered;
    int16_t house_population;
    int16_t house_population_room;
    int16_t distance_from_entry;
    int16_t house_highest_population;
    int16_t unreachable_ticks;
    int16_t road_access_x;
    int16_t road_access_y;
    int16_t figure_id;
    int16_t figure_id2;
    int16_t immigrant_figure_id;
    int16_t figure_id4;
    uint8_t figure_spawn_delay;
    uint8_t reserved_1;
    uint8_t figure_roam_direction;
    uint8_t has_water_access;
    uint8_t water_dependency;
    uint8_t padding_4;
    int16_t prev_part_building_id;
    int16_t next_part_building_id;
    int16_t loads_stored;
    uint8_t reserved_2;
    uint8_t has_well_access;
    int16_t num_workers;
    int8_t labor_category;
    uint8_t output_resource;
    uint8_t has_road_access;
    uint8_t padding_5;
    int16_t damage_risk;
    int16_t fire_risk;
    int16_t fire_duration;
    uint8_t fire_proof;
    uint8_t house_figure_generation_delay;
    uint8_t house_tax_coverage;
    uint8_t padding_6;
    int16_t formation_id;
    uint8_t padding_7[2];
    union {
        building_house house;
        building_pyramid pyramid;
        building_monument monument;
        building_obelisk obelisk;
        building_abu_simbel abu_simbel;
        building_mausoleum mausoleum;
        building_royal_tomb royal_tomb;
        building_market market;
        building_granary granary;
        building_storage_yard storage_yard;
        building_industry industry;
        building_shipyard shipyard;
        building_native native;
        building_entertainment entertainment;
        building_dock dock;
        building_temple_complex temple_complex;
        building_oriented oriented;
        building_water_lift water_lift;
    } data;
    int32_t tax_income_or_storage;
    uint8_t house_months_without_food;
    uint8_t ruin_has_plague;
    int8_t desirability;
    uint8_t is_deleted;
    uint8_t is_adjacent_to_water;
    uint8_t storage_id;
    union {
        int8_t house;
        uint8_t native_anger;
    } sentiment;
    uint8_t show_on_problem_overlay;
    int32_t linked_building_id;
    int16_t monument_remaining_blocks;
    uint8_t queue_index;
    uint8_t queue_slot;
    int16_t second_input_stock;
    int16_t second_input_resource;
    int32_t venue_plaza_grid_offset;
    int32_t venue_orientation;
    int32_t venue_has_plaza;
    int32_t info_orders_toggle;
    int32_t info_orders_scroll;
    int32_t crime_risk;
    int32_t house_malaria_risk;
    int32_t house_plague_distance;
    int32_t house_sick_months;
    int32_t house_water_distance;
    int32_t house_marsh_distance;
    int32_t house_plague_risk;
    int32_t problem_flags;
    int16_t incoming_carts;
    int16_t incoming_loads;
} building;

static_assert(sizeof(building) == 0x108, "building is a savegame record of 264 bytes");
static_assert(offsetof(building, prev_part_building_id) == 0x38 && offsetof(building, data.pyramid.worker_figure_id) == 0x64 &&
                  offsetof(building, data.pyramid.completion_shown) == 0x80 && offsetof(building, data.monument.work_flags) == 0x64 &&
                  offsetof(building, monument_remaining_blocks) == 0xC8 && offsetof(building, house_population) == 0x1C &&
                  offsetof(building, immigrant_figure_id) == 0x2E && offsetof(building, has_water_access) == 0x35 &&
                  offsetof(building, tax_income_or_storage) == 0xB8, "building is the savegame's layout");
static_assert(offsetof(building, houses_covered) == 0x18 && offsetof(building, distance_from_entry) == 0x20 && offsetof(building, figure_id4) == 0x30 &&
                  offsetof(building, has_well_access) == 0x3F && offsetof(building, num_workers) == 0x40 && offsetof(building, output_resource) == 0x43 &&
                  offsetof(building, damage_risk) == 0x46 && offsetof(building, house_tax_coverage) == 0x4E && offsetof(building, formation_id) == 0x50 &&
                  offsetof(building, data.house.juggler) == 0x6C && offsetof(building, data.house.water_supply) == 0x77 &&
                  offsetof(building, data.house.temple) == 0x79 && offsetof(building, data.house.food_types) == 0x7F &&
                  offsetof(building, data.house.num_gods) == 0x83 && offsetof(building, data.house.evolve_text_id) == 0x85 &&
                  offsetof(building, data.house.is_sick) == 0x87 && offsetof(building, data.house.tax_income) == 0x8C &&
                  offsetof(building, house_months_without_food) == 0xBC && offsetof(building, storage_id) == 0xC1 &&
                  offsetof(building, linked_building_id) == 0xC4 && offsetof(building, queue_index) == 0xCA &&
                  offsetof(building, second_input_resource) == 0xCE && offsetof(building, crime_risk) == 0xE4 &&
                  offsetof(building, house_plague_risk) == 0xFC && offsetof(building, is_deleted) == 0xBF &&
                  offsetof(building, venue_plaza_grid_offset) == 0xD0 && offsetof(building, info_orders_scroll) == 0xE0 &&
                  offsetof(building, data.house.courthouse) == 0x70 && offsetof(building, data.monument.completion_shown) == 0x68 &&
                  offsetof(building, problem_flags) == 0x100 && offsetof(building, incoming_loads) == 0x106, "building is the savegame's layout");
static_assert(offsetof(building, player) == 0x01 && offsetof(building, x) == 0x06 && offsetof(building, grid_offset) == 0x0C &&
                  offsetof(building, type) == 0x10 && offsetof(building, subtype) == 0x12 && offsetof(building, road_network_id) == 0x14 &&
                  offsetof(building, created_sequence) == 0x16 && offsetof(building, percentage_houses_covered) == 0x1A &&
                  offsetof(building, house_population_room) == 0x1E && offsetof(building, house_highest_population) == 0x22 &&
                  offsetof(building, unreachable_ticks) == 0x24 && offsetof(building, road_access_x) == 0x26 && offsetof(building, figure_id) == 0x2A &&
                  offsetof(building, figure_id2) == 0x2C && offsetof(building, figure_spawn_delay) == 0x32 && offsetof(building, reserved_1) == 0x33 &&
                  offsetof(building, figure_roam_direction) == 0x34 && offsetof(building, water_dependency) == 0x36 &&
                  offsetof(building, loads_stored) == 0x3C && offsetof(building, reserved_2) == 0x3E && offsetof(building, labor_category) == 0x42 &&
                  offsetof(building, has_road_access) == 0x44 && offsetof(building, fire_risk) == 0x48 && offsetof(building, fire_duration) == 0x4A &&
                  offsetof(building, fire_proof) == 0x4C && offsetof(building, house_figure_generation_delay) == 0x4D && offsetof(building, data) == 0x54 &&
                  offsetof(building, ruin_has_plague) == 0xBD && offsetof(building, desirability) == 0xBE && offsetof(building, is_adjacent_to_water) == 0xC0 &&
                  offsetof(building, sentiment) == 0xC2 && offsetof(building, show_on_problem_overlay) == 0xC3 && offsetof(building, queue_slot) == 0xCB &&
                  offsetof(building, second_input_stock) == 0xCC && offsetof(building, venue_orientation) == 0xD4 &&
                  offsetof(building, venue_has_plaza) == 0xD8 && offsetof(building, info_orders_toggle) == 0xDC &&
                  offsetof(building, house_malaria_risk) == 0xE8 && offsetof(building, house_plague_distance) == 0xEC &&
                  offsetof(building, house_sick_months) == 0xF0 && offsetof(building, house_water_distance) == 0xF4 &&
                  offsetof(building, house_marsh_distance) == 0xF8 && offsetof(building, incoming_carts) == 0x104, "building is the savegame's layout");
static_assert(offsetof(building, data.house.inventory) == 0x54 && offsetof(building, data.house.musician) == 0x6D &&
                  offsetof(building, data.house.physician) == 0x6E && offsetof(building, data.house.dancer) == 0x6F &&
                  offsetof(building, data.house.senet) == 0x71 && offsetof(building, data.house.school) == 0x72 &&
                  offsetof(building, data.house.library) == 0x73 && offsetof(building, data.house.reserved_1) == 0x74 &&
                  offsetof(building, data.house.dentist) == 0x75 && offsetof(building, data.house.apothecary) == 0x76 &&
                  offsetof(building, data.house.mortuary) == 0x78 && offsetof(building, data.house.no_space_to_expand) == 0x7E &&
                  offsetof(building, data.house.entertainment) == 0x80 && offsetof(building, data.house.education) == 0x81 &&
                  offsetof(building, data.house.health) == 0x82 && offsetof(building, data.house.devolve_delay) == 0x84 &&
                  offsetof(building, data.house.zoo) == 0x86 && offsetof(building, data.house.tax_bonus) == 0x88 && sizeof(building_house) == 0x64,
              "building is the savegame's layout");
static_assert(offsetof(building, data.pyramid.stage) == 0x55 && offsetof(building, data.pyramid.blocks) == 0x56 &&
                  offsetof(building, data.pyramid.ramp1_height) == 0x59 && offsetof(building, data.pyramid.prep_mask) == 0x5B &&
                  offsetof(building, data.pyramid.ramp_image_id) == 0x68 && offsetof(building, data.pyramid.ramp_link_building_id) == 0x6C &&
                  sizeof(building_pyramid) == 0x64 && offsetof(building, data.monument.city_slot) == 0x54 && offsetof(building, data.monument.stage) == 0x58 &&
                  offsetof(building, data.monument.worker_figure_id) == 0x5C && offsetof(building, data.monument.is_main_part) == 0x60 &&
                  sizeof(building_monument) == 0x64 && offsetof(building, data.obelisk.progress) == 0x55 &&
                  offsetof(building, data.obelisk.completion_shown) == 0x80 && sizeof(building_obelisk) == 0x64 &&
                  offsetof(building, data.abu_simbel.stage) == 0x54 && offsetof(building, data.abu_simbel.worker_figure_id) == 0x5C &&
                  offsetof(building, data.abu_simbel.completion_shown) == 0x60 && offsetof(building, data.abu_simbel.built_parts) == 0x61 &&
                  sizeof(building_abu_simbel) == 0x64 && offsetof(building, data.mausoleum.is_main_part) == 0x60 &&
                  offsetof(building, data.mausoleum.work_flags) == 0x64 && offsetof(building, data.mausoleum.origin_x) == 0x68 &&
                  offsetof(building, data.mausoleum.origin_y) == 0x6C && offsetof(building, data.mausoleum.orientation) == 0x70 &&
                  offsetof(building, data.mausoleum.completion_shown) == 0x74 && sizeof(building_mausoleum) == 0x64 &&
                  offsetof(building, data.royal_tomb.completed) == 0x58 && offsetof(building, data.royal_tomb.stage) == 0x60 &&
                  offsetof(building, data.royal_tomb.can_take_workers) == 0x64 && offsetof(building, data.royal_tomb.chamber_kind) == 0x68 &&
                  offsetof(building, data.royal_tomb.neighbour_offsets) == 0x6C && offsetof(building, data.royal_tomb.remaining_work) == 0x84 &&
                  offsetof(building, data.royal_tomb.anim_key) == 0x90 && offsetof(building, data.royal_tomb.completed_image) == 0x94 &&
                  offsetof(building, data.royal_tomb.stock) == 0x98 && offsetof(building, data.royal_tomb.closed_image_variant) == 0x9C &&
                  offsetof(building, data.royal_tomb.completion_shown) == 0x9D && sizeof(building_royal_tomb) == 0x64, "building is the savegame's layout");
static_assert(offsetof(building, data.market.inventory) == 0x56 && offsetof(building, data.market.good_demands) == 0x9E &&
                  offsetof(building, data.market.fetch_inventory_id) == 0xAC && offsetof(building, data.market.second_trader_id) == 0xB4 &&
                  offsetof(building, data.market.second_buyer_id) == 0xB6 && sizeof(building_market) == 0x64 &&
                  offsetof(building, data.granary.resource_stored) == 0x56 && offsetof(building, data.granary.second_cart_id) == 0xB6 &&
                  sizeof(building_granary) == 0x64 && offsetof(building, data.storage_yard.second_cart_id) == 0xB6 && sizeof(building_storage_yard) == 0x64 &&
                  offsetof(building, data.industry.progress) == 0x54 && offsetof(building, data.industry.reserved_2) == 0x5C &&
                  offsetof(building, data.industry.fish_caught) == 0x64 && offsetof(building, data.industry.worker_count) == 0xA6 &&
                  offsetof(building, data.industry.labor_days_left) == 0xA7 && offsetof(building, data.industry.orientation) == 0xAA &&
                  offsetof(building, data.industry.curse_days_left) == 0xAD && offsetof(building, data.industry.worker_figure_id) == 0xB4 &&
                  sizeof(building_industry) == 0x64 && offsetof(building, data.shipyard.orientation) == 0xAA &&
                  offsetof(building, data.shipyard.boat_order) == 0xB4 && offsetof(building, data.shipyard.reserved_3) == 0xB6 &&
                  sizeof(building_shipyard) == 0x64 && offsetof(building, data.industry.second_cart_id) == 0xB6 &&
                  offsetof(building, data.native.image_variant) == 0x54 && sizeof(building_native) == 0x64 &&
                  offsetof(building, data.entertainment.juggler_id) == 0x9E && offsetof(building, data.entertainment.musician_id) == 0xA0 &&
                  offsetof(building, data.entertainment.dancer_id) == 0xA2 && offsetof(building, data.entertainment.num_shows) == 0xA6 &&
                  offsetof(building, data.entertainment.show_days_left) == 0xA7 && sizeof(building_entertainment) == 0x64 &&
                  offsetof(building, data.dock.unloading_docker_id) == 0x54 && offsetof(building, data.dock.ship_wait_days) == 0xA7 &&
                  offsetof(building, data.dock.orientation) == 0xAA && offsetof(building, data.dock.docker_ids) == 0xAE &&
                  offsetof(building, data.dock.ship_id) == 0xB4 && offsetof(building, data.dock.docked_boat_id) == 0xB6 && sizeof(building_dock) == 0x64 &&
                  offsetof(building, data.temple_complex.upgrades) == 0xA9 && offsetof(building, data.temple_complex.orientation) == 0xAA &&
                  sizeof(building_temple_complex) == 0x64 && offsetof(building, data.oriented.orientation) == 0xAA && sizeof(building_oriented) == 0x64 &&
                  offsetof(building, data.water_lift.aqueduct_links) == 0x9E && sizeof(building_water_lift) == 0x64, "building is the savegame's layout");

enum { MARKET_SLOTS = 27 };

typedef struct building_table {
    building records[MAX_BUILDINGS];
    int32_t market_nearest_storage[MARKET_SLOTS];
} building_table;

static_assert(sizeof(building_table) == 1056108, "building_table is a savegame record of 1056108 bytes");
static_assert(offsetof(building_table, market_nearest_storage) == 0x101D00, "building_table is the savegame's layout");

#endif
