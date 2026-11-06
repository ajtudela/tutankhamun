// The player's ships, the transport and the warship: their states at, to and in the queues of the shipwrights' docks,
// taking a legion on board and landing it, ramming and evading enemy ships and sinking; the dock queues, the best
// dock, and the warship's search for an enemy to face or attack.
//
// The ship queues and searches return or read odd values:
// - ship_queue_at_dock returns 1 when queued, else the building id with its low byte cleared;
// - ship_leave_dock_queue only knows the queue records 1..9;
// - the landing point search gets the figure id and 0 as its first guesses;
// - the warships' land enemy range is the current figure's missile range, not the searching ship's.

#include "figure/ships_player.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "core/calc.h"
#include "core/queue.h"
#include "core/random.h"
#include "figure/boats.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/formation_orders.h"
#include "figure/formation_types.h"
#include "figure/military.h"
#include "figure/military_internal.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/routing.h"
#include "map/terrain.h"
#include "map/water.h"
#include "platform/platform_system.h"
#include <stdint.h>


int32_t dock_queue_size(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type != BUILDING_SHIPWRIGHT || b->state != BUILDING_STATE_IN_USE) {
        return DOCK_QUEUE_NONE;
    }
    int32_t index = b->queue_slot;
    if (index == 0) {
        return DOCK_QUEUE_NONE;
    }
    return dock_queue_get(index - 1)->queue.count;
}

int32_t ship_find_best_dock(int32_t figure_id)
{
    int32_t best_distance = 100000;
    int32_t best_id = 0;
    int32_t best_size = DOCK_QUEUE_MAX_SHIPS;
    const figure *f = figure_get(figure_id);
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_SHIPWRIGHT || b->num_workers == 0) {
            continue;
        }
        int32_t size = dock_queue_size(id);
        if (size == DOCK_QUEUE_NONE) {
            continue;
        }
        int32_t dx = b->x - f->x;
        int32_t dy = b->y - f->y;
        if (size < best_size) {
            best_id = id;
            best_size = size;
            best_distance = (dx * dx) + (dy * dy);
        } else if (size == best_size) {
            int32_t distance = (dx * dx) + (dy * dy);
            if (distance < best_distance) {
                best_distance = distance;
                best_id = id;
                best_size = size;
            }
        }
    }
    return best_id;
}

int32_t ship_queue_at_dock(int32_t figure_id, int32_t building_id)
{
    int32_t index = building_get(building_id)->queue_slot;
    if (index == 0) {
        return building_id & ~0xFF;
    }
    if (figure_get(figure_id)->action_state == FIGURE_ACTION_CORPSE) {
        return (building_id & ~0xFF);
    }
    dyn_queue_push(&dock_queue_get(index - 1)->queue, figure_id);
    return 1;
}

int32_t ship_leave_dock_queue(int32_t figure_id)
{
    int32_t building_id = figure_get(figure_id)->destination_building_id;
    int32_t index = building_get(building_id)->queue_slot;
    if (index != 0 && index < 10) {
        dyn_queue_remove(&dock_queue_get(index - 1)->queue, figure_id);
        return 1;
    }
    return 0;
}

int32_t transport_legion_ready(int32_t figure_id)
{
    int32_t formation_id = figure_get(figure_id)->ship_formation_id;
    if (formation_id <= 0) {
        return 0;
    }
    const formation *legion = formation_get(formation_id);
    int32_t count = legion->num_figures;
    int32_t ready = 0;
    const int16_t *figures = legion->figures;
    for (int32_t i = 0; i < count; i++) {
        if (figures[i] == 0) {
            break;
        }
        const figure *soldier = figure_get(figures[i]);
        if (soldier->action_state != FIGURE_ACTION_CORPSE && soldier->direction == DIRECTION_AT_DESTINATION) {
            ready++;
        }
    }
    if (ready == count) {
        return formation_id;
    }
    return 0;
}

void transport_legion_board(int32_t formation_id)
{
    formation *legion = formation_get(formation_id);
    legion->is_at_fort = 0;
    const int16_t *figures = legion->figures;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t id = figures[i];
        if (id == 0) {
            return;
        }
        figure *soldier = figure_get(id);
        if (soldier->action_state != FIGURE_ACTION_CORPSE) {
            soldier->alternative_location_index = 0;
            soldier->action_state_before_attack = soldier->action_state;
            soldier->action_state = FIGURE_ACTION_SOLDIER_ON_SHIP;
            figure_route_remove(id);
            map_figure_delete(id);
        }
    }
}

