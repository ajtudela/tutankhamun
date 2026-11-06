// Helpers of the enemy formation update: the soldier strength map built from the player's legions and the strongest
// reachable tile in it, the attack target of an enemy formation by its attack kind's priority list, the reset of a
// formation's figures to their first action, and the enemies the kingdom sends away.
//
// Results leak out of the searches:
// - the square walks leave map_result_x / _y where their loops end;
// - the target search returns the building id with its low byte replaced by 1, and callers read the low byte.

#include "figure/formation_enemy.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "city/city_data.h"
#include "city/city_player.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_enemy.h"
#include <stdint.h>
#include <string.h>

void formation_enemy_set_fleeing(void)
{
    city_player *player = city_player_current();
    int32_t left = player->enemies_to_leave;
    if (left <= 0) {
        return;
    }
    int32_t first_offset = 0;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_ALIVE) {
            continue;
        }
        if (left <= 0) {
            break;
        }
        int32_t type = f->type;
        if ((type >= FIGURE_ENEMY_ARCHER && type <= FIGURE_ENEMY_BARBARIAN_HEAVY_2) || type == FIGURE_KINGDOM_SOLDIER || type == FIGURE_BEDOUIN ||
            type == FIGURE_ENEMY_EGYPTIAN_INFANTRY || type == FIGURE_ENEMY_EGYPTIAN_ARCHER || type == FIGURE_ENEMY_EGYPTIAN_CHARIOT) {
            left--;
            f->action_state = FIGURE_ACTION_CORPSE;
            if (first_offset == 0) {
                first_offset = f->grid_offset;
            }
        }
    }
    player->enemies_to_leave = 0;
    city_message_use_popup = 1;
    city_message_post(304, 0, first_offset, 0, 0);
}

void formation_enemy_set_figures_to_initial(int32_t formation_id)
{
    const int16_t *figures = formation_get(formation_id)->figures;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        if (figures[i] > 0) {
            figure *f = figure_get(figures[i]);
            if (f->action_state != FIGURE_ACTION_ATTACK && f->action_state != FIGURE_ACTION_CORPSE) {
                f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
                f->wait_ticks = 0;
            }
        }
    }
}

void map_soldier_strength_add(int32_t x, int32_t y, int32_t radius, int32_t amount)
{
    int32_t x_min = x - radius;
    int32_t y_min = y - radius;
    int32_t x_max = x + radius;
    int32_t y_max = y + radius;
    if (x_min < 0) {
        x_min = 0;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    map_result_y = y_min;
    if (y_min > y_max) {
        return;
    }
    map_result_y = y_max + 1;
    for (int32_t yy = y_min; yy <= y_max; yy++) {
        int32_t offset = map_info.grid_start + yy * GRID_SIZE + x_min;
        map_result_x = x_min;
        if (x_min > x_max) {
            continue;
        }
        map_result_x = x_max + 1;
        for (int32_t xx = x_min; xx <= x_max; xx++, offset++) {
            uint8_t value = (uint8_t) (map_grids.soldier_strength[offset] + amount);
            map_grids.soldier_strength[offset] = value;
            int32_t figure_id = map_grids.figure_ids[offset];
            if (figure_id > 0) {
                uint8_t type = figure_get(figure_id)->type;
                if (type >= FIGURE_ARCHER && type <= FIGURE_CHARIOTEER) {
                    map_grids.soldier_strength[offset] = (uint8_t) (value + 2);
                }
            }
        }
    }
}

void formation_update_soldier_strength(void)
{
    memset(map_grids.soldier_strength, 0, GRID_CELLS);
    for (int32_t id = 1; id <= FORMATION_MAX_LEGIONS; id++) {
        const formation *legion = formation_get(id);
        if (legion->in_use != 1 || legion->is_legion == 0) {
            continue;
        }
        int32_t count = legion->num_figures;
        for (int32_t ring = 0; ring < SOLDIER_STRENGTH_RINGS; ring++) {
            if (count > (ring * SOLDIERS_PER_STRENGTH_RING)) {
                map_soldier_strength_add(legion->x, legion->y, SOLDIER_STRENGTH_MAX_RADIUS - ring, 1);
            }
        }
    }
}

int32_t map_soldier_strength_get_max(int32_t x, int32_t y, int32_t radius)
{
    int32_t x_min = x - radius;
    int32_t x_max = x + radius;
    int32_t y_max = y + radius;
    int32_t y_min = y - radius;
    if (x_min < 0) {
        x_min = 0;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    int32_t best = 0;
    int32_t best_x = x;
    int32_t best_y = y;
    map_result_y = y_min;
    if (y_min > y_max) {
        return 0;
    }
    int32_t yy;
    for (yy = y_min; yy <= y_max; yy++) {
        int32_t offset = map_info.grid_start + yy * GRID_SIZE + x_min;
        map_result_x = x_min;
        if (x_min > x_max) {
            continue;
        }
        int32_t xx;
        for (xx = x_min; xx <= x_max; xx++, offset++) {
            if (routing_grids.distance[offset] > 0 && map_grids.soldier_strength[offset] > best) {
                best = map_grids.soldier_strength[offset];
                best_x = xx;
                best_y = yy;
            }
        }
        map_result_x = xx;
    }
    map_result_y = yy;
    if (best <= 0) {
        return 0;
    }
    map_result_y = best_y;
    map_result_x = best_x;
    return 1;
}

static void find_target_building(int32_t formation_id, const int16_t *list, int32_t *best_index, int32_t *best_distance, int32_t *best_id)
{
    const formation *enemies = formation_get(formation_id);
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || map_grids.soldier_strength[b->grid_offset] != 0 || routing_grids.distance[b->grid_offset] <= 0) {
            continue;
        }
        for (int32_t i = 0; i <= *best_index && i < ENEMY_ATTACK_LIST_LENGTH; i++) {
            if (list[i] == 0) {
                break;
            }
            if (b->type != list[i]) {
                continue;
            }
            int32_t distance = calc_maximum_distance(enemies->x, enemies->y, b->x, b->y);
            if (i < *best_index) {
                *best_index = i;
                *best_distance = distance;
                *best_id = id;
            } else if (distance < *best_distance) {
                *best_distance = distance;
                *best_id = id;
            }
            break;
        }
    }
}

bool formation_enemy_set_target_building(int32_t formation_id)
{
    formation *enemies = formation_get(formation_id);
    int32_t kind = enemies->attack_type;
    if (kind == FORMATION_ATTACK_RANDOM) {
        kind = crt_rand() & 3;
    }
    int32_t best_index = ENEMY_ATTACK_LIST_LENGTH;
    int32_t best_id = 0;
    int32_t best_distance = 10000;
    map_routing_noncitizen_can_travel_through_everything(enemies->x, enemies->y, -2, -2);
    find_target_building(formation_id, enemy_attack_priority + kind * ENEMY_ATTACK_LIST_LENGTH, &best_index, &best_distance, &best_id);
    if (best_id <= 0) {
        find_target_building(formation_id, enemy_attack_fallback, &best_index, &best_distance, &best_id);
        if (best_id <= 0) {
            return false;
        }
    }
    const building *b = building_get(best_id);
    if (b->type != BUILDING_STORAGE_YARD && b->type != BUILDING_STORAGE_YARD_UP) {
        enemies->destination_x = b->x;
        enemies->destination_y = b->y;
        enemies->destination_building_id = (int16_t) best_id;
        return true;
    }
    enemies->destination_x = (int16_t) (b->x + 1);
    enemies->destination_y = b->y;
    enemies->destination_building_id = (int16_t) (best_id + 1);
    return true;
}
