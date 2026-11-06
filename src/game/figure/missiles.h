// The missiles of figure/missiles.c: the missile actions, the hit test, the javelin target search, the flight, the
// defense rules and the missile images.

#ifndef FIGURE_MISSILES_H
#define FIGURE_MISSILES_H

#include <stdint.h>

enum missile_flight {
    MISSILE_SPEED = 4,
    MISSILE_FLIGHT_TICKS = 120,
};

enum missile_rule {
    MISSILE_MAX_DEFENSE = 20,
    MISSILE_DAMAGE_BASE = 30,
};

enum missile_image {
    MISSILE_IMAGE_ARROW = 16,
    MISSILE_IMAGE_BOLT = 32,
    MISSILE_IMAGE_DIRECTIONS = 16,
    SPEAR_IMAGE_DIRECTIONS = 32,
};

void figure_arrow_action(void);
void figure_bolt_action(void);
void figure_spear_action(void);
int32_t missile_hit_target(int32_t figure_id);
int32_t soldier_find_enemy(int32_t x, int32_t y, int32_t max_distance, int32_t figure_id);

#endif
