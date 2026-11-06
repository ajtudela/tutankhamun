// The military figures and combat of figure/military.c, the tunable military_rules (by MILITARY_RULE_*) and the
// tables military_difficulties, chariot_cart_offsets, transport_escapes and soldier_alternative_spots, the ships'
// modes and the runs of the explosion table.

#ifndef FIGURE_MILITARY_H
#define FIGURE_MILITARY_H

#include "city/difficulty.h"
#include "map/grid.h"
#include <stdbool.h>
#include <stdint.h>

enum military_rule {
    MILITARY_RULE_MELEE_HIT_TICKS,
    MILITARY_RULE_FLANK_ATTACK_BONUS,
    MILITARY_RULE_LINE_ATTACK_BONUS,
    MILITARY_RULE_MOP_UP_ATTACK_BONUS,
    MILITARY_RULE_LINE_DEFENSE_BONUS,
    MILITARY_RULE_MOP_UP_DEFENSE_PENALTY,
    MILITARY_RULE_FACING_AWAY_DEFENSE_PENALTY,
    MILITARY_RULE_KINGDOM_SOLDIER_LINE_BONUS,
    MILITARY_RULE_MAX_EXPERIENCE,
    MILITARY_RULE_CHARGE_MIN_STEPS,
    MILITARY_RULE_CHARGE_MAX_STEPS,
    MILITARY_RULE_CHARGE_ATTACK_FACTOR,
    MILITARY_RULE_JAVELIN_DAMAGE,
    MILITARY_RULE_BOLT_DAMAGE,
    MILITARY_RULE_MISSILE_ATTACK_TIMEOUT,
    MILITARY_RULE_MISSILE_FACING_AWAY_PENALTY,
    MILITARY_RULE_MISSILE_SINGLE_LINE_BONUS,
    MILITARY_RULE_MISSILE_MOP_UP_PENALTY,
    MILITARY_RULE_MISSILE_DOCKED_WARSHIP_BONUS,
    MILITARY_RULE_TARGET_PRIORITY_ENEMY_SOLDIER,
    MILITARY_RULE_TARGET_PRIORITY_ANIMAL,
    MILITARY_RULE_TARGET_PRIORITY_LOADED_TRANSPORT,
    MILITARY_RULE_TARGET_PRIORITY_TRANSPORT,
    MILITARY_RULE_TARGET_PRIORITY_WARSHIP,
    MILITARY_RULE_SENTRY_REST_TICKS,
    MILITARY_RULE_SENTRY_SEARCH_TICKS,
    MILITARY_RULE_SENTRY_SHOOT_DISTANCE,
    MILITARY_RULE_BALLISTA_SEARCH_TICKS,
    MILITARY_RULE_BALLISTA_AIM_TICKS,
    MILITARY_RULE_BALLISTA_RELOAD_TICKS,
    MILITARY_RULE_WARSHIP_HUNT_RANGE,
    MILITARY_RULE_WARSHIP_PATROL_EXTRA_RANGE,
    MILITARY_RULE_WARSHIP_PATROL_RADIUS,
    MILITARY_RULE_SHIP_DAMAGED_PERCENTAGE,
    MILITARY_RULE_TRANSPORT_EVADE_TICKS,
    MILITARY_RULE_TRANSPORT_EVADE_RANGE,
    MILITARY_RULE_SOLDIER_ENGAGE_RADIUS,
    MILITARY_RULE_SOLDIER_MOP_UP_RANGE,
    MILITARY_RULE_SOLDIER_ATTACK_FORMATION_RANGE,
    MILITARY_RULE_CRIMINAL_WAIT_TICKS,
    MILITARY_RULE_CRIMINAL_SPEED_FACTOR,
    MILITARY_RULE_CART_THIEF_FLEE_TICKS,
    MILITARY_RULE_CART_THEFT_AMOUNT,
    MILITARY_RULE_TOMB_ROBBER_KILLED_RATING,
    MILITARY_RULE_COUNT
};

