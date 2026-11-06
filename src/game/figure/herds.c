// Herds of animals: formation_herd_update spawns the animals of each herd near its home while it has fewer than its
// maximum and moves the herd's destination around within its roam radius; a building placed near a herd scares it
// away.
//
// The herd spawn mixes up arguments and tables:
// - figure_create gets the figure type as its player too;
// - a predator is created on the herd_spawn_offsets tile while the tile tested was the herd_roaming_offsets one;
// - the predators' roam radius is drawn with the C runtime's rand, the prey's with the game's random generator.

#include "figure/herds.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "figure/animals_internal.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/formation.h"
#include "map/grid.h"
#include "map/routing.h"
#include <stddef.h>
#include <stdint.h>

static int32_t herd_find_spawn_tile(int32_t formation_id, int32_t *dir, int32_t *grid_offset, bool is_prey)
{
    const formation *herd = formation_get(formation_id);
    int32_t tries;
    for (tries = 0; tries < HERD_OFFSET_COUNT; tries++) {
        *grid_offset = herd->home_x + map_info.grid_start + herd_roaming_offsets()[*dir].x + (herd_roaming_offsets()[*dir].y + herd->home_y) * GRID_SIZE;
        if (animal_terrain_allowed_climate(map_grids.terrain[*grid_offset], is_prey) && !map_grids.building_ids[*grid_offset]) {
            break;
        }
        if (++*dir >= HERD_OFFSET_COUNT) {
            *dir = 0;
        }
    }
    return tries;
}

static void herd_new_animal(int32_t figure_id, int32_t formation_id)
{
    figure *f = figure_get(figure_id);
    f->action_state = FIGURE_ACTION_HERD_SPAWNED;
    f->formation_id = (int16_t) formation_id;
    f->wait_ticks = HERD_SPAWNED_WAIT_TICKS;
}

static int32_t herd_move_destination(int32_t formation_id, int32_t radius, bool is_prey, int32_t *x, int32_t *y)
{
    formation *herd = formation_get(formation_id);
    if (herd->destination_x == herd->home_x && herd->destination_y == herd->home_y) {
        if (!animal_find_reachable_tile_in_area(herd->destination_x, herd->destination_y, radius, x, y, is_prey)) {
            return 0;
        }
    } else {
        random_generate_next();
        if (random_generator_state.random1_15bit & 7) {
            if (!animal_find_reachable_tile_in_area(herd->destination_x, herd->destination_y, radius, x, y, is_prey)) {
                return 0;
            }
        } else {
            *x = herd->home_x;
            *y = herd->home_y;
        }
    }
    if (!formation_enemy_move_formation_to(formation_id, *x, *y)) {
        return 0;
    }
    herd->destination_x = (int16_t) formation_move_result_x;
    herd->destination_y = (int16_t) formation_move_result_y;
    return 1;
}

static void update_predators(int32_t formation_id, int32_t type, int32_t max_figures)
{
    formation *herd = formation_get(formation_id);
    const climate_animal *record = scenario_predator();
    herd->figure_type = (int16_t) type;
    herd->max_figures = (uint8_t) max_figures;
    if (herd->num_figures < (uint8_t) max_figures && (!(uint8_t) record->spawn_only_when_present || herd->num_figures)) {
        if (++herd->herd_spawn_delay <= record->spawn_delay) {
            return;
        }
        herd->herd_spawn_delay = 0;
        random_generate_next();
        int32_t dir = random_generator_state.random1_15bit & (HERD_OFFSET_COUNT - 1);
        int32_t grid_offset;
        int32_t tries = herd_find_spawn_tile(formation_id, &dir, &grid_offset, 0);
        if (map_grids.building_ids[grid_offset] || !animal_terrain_allowed_climate(map_grids.terrain[grid_offset], 0) || tries == HERD_OFFSET_COUNT) {
            return;
        }
        random_generate_next();
        int16_t figure_type = herd->figure_type;
        int32_t spawn_x = herd_spawn_offsets()[dir].x + herd->home_x;
        int32_t spawn_y = herd_spawn_offsets()[dir].y + herd->home_y;
        int32_t figure_id = figure_create(figure_type, figure_type, spawn_x, spawn_y, random_generator_state.random1_15bit & 7);
        herd_new_animal(figure_id, formation_id);
        return;
    }
    herd->advance_ticks++;
    if (record->move_delay >= herd->advance_ticks) {
        return;
    }
    int32_t radius = record->roam_radius;
    herd->advance_ticks = 0;
    herd->herd_direction = 0;
    int32_t x = herd->destination_x;
    int32_t y = herd->destination_y;
    radius >>= 1;
    if (radius) {
        radius += crt_rand() % radius;
    }
    herd_move_destination(formation_id, radius, 0, &x, &y);
}

