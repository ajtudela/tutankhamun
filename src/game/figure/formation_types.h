// The formation record (record 0 unused): a legion of the player with its fort, standard and orders, an invading
// army's unit with its attack cycle, or a herd of animals with its home, roaming and spawning; and enemy_army_totals,
// the strength of the legions and of the enemies.

#ifndef FIGURE_FORMATION_TYPES_H
#define FIGURE_FORMATION_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_FORMATIONS 50

enum formation_size {
    FORMATION_MAX_FIGURES = 16,
    FORMATION_MAX_LEGIONS = 6,
};

typedef struct {
    uint8_t in_use;
    uint8_t faction;
    uint8_t legion_id;
    uint8_t is_at_fort;
    int16_t figure_type;
    int16_t building_id;
    int16_t figures[FORMATION_MAX_FIGURES];
    uint8_t num_figures;
    uint8_t max_figures;
    int16_t layout;
    int16_t morale;
    int16_t x;
    int16_t y;
    int16_t order_x;
    int16_t order_y;
    int16_t home_x;
    int16_t home_y;
    int16_t destination_x;
    int16_t destination_y;
    int16_t destination_building_id;
    int16_t standard_figure_id;
    uint8_t is_legion;
    uint8_t reserved_1;
    int16_t attack_type;
    int16_t legion_recruit_type;
    int16_t has_military_training;
    int16_t total_damage;
    int16_t max_total_damage;
    union {
        int16_t ticks;
        uint8_t bytes[2];
    } wait;
    int16_t recent_fight;
    int16_t regroup_ticks;
    int16_t advance_ticks;
    int16_t halt_ticks;
    int16_t invasion_formation_index;
    int16_t is_halted;
    int16_t missile_fired;
    int16_t missile_attack_timeout;
    int16_t missile_attack_formation_id;
    int16_t prev_layout;
    int16_t cursed_by_seth;
    uint8_t months_low_morale;
    uint8_t empire_service;
    uint8_t ticks_without_target;
    uint8_t herd_kind;
    uint8_t enemy_type;
    uint8_t direction;
    int16_t prev_x;
    int16_t prev_y;
    uint8_t direction_hold_ticks;
    uint8_t orientation;
    uint8_t months_from_home;
    uint8_t months_very_low_morale;
    uint8_t herd_state;
    uint8_t herd_spawn_delay;
    uint8_t herd_direction;
    uint8_t alternate_layout;
    int16_t invasion_event_id;
    int16_t experience;
    int16_t saved_layout;
    int16_t has_move_order;
    int16_t move_order_ticks;
    int16_t request_event_id;
    int32_t ship_figure_id;
    int16_t invasion_sequence;
    uint8_t reserved_2[2];
    int32_t attack_formation_id;
} formation;

static_assert(sizeof(formation) == 0x90, "formation is a savegame record of 144 bytes");
static_assert(offsetof(formation, legion_id) == 0x02 && offsetof(formation, figure_type) == 0x04 && offsetof(formation, building_id) == 0x06 &&
                  offsetof(formation, figures) == 0x08 && offsetof(formation, num_figures) == 0x28 && offsetof(formation, layout) == 0x2A &&
                  offsetof(formation, x) == 0x2E && offsetof(formation, order_x) == 0x32 && offsetof(formation, home_x) == 0x36 &&
                  offsetof(formation, destination_x) == 0x3A && offsetof(formation, destination_building_id) == 0x3E &&
                  offsetof(formation, standard_figure_id) == 0x40 && offsetof(formation, is_legion) == 0x42 && offsetof(formation, attack_type) == 0x44 &&
                  offsetof(formation, has_military_training) == 0x48 && offsetof(formation, max_total_damage) == 0x4C && offsetof(formation, wait) == 0x4E &&
                  offsetof(formation, recent_fight) == 0x50 && offsetof(formation, advance_ticks) == 0x54 &&
                  offsetof(formation, invasion_formation_index) == 0x58 && offsetof(formation, is_halted) == 0x5A &&
                  offsetof(formation, missile_attack_formation_id) == 0x60 && offsetof(formation, cursed_by_seth) == 0x64 &&
                  offsetof(formation, months_low_morale) == 0x66 && offsetof(formation, ticks_without_target) == 0x68 &&
                  offsetof(formation, enemy_type) == 0x6A && offsetof(formation, prev_x) == 0x6C && offsetof(formation, direction_hold_ticks) == 0x70 &&
                  offsetof(formation, months_very_low_morale) == 0x73 && offsetof(formation, herd_direction) == 0x76 &&
                  offsetof(formation, alternate_layout) == 0x77 && offsetof(formation, invasion_event_id) == 0x78 && offsetof(formation, experience) == 0x7A &&
                  offsetof(formation, has_move_order) == 0x7E && offsetof(formation, request_event_id) == 0x82 && offsetof(formation, ship_figure_id) == 0x84 &&
                  offsetof(formation, invasion_sequence) == 0x88 && offsetof(formation, attack_formation_id) == 0x8C, "formation is the savegame's layout");

typedef formation formation_table[MAX_FORMATIONS];

typedef struct enemy_army_totals_record {
    int32_t update_counter;
    int32_t legion_strength;
    int32_t legion_formations;
    int32_t enemy_strength;
    int32_t enemy_formations;
} enemy_army_totals_record;

static_assert(sizeof(enemy_army_totals_record) == 20, "enemy_army_totals_record is a savegame record of 20 bytes");
static_assert(offsetof(enemy_army_totals_record, enemy_formations) == 0x10, "enemy_army_totals_record is the savegame's layout");

#endif
