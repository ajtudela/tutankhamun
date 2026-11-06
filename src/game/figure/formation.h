// The formations (the record is in figure/formation_types.h): layouts, attack kinds, herd kinds and states, the
// legions' recruit kinds and ids, the morale rules, the invaders' cycle timings, and the formation functions other
// modules call. formation_memory_past_records gives the byte at an offset past the 50 records as the original lays its
// memory out: the four figure counters (figure_roaming_enabled, figure_search_best_distance, figure_highest_id,
// figure_unused_counter), then city_figures (the records, then the route paths); 0 past those. Two places index the
// formations by an id that can be 50 or more and land there: Seth's curse and the removal of a transport ship.

#ifndef FIGURE_FORMATION_H
#define FIGURE_FORMATION_H

#include "figure/formation_types.h"
#include "window/window_data.h"
#include <stdint.h>

enum formation_layout {
    FORMATION_COLUMN = 0,
    FORMATION_DOUBLE_LINE_1 = 1,
    FORMATION_DOUBLE_LINE_2 = 2,
    FORMATION_SINGLE_LINE_1 = 3,
    FORMATION_SINGLE_LINE_2 = 4,
    FORMATION_TORTOISE = 5,
    FORMATION_MOP_UP = 6,
    FORMATION_AT_REST = 7,
    FORMATION_ENEMY_MOB = 8,
    FORMATION_HERD = 9,
    FORMATION_ENEMY_DOUBLE_LINE = 10,
    FORMATION_ENEMY_WIDE_COLUMN = 11,
    FORMATION_ENEMY12 = 12,
    FORMATION_CHARGE = 15,
};

enum formation_attack {
    FORMATION_ATTACK_FOOD_CHAIN = 0,
    FORMATION_ATTACK_GOLD_STORES = 1,
    FORMATION_ATTACK_BEST_BUILDINGS = 2,
    FORMATION_ATTACK_TROOPS = 3,
    FORMATION_ATTACK_RANDOM = 4,
};

enum legion_recruit {
    LEGION_RECRUIT_NONE = 0,
    LEGION_RECRUIT_ARCHER = 1,
    LEGION_RECRUIT_INFANTRY = 2,
    LEGION_RECRUIT_CHARIOTEER = 3,
};

enum legion_ids {
    LEGION_IDS = 8,
    MISSION_LEGION_SLOTS = 3,
    LEGION_STANDARD_FORT_DX = 3,
    LEGION_STANDARD_FORT_DY = -1,
    LEGION_NO_REQUEST = -1,
};

enum legion_training {
    LEGION_ACADEMY_EXPERIENCE = 25,
    LEGION_MAX_EXPERIENCE = 100,
    BARRACKS_STOCK_PER_RECRUIT = 100,
};

enum formation_morale_rule {
    FORMATION_MORALE_RULE_NEW_LEGION,
    FORMATION_MORALE_RULE_CHARIOTEERS_MAX,
    FORMATION_MORALE_RULE_TRAINED_CHARIOTEERS_MAX,
    FORMATION_MORALE_RULE_KINGDOM_SOLDIERS_MAX,
    FORMATION_MORALE_RULE_FOOT_SOLDIERS_MAX,
    FORMATION_MORALE_RULE_TRAINED_FOOT_SOLDIERS_MAX,
    FORMATION_MORALE_RULE_ENEMY_MAX,
    FORMATION_MORALE_RULE_ENEMY_13_MAX,
    FORMATION_MORALE_RULE_AT_FORT_GAIN,
    FORMATION_MORALE_RULE_AWAY_LOSS,
    FORMATION_MORALE_RULE_MONTHS_AWAY_WITHOUT_LOSS,
    FORMATION_MORALE_RULE_LOW,
    FORMATION_MORALE_RULE_VERY_LOW,
    FORMATION_MORALE_RULE_ROUT_SWING,
    FORMATION_MORALE_RULE_DEATH_SHARE_1,
    FORMATION_MORALE_RULE_DEATH_SHARE_2,
    FORMATION_MORALE_RULE_DEATH_SHARE_3,
    FORMATION_MORALE_RULE_DEATH_SHARE_4,
    FORMATION_MORALE_RULE_DEATH_SHARE_5,
    FORMATION_MORALE_RULE_DEATH_LOSS_1,
    FORMATION_MORALE_RULE_DEATH_LOSS_2,
    FORMATION_MORALE_RULE_DEATH_LOSS_3,
    FORMATION_MORALE_RULE_DEATH_LOSS_4,
    FORMATION_MORALE_RULE_DEATH_LOSS_5,
    FORMATION_MORALE_RULE_DEATH_LOSS_6,
    FORMATION_MORALE_RULE_COUNT,
};