void transport_legion_land(int32_t formation_id, int32_t x, int32_t y)
{
    const int16_t *figures = formation_get(formation_id)->figures;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t id = figures[i];
        if (id == 0) {
            return;
        }
        figure *soldier = figure_get(id);
        if (soldier->action_state != FIGURE_ACTION_CORPSE) {
            soldier->grid_offset = map_grid_offset(x, y);
            soldier->previous_tile_x = (int16_t) x;
            soldier->x = (int16_t) x;
            soldier->previous_tile_y = (int16_t) y;
            soldier->y = (int16_t) y;
            soldier->action_state = soldier->action_state_before_attack;
            soldier->cross_country.x = (int16_t) (soldier->x * 15);
            soldier->cross_country.y = (int16_t) (soldier->y * 15);
            map_figure_add(id);
        }
    }
}

void ship_go_to_best_dock(int32_t figure_id)
{
    int32_t dock_id = ship_find_best_dock(figure_current_id);
    figure *f = figure_get(figure_id);
    if (dock_id > 0) {
        if (f->type == FIGURE_TRANSPORT_SHIP) {
            f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK;
        } else if (f->type == FIGURE_WARSHIP) {
            f->action_state = FIGURE_ACTION_WARSHIP_GOING_TO_DOCK;
        }
        int32_t saved_x = map_result_x;
        int32_t saved_y = map_result_y;
        const building *b = building_get(dock_id);
        map_water_tile_next_to(b->x, b->y, b->size);
        f->source_x = f->x;
        f->source_y = f->y;
        f->direction = f->previous_tile_direction;
        f->destination_building_id = (int16_t) dock_id;
        f->destination_x = (int16_t) map_result_x;
        f->destination_y = (int16_t) map_result_y;
        map_result_y = saved_y;
        map_result_x = saved_x;
        figure_route_remove(figure_id);
        return;
    }
    if (f->type == FIGURE_TRANSPORT_SHIP || f->type == FIGURE_WARSHIP) {
        f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_WAITING;
    }
}

void transport_sink(int32_t figure_id)
{
    figure_combat_handle_corpse(figure_id);
    figure *f = figure_get(figure_id);
    if (f->wait_ticks != TRANSPORT_LEGION_DROWNS_TICK) {
        return;
    }
    if (f->has_troops_on_board) {
        int32_t formation_id = f->ship_formation_id;
        formation *legion = formation_get(formation_id);
        legion->ship_figure_id = 0;
        const int16_t *figures = legion->figures;
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            int32_t id = figures[i];
            if (id == 0) {
                break;
            }
            figure_get(id)->state = FIGURE_STATE_DEAD;
        }
    }
    formation_calculate_figures();
}

static void ship_head_for_dock(int32_t figure_id, int32_t building_id)
{
    figure *f = figure_get(figure_id);
    building *b = building_get(building_id);
    int32_t saved_x = map_result_x;
    int32_t saved_y = map_result_y;
    b->data.dock.ship_id = (int16_t) figure_id;
    map_water_tile_next_to(b->x, b->y, b->size);
    f->source_x = f->x;
    f->source_y = f->y;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    map_result_y = saved_y;
    map_result_x = saved_x;
    figure_route_remove(figure_id);
}

void transport_wait_at_dock(int32_t figure_id, int32_t building_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (++f->wait_ticks < SHIP_DOCK_WAIT_TICKS) {
        return;
    }
    f->wait_ticks = 0;
    f->animation_frame = 0;
    if (building_id != 0) {
        f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_SAILING;
        ship_head_for_dock(figure_id, building_id);
    }
}

void warship_wait_at_dock(int32_t figure_id, int32_t *moving)
{
    *moving = 0;
    figure *f = figure_get(figure_id);
    if (++f->wait_ticks < SHIP_DOCK_WAIT_TICKS) {
        return;
    }
    f->wait_ticks = 0;
    if (f->building_id != 0) {
        f->action_state = FIGURE_ACTION_WARSHIP_SAILING;
        ship_head_for_dock(figure_id, f->building_id);
    }
}

