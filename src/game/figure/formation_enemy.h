// The enemy formation helpers of figure/formation_enemy.c: the soldier strength map, the attack target of an enemy
// formation, the reset of its figures and the enemies leaving the city.

#ifndef FIGURE_FORMATION_ENEMY_H
#define FIGURE_FORMATION_ENEMY_H

#include <stdbool.h>
#include <stdint.h>

enum enemy_attack_list {
    ENEMY_ATTACK_LIST_LENGTH = 100,
};

enum soldier_strength {
    SOLDIER_STRENGTH_RINGS = 6,
    SOLDIER_STRENGTH_MAX_RADIUS = 7,
    SOLDIERS_PER_STRENGTH_RING = 3,
};

void formation_enemy_set_figures_to_initial(int32_t formation_id);
void formation_update_soldier_strength(void);
void formation_enemy_set_fleeing(void);
void map_soldier_strength_add(int32_t x, int32_t y, int32_t radius, int32_t amount);
int32_t map_soldier_strength_get_max(int32_t x, int32_t y, int32_t radius);
bool formation_enemy_set_target_building(int32_t formation_id);

#endif