enum formation_morale {
    FORMATION_MONTHS_AWAY_MAX = 100,
    FORMATION_DEATH_SHARE_STEPS = 5,
};

extern int32_t formation_morale_rules[FORMATION_MORALE_RULE_COUNT];

enum formation_timing {
    FORMATION_RECENT_FIGHT_TICKS = 6,
    ENEMY_FORMATION_GATHER_TICKS = 32,
    ENEMY_FORMATION_FLEE_CHECK_TICKS = 45,
    ENEMY_FORMATION_MAX_TICKS_WITHOUT_TARGET = 48,
};

enum enemy_formation_cycle {
    ENEMY_CYCLE_GATHER_REGROUP = 4,
    ENEMY_CYCLE_ADVANCE = 6,
    ENEMY_CYCLE_REGROUP = 4,
    ENEMY_CYCLE_HALT = 2,
    ENEMY_CYCLE_NO_TARGET_ADVANCE = 1,
    ENEMY_CYCLE_NO_TARGET_REGROUP = 12,
    ENEMY_CYCLE_NO_TARGET_HALT = 1,
    ENEMY_MOB_CYCLE_ADVANCE = 2,
    ENEMY_MOB_CYCLE_FRONT_REGROUP = 4,
    ENEMY_MOB_CYCLE_FRONT_HALT = 2,
    ENEMY_MOB_CYCLE_MIDDLE_REGROUP = 5,
    ENEMY_MOB_CYCLE_MIDDLE_HALT = 3,
    ENEMY_MOB_CYCLE_BACK_REGROUP = 6,
    ENEMY_MOB_CYCLE_BACK_HALT = 4,
    ENEMY_MOB_CYCLE_NO_TARGET_EXTRA_REGROUP = 6,
};

enum enemy_formation_search {
    ENEMY_SOLDIERS_NEAR_RADIUS = 16,
    ENEMY_SOLDIERS_FAR_RADIUS = 32,
    ENEMY_ROUTE_ANY_DISTANCE = 100000,
    ENEMY_PLACEMENT_ROUTE_TILES = 600,
    ENEMY_PLACEMENT_MAX_RADIUS = 10,
};

enum formation_turn {
    FORMATION_KEEPS_DIRECTION = -1,
    FORMATION_TURNS_ALONG_X = 0,
    FORMATION_TURNS_ALONG_Y = 1,
};

enum enemy_target_search {
    ENEMY_TARGET_NONE = 0,
    ENEMY_TARGET_SOLDIERS_NEAR = 1,
    ENEMY_TARGET_SOLDIERS_FAR = 2,
};

enum herd_kind {
    HERD_KIND_NONE = 0,
    HERD_KIND_PREDATORS = 1,
    HERD_KIND_PREY = 2,
};

enum herd_state {
    HERD_STATE_ROAMING = 0,
    HERD_STATE_STAMPEDE = 17,
};

static inline formation *formation_get(int32_t id)
{
    return &formation_array[id];
}

uint8_t *formation_memory_past_records(int32_t offset);
int32_t formation_create_legion(int32_t building_id);
int32_t formation_enemy_move_formation_to(int32_t formation_id, int32_t x, int32_t y);
int32_t building_barracks_create_soldier(int32_t building_id);
int32_t building_barracks_create_tower_sentry(int32_t building_id);
void formation_legion_update_recruit_type(int32_t building_id);
void formation_calculate_legion_totals(void);
void formation_calculate_figures(void);
int32_t formation_legion_is_on_ship(int32_t formation_id);
void formation_update_directions(void);
void formation_legion_decrease_damage(void);
void formation_update_monthly_morale_deployed(void);
void formation_legion_set_max_figures(void);
void formation_legion_update(void);
void formation_enemy_update(void);
void formation_set_enemy_destination(int32_t formation_id);
void formation_update_all(int32_t skip_morale);
void formation_update_monthly_morale_at_rest(void);
void formation_change_morale(int32_t formation_id, int32_t amount);
void formation_update_morale_after_death(int32_t formation_id);
void formation_change_all_legion_morale(int32_t amount);
void formation_change_all_enemy_morale(int32_t amount);
int32_t formation_find_closest_academy(int32_t formation_id);

#endif