static void ship_arrived_at_dock(int32_t figure_id, figure *f, int32_t *moving)
{
    f->wait_ticks = 0;
    *moving = 0;
    if (dock_queue_size(f->destination_building_id) >= 1 && ship_find_best_dock(figure_id) != f->destination_building_id) {
        ship_go_to_best_dock(figure_id);
        return;
    }
    ship_queue_at_dock(figure_id, f->destination_building_id);
    f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_QUEUED_AT_DOCK;
}

void transport_sail_to_dock(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    int32_t direction = f->direction;
    if (direction == DIRECTION_AT_DESTINATION) {
        ship_arrived_at_dock(figure_id, f, moving);
    } else if (direction == DIRECTION_REROUTE || direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
    }
}

void warship_sail_to_dock(int32_t figure_id, int32_t *moving)
{
    ship_move(figure_id, SHIP_MOVE_STRAIGHT);
    figure *f = figure_get(figure_id);
    int32_t direction = f->direction;
    if (direction == DIRECTION_AT_DESTINATION) {
        ship_arrived_at_dock(figure_id, f, moving);
    } else if (direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
}

void transport_sail_to_legion(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    int32_t direction = f->direction;
    if (direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
        int32_t formation_id = transport_legion_ready(figure_id);
        if (formation_id == 0) {
            return;
        }
        if (formation_id == f->ship_formation_id) {
            formation *legion = formation_get(formation_id);
            if (legion->ship_figure_id == 0) {
                f->has_troops_on_board = 1;
                legion->order_x = f->x;
                legion->order_y = f->y;
                transport_legion_board(formation_id);
                f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_SAILING;
                legion->ship_figure_id = figure_id;
                f->wait_ticks = 0;
                return;
            }
            f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION;
            f->ship_formation_id = 0;
        }
        f->wait_ticks = 0;
    } else if (direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
        f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_EVADING;
    }
}

void transport_sail_to_landing(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    int32_t formation_id = f->ship_formation_id;
    int32_t direction = f->direction;
    if (direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
        if (f->ship_landing_x == 0 && f->ship_landing_y == 0) {
            return;
        }
        int32_t x = figure_id;
        int32_t y = 0;
        map_water_landing_point(f->ship_landing_x, f->ship_landing_y, f->x, f->y, &x, &y);
        map_find_nearest_free_land_tile(&x, &y);
        formation *legion = formation_get(formation_id);
        legion->order_x = (int16_t) x;
        legion->order_y = (int16_t) y;
        legion->x = (int16_t) x;
        legion->y = (int16_t) y;
        legion->ship_figure_id = 0;
        transport_legion_land(formation_id, x, y);
        f->has_troops_on_board = 0;
        f->ship_formation_id = 0;
        formation_legion_move_to(f->ship_landing_x, f->ship_landing_y, formation_id, 0);
        f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_SAILING;
        f->ship_landing_x = 0;
        f->ship_landing_y = 0;
    } else if (direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
        f->action_state = FIGURE_ACTION_TRANSPORT_SHIP_EVADING;
    }
}

int32_t figure_find_enemy_warship_in_range(int32_t x, int32_t y, int32_t range)
{
    int32_t best = 0;
    figure_search_best_distance = range;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *e = figure_get(id);
        if (e->state != FIGURE_STATE_ALIVE || e->action_state == FIGURE_ACTION_CORPSE || e->type != FIGURE_ENEMY_WARSHIP) {
            continue;
        }
        int32_t ex = e->x;
        int32_t ey = e->y;
        int32_t distance = calc_maximum_distance(ex, ey, x, y);
        if (distance < figure_search_best_distance && figure_movement_can_launch_cross_country_missile(x, y, ex, ey)) {
            best = id;
            figure_search_best_distance = distance;
        }
    }
    if (best == 0) {
        return 0;
    }
    const figure *enemy = figure_get(best);
    map_result_y = enemy->y;
    map_result_x = enemy->x;
    return best;
}

int32_t figure_find_enemy_warship_near(int32_t figure_id, int32_t range)
{
    const figure *f = figure_get(figure_id);
    return figure_find_enemy_warship_in_range(f->x, f->y, range);
}

