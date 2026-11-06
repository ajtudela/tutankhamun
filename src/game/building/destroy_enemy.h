// Buildings destroyed by enemies (building/destroy_enemy.c), the tower sentry helpers and the share of savings
// and palace money enemies steal by difficulty.

#ifndef BUILDING_DESTROY_ENEMY_H
#define BUILDING_DESTROY_ENEMY_H

#include "city/difficulty.h"
#include <stdint.h>

int32_t building_main_part_id(int32_t building_id);
void building_destroy_by_enemy(int32_t x, int32_t y, int32_t grid_offset);
void building_destroy_increase_enemy_damage(int32_t grid_offset, int32_t max_damage);
void figure_kill_tower_sentries_at(int32_t x, int32_t y);
void figure_tower_sentry_reroute(void);
int32_t building_destroy_counts_for_peace(int32_t type);

extern int32_t enemy_theft_percentages[DIFFICULTY_COUNT];

#endif