static void update_prey(int32_t formation_id, int32_t type, int32_t max_figures, int32_t osiris_bonus)
{
    formation *herd = formation_get(formation_id);
    const climate_animal *record = scenario_prey();
    herd->figure_type = (int16_t) type;
    herd->max_figures = (uint8_t) max_figures;
    herd->herd_state = herd_count_animals_in_state(formation_id, FIGURE_ACTION_HERD_STAMPEDE) ? HERD_STATE_STAMPEDE : HERD_STATE_ROAMING;
    if (herd->num_figures < herd->max_figures) {
        int32_t delay = osiris_bonus ? (record->spawn_delay / 2) : record->spawn_delay;
        if (++herd->herd_spawn_delay > delay) {
            herd->herd_spawn_delay = 0;
            random_generate_next();
            int32_t dir = random_generator_state.random1_15bit & (HERD_OFFSET_COUNT - 1);
            int32_t grid_offset;
            int32_t tries = herd_find_spawn_tile(formation_id, &dir, &grid_offset, 1);
            if (map_grids.building_ids[grid_offset] || !animal_terrain_allowed_climate(map_grids.terrain[grid_offset], 1)) {
                return;
            }
            if (tries != HERD_OFFSET_COUNT) {
                int16_t figure_type = herd->figure_type;
                const formation_layout_spot *spot = &herd_roaming_offsets()[dir];
                int32_t figure_id = figure_create(figure_type, figure_type, spot->x + herd->home_x, spot->y + herd->home_y, 0);
                herd_new_animal(figure_id, formation_id);
            }
        }
    }
    herd->advance_ticks++;
    if (herd->advance_ticks <= record->move_delay || herd_count_animals_in_state(formation_id, FIGURE_ACTION_HERD_ALERT) ||
        herd_count_animals_in_state(formation_id, FIGURE_ACTION_HERD_FLEE) || herd->herd_state == HERD_STATE_STAMPEDE) {
        return;
    }
    herd->advance_ticks = 0;
    herd->herd_direction = 0;
    int32_t radius = record->roam_radius >> 1;
    if (radius) {
        random_generate_next();
        radius += random_generator_state.random1_15bit % radius;
    }
    random_generate_next();
    herd->herd_direction = (uint8_t) (random_generator_state.random1_15bit & 7);
    int32_t x, y;
    if (!herd_move_destination(formation_id, radius, 1, &x, &y)) {
        return;
    }
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t figure_id = herd->figures[i];
        if (figure_id > 0) {
            figure *f = figure_get(figure_id);
            int32_t action = f->action_state;
            if (action != FIGURE_ACTION_ATTACK && action != FIGURE_ACTION_CORPSE && action != FIGURE_ACTION_HERD_HUNTED && action != FIGURE_ACTION_HERD_ALERT &&
                action != FIGURE_ACTION_HERD_FLEE && action != FIGURE_ACTION_HERD_STAMPEDE) {
                f->action_state = FIGURE_ACTION_HERD_CHOOSE;
                f->wait_ticks = 0;
            }
        }
    }
}

void formation_herd_update(void)
{
    const climate_animal *prey = scenario_prey();
    int32_t prey_type = prey->figure_type;
    int32_t prey_max = prey->max_figures;
    const climate_animal *predator = scenario_predator();
    int32_t predator_type = predator->figure_type;
    int32_t predator_max = predator->max_figures;
    int32_t osiris_bonus = 0;
    const city_player *player = city_player_current();
    if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_OSIRIS &&
        building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ORACLE)) {
        osiris_bonus = 1;
    }
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        const formation *herd = formation_get(id);
        if (herd->in_use != 1 || herd->is_legion || herd->herd_kind == HERD_KIND_NONE) {
            continue;
        }
        if (herd->herd_kind == HERD_KIND_PREDATORS) {
            update_predators(id, predator_type, predator_max);
        } else {
            update_prey(id, prey_type, prey_max, osiris_bonus);
        }
    }
}

void formation_herds_scared_by_building(int32_t x, int32_t y)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *herd = formation_get(id);
        if (herd->in_use != 1 || herd->is_legion != 0 || herd->herd_kind == HERD_KIND_NONE || herd->num_figures == 0) {
            continue;
        }
        int32_t distance = calc_maximum_distance(herd->x, herd->y, x, y);
        if (distance < HERD_SCARE_DISTANCE) {
            herd->wait.bytes[0] = HERD_SCARED_WAIT_TICKS;
            herd->wait.bytes[1] = 0;
            herd->herd_direction = (uint8_t) calc_general_direction(x, y, herd->x, herd->y);
        }
    }
}
