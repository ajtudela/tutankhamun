// The player's transports and warships of figure/ships_player.c, the dock queue records, the warship's modes, the
// ship_move modes and the warship's target scores.

#ifndef FIGURE_SHIPS_PLAYER_H
#define FIGURE_SHIPS_PLAYER_H

#include "core/queue.h"
#include "figure/dock_queue_types.h"
#include "figure/military.h"
#include "io/io_data.h"
#include <stddef.h>
#include <stdint.h>

enum dock_queue_limit {
    DOCK_QUEUE_NONE = -1,
    DOCK_QUEUE_MAX_SHIPS = 200,
};

enum ship_limit {
    SHIP_DOCK_WAIT_TICKS = 50,
    SHIP_FACE_ENEMY_WAIT = 19,
    SHIP_FACE_ENEMY_RANGE = 5,
    TRANSPORT_LEGION_DROWNS_TICK = 2,
    WARSHIP_RAM_COOLDOWN = 15,
    WARSHIP_EVADE_TICKS = 75,
    WARSHIP_EVADE_RANGE = 18,
    WARSHIP_SHOOT_TICKS = 100,
    WARSHIP_TARGET_CHECK_TICKS = 10,
};

enum ship_move_kind {
    SHIP_MOVE_STRAIGHT = 0,
    SHIP_MOVE_CAUTIOUS = 1,
};

enum warship_mode {
    WARSHIP_MODE_FOLLOW_TARGET = 9,
    WARSHIP_MODE_SAILING = 11,
    WARSHIP_MODE_WAITING = 16,
    WARSHIP_MODE_PATROL = 22,
};

enum warship_target_score {
    WARSHIP_SCORE_BOAT = 5,
    WARSHIP_SCORE_TRANSPORT = 10,
    WARSHIP_SCORE_WARSHIP = 15,
    WARSHIP_SCORE_LAND_ENEMY = 17,
    WARSHIP_SCORE_LOADED_TRANSPORT = 25,
};

static inline dock_queue *dock_queue_get(int32_t index)
{
    return &dock_queues.queues[index];
}

void building_repair_queue_add(int32_t building_id);
void building_repair_queue_clear(void);
void building_repair_queue_remove(int32_t building_id);
void ship_go_to_best_dock(int32_t figure_id);
void transport_sink(int32_t figure_id);
void transport_docked(int32_t figure_id);
void warship_shoot(int32_t figure_id);
void warship_docked(int32_t figure_id);
void transport_legion_board(int32_t formation_id);
void transport_legion_land(int32_t formation_id, int32_t x, int32_t y);
int32_t dock_queue_size(int32_t building_id);
int32_t ship_find_best_dock(int32_t figure_id);
int32_t ship_queue_at_dock(int32_t figure_id, int32_t building_id);
int32_t ship_leave_dock_queue(int32_t figure_id);
int32_t transport_legion_ready(int32_t figure_id);
void transport_wait_at_dock(int32_t figure_id, int32_t building_id, int32_t *moving);
void warship_wait_at_dock(int32_t figure_id, int32_t *moving);
void transport_sail_to_dock(int32_t figure_id, int32_t *moving);
void warship_sail_to_dock(int32_t figure_id, int32_t *moving);
void transport_sail_to_legion(int32_t figure_id, int32_t *moving);
void transport_sail_to_landing(int32_t figure_id, int32_t *moving);
int32_t figure_find_enemy_warship_in_range(int32_t x, int32_t y, int32_t range);
int32_t figure_find_enemy_warship_near(int32_t figure_id, int32_t range);
int32_t warship_find_ram_target(int32_t figure_id);
void warship_move_attack(int32_t figure_id, int32_t *moving);
void warship_sail_to_destination(int32_t figure_id, int32_t *moving);
void warship_evading(int32_t figure_id, int32_t *moving);
void warship_attack_ship(int32_t figure_id, int32_t *moving);
int32_t figure_warship_find_enemy_at(int32_t x, int32_t y, int32_t range);
int32_t figure_warship_find_enemy(int32_t figure_id, int32_t range);

#endif