static void ship_face_enemy(int32_t figure_id, figure *f)
{
    f->wait_ticks = 0;
    int32_t enemy_id = figure_find_enemy_warship_near(figure_id, SHIP_FACE_ENEMY_RANGE);
    const figure *e = figure_get(enemy_id);
    f->previous_tile_direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, e->x, e->y);
}

void transport_docked(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (++f->wait_ticks > SHIP_FACE_ENEMY_WAIT) {
        ship_face_enemy(figure_id, f);
    }
}

void warship_docked(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (++f->wait_ticks > SHIP_FACE_ENEMY_WAIT) {
        f->wait_ticks = 0;
        if (f->ship_move_mode != SHIP_MOVE_RESTING) {
            ship_face_enemy(figure_id, f);
        }
    }
}

int32_t warship_find_ram_target(int32_t figure_id)
{
    static const int32_t offsets[DIRECTION_COUNT] = { -GRID_SIZE, -GRID_SIZE + 1, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };
    const figure *f = figure_get(figure_id);
    int32_t grid_offset = map_info.grid_start + f->x + (f->y * GRID_SIZE);
    int32_t direction = (uint8_t) f->direction;
    if (direction > DIRECTION_TOP_LEFT) {
        return 0;
    }
    int32_t target = ship_find_target_at(grid_offset, figure_id);
    if (target == 0) {
        return ship_find_target_at(grid_offset + offsets[direction], figure_id);
    }
    return target;
}

void warship_move_attack(int32_t figure_id, int32_t *moving)
{
    ship_move(figure_id, SHIP_MOVE_CAUTIOUS);
    figure *f = figure_get(figure_id);
    if (f->ram_cooldown != 0) {
        int32_t cooldown = f->ram_cooldown - 1;
        if (cooldown < 0) {
            cooldown = 0;
        } else if (cooldown >= WARSHIP_RAM_COOLDOWN) {
            cooldown = WARSHIP_RAM_COOLDOWN;
        }
        f->ram_cooldown = cooldown;
    } else {
        int32_t target_id = warship_find_ram_target(figure_id);
        if (target_id != 0) {
            figure *target = figure_get(target_id);
            int32_t target_direction = target->direction;
            if (target_direction >= DIRECTION_COUNT) {
                target_direction = target->previous_tile_direction;
            }
            int32_t divisor = figure_ship_direction_table[(target_direction * DIRECTION_COUNT) + f->direction];
            int32_t damage = (f->ship.steps < figure_properties[f->type].ship_steps) ? f->ram_damage_fast : f->ram_damage_slow;
            damage /= divisor;
            ship_play_ram_sound(divisor);
            target->ship_hit_points -= damage;
            figure_route_remove(figure_id);
            int32_t turn = (random_generator_state.random1_7bit & 2) - 1;
            int32_t own = f->direction + turn;
            if (own >= DIRECTION_TOP_LEFT) {
                own = DIRECTION_TOP_LEFT;
            } else if (own < 0) {
                own = 0;
            }
            int32_t other = target->direction - turn;
            if (other >= DIRECTION_TOP_LEFT) {
                other = DIRECTION_TOP_LEFT;
            } else if (other < 0) {
                other = 0;
            }
            f->direction = (int8_t) own;
            target->direction = (int8_t) other;
            f->ram_cooldown = WARSHIP_RAM_COOLDOWN;
            f->action_state = FIGURE_ACTION_WARSHIP_HUNTING;
            f->target_figure_id = 0;
            figure_create_explosion_cloud(target->x, target->y, 0, 0);
        }
    }
    int32_t direction = f->direction;
    if (direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
    } else if (direction == DIRECTION_REROUTE || direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
    }
}

void warship_sail_to_destination(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (f->x == f->destination_x && f->y == f->destination_y) {
        f->direction = DIRECTION_AT_DESTINATION;
    } else {
        ship_move(figure_id, SHIP_MOVE_STRAIGHT);
        *moving = 1;
    }
    int32_t direction = f->direction;
    if (direction == DIRECTION_REROUTE || direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
        return;
    }
    if (direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
        int32_t state = f->action_state;
        if (state != FIGURE_ACTION_WARSHIP_DOCKED && state != FIGURE_ACTION_WARSHIP_HUNTING && state != FIGURE_ACTION_WARSHIP_GOING_TO_DOCK &&
            (uint8_t) ship_is_docked(figure_id)) {
            f->action_state = FIGURE_ACTION_WARSHIP_PATROLLING;
            f->warship_secondary_action = WARSHIP_MODE_PATROL;
            f->patrol_x = f->x;
            f->patrol_y = f->y;
        }
    }
}

