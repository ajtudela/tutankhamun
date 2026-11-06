// The work camp laborers of figure/laborer.c and the work they find.

#ifndef FIGURE_LABORER_H
#define FIGURE_LABORER_H

#include "flood/flood_types.h"
#include <stdbool.h>
#include <stdint.h>

enum laborer_rule {
    LABORER_RULE_LOAD,
    LABORER_RULE_YARD_WAIT_TICKS,
    LABORER_RULE_RAMP_ROUTE_RETRY_TICKS,
    LABORER_RULE_RAMP_ROUTE_GIVE_UP_TICKS,
    LABORER_RULE_DELIVERED_WAIT_TICKS,
    LABORER_RULE_FERRY_WAIT_TICKS,
    LABORER_RULE_MAX_ROAM_TICKS,
    LABORER_RULE_FARM_WALK_TICKS,
    LABORER_RULE_HOME_ROAD_RADIUS,
    LABORER_RULE_MAX_OUT,
    LABORER_RULE_LABOR_SEEKER_HOUSES,
    LABORER_RULE_SPAWN_DELAY_FULL,
    LABORER_RULE_SPAWN_DELAY_75_PERCENT,
    LABORER_RULE_SPAWN_DELAY_50_PERCENT,
    LABORER_RULE_SPAWN_DELAY_25_PERCENT,
    LABORER_RULE_SPAWN_DELAY_LOW,
    LABORER_RULE_FARM_SEARCH_DISTANCE,
    LABORER_RULE_FARM_BUSY_LABOR_DAYS,
    LABORER_RULE_FARM_LABOR_DAYS,
    LABORER_RULE_CAMP_LABOR_DAYS,
    LABORER_RULE_LEVEL_TILES_IN_A_ROW,
    LABORER_RULE_MONUMENT_WORK_SEARCHES,
    LABORER_RULE_COUNT,
};

extern int32_t laborer_rules[LABORER_RULE_COUNT];

enum laborer_limit {
    LABORER_TASK_CANDIDATES = 500,
    LABORER_NO_DISTANCE = 10000,
    LABORER_SLED_PULLERS = 5,
    LABORER_TRAIN_MAX_LINKS = 8,
    LABORER_LEVEL_CHECK_TICKS = 30,
};

enum laborer_animation {
    LABORER_ANIMATION_FRAMES = 12,
    LABORER_CYCLE_TICKS = 120,
    LABORER_ATTACK_FRAMES = 32,
    LABORER_SLED_SPACING = 12,
    LABORER_PULLER_SPACING = 7,
    LABORER_TRAIN_CATCH_UP_TICKS = 8,
};

void building_work_camp_spawn_figure(floods *self, int32_t building_id);
void figure_laborer_find_work(floods *self, int32_t figure_id);
void laborer_arrive_at_farm(floods *self, int32_t figure_id);
bool laborer_farm_work_waiting(floods *self);
int32_t figure_laborer_partner_still_working(int32_t figure_id);
void figure_laborer_action(void);
void figure_sled_action(void);
void figure_sled_puller_action(void);
int32_t figure_find_train_leader(int32_t figure_id, int32_t *position);
void figure_movement_follow_closely(int32_t figure_id, int32_t leader_id, int32_t climb_step, int32_t distance);

#endif
