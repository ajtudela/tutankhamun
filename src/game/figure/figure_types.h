// The figure record and the record of city_figures (the MAX_FIGURES figures followed by the route paths, saved as two
// chunks). figure_current_id is MAX_FIGURES after the figure loops, so the figure written through it there lands in
// the first route path.

#ifndef FIGURE_FIGURE_TYPES_H
#define FIGURE_FIGURE_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_PATH_LENGTH 500
#define MAX_PATHS 1000

#define MAX_FIGURES 2000

enum figure_tile_list {
    FIGURE_TILE_INDEX_MAX = 20,
};

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201)
#endif
enum figure_state {
    FIGURE_STATE_NONE = 0,
    FIGURE_STATE_ALIVE = 1,
    FIGURE_STATE_DEAD = 2,
    FIGURE_STATE_SUNK = 4,
};

enum figure_boat_kind {
    FIGURE_BOAT_NONE = 0,
    FIGURE_BOAT_SHIP = 1,
    FIGURE_BOAT_FLOTSAM = 2,
};

enum figure_terrain_usage {
    TERRAIN_USAGE_ANY = 0,
    TERRAIN_USAGE_ROADS = 1,
    TERRAIN_USAGE_ENEMY = 2,
    TERRAIN_USAGE_PREFER_ROADS = 3,
    TERRAIN_USAGE_WALLS = 4,
    TERRAIN_USAGE_ANIMAL = 5,
    TERRAIN_USAGE_AMPHIBIA = 6,
    TERRAIN_USAGE_ROADS_OR_MONUMENT = 7,
    TERRAIN_USAGE_NO_FERRY = 9,
    TERRAIN_USAGE_ANY_OR_FORT = 10,
    TERRAIN_USAGE_ANYWHERE = 11,
};

