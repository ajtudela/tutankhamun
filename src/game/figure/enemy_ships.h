// The enemy ships of figure/enemy_ships.c: their actions and helpers, the escape moves of evading ships
// (ship_escape_moves), the farthest water tile on a line and formation_create.

#ifndef FIGURE_ENEMY_SHIPS_H
#define FIGURE_ENEMY_SHIPS_H

#include "map/grid.h"
#include <stdbool.h>
#include <stdint.h>

enum formation_create_limit {
    FORMATION_FIRST_CREATED_ID = 10,
    FORMATION_CREATED_MORALE = 100,
};

enum enemy_ship_limit {
    ENEMY_SHIP_REROUTE_LAST_TICK = 99,
    ENEMY_SHIP_ANIMATION_FRAMES = 12,
    ENEMY_SHIP_FRAME_TICKS = 3,
    ENEMY_SHIP_DAMAGED_PERCENT = 15,
    ENEMY_SHIP_SINKING_TICKS = 32,
    ENEMY_SHIP_LAST_BUBBLE_FRAME = 22,
    ENEMY_TRANSPORT_EVADE_WAIT = 100,
    ENEMY_TRANSPORT_LANDING_WAIT = 50,
    ENEMY_TRANSPORT_FORMATION_SIZE = 16,
    ENEMY_TRANSPORT_WAIT_SLOTS = 20,
    ENEMY_TRANSPORT_WAIT_STEP = 10,
    ENEMY_WARSHIP_DOCKED_TARGET_REDUCTION = 20,
    ENEMY_WARSHIP_ATTACK_TICKS = 50,
    ENEMY_WARSHIP_PATROL_BASE_TICKS = 75,
    ENEMY_WARSHIP_PATROL_RANDOM_TICKS = 30,
    ENEMY_WARSHIP_STAY_MONTHS = 4,
    INVASION_LAST_ID_MAX = 32000,
    SHIP_RAM_SOUND_FIRST = 11,
    SHIP_RAM_DIVISOR_MAX = 5,
};

typedef struct ship_escape_move {
    int32_t dx1;
    int32_t dy1;
    int32_t dx2;
    int32_t dy2;
    int32_t dx3;
    int32_t dy3;
} ship_escape_move;

extern ship_escape_move ship_escape_moves[DIRECTION_COUNT];

int32_t map_water_farthest_on_line(int32_t x, int32_t y, int32_t dst_x, int32_t dst_y, int32_t *out_x, int32_t *out_y);
void enemy_ship_reroute_tick(void);
void enemy_ship_sink(int32_t figure_id);
void enemy_transport_unload(int32_t figure_id, int32_t x, int32_t y);
void enemy_transport_go_away(int32_t figure_id);
void enemy_warship_shoot(int32_t figure_id);
void ship_play_ram_sound(int32_t divisor);
void ship_pick_escape_point(int32_t *x, int32_t *y, int32_t dx1, int32_t dy1, int32_t dx2, int32_t dy2, int32_t dx3, int32_t dy3, int32_t figure_id);
void figure_count_traders(void);
int32_t formation_create(int32_t figure_type, int32_t layout, int32_t orientation, int32_t x, int32_t y);
int32_t ship_find_ship_at(int32_t grid_offset, int32_t figure_id);
int32_t ship_find_ram_target(int32_t figure_id);
void enemy_transport_wait(int32_t figure_id, int32_t *moving);
void enemy_transport_move(int32_t figure_id, int32_t *moving);
void enemy_transport_go_to_landing(int32_t figure_id, int32_t *moving);
void enemy_transport_evade(int32_t figure_id, int32_t *moving);
void enemy_warship_wait(int32_t figure_id, int32_t *moving);
void enemy_warship_move(int32_t figure_id, int32_t *moving);
void enemy_warship_ram(int32_t figure_id, int32_t *moving);
void enemy_warship_evade(int32_t figure_id, int32_t *moving);
void enemy_warship_attack(int32_t figure_id, int32_t *moving);
int32_t enemy_landing_water_tile(int32_t *x, int32_t *y);
void enemy_warship_patrol(int32_t figure_id, int32_t *moving);
void enemy_warship_leave(int32_t figure_id, int32_t *moving);
bool figure_military_is_away_on_mission(int32_t figure_id);
int32_t enemy_warship_find_target(int32_t figure_id, int32_t range);
int32_t enemy_warship_find_target_at(int32_t x, int32_t y, int32_t range);
void figure_enemy_transport_action(void);
void figure_enemy_warship_action(void);
void figure_egyptian_warship_action(void);
void figure_egyptian_transport_action(void);

#endif
