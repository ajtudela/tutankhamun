// The service walkers of figure/walkers.c: their constants, walker_rules and the functions other files call.

#ifndef FIGURE_WALKERS_H
#define FIGURE_WALKERS_H

#include "core/crt_compat.h"
#include <stdint.h>

enum walker_constants {
    WALKER_ANIMATION_FRAMES = 12,
    ENTERTAINER_ATTACK_FRAMES = 32,
    ENTERTAINER_DEATH_FRAMES = 8,
    FIREMAN_FIGHT_FIRE_FRAMES = 36,
    ENTERTAINER_NO_VENUE_DISTANCE = 1000,
    WALKER_NO_TARGET_DISTANCE = 10000,
    CONSTABLE_ATTACK_SOUND = 10,
    WALKER_SOUND_VARIANT = 2,
    FESTIVAL_SQUARE_SIZE = 5,
    FOUGHT_RUIN_DISTANCE = 3,
};

enum walker_rule {
    WALKER_RULE_ROAMER_ROAM_TICKS = 0,
    WALKER_RULE_PATRICIAN_ROAM_TICKS = 1,
    WALKER_RULE_MISSIONARY_ROAM_TICKS = 2,
    WALKER_RULE_TAX_COLLECTOR_ROAM_TICKS = 3,
    WALKER_RULE_ENTERTAINER_ROAM_TICKS = 4,
    WALKER_RULE_ENGINEER_ROAM_TICKS = 5,
    WALKER_RULE_FIREMAN_ROAM_TICKS = 6,
    WALKER_RULE_LAW_WALKER_ROAM_TICKS = 7,
    WALKER_RULE_MARKET_TRADER_EMPTY_TURN_TICKS = 8,
    WALKER_RULE_ENTERTAINER_VENUE_WALK_TICKS = 9,
    WALKER_RULE_HOME_ROAD_RADIUS = 10,
    WALKER_RULE_FERRY_WAIT_TICKS = 11,
    WALKER_RULE_FIRE_SEARCH_TICKS = 12,
    WALKER_RULE_FIRE_SEARCH_DISTANCE = 13,
    WALKER_RULE_FIRE_FIGHT_TICKS = 14,
    WALKER_RULE_FIRE_OUT_DURATION = 15,
    WALKER_RULE_ENEMY_SEARCH_TICKS = 16,
    WALKER_RULE_ENEMY_SEARCH_DISTANCE = 17,
    WALKER_RULE_FESTIVAL_TRIPS = 18,
    WALKER_RULE_COUNT = 19,
};

extern int32_t walker_rules[WALKER_RULE_COUNT];

void figure_roamer_action(void);
int32_t figure_entertainer_determine_destination(int32_t figure_id);
int32_t figure_closest_enemy_for_constable(int32_t x, int32_t y, int32_t max_distance);
int32_t building_maintenance_get_closest_burning_ruin(int32_t x, int32_t y);
int32_t fireman_fight_fire(int32_t figure_id);
int32_t constable_fight_enemy(int32_t figure_id);
void figure_labor_seeker_action(void);
void figure_tax_collector_action(void);
void figure_engineer_action(void);
void figure_fireman_action(void);
void figure_entertainer_action(void);
void figure_market_trader_action(void);
void figure_priest_action(void);
void figure_teacher_action(void);
void figure_librarian_action(void);
void figure_dentist_action(void);
void figure_physician_action(void);
void figure_apothecary_action(void);
void figure_embalmer_action(void);
void figure_worker_action(void);
void figure_patrician_action(void);
void figure_missionary_action(void);
void figure_water_carrier_action(void);
void figure_constable_action(void);
void figure_magistrate_action(void);
void figure_festival_priest_action(void);
void figure_showman_action(void);

#endif