typedef struct {
    uint8_t alternative_location_index;
    uint8_t animation_frame;
    uint8_t answers_requests;
    uint8_t flotsam_skip_tick;
    int32_t image_id;
    int16_t next_figure_id_on_same_tile;
    uint8_t type;
    union {
        uint8_t resource_id;
        uint8_t flotsam_kind;
        uint8_t editor_flag_number;
    };
    uint8_t use_cross_country;
    uint8_t is_friendly;
    uint8_t state;
    uint8_t faction_id;
    uint8_t action_state_before_attack;
    int8_t direction;
    int8_t previous_tile_direction;
    int8_t attack_direction;
    int16_t x;
    int16_t y;
    int16_t previous_tile_x;
    int16_t previous_tile_y;
    uint8_t missile_damage;
    uint8_t damage;
    uint8_t reserved_1[2];
    int32_t grid_offset;
    int16_t destination_x;
    int16_t destination_y;
    int32_t destination_grid_offset;
    int16_t source_x;
    int16_t source_y;
    union {
        int16_t formation_position_x;
        int16_t ship_landing_x;
    };
    union {
        int16_t formation_position_y;
        int16_t ship_landing_y;
    };
    uint8_t reserved_2[2];
    int16_t wait_ticks;
    uint8_t action_state;
    int8_t progress_on_tile;
    int16_t routing_path_id;
    int16_t routing_path_current_tile;
    int16_t routing_path_length;
    uint8_t in_building_wait_ticks;
    uint8_t is_on_road;
    int16_t max_roam_length;
    int16_t roam_length;
    uint8_t roam_choose_destination;
    uint8_t roam_random_counter;
    int8_t roam_turn_direction;
    int8_t roam_ticks_until_next_turn;
    union {
        struct {
            int16_t x;
            int16_t y;
            int16_t destination_x;
            int16_t destination_y;
            int16_t delta_x;
            int16_t delta_y;
            int16_t delta_xy;
        } cross_country;
        struct {
            int16_t previous_part;
            int16_t previous_height;
            int16_t part;
            int16_t height;
            int16_t progress;
            int16_t total;
        } pyramid_work;
    };
    uint8_t cc_direction;
    uint8_t speed_multiplier;
    int16_t building_id;
    union {
        int16_t immigrant_building_id;
        int16_t tomb_building_id;
    };
    union {
        int16_t destination_building_id;
        int16_t prey_figure_id;
    };
    int16_t formation_id;
    uint8_t index_in_formation;
    uint8_t formation_at_rest;
    uint8_t migrant_num_people;
    uint8_t is_ghost;
    union {
        uint8_t min_max_seen;
        uint8_t flotsam_special;
    };
    uint8_t reserved_3;
    int16_t leading_figure_id;
    uint8_t attack_image_offset;
    union {
        uint8_t wait_ticks_missile;
        uint8_t hunt_ticks;
        uint8_t cycle_ticks;
    };
    int8_t cart_offset_x;
    int8_t cart_offset_y;
    uint8_t empire_city_id;
    uint8_t trader_amount_bought;
    int16_t name_id;
    uint8_t terrain_usage;
    uint8_t reserved_4;
    union {
        int16_t resource_amount;
        int16_t extinguish_ticks;
    };
    uint8_t is_boat;
    uint8_t height_adjusted_ticks;
    uint8_t current_height;
    uint8_t target_height;
    union {
        uint8_t collecting_item_id;
        uint8_t ship_dock_index;
    };
    uint8_t ship_dock_attempts;
    uint8_t phrase_sequence_exact;
    uint8_t phrase_id;
    uint8_t phrase_sequence_city;
    uint8_t trader_id;
    union {
        uint8_t wait_ticks_next_target;
        uint8_t ship_leave_month;
    };
    uint8_t reserved_5;
    int16_t target_figure_id;
    int16_t targeted_by_figure_id;
    int16_t created_sequence;
    int16_t target_figure_created_sequence;
    uint8_t index_on_same_tile;
    uint8_t num_attackers;
    int16_t attacker_id1;
    int16_t attacker_id2;
    int16_t opponent_id;
    union {
        uint8_t ship_formation_id;
        uint8_t flee_ticks;
        uint8_t spot_wait_ticks;
        uint8_t reroute_ticks;
    };
    uint8_t buyer_storages_visited;
    uint8_t reserved_6[4];
    int32_t name_group;
    int32_t buyer_resources[4];
    int32_t buyer_storage_ids[4];
    int32_t buyer_amounts_bought[4];
    uint8_t reserved_7[80];
    int32_t ferry_path_index;
    int16_t pyramid_walk_mode;
    uint8_t reserved_8[2];
    int32_t reserved_9;
    int32_t ship_hit_points;
    int32_t ship_max_hit_points;
    uint8_t has_troops_on_board;
    uint8_t phrase_group;
    int16_t request_event_id;
    union {
        struct {
            int32_t steps;
            int32_t reserved_1;
            int32_t tile_x;
            int32_t tile_y;
        } charge;
        struct {
            int32_t reserved_1;
            int32_t hunter_building_id;
            int32_t reserved_2[2];
            int32_t frame_count;
            int32_t frame;
        } animal;
        struct {
            int32_t steps;
            int32_t moves;
            int32_t speed;
            int32_t max_straight_steps;
            int32_t model_column_m;
            int32_t model_column_n;
        } ship;
    };
    int16_t patrol_x;
    int16_t patrol_y;
    int32_t ram_damage_slow;
    union {
        uint8_t retry_count;
        uint8_t ship_move_mode;
        uint8_t lost_ticks;
    };
    uint8_t reserved_10[3];
    int32_t ram_damage_fast;
    int32_t warship_secondary_action;
    uint8_t move_speed_counter;
    uint8_t reserved_11[3];
    union {
        int32_t ram_cooldown;
        int32_t festival_trips;
    };
    uint8_t reserved_12[4];
    int32_t invasion_soldiers;
    int32_t invasion_id;
    int32_t invasion_nation;
    int32_t invasion_formation_param;
    int32_t invasion_event_id;
    int32_t cart_image_id;
} figure;
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