void warship_evading(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (f->warship_secondary_action == WARSHIP_MODE_SAILING) {
        warship_sail_to_destination(figure_id, moving);
        if (f->direction == DIRECTION_AT_DESTINATION) {
            f->warship_secondary_action = WARSHIP_MODE_WAITING;
            *moving = 0;
        }
        return;
    }
    if (f->warship_secondary_action != WARSHIP_MODE_WAITING) {
        return;
    }
    *moving = 0;
    if (++f->wait_ticks < WARSHIP_EVADE_TICKS) {
        return;
    }
    f->wait_ticks = 0;
    int32_t enemy_id = ship_find_closest(figure_id, WARSHIP_EVADE_RANGE, 0);
    if (!enemy_id) {
        return;
    }
    const figure *enemy = figure_get(enemy_id);
    int32_t direction = (uint8_t) calc_general_direction(enemy->x, enemy->y, f->x, f->y);
    if (direction > DIRECTION_LEFT) {
        direction = DIRECTION_TOP_LEFT;
    }
    const ship_escape_move *m = &ship_escape_moves[direction];
    int32_t x;
    int32_t y;
    ship_pick_escape_point(&x, &y, m->dx1, m->dy1, m->dx2, m->dy2, m->dx3, m->dy3, figure_id);
    f->destination_x = (int16_t) x;
    f->destination_y = (int16_t) y;
    figure_route_remove(figure_id);
    f->warship_secondary_action = WARSHIP_MODE_SAILING;
}

static void warship_shoot_at(int32_t figure_id, figure *f, int32_t x, int32_t y)
{
    if (f->attack_image_offset) {
        if (f->attack_image_offset == 1) {
            figure_create_missile(figure_id, f->x, f->y, x, y, FIGURE_JAVELIN);
        }
        if (++f->attack_image_offset > WARSHIP_SHOOT_TICKS) {
            f->attack_image_offset = 0;
        }
    }
}

void warship_shoot(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    uint8_t wait = ++f->wait_ticks_missile;
    if (wait > figure_properties[f->type].missile_delay) {
        f->wait_ticks_missile = 0;
        f->attack_image_offset = figure_warship_find_enemy(figure_id, figure_properties[f->type].missile_range) ? 1 : 0;
    }
    warship_shoot_at(figure_id, f, map_result_x, map_result_y);
}

void warship_attack_ship(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    int32_t target_id = f->target_figure_id;
    if (target_id == 0 || figure_get(target_id)->state != FIGURE_STATE_ALIVE || figure_get(target_id)->action_state == FIGURE_ACTION_CORPSE) {
        target_id = figure_warship_find_enemy(figure_id, GRID_SIZE);
        f->target_figure_id = (int16_t) target_id;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
    }
    if (f->warship_secondary_action == WARSHIP_MODE_FOLLOW_TARGET) {
        figure *target = figure_get(target_id);
        int32_t distance = calc_maximum_distance(f->x, f->y, target->x, target->y);
        int32_t shoot = 0;
        if (distance < figure_properties[f->type].missile_range) {
            uint8_t wait = ++f->wait_ticks_missile;
            if (wait > figure_properties[f->type].missile_delay) {
                f->wait_ticks_missile = 0;
                shoot = target_id != 0;
            }
        }
        f->attack_image_offset = (uint8_t) shoot;
        warship_shoot_at(figure_id, f, target->x, target->y);
        warship_move_attack(figure_id, moving);
        if (f->direction == DIRECTION_AT_DESTINATION) {
            f->warship_secondary_action = WARSHIP_MODE_WAITING;
            figure_route_remove(figure_id);
            return;
        }
        if (++f->wait_ticks > WARSHIP_TARGET_CHECK_TICKS) {
            f->wait_ticks = 0;
            if (f->destination_x != target->x && f->destination_y != target->y) {
                f->warship_secondary_action = WARSHIP_MODE_WAITING;
                figure_route_remove(figure_id);
            }
        }
        return;
    }
    if (f->warship_secondary_action != WARSHIP_MODE_WAITING) {
        return;
    }
    if (target_id != 0) {
        f->warship_secondary_action = WARSHIP_MODE_FOLLOW_TARGET;
        f->direction = f->previous_tile_direction;
        const figure *target = figure_get(target_id);
        f->destination_x = target->x;
        f->destination_y = target->y;
        f->target_figure_id = (int16_t) target_id;
        figure_route_remove(figure_id);
        return;
    }
    f->action_state = f->action_state_before_attack;
    f->destination_x = f->x;
    f->destination_y = f->y;
    f->patrol_x = f->x;
    f->patrol_y = f->y;
    if (f->action_state == FIGURE_ACTION_WARSHIP_PATROLLING) {
        f->warship_secondary_action = WARSHIP_MODE_PATROL;
    } else if (f->action_state == FIGURE_ACTION_WARSHIP_HUNTING) {
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
    }
    f->target_figure_id = 0;
    figure_route_remove(figure_id);
}

