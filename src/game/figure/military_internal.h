// Shared by the military files of figure/ and the building and scenario files that create enemies:
// a few functions of enemy.c, enemy_ships.c and ships_player.c.

#ifndef FIGURE_MILITARY_INTERNAL_H
#define FIGURE_MILITARY_INTERNAL_H

#include "building/building.h"
#include "city/city_player.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/military.h"
#include "figure/route.h"
#include "map/routing.h"
#include "sound/sound_play.h"
#include <stdint.h>

void figure_enemy_properties_init(void);
void figure_count_traders(void);
int32_t figure_warship_find_enemy_at(int32_t x, int32_t y, int32_t range);
int32_t figure_warship_find_enemy(int32_t figure_id, int32_t range);
int32_t figure_enemy_near_building_site(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

#endif