enum military_table_size {
    EXPLOSION_CLOUDS_SILENT = 4,
    TRANSPORT_ESCAPE_POINTS = 3,
    SOLDIER_ALTERNATIVE_SPOTS = 168,
};

enum figure_corpse {
    FIGURE_CORPSE_LAST_TICK = 127,
    FIGURE_SENTRY_ATTACK_FRAMES = 128,
};

enum ship_mode {
    SHIP_MODE_MOVING = 9,
    SHIP_MODE_WAITING = 16,
    SHIP_MODE_PATROL_ATTACK = 21,
    SHIP_MODE_PATROLLING = 22,
};

enum ship_move_mode {
    SHIP_MOVE_FULL_SPEED = 0,
    SHIP_MOVE_SLOW = 1,
    SHIP_MOVE_RESTING = 2,
};

typedef struct military_difficulty {
    int32_t theft_percentage;
    int32_t tomb_robbery_rating;
} military_difficulty;

typedef struct military_offset {
    int32_t dx;
    int32_t dy;
} military_offset;

typedef struct transport_escape {
    int32_t dx[TRANSPORT_ESCAPE_POINTS];
    int32_t dy[TRANSPORT_ESCAPE_POINTS];
} transport_escape;

extern int32_t military_rules[MILITARY_RULE_COUNT];
extern military_difficulty military_difficulties[DIFFICULTY_COUNT];
extern military_offset chariot_cart_offsets[DIRECTION_COUNT];
extern transport_escape transport_escapes[DIRECTION_COUNT];
extern military_offset soldier_alternative_spots[SOLDIER_ALTERNATIVE_SPOTS];

void figure_play_die_sound(int32_t figure_id);
void figure_play_hit_sound(int32_t figure_id);
void figure_set_state_dead(int32_t figure_id);
int32_t attack_is_same_direction(int32_t dir1, int32_t dir2);
void figure_combat_handle_corpse(int32_t figure_id);
void figure_combat_resume_activity_after_attack(int32_t figure_id);
void figure_combat_attack_figure_at(int32_t figure_id, int32_t opponent_id);
void figure_combat_attack_adjacent(int32_t figure_id);
int32_t figure_target_is_alive(int32_t figure_id);
void figure_combat_handle_attack(int32_t figure_id);
void figure_combat_hit_opponent(int32_t figure_id, int32_t opponent_id);
int32_t figure_create_missile(int32_t figure_id, int32_t x, int32_t y, int32_t dst_x, int32_t dst_y, int32_t type);
int32_t figure_combat_get_missile_target_for_soldier(int32_t figure_id, int32_t max_distance);
void figure_create_explosion_cloud(int32_t x, int32_t y, int32_t size, bool with_sound);
void figure_explosion_cloud_action(void);
void figure_criminal_action(void);
int32_t tomb_robber_find_target(int32_t figure_id);
void figure_tomb_robber_robbed_monument(void);
void figure_tomb_robber_action(void);
void figure_military_standard_action(void);
void figure_movement_move_ticks_tower_sentry(int32_t figure_id, int32_t num_ticks);
int32_t tower_sentry_find_target(int32_t figure_id);
int32_t tower_sentry_init_patrol(int32_t figure_id);
void figure_tower_sentry_action(void);
void figure_javelin_action(void);
void figure_ballista_action(void);
int32_t ship_is_docked(int32_t figure_id);
void ship_move_to_dock(int32_t figure_id, int32_t *moving);
void ship_evade(int32_t figure_id, int32_t *moving);
int32_t ship_find_closest(int32_t figure_id, int32_t size, int32_t any);
void ship_move(int32_t figure_id, int32_t cautious);
int32_t ship_find_target_at(int32_t grid_offset, int32_t figure_id);
void warship_patrol(int32_t figure_id, int32_t *moving);
int32_t figure_next_route_direction(int32_t figure_id);
void figure_warship_action(void);
void figure_transport_ship_action(void);
void figure_soldier_action(void);

#endif