void building_repair_queue_clear(void)
{
    for (int32_t i = 0; i < DOCK_QUEUES; i++) {
        dock_queue_get(i)->queue.flag = 0;
        dock_queue_get(i)->building_id = 0;
    }
}

void building_repair_queue_add(int32_t building_id)
{
    for (int32_t i = 0; i < DOCK_QUEUES; i++) {
        if (dock_queue_get(i)->queue.flag == 0) {
            building_get(building_id)->queue_slot = (uint8_t) (i + 1);
            dock_queue_get(i)->queue.flag = 1;
            dock_queue_get(i)->building_id = building_id;
            return;
        }
    }
    platform_debug_output("unable to find a repair queue for this shipyard!\n");
}

void building_repair_queue_remove(int32_t building_id)
{
    int32_t index = building_get(building_id)->queue_slot - 1;
    if (index >= 0 && index < DOCK_QUEUES) {
        dock_queue_get(index)->queue.flag = 0;
        dock_queue_get(index)->building_id = 0;
    }
}

static int32_t chebyshev(int32_t x, int32_t y, int32_t fx, int32_t fy)
{
    return calc_maximum_distance(x, y, fx, fy);
}

int32_t figure_warship_find_enemy_at(int32_t x, int32_t y, int32_t range)
{
    int32_t best = 0;
    int32_t best_score = 0;
    figure_search_best_distance = range;
    for (int32_t f = 1; f < MAX_FIGURES; f++) {
        const figure *candidate = figure_get(f);
        if (candidate->state != FIGURE_STATE_ALIVE || candidate->action_state == FIGURE_ACTION_CORPSE) {
            continue;
        }
        int32_t score;
        int32_t distance;
        int32_t fx = candidate->x;
        int32_t fy = candidate->y;
        switch (candidate->type) {
            case FIGURE_ENEMY_ARCHER:
            case FIGURE_ENEMY_INFANTRY:
            case FIGURE_ENEMY_CHARIOT:
            case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
            case FIGURE_ENEMY_EGYPTIAN_ARCHER:
            case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
            case FIGURE_BEDOUIN:
                distance = chebyshev(x, y, fx, fy);
                if (distance > figure_properties[figure_get(figure_current_id)->type].missile_range) {
                    continue;
                }
                score = WARSHIP_SCORE_LAND_ENEMY;
                break;
            case FIGURE_ENEMY_TRANSPORT_SHIP:
            case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
                score = candidate->has_troops_on_board ? WARSHIP_SCORE_LOADED_TRANSPORT : WARSHIP_SCORE_TRANSPORT;
                distance = chebyshev(x, y, fx, fy);
                break;
            case FIGURE_ENEMY_WARSHIP:
            case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
                distance = chebyshev(x, y, fx, fy);
                score = WARSHIP_SCORE_WARSHIP;
                break;
            default:
                continue;
        }
        if (score - distance > best_score - figure_search_best_distance) {
            figure_search_best_distance = distance;
            best = f;
            best_score = score;
        }
    }
    if (best == 0) {
        return 0;
    }
    const figure *enemy = figure_get(best);
    map_result_y = enemy->y;
    map_result_x = enemy->x;
    return best;
}

int32_t figure_warship_find_enemy(int32_t figure_id, int32_t range)
{
    const figure *f = figure_get(figure_id);
    return figure_warship_find_enemy_at(f->x, f->y, range);
}