static_assert(sizeof(figure) == 0x184, "figure is a savegame record of 388 bytes");
static_assert(offsetof(figure, progress_on_tile) == 0x39 && offsetof(figure, is_boat) == 0x76 && offsetof(figure, cc_direction) == 0x58 &&
                  offsetof(figure, move_speed_counter) == 0x160, "figure is the savegame's layout");
static_assert(offsetof(figure, alternative_location_index) == 0x00 && offsetof(figure, animation_frame) == 0x01 && offsetof(figure, answers_requests) == 0x02 &&
                  offsetof(figure, flotsam_skip_tick) == 0x03 && offsetof(figure, image_id) == 0x04 && offsetof(figure, next_figure_id_on_same_tile) == 0x08 &&
                  offsetof(figure, type) == 0x0A && offsetof(figure, resource_id) == 0x0B && offsetof(figure, flotsam_kind) == 0x0B &&
                  offsetof(figure, grid_offset) == 0x20 && offsetof(figure, formation_position_x) == 0x30 && offsetof(figure, formation_position_y) == 0x32 &&
                  offsetof(figure, ship_landing_x) == 0x30 && offsetof(figure, ship_landing_y) == 0x32 && offsetof(figure, reserved_2) == 0x34 &&
                  offsetof(figure, wait_ticks) == 0x36 && offsetof(figure, cross_country.x) == 0x4A && offsetof(figure, cross_country.destination_x) == 0x4E &&
                  offsetof(figure, cross_country.delta_xy) == 0x56 && offsetof(figure, pyramid_work.previous_part) == 0x4A &&
                  offsetof(figure, pyramid_work.part) == 0x4E && offsetof(figure, pyramid_work.total) == 0x54 &&
                  offsetof(figure, immigrant_building_id) == 0x5C && offsetof(figure, tomb_building_id) == 0x5C &&
                  offsetof(figure, destination_building_id) == 0x5E && offsetof(figure, prey_figure_id) == 0x5E && offsetof(figure, formation_id) == 0x60 &&
                  offsetof(figure, index_in_formation) == 0x62 && offsetof(figure, formation_at_rest) == 0x63 && offsetof(figure, migrant_num_people) == 0x64 &&
                  offsetof(figure, is_ghost) == 0x65 && offsetof(figure, min_max_seen) == 0x66 && offsetof(figure, flotsam_special) == 0x66 &&
                  offsetof(figure, reserved_3) == 0x67 && offsetof(figure, leading_figure_id) == 0x68 && offsetof(figure, attack_image_offset) == 0x6A &&
                  offsetof(figure, wait_ticks_missile) == 0x6B && offsetof(figure, hunt_ticks) == 0x6B && offsetof(figure, cycle_ticks) == 0x6B &&
                  offsetof(figure, cart_offset_x) == 0x6C && offsetof(figure, cart_offset_y) == 0x6D && offsetof(figure, empire_city_id) == 0x6E &&
                  offsetof(figure, trader_amount_bought) == 0x6F && offsetof(figure, name_id) == 0x70 && offsetof(figure, terrain_usage) == 0x72 &&
                  offsetof(figure, reserved_4) == 0x73 && offsetof(figure, resource_amount) == 0x74 && offsetof(figure, extinguish_ticks) == 0x74 &&
                  offsetof(figure, target_height) == 0x79 && offsetof(figure, collecting_item_id) == 0x7A && offsetof(figure, ship_dock_index) == 0x7A &&
                  offsetof(figure, ship_dock_attempts) == 0x7B && offsetof(figure, phrase_sequence_exact) == 0x7C && offsetof(figure, phrase_id) == 0x7D &&
                  offsetof(figure, phrase_sequence_city) == 0x7E && offsetof(figure, trader_id) == 0x7F && offsetof(figure, wait_ticks_next_target) == 0x80 &&
                  offsetof(figure, ship_leave_month) == 0x80 && offsetof(figure, reserved_5) == 0x81 && offsetof(figure, target_figure_id) == 0x82 &&
                  offsetof(figure, targeted_by_figure_id) == 0x84 && offsetof(figure, created_sequence) == 0x86 &&
                  offsetof(figure, target_figure_created_sequence) == 0x88 && offsetof(figure, index_on_same_tile) == 0x8A &&
                  offsetof(figure, num_attackers) == 0x8B && offsetof(figure, attacker_id1) == 0x8C && offsetof(figure, attacker_id2) == 0x8E &&
                  offsetof(figure, opponent_id) == 0x90 && offsetof(figure, ship_formation_id) == 0x92 && offsetof(figure, flee_ticks) == 0x92 &&
                  offsetof(figure, spot_wait_ticks) == 0x92 && offsetof(figure, reroute_ticks) == 0x92 && offsetof(figure, buyer_storages_visited) == 0x93 &&
                  offsetof(figure, reserved_6) == 0x94 && offsetof(figure, name_group) == 0x98 && offsetof(figure, buyer_resources) == 0x9C &&
                  offsetof(figure, buyer_storage_ids) == 0xAC && offsetof(figure, buyer_amounts_bought) == 0xBC && offsetof(figure, reserved_7) == 0xCC &&
                  offsetof(figure, ferry_path_index) == 0x11C && offsetof(figure, pyramid_walk_mode) == 0x120 && offsetof(figure, reserved_8) == 0x122 &&
                  offsetof(figure, reserved_9) == 0x124 && offsetof(figure, ship_hit_points) == 0x128 && offsetof(figure, ship_max_hit_points) == 0x12C &&
                  offsetof(figure, has_troops_on_board) == 0x130 && offsetof(figure, phrase_group) == 0x131 && offsetof(figure, request_event_id) == 0x132 &&
                  offsetof(figure, charge.steps) == 0x134 && offsetof(figure, charge.tile_x) == 0x13C && offsetof(figure, charge.tile_y) == 0x140 &&
                  offsetof(figure, animal.hunter_building_id) == 0x138 && offsetof(figure, animal.frame_count) == 0x144 &&
                  offsetof(figure, animal.frame) == 0x148 && offsetof(figure, ship.steps) == 0x134 && offsetof(figure, ship.moves) == 0x138 &&
                  offsetof(figure, ship.speed) == 0x13C && offsetof(figure, ship.max_straight_steps) == 0x140 &&
                  offsetof(figure, ship.model_column_m) == 0x144 && offsetof(figure, ship.model_column_n) == 0x148 && offsetof(figure, patrol_x) == 0x14C &&
                  offsetof(figure, patrol_y) == 0x14E && offsetof(figure, ram_damage_slow) == 0x150 && offsetof(figure, retry_count) == 0x154 &&
                  offsetof(figure, ship_move_mode) == 0x154 && offsetof(figure, lost_ticks) == 0x154 && offsetof(figure, reserved_10) == 0x155 &&
                  offsetof(figure, ram_damage_fast) == 0x158 && offsetof(figure, warship_secondary_action) == 0x15C && offsetof(figure, reserved_11) == 0x161 &&
                  offsetof(figure, ram_cooldown) == 0x164 && offsetof(figure, festival_trips) == 0x164 && offsetof(figure, reserved_12) == 0x168 &&
                  offsetof(figure, invasion_soldiers) == 0x16C && offsetof(figure, invasion_id) == 0x170 && offsetof(figure, invasion_nation) == 0x174 &&
                  offsetof(figure, invasion_formation_param) == 0x178 && offsetof(figure, invasion_event_id) == 0x17C &&
                  offsetof(figure, cart_image_id) == 0x180, "figure is the savegame's layout");

typedef struct figure_table {
    figure records[MAX_FIGURES];
    uint8_t route_paths[MAX_PATHS * MAX_PATH_LENGTH];
} figure_table;

static_assert(sizeof(figure_table) == 1276000, "figure_table is a savegame record of 1276000 bytes");
static_assert(offsetof(figure_table, route_paths) == 0xBD740, "figure_table is the savegame's layout");

#endif
