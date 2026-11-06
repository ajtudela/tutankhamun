// Animals, hunters and gatherers: the herd animals (birds, ostrich, antelope) and their reactions to threats, the
// predators (crocodile, hippo, asp) and their hunt, the hunter and its spear, the lumberjack and the reed gatherer, and
// the ring searches for figures and tiles they share. The waits, searches and loads are animal_rules, the hippo's and
// crocodile's choice of their next action animal_choices (both from config files).
//
// The searches keep these irregularities:
// - the C runtime's rand is reseeded with the time before the predators' and herd animals' searches;
// - animal_find_flee_tile_from tests the tile of the grid offset table but returns the left-right mirrored tile;
// - the state search of the figure searches steps map_result_y back instead of its own y and tests terrain on side 0;
// - the ostrich, antelope and predators bring their image direction into 0..7 only on some branches.

#include "figure/animals.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "figure/animals_internal.h"
#include "figure/boats.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing.h"
#include "map/routing_land.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/vegetation.h"
#include "platform/platform_time.h"
#include <stdbool.h>
#include <stdint.h>

int32_t animal_rules[ANIMAL_RULE_COUNT] = {
    [ANIMAL_RULE_DYING_TICKS] = 500,
    [ANIMAL_RULE_NO_TILE_WAIT_TICKS] = 100,
    [ANIMAL_RULE_FERRY_WAIT_TICKS] = 100,
    [ANIMAL_RULE_HERD_ALERT_TICKS] = 20,
    [ANIMAL_RULE_HERD_REST_BASE_TICKS] = 150,
    [ANIMAL_RULE_HERD_REST_RANDOM_TICKS] = 150,
    [ANIMAL_RULE_PREDATOR_REST_BASE_TICKS] = 250,
    [ANIMAL_RULE_PREDATOR_REST_RANDOM_TICKS] = 100,
    [ANIMAL_RULE_PREDATOR_CHASE_TICKS_PER_RING] = 18,
    [ANIMAL_RULE_HIPPO_ANIMATE_TICKS] = 20,
    [ANIMAL_RULE_HIPPO_AFTER_HUNT_TICKS] = 200,
    [ANIMAL_RULE_ASP_REST_BASE_TICKS] = 50,
    [ANIMAL_RULE_ASP_REST_RANDOM_TICKS] = 100,
    [ANIMAL_RULE_GATHERER_CUT_TICKS] = 300,
    [ANIMAL_RULE_HUNTER_SHOOT_TIMEOUT_TICKS] = 300,
    [ANIMAL_RULE_HUNTER_RETRY_TICKS] = 20,
    [ANIMAL_RULE_HUNTER_TAKE_PREY_TICKS] = 18,
    [ANIMAL_RULE_HUNTER_SHOOT_DISTANCE] = 3,
    [ANIMAL_RULE_HUNTER_SEARCH_PATH_LENGTH] = 1000,
    [ANIMAL_RULE_HUNTER_NEAR_SEARCH_PATH_LENGTH] = 6,
    [ANIMAL_RULE_HUNTER_HOMEWARD_SEARCH_PATH_LENGTH] = 100,
    [ANIMAL_RULE_HUNTER_GAME_LOAD] = 100,
    [ANIMAL_RULE_LUMBERJACK_WOOD_LOAD] = 25,
    [ANIMAL_RULE_REED_GATHERER_REEDS_LOAD] = 50,
};

animal_choice animal_choices[ANIMAL_CHOICE_COUNT] = {
    [ANIMAL_CHOICE_HIPPO_IDLE] = { { 50, 50, 0, 0 } },
    [ANIMAL_CHOICE_HIPPO_OTHER] = { { 29, 70, 0, 1 } },
    [ANIMAL_CHOICE_HIPPO_IN_WATER] = { { 75, 5, 20, 0 } },
    [ANIMAL_CHOICE_HIPPO_ANIMATING] = { { 90, 1, 9, 0 } },
    [ANIMAL_CHOICE_CROCODILE_IDLE] = { { 50, 50, 0, 0 } },
    [ANIMAL_CHOICE_CROCODILE_IN_WATER] = { { 90, 10, 0, 0 } },
    [ANIMAL_CHOICE_CROCODILE_OTHER] = { { 25, 75, 0, 0 } },
};

static int16_t random_rest_ticks(int32_t base_rule, int32_t random_rule)
{
    return (int16_t) ((random_generator_state.random1_15bit % animal_rules[random_rule]) + animal_rules[base_rule]);
}

int32_t herd_count_animals_in_state(int32_t formation_id, uint32_t action_state)
{
    int32_t count = 0;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state && figure_is_herd_animal(f->type) && f->formation_id == formation_id && f->action_state == action_state) {
            count++;
        }
    }
    return count;
}

bool animal_is_amphibious_climate(bool is_prey)
{
    if (is_prey) {
        return scenario_prey()->amphibious;
    }
    return scenario_predator()->amphibious;
}

bool animal_terrain_allowed_climate(uint32_t terrain, bool is_prey)
{
    uint32_t allowed = is_prey ? (uint32_t) scenario_prey()->terrain : (uint32_t) scenario_predator()->terrain;
    if (terrain & ANIMAL_TERRAIN_BLOCKED) {
        return 0;
    }
    uint32_t special = terrain & ANIMAL_TERRAIN_SPECIAL;
    if (allowed || special) {
        if (editor_active == 1) {
            return (special & allowed) != 0;
        }
        if ((terrain & TERRAIN_WATER) && !(allowed & TERRAIN_WATER)) {
            return 0;
        }
        if ((terrain & TERRAIN_MARSH) && !(allowed & TERRAIN_MARSH)) {
            return 0;
        }
    }
    return 1;
}

int32_t animal_find_free_neighbour_direction(int32_t x, int32_t y, int32_t direction, int32_t src_x, int32_t src_y, bool is_prey)
{
    for (int32_t tries = 0;;) {
        int32_t nx = herd_roaming_offsets()[direction].x + x;
        int32_t ny = herd_roaming_offsets()[direction].y + y;
        int32_t grid_offset = map_info.grid_start + ny * GRID_SIZE + nx;
        int32_t reachable;
        if (!animal_is_amphibious_climate(is_prey)) {
            reachable = map_routing_animal_can_travel(src_x, src_y, nx, ny);
        } else {
            reachable = map_routing_amphibia_can_travel(src_x, src_y, nx, ny);
            if (!reachable) {
                reachable = map_routing_citizen_can_travel_over_land_any(src_x, src_y, nx, ny);
            }
        }
        if (reachable && !map_grids.figure_ids[grid_offset] && animal_terrain_allowed_climate(map_grids.terrain[grid_offset], is_prey)) {
            return direction;
        }
        if (++direction > 15) {
            direction = 0;
        }
        if (++tries > 15) {
            return -1;
        }
    }
}

bool animal_find_flee_tile_from(int32_t x, int32_t y, int32_t from_x, int32_t from_y, int32_t *out_x, int32_t *out_y)
{
    static const int32_t dy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
    static const int32_t dx[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
    static const int32_t offsets[8] = { -GRID_SIZE, -GRID_SIZE + 1, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };
    int32_t dir = calc_missile_shooter_direction(from_x, from_y, x, y) & 0xFF;
    int32_t end = dir - 1;
    if (end < 0) {
        end = dir + 7;
    }
    int32_t base = map_info.grid_start + y * GRID_SIZE + x;
    while (dir != end) {
        int32_t grid_offset = offsets[dir] + base;
        if (!map_grids.desirability[grid_offset] && !(map_grids.terrain[grid_offset] & TERRAIN_DIKE) && animal_terrain_allowed_climate(map_grids.terrain[grid_offset], 1)) {
            *out_x = dx[dir] + x;
            *out_y = dy[dir] + y;
            if (map_routing_animal_can_travel(x, y, *out_x, *out_y)) {
                return 1;
            }
        }
        if (++dir > 7) {
            dir -= 8;
        }
    }
    return 0;
}

int32_t predator_prey_on_tile_at(int32_t grid_offset, int32_t predator_id)
{
    int32_t id = map_grids.figure_ids[grid_offset];
    while (id) {
        figure *f = figure_get(id);
        if (predator_id != id && f->state && f->action_state != FIGURE_ACTION_CORPSE) {
            int32_t type = f->type;
            if (figure_properties[type].category && figure_get(predator_id)->type != type && !figure_is_boat(type)) {
                return id;
            }
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

int32_t animal_find_reachable_tile_in_area(int32_t x, int32_t y, int32_t radius, int32_t *out_x, int32_t *out_y, bool is_prey)
{
    int32_t size = radius * 2 + 1;
    *out_x = x - radius;
    *out_y = y - radius;
    int32_t row_start = *out_x + (y - radius) * GRID_SIZE + map_info.grid_start;
    for (int32_t ring = 0; ring < GRID_SIZE; ring++) {
        static const int32_t steps[4][3] = { { 1, 1, 0 }, { GRID_SIZE, 0, 1 }, { -1, -1, 0 }, { -GRID_SIZE, 0, -1 } };
        map_tile_cursor_offset = row_start;
        for (int32_t side = 0; side < 4; side++) {
            const int32_t *step = steps[side];
            for (int32_t i = 0; i < size; i++) {
                int32_t g = map_tile_cursor_offset;
                if (*out_x >= 0 && *out_y >= 0 && *out_x < map_info.grid_width && *out_y < map_info.grid_height && !map_grids.desirability[g] &&
                    !(map_grids.terrain[g] & TERRAIN_DIKE) && animal_terrain_allowed_climate(map_grids.terrain[g], is_prey) &&
                    map_routing_animal_can_travel(x, y, *out_x, *out_y)) {
                    return 1;
                }
                map_tile_cursor_offset += step[0];
                *out_x += step[1];
                *out_y += step[2];
            }
            map_tile_cursor_offset -= step[0];
            *out_x -= step[1];
            *out_y -= step[2];
        }
        if (radius == 0) {
            return 0;
        }
        size -= 2;
        row_start += GRID_SIZE + 1;
        *out_x += 1;
        radius--;
        *out_y += 1;
    }
    return 0;
}

typedef int32_t (*ring_tile_test)(int32_t grid_offset, int32_t side, const void *data);

static int32_t search_rings(int32_t x, int32_t y, int32_t *out_x, int32_t *out_y, int32_t rings, ring_tile_test test, const void *data, int32_t y_quirk)
{
    static const int32_t steps[4][3] = { { 1, 1, 0 }, { GRID_SIZE, 0, 1 }, { -1, -1, 0 }, { -GRID_SIZE, 0, -1 } };
    int32_t row_start = map_info.grid_start + y * GRID_SIZE + x;
    int32_t size = 1;
    *out_x = x;
    *out_y = y;
    for (int32_t ring = 0; ring < GRID_SIZE;) {
        map_tile_cursor_offset = row_start;
        for (int32_t side = 0; side < 4; side++) {
            const int32_t *step = steps[side];
            for (int32_t i = 0; i < size; i++) {
                if (*out_x >= 0 && *out_y >= 0 && *out_x < map_info.grid_width && *out_y < map_info.grid_height) {
                    int32_t result = test(map_tile_cursor_offset, side, data);
                    if (result) {
                        return result;
                    }
                }
                map_tile_cursor_offset += step[0];
                *out_x += step[1];
                *out_y += step[2];
            }
            map_tile_cursor_offset -= step[0];
            *out_x -= step[1];
            if (y_quirk && side == 1) {
                map_result_y--;
            } else {
                *out_y -= step[2];
            }
        }
        if (rings == 0) {
            return 0;
        }
        ring++;
        size += 2;
        row_start -= GRID_SIZE + 1;
        *out_x -= 1;
        *out_y -= 1;
        rings--;
    }
    return 0;
}

typedef struct {
    int32_t num_types;
    const int32_t *types;
    uint32_t terrain_mask;
    int32_t exclude_id;
    int32_t check_state;
    uint32_t action_state;
} figure_search;

static int32_t figure_search_tile(int32_t grid_offset, int32_t side, const void *data)
{
    const figure_search *s = data;
    for (int32_t t = 0; t < s->num_types; t++) {
        int32_t id = map_grids.figure_ids[grid_offset];
        while (id) {
            figure *f = figure_get(id);
            if ((uint32_t) f->type == (uint32_t) s->types[t] && s->exclude_id != id && (!s->check_state || f->action_state == s->action_state)) {
                return id;
            }
            id = f->next_figure_id_on_same_tile;
        }
    }
    if (s->check_state && side != 0) {
        return 0;
    }
    uint32_t terrain = map_grids.terrain[grid_offset];
    if (!(terrain & TERRAIN_DIKE) && (s->terrain_mask & terrain)) {
        return s->num_types + 1;
    }
    return 0;
}

int32_t animal_find_nearest_figure_of_types(
    int32_t x, int32_t y, int32_t num_types, const int32_t *types, uint32_t terrain_mask, int32_t *out_x, int32_t *out_y, int32_t rings, int32_t exclude_id)
{
    figure_search s = { num_types, types, terrain_mask, exclude_id, 0, 0 };
    return search_rings(x, y, out_x, out_y, rings, figure_search_tile, &s, 0);
}

int32_t animal_find_nearest_figure_of_types_in_state(int32_t x, int32_t y, int32_t num_types, const int32_t *types, uint32_t terrain_mask, int32_t *out_x,
    int32_t *out_y, int32_t rings, int32_t exclude_id, uint32_t action_state)
{
    figure_search s = { num_types, types, terrain_mask, exclude_id, 1, action_state };
    return search_rings(x, y, out_x, out_y, rings, figure_search_tile, &s, 1);
}

typedef struct {
    tile_callback callback;
    int32_t arg;
} callback_search;

static int32_t callback_search_tile(int32_t grid_offset, int32_t side, const void *data)
{
    (void) side;
    const callback_search *s = data;
    if (map_grids.terrain[grid_offset] & TERRAIN_DIKE) {
        return 0;
    }
    return s->callback(grid_offset, s->arg);
}

int32_t animal_search_rings_for(int32_t x, int32_t y, int32_t *out_x, int32_t *out_y, int32_t rings, int32_t arg, tile_callback callback)
{
    callback_search s = { callback, arg };
    return search_rings(x, y, out_x, out_y, rings, callback_search_tile, &s, 0);
}

static void reseed_crt_rand(void)
{
    crt_srand((uint32_t) platform_time_ms());
}

bool predator_find_prey_nearby(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (!figure_is_predator_animal(f->type) || !f->state) {
        return 0;
    }
    int32_t action = f->action_state;
    if (action == FIGURE_ACTION_PREDATOR_DYING || action == FIGURE_ACTION_CORPSE || action == FIGURE_ACTION_ATTACK || action == FIGURE_ACTION_PREDATOR_CHASE) {
        return 0;
    }
    uint8_t counter = ++f->hunt_ticks;
    const climate_animal *record = scenario_predator();
    if (record->hunt_delay <= (int32_t) counter) {
        int32_t radius = record->hunt_radius;
        int32_t x, y;
        f->hunt_ticks = 0;
        reseed_crt_rand();
        int32_t prey_id = animal_search_rings_for(f->x, f->y, &x, &y, radius, figure_id, predator_prey_on_tile_at);
        if (prey_id) {
            figure *prey = figure_get(prey_id);
            f->destination_x = prey->x;
            f->destination_y = prey->y;
            f->action_state = FIGURE_ACTION_PREDATOR_CHASE;
            f->wait_ticks = (int16_t) ((int16_t) radius * animal_rules[ANIMAL_RULE_PREDATOR_CHASE_TICKS_PER_RING]);
            f->prey_figure_id = (int16_t) prey_id;
            return 1;
        }
    }
    return 0;
}

bool herd_animal_react_to_threats(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t x, y;
    if (!figure_is_herd_animal(f->type) || !f->state || f->action_state == FIGURE_ACTION_HERD_HUNTED || f->action_state == FIGURE_ACTION_CORPSE) {
        return 0;
    }
    if (f->action_state == FIGURE_ACTION_HERD_FLEE || f->action_state == FIGURE_ACTION_HERD_STAMPEDE) {
        return 1;
    }
    reseed_crt_rand();
    int32_t mate_id = animal_find_nearest_figure_of_types_in_state(f->x, f->y, 3, animal_prey_types, 0, &x, &y, 1, figure_id, FIGURE_ACTION_HERD_FLEE);
    if (mate_id) {
        figure_route_remove(figure_id);
        if (f->action_state != FIGURE_ACTION_HERD_ALERT && f->action_state != FIGURE_ACTION_HERD_ALERTED_BY_MATE) {
            f->action_state = FIGURE_ACTION_HERD_ALERTED_BY_MATE;
            f->flee_ticks = 0;
            return 1;
        }
        f->action_state = FIGURE_ACTION_HERD_FLEE;
        const figure *mate = figure_get(mate_id);
        random_generate_next();
        f->destination_x = (int16_t) (mate->destination_x + (random_generator_state.random1_15bit & 3) - 2);
        random_generate_next();
        f->destination_y = (int16_t) (mate->destination_y + (random_generator_state.random1_15bit & 3) - 2);
        return 1;
    }
    if (f->action_state == FIGURE_ACTION_HERD_ALERT) {
        int32_t threat_id = animal_find_nearest_figure_of_types(f->x, f->y, 4, animal_threat_types, 0, &x, &y, 2, figure_id);
        if (threat_id) {
            random_generate_next();
            int32_t flee_x, flee_y;
            if ((random_generator_state.random1_15bit & 3) == 0) {
                figure *threat = figure_get(threat_id);
                if (animal_find_flee_tile_from(f->x, f->y, threat->x, threat->y, &flee_x, &flee_y)) {
                    f->destination_x = (int16_t) flee_x;
                    f->destination_y = (int16_t) flee_y;
                    f->action_state = FIGURE_ACTION_HERD_FLEE;
                    return 1;
                }
            }
        }
        if ((f->action_state == FIGURE_ACTION_HERD_ALERT || f->action_state == FIGURE_ACTION_HERD_ALERTED_BY_MATE) &&
            ++f->flee_ticks > animal_rules[ANIMAL_RULE_HERD_ALERT_TICKS]) {
            figure_route_remove(figure_id);
            f->action_state = FIGURE_ACTION_HERD_CHOOSE;
        }
    } else {
        if (animal_find_nearest_figure_of_types_in_state(f->x, f->y, 3, animal_prey_types, 0, &x, &y, 3, figure_id, FIGURE_ACTION_HERD_ALERT)) {
            f->action_state = FIGURE_ACTION_HERD_ALERTED_BY_MATE;
            f->flee_ticks = 0;
            return 1;
        }
        if (animal_find_nearest_figure_of_types(f->x, f->y, 4, animal_threat_types, 0, &x, &y, 6, figure_id)) {
            f->action_state = FIGURE_ACTION_HERD_ALERT;
            f->flee_ticks = 0;
            return 1;
        }
    }
    return 0;
}

static void go_to_water(int32_t figure_id, const int32_t *offsets, const animal_offset *steps)
{
    figure *f = figure_get(figure_id);
    random_generate_next();
    int32_t dir = random_generator_state.random1_15bit % 8;
    int32_t tries;
    for (tries = 0; tries < 8; tries++) {
        uint32_t terrain = map_grids.terrain[offsets[dir] + f->grid_offset];
        if ((terrain & TERRAIN_WATER) && !(terrain & TERRAIN_DIKE)) {
            f->destination_x = (int16_t) (steps[dir].x + f->x);
            f->destination_y = (int16_t) (steps[dir].y + f->y);
            break;
        }
        if (++dir > 7) {
            dir = 0;
        }
    }
    if (tries == 8) {
        int32_t x, y;
        if (animal_find_nearest_figure_of_types(f->x, f->y, 0, 0, 4, &x, &y, 5, figure_id)) {
            f->destination_x = (int16_t) x;
            f->destination_y = (int16_t) y;
        } else {
            f->destination_x = f->x;
            f->destination_y = f->y;
        }
    }
}

void hippo_start_action(int32_t figure_id, int32_t action_state)
{
    figure *f = figure_get(figure_id);
    f->action_state = (uint8_t) action_state;
    if (action_state == FIGURE_ACTION_PREDATOR_REST) {
        random_generate_next();
        f->wait_ticks = random_rest_ticks(ANIMAL_RULE_PREDATOR_REST_BASE_TICKS, ANIMAL_RULE_PREDATOR_REST_RANDOM_TICKS);
        f->animal.frame_count = 12;
        f->animal.frame = 0;
    } else if (action_state == FIGURE_ACTION_HIPPO_GO_TO_WATER) {
        f->animal.frame_count = 12;
        go_to_water(figure_id, hippo_water_grid_offsets, hippo_water_steps);
        f->animal.frame = 0;
    } else if (action_state == FIGURE_ACTION_HIPPO_ANIMATE) {
        f->animal.frame_count = 12;
        f->wait_ticks = animal_rules[ANIMAL_RULE_HIPPO_ANIMATE_TICKS];
        f->animal.frame = 0;
    } else {
        predator_hunt_or_choose_action(figure_id);
        f->animal.frame_count = 7;
        f->wait_ticks = animal_rules[ANIMAL_RULE_HIPPO_AFTER_HUNT_TICKS];
        f->animal.frame = 0;
    }
}

static int32_t weighted_choice(const int32_t *weights, int32_t count)
{
    random_generate_next();
    int32_t sum = 0;
    int32_t i;
    for (i = 0; i < count; i++) {
        sum += weights[i];
        if (random_generator_state.random1_15bit % 100 < sum) {
            break;
        }
    }
    return i;
}

void hippo_choose_next_action(int32_t figure_id)
{
    const int32_t *weights;
    switch (figure_get(figure_id)->action_state) {
        case FIGURE_ACTION_PREDATOR_CHOOSE:
        case FIGURE_ACTION_HIPPO_CHARGE:
            weights = animal_choices[ANIMAL_CHOICE_HIPPO_IDLE].weights;
            break;
        case FIGURE_ACTION_HIPPO_GO_TO_WATER:
            weights = animal_choices[ANIMAL_CHOICE_HIPPO_IN_WATER].weights;
            break;
        case FIGURE_ACTION_HIPPO_ANIMATE:
            weights = animal_choices[ANIMAL_CHOICE_HIPPO_ANIMATING].weights;
            break;
        default:
            weights = animal_choices[ANIMAL_CHOICE_HIPPO_OTHER].weights;
            break;
    }
    int32_t i = weighted_choice(weights, ANIMAL_CHOICE_ACTIONS);
    hippo_start_action(figure_id, hippo_actions[i]);
}

void predator_hunt_or_choose_action(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (!figure_is_predator_animal(f->type) || !f->state || f->action_state == FIGURE_ACTION_PREDATOR_DYING || f->action_state == FIGURE_ACTION_CORPSE) {
        return;
    }
    int32_t x, y;
    int32_t prey_id = animal_search_rings_for(f->x, f->y, &x, &y, scenario_predator()->hunt_radius << 1, figure_id, predator_prey_on_tile_at);
    if (prey_id) {
        figure *prey = figure_get(prey_id);
        f->destination_x = prey->x;
        f->destination_y = prey->y;
        f->prey_figure_id = (int16_t) prey_id;
        return;
    }
    hippo_choose_next_action(figure_id);
}

void crocodile_start_action(int32_t figure_id, int32_t action_state)
{
    figure *f = figure_get(figure_id);
    f->action_state = (uint8_t) action_state;
    if (action_state != FIGURE_ACTION_CROCODILE_GO_TO_WATER) {
        random_generate_next();
        f->wait_ticks = random_rest_ticks(ANIMAL_RULE_PREDATOR_REST_BASE_TICKS, ANIMAL_RULE_PREDATOR_REST_RANDOM_TICKS);
        f->animal.frame_count = 12;
        f->animal.frame = 0;
        return;
    }
    f->animal.frame_count = 11;
    go_to_water(figure_id, crocodile_water_grid_offsets, crocodile_water_steps);
    f->animal.frame = 0;
}

void crocodile_choose_next_action(int32_t figure_id)
{
    int32_t action = figure_get(figure_id)->action_state;
    int32_t kind = (action == FIGURE_ACTION_PREDATOR_CHOOSE) ? ANIMAL_CHOICE_CROCODILE_IDLE
        : (action == FIGURE_ACTION_CROCODILE_GO_TO_WATER)    ? ANIMAL_CHOICE_CROCODILE_IN_WATER
                                                             : ANIMAL_CHOICE_CROCODILE_OTHER;
    int32_t i = weighted_choice(animal_choices[kind].weights, ANIMAL_CHOICE_CROCODILE_ACTIONS);
    crocodile_start_action(figure_id, crocodile_actions[i]);
}

void figure_seagulls_action(void)
{
    figure *f = figure_get(figure_current_id);
    f->terrain_usage = TERRAIN_USAGE_ANY;
    f->is_ghost = 1;
    f->use_cross_country = 1;
}

int32_t hunter_find_prey_target(int32_t x, int32_t y, int32_t max_path_length)
{
    int32_t grid_offsets[HUNTER_PREY_CANDIDATES];
    int32_t ids[HUNTER_PREY_CANDIDATES];
    int32_t count = 0;
    figure *hunter = figure_get(figure_current_id);
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_ALIVE && figure_is_herd_animal(f->type) && f->action_state != FIGURE_ACTION_CORPSE &&
            f->action_state != FIGURE_ACTION_HERD_HUNTED && (!f->animal.hunter_building_id || f->animal.hunter_building_id == hunter->building_id)) {
            ids[count] = id;
            grid_offsets[count] = map_info.grid_start + f->x + f->y * GRID_SIZE;
            if (++count == HUNTER_PREY_CANDIDATES) {
                break;
            }
        }
    }
    if (count) {
        int32_t index = map_routing_citizen_find_first_of_prefer_roads(x, y, grid_offsets, count);
        if (index && hunter->routing_path_length <= max_path_length) {
            figure_route_remove(figure_current_id);
            return ids[index - 1];
        }
    }
    return 0;
}

static void hunter_spear_set_image(figure *f)
{
    int32_t dir = f->direction - map_info.orientation * 4;
    if (dir < 0) {
        dir += 32;
    }
    figure_image_direction = dir;
    if (scenario_data.climate_type == CLIMATE_CENTRAL) {
        f->image_id = f->animation_frame + image_group(IMAGE_GROUP_FIGURE_HUNTER_SPEAR_CENTRAL);
        f->cart_image_id = 0;
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_HUNTER_SPEAR) + dir;
    f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_HUNTER_SPEAR_SHADOW) + dir;
}

void figure_hunter_spear_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->use_cross_country = 1;
    if (++f->progress_on_tile > HUNTER_SPEAR_FLIGHT_TICKS) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (++f->animation_frame > 4) {
        f->animation_frame = 0;
    }
    int32_t landed = figure_movement_move_ticks_cross_country(figure_id, 3);
    int32_t prey_id = map_figure_of_type_at(figure_id, FIGURE_ANTELOPE);
    if (!prey_id) {
        prey_id = map_figure_of_type_at(figure_id, FIGURE_BIRDS);
        if (!prey_id) {
            prey_id = map_figure_of_type_at(figure_id, FIGURE_OSTRICH);
        }
    }
    if (prey_id) {
        figure *prey = figure_get(prey_id);
        if (prey->action_state != FIGURE_ACTION_HERD_HUNTED && prey->action_state != FIGURE_ACTION_CORPSE) {
            prey->action_state = FIGURE_ACTION_HERD_HUNTED;
            prey->wait_ticks = 0;
            figure *hunter = figure_get(f->building_id);
            hunter->prey_figure_id = (int16_t) prey_id;
            hunter->action_state = FIGURE_ACTION_HUNTER_GO_TO_KILL;
            hunter->destination_x = prey->x;
            hunter->destination_y = prey->y;
            f->state = FIGURE_STATE_DEAD;
            hunter_spear_set_image(f);
            return;
        }
    }
    if (landed) {
        figure_get(f->building_id)->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
        f->state = FIGURE_STATE_DEAD;
    }
    hunter_spear_set_image(f);
}

static int32_t distance_to_prey(const figure *f, const figure *prey)
{
    return calc_maximum_distance(f->x, f->y, prey->x, prey->y);
}

static void walk_home(figure *f)
{
    const building *home = building_get(f->building_id);
    f->destination_x = home->road_access_x;
    f->destination_y = home->road_access_y;
}

static void hunter_target(figure *f, int32_t prey_id, int32_t set_action)
{
    figure *prey = figure_get(prey_id);
    f->destination_x = prey->x;
    f->destination_y = prey->y;
    f->prey_figure_id = (int16_t) prey_id;
    figure_route_remove(figure_current_id);
    if (prey->action_state != FIGURE_ACTION_HERD_HUNTED) {
        prey->animal.hunter_building_id = f->building_id;
        if (set_action) {
            f->action_state = FIGURE_ACTION_HUNTER_GO_TO_PREY;
        }
    }
}

void figure_hunter_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t shoot_frame = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    int32_t speed = figure_properties[f->type].speed;
    if (building_get(f->building_id)->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    int32_t wrapped = (uint8_t) (f->animation_frame + 1) > 11;
    f->animation_frame++;
    if (wrapped) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_HUNTER_CREATED:
            f->is_ghost = 1;
            if (--f->wait_ticks < 1) {
                f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
                f->animation_frame = 0;
            }
            break;
        case FIGURE_ACTION_HUNTER_LOOK_FOR_PREY:
            f->wait_ticks--;
            f->direction = f->previous_tile_direction;
            if (f->wait_ticks < 1) {
                int32_t prey_id = hunter_find_prey_target(f->x, f->y, animal_rules[ANIMAL_RULE_HUNTER_SEARCH_PATH_LENGTH]);
                if (!prey_id) {
                    f->action_state = FIGURE_ACTION_HUNTER_RETURN_HOME;
                    walk_home(f);
                    if (f->destination_x == f->x && f->destination_y == f->y) {
                        f->is_ghost = 1;
                        f->state = FIGURE_STATE_DEAD;
                        return;
                    }
                    figure_route_remove(figure_id);
                } else {
                    hunter_target(f, prey_id, 1);
                }
            }
            break;
        case FIGURE_ACTION_HUNTER_GO_TO_PREY: {
            figure_movement_move_ticks(figure_id, speed);
            int32_t dir = f->direction;
            if (dir == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
                f->wait_ticks = animal_rules[ANIMAL_RULE_HUNTER_RETRY_TICKS];
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (wrapped || dir == DIRECTION_AT_DESTINATION) {
                int32_t prey_id = hunter_find_prey_target(f->x, f->y, animal_rules[ANIMAL_RULE_HUNTER_NEAR_SEARCH_PATH_LENGTH]);
                if (!prey_id) {
                    if (f->direction == DIRECTION_AT_DESTINATION) {
                        f->action_state = FIGURE_ACTION_HUNTER_RETURN_HOME;
                        walk_home(f);
                    }
                } else {
                    figure_route_remove(figure_id);
                    figure *prey = figure_get(prey_id);
                    f->destination_x = prey->x;
                    f->destination_y = prey->y;
                    if (prey->action_state != FIGURE_ACTION_HERD_HUNTED) {
                        prey->animal.hunter_building_id = f->building_id;
                        if (distance_to_prey(f, prey) < animal_rules[ANIMAL_RULE_HUNTER_SHOOT_DISTANCE]) {
                            f->destination_x = prey->x;
                            f->destination_y = prey->y;
                            f->direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, prey->x, prey->y);
                            f->action_state = FIGURE_ACTION_HUNTER_SHOOT;
                            f->wait_ticks = 0;
                        }
                    }
                }
            }
            break;
        }
        case FIGURE_ACTION_HUNTER_SHOOT:
            shoot_frame = f->wait_ticks;
            if (f->wait_ticks == HUNTER_SHOT_TICK) {
                if (scenario_data.climate_type == CLIMATE_CENTRAL) {
                    sound_figure_play(figure_id, ANIMAL_SOUND_HUNTER_SHOT_CENTRAL, ANIMAL_SOUND_VARIANT);
                }
                if (scenario_data.climate_type == CLIMATE_NORTHERN) {
                    sound_figure_play(figure_id, ANIMAL_SOUND_HUNTER_SHOT_NORTHERN, ANIMAL_SOUND_VARIANT);
                }
                if (scenario_data.climate_type == CLIMATE_DESERT) {
                    sound_figure_play(figure_id, ANIMAL_SOUND_HUNTER_SHOT_DESERT, ANIMAL_SOUND_VARIANT);
                }
                figure_create_missile(figure_id, f->x, f->y, f->destination_x, f->destination_y, FIGURE_HUNTER_SPEAR);
            }
            if (shoot_frame > 11) {
                shoot_frame = 0;
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_HUNTER_SHOOT_TIMEOUT_TICKS]) {
                f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
            }
            break;
        case FIGURE_ACTION_HUNTER_GO_TO_KILL: {
            figure_movement_move_ticks(figure_id, speed);
            int32_t dir = f->direction;
            if (dir == DIRECTION_AT_DESTINATION) {
                int32_t prey_id = f->prey_figure_id;
                figure *prey = figure_get(prey_id);
                if (!prey_id || prey->grid_offset != f->grid_offset || !prey->state || prey->action_state != FIGURE_ACTION_HERD_HUNTED) {
                    f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
                    f->wait_ticks = animal_rules[ANIMAL_RULE_HUNTER_RETRY_TICKS];
                } else {
                    prey->state = FIGURE_STATE_DEAD;
                    f->action_state = FIGURE_ACTION_HUNTER_TAKE_PREY;
                    f->wait_ticks = 0;
                }
            } else if (dir == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
                f->wait_ticks = 0;
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_HUNTER_TAKE_PREY:
            if (++f->wait_ticks > ((scenario_data.climate_type != CLIMATE_CENTRAL) ? animal_rules[ANIMAL_RULE_HUNTER_TAKE_PREY_TICKS] : 0)) {
                walk_home(f);
                f->action_state = FIGURE_ACTION_HUNTER_RETURN_WITH_PREY;
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_HUNTER_RETURN_WITH_PREY: {
            figure_movement_move_ticks(figure_id, speed);
            int32_t dir = f->direction;
            if (dir == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
                building_get(f->building_id)->loads_stored += animal_rules[ANIMAL_RULE_HUNTER_GAME_LOAD];
            } else if (dir == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_HUNTER_RETURN_HOME: {
            figure_movement_move_ticks(figure_id, speed);
            if (wrapped) {
                int32_t prey_id = hunter_find_prey_target(f->x, f->y, animal_rules[ANIMAL_RULE_HUNTER_HOMEWARD_SEARCH_PATH_LENGTH]);
                if (prey_id) {
                    hunter_target(f, prey_id, 1);
                }
            }
            int32_t dir = f->direction;
            if (dir != DIRECTION_AT_DESTINATION) {
                if (dir == DIRECTION_REROUTE) {
                    figure_route_remove(figure_id);
                } else if (dir == DIRECTION_LOST) {
                    f->state = FIGURE_STATE_DEAD;
                }
                break;
            }
            figure_route_remove(figure_id);
            f->state = FIGURE_STATE_DEAD;
            f->wait_ticks = 0;
            break;
        }
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_FERRY_WAIT_TICKS]) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
                }
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->is_ghost = 1;
            break;
        default:
            f->action_state = FIGURE_ACTION_HUNTER_LOOK_FOR_PREY;
            break;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    int32_t walk, corpse, attack, attack_hunt, shoot, take, carry, carry_still;
    switch (scenario_data.climate_type) {
        case CLIMATE_CENTRAL:
            walk = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL;
            corpse = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL_DEATH;
            attack = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL_ATTACK;
            attack_hunt = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL_ATTACK;
            shoot = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL_ATTACK;
            take = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL;
            carry = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL;
            carry_still = IMAGE_GROUP_FIGURE_HUNTER_CENTRAL;
            break;
        case CLIMATE_NORTHERN:
            walk = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN;
            corpse = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_DEATH;
            shoot = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_SHOOT;
            attack = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_ATTACK;
            attack_hunt = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_ATTACK_CARRYING;
            take = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_TAKE;
            carry = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_CARRY;
            carry_still = IMAGE_GROUP_FIGURE_HUNTER_NORTHERN_CARRY;
            break;
        default:
            walk = IMAGE_GROUP_FIGURE_HUNTER_DESERT;
            corpse = IMAGE_GROUP_FIGURE_HUNTER_DESERT_DEATH;
            shoot = IMAGE_GROUP_FIGURE_HUNTER_DESERT_SHOOT;
            attack = IMAGE_GROUP_FIGURE_HUNTER_DESERT_ATTACK;
            attack_hunt = IMAGE_GROUP_FIGURE_HUNTER_DESERT_ATTACK_CARRYING;
            take = IMAGE_GROUP_FIGURE_HUNTER_DESERT_TAKE;
            carry = IMAGE_GROUP_FIGURE_HUNTER_DESERT_CARRY;
            carry_still = IMAGE_GROUP_FIGURE_HUNTER_DESERT_CARRY;
            break;
    }
    int32_t dir = figure_image_direction;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(corpse);
            return;
        case FIGURE_ACTION_ATTACK:
            if (f->action_state_before_attack != FIGURE_ACTION_HUNTER_RETURN_WITH_PREY) {
                f->image_id = image_group(attack) + f->animation_frame * 8 + dir;
            } else {
                f->image_id = image_group(attack_hunt) + f->animation_frame * 8 + dir;
            }
            return;
        case FIGURE_ACTION_ON_FERRY:
            return;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            if (f->action_state_before_attack == FIGURE_ACTION_HUNTER_RETURN_WITH_PREY) {
                f->image_id = image_group(carry_still) + dir;
            } else {
                f->image_id = image_group(walk) + dir;
            }
            return;
        case FIGURE_ACTION_HUNTER_TAKE_PREY:
            f->image_id = image_group(take) + f->wait_ticks * 8 + dir;
            return;
        case FIGURE_ACTION_HUNTER_RETURN_WITH_PREY:
            f->image_id = image_group(carry) + f->animation_frame * 8 + dir;
            return;
        case FIGURE_ACTION_HUNTER_CREATED:
            f->image_id = 0;
            return;
        case FIGURE_ACTION_HUNTER_SHOOT:
            f->image_id = image_group(shoot) + shoot_frame * 8 + dir;
            return;
        default:
            f->image_id = image_group(walk) + f->animation_frame * 8 + dir;
            return;
    }
}

static int32_t gatherer_can_cut_here(const figure *f)
{
    int32_t on_tile = map_grids.figure_ids[f->grid_offset];
    return map_grids.vegetation_growth[f->grid_offset] == VEGETATION_FULLY_GROWN &&
        (!on_tile || figure_get(on_tile)->action_state != FIGURE_ACTION_LUMBERJACK_CUT_TREE);
}

void figure_lumberjack_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    f->is_ghost = 0;
    int32_t home_id = f->building_id;
    int32_t speed = figure_properties[f->type].speed;
    if (building_get(home_id)->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    if (++f->animation_frame > 11) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LUMBERJACK) + figure_image_direction;
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_FERRY_WAIT_TICKS]) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_LUMBERJACK_FIND_TREE;
                }
            }
            break;
        case FIGURE_ACTION_LUMBERJACK_FIND_TREE:
            if (--f->wait_ticks < 1) {
                if (map_vegetation_find_reachable_all(f->x, f->y, TERRAIN_TREE)) {
                    f->action_state = FIGURE_ACTION_LUMBERJACK_GO_TO_TREE;
                    f->destination_x = map_result_x;
                    f->destination_y = map_result_y;
                    f->animation_frame = 0;
                    break;
                }
                f->state = FIGURE_STATE_DEAD;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_LUMBERJACK_GO_TO_TREE:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = gatherer_can_cut_here(f) ? FIGURE_ACTION_LUMBERJACK_CUT_TREE : FIGURE_ACTION_LUMBERJACK_FIND_TREE;
                f->wait_ticks = 0;
                f->direction = f->previous_tile_direction;
            } else if (f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_CORPSE;
                f->wait_ticks = 0;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_LUMBERJACK_CUT_TREE:
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_GATHERER_CUT_TICKS]) {
                int32_t grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
                map_grids.vegetation_growth[grid_offset] = 0;
                map_tiles_set_vegetation_image(grid_offset);
                f->action_state = FIGURE_ACTION_LUMBERJACK_RETURN_HOME;
                walk_home(f);
            }
            break;
        case FIGURE_ACTION_LUMBERJACK_RETURN_HOME:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
                f->direction = f->previous_tile_direction;
                building_get(f->building_id)->loads_stored += animal_rules[ANIMAL_RULE_LUMBERJACK_WOOD_LOAD];
            } else if (f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_CORPSE;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_LUMBERJACK_FIND_TREE;
            f->wait_ticks = 10;
            break;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_LUMBERJACK_DEATH);
            break;
        case FIGURE_ACTION_LUMBERJACK_CUT_TREE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LUMBERJACK_CUT) + f->animation_frame * 8 + figure_image_direction;
            break;
        case FIGURE_ACTION_LUMBERJACK_FIND_TREE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LUMBERJACK) + figure_image_direction;
            break;
        case FIGURE_ACTION_LUMBERJACK_RETURN_HOME:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LUMBERJACK_CARRY) + f->animation_frame * 8 + figure_image_direction;
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_LUMBERJACK) + f->animation_frame * 8 + figure_image_direction;
            break;
    }
}

void figure_reed_gatherer_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    f->is_ghost = 0;
    if (f->building_id && building_get(f->building_id)->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->is_ghost = 1;
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            f->image_id = image_group(IMAGE_GROUP_FIGURE_REED_GATHERER) + figure_image_direction;
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_FERRY_WAIT_TICKS]) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_REED_GATHERER_FIND_REEDS;
                }
            }
            break;
        case FIGURE_ACTION_REED_GATHERER_FIND_REEDS:
            if (++f->wait_ticks > 10) {
                if (map_sweep_countdown || !map_vegetation_find_reachable_all(f->x, f->y, TERRAIN_MARSH)) {
                    f->state = FIGURE_STATE_DEAD;
                    map_figure_delete(figure_id);
                    return;
                }
                f->action_state = FIGURE_ACTION_REED_GATHERER_GO_TO_REEDS;
                f->destination_x = map_result_x;
                f->destination_y = map_result_y;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_REED_GATHERER_GO_TO_REEDS:
            figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                if (gatherer_can_cut_here(f)) {
                    f->action_state = FIGURE_ACTION_REED_GATHERER_CUT_REEDS;
                    f->wait_ticks = 0;
                    f->direction = f->previous_tile_direction;
                } else {
                    f->action_state = FIGURE_ACTION_REED_GATHERER_FIND_REEDS;
                    f->wait_ticks = 0;
                }
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->direction = f->previous_tile_direction;
            }
            break;
        case FIGURE_ACTION_REED_GATHERER_CUT_REEDS:
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_GATHERER_CUT_TICKS]) {
                int32_t grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
                map_grids.vegetation_growth[grid_offset] = 0;
                map_tiles_set_vegetation_image(grid_offset);
                f->action_state = FIGURE_ACTION_REED_GATHERER_RETURN_HOME;
                walk_home(f);
            }
            break;
        case FIGURE_ACTION_REED_GATHERER_RETURN_HOME:
            figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
                building_get(f->building_id)->loads_stored += animal_rules[ANIMAL_RULE_REED_GATHERER_REEDS_LOAD];
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->direction = f->previous_tile_direction;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_REED_GATHERER_FIND_REEDS;
            break;
    }
    if (++f->animation_frame >= (f->action_state != FIGURE_ACTION_REED_GATHERER_CUT_REEDS ? 12 : 15)) {
        f->animation_frame = 0;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_REED_GATHERER_CUT_REEDS:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_REED_GATHERER_CUT) + f->animation_frame * 8 + figure_image_direction;
            break;
        case FIGURE_ACTION_REED_GATHERER_FIND_REEDS:
        case FIGURE_ACTION_ATTACK:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_REED_GATHERER) + figure_image_direction;
            break;
        case FIGURE_ACTION_REED_GATHERER_RETURN_HOME:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_REED_GATHERER_CARRY) + f->animation_frame * 8 + figure_image_direction;
            break;
        case FIGURE_ACTION_CORPSE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_REED_GATHERER_DEATH) + (f->wait_ticks < 8 ? f->wait_ticks : 7);
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_REED_GATHERER) + f->animation_frame * 8 + figure_image_direction;
            break;
    }
}

static void herd_animal_check_hunter(figure *f)
{
    const building *b = building_get(f->animal.hunter_building_id);
    if (b->type != BUILDING_HUNTING_LODGE || !b->data.industry.worker_count || b->state != BUILDING_STATE_IN_USE || !b->num_workers) {
        f->animal.hunter_building_id = 0;
    }
}

static void herd_animal_walk_near(int32_t center_x, int32_t center_y)
{
    figure *f = figure_get(figure_current_id);
    random_generate_next();
    int32_t dir = animal_find_free_neighbour_direction(center_x, center_y, random_generator_state.random1_15bit % 16, f->x, f->y, 1);
    if (dir < 0 || dir > 15) {
        f->wait_ticks = animal_rules[ANIMAL_RULE_NO_TILE_WAIT_TICKS];
        return;
    }
    f->destination_x = (int16_t) (herd_roaming_offsets()[dir].x + center_x);
    f->destination_y = (int16_t) (herd_roaming_offsets()[dir].y + center_y);
    f->action_state = FIGURE_ACTION_HERD_WALK;
}

void figure_birds_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->animal.frame++;
    int32_t speed = figure_properties[f->type].speed;
    herd_animal_check_hunter(f);
    if (f->animal.frame_count <= f->animal.frame) {
        f->animal.frame = 0;
        f->animation_frame++;
    }
    f->terrain_usage = TERRAIN_USAGE_ANIMAL;
    city_player_current()->animals++;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    const formation *herd = formation_get(f->formation_id);
    if (++f->animation_frame > 49) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        default:
            f->action_state = FIGURE_ACTION_HERD_CHOOSE;
            break;
        case FIGURE_ACTION_HERD_CHOOSE:
        case FIGURE_ACTION_HERD_REST:
            if (--f->wait_ticks < 1) {
                herd_animal_walk_near(herd->home_x, herd->home_y);
            }
            break;
        case FIGURE_ACTION_HERD_MOVE:
        case FIGURE_ACTION_HERD_WALK:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_REST;
                random_generate_next();
                f->wait_ticks = random_rest_ticks(ANIMAL_RULE_HERD_REST_BASE_TICKS, ANIMAL_RULE_HERD_REST_RANDOM_TICKS);
            }
            break;
        case FIGURE_ACTION_HERD_HUNTED:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_DYING_TICKS]) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_HERD_ALERT:
        case FIGURE_ACTION_HERD_ALERTED_BY_MATE:
            herd_animal_react_to_threats(figure_id);
            break;
        case FIGURE_ACTION_HERD_FLEE:
        case FIGURE_ACTION_HERD_STAMPEDE:
            figure_movement_move_ticks(figure_id, speed * 2);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_ALERT;
            }
            break;
        case FIGURE_ACTION_HERD_SPAWNED:
            if (--f->wait_ticks < 1) {
                f->animal.frame = 0;
                f->animal.frame_count = 8;
                f->action_state = FIGURE_ACTION_HERD_CHOOSE;
            }
            f->image_id = 0;
            return;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE || f->action_state == FIGURE_ACTION_HERD_HUNTED) {
        if (f->wait_ticks < 25) {
            f->image_id = f->wait_ticks + image_group(IMAGE_GROUP_FIGURE_BIRDS_DEATH);
        } else {
            f->image_id = 0;
        }
        return;
    }
    f->image_id = (f->animation_frame >> 1) + image_group(IMAGE_GROUP_FIGURE_BIRDS);
}

static int32_t animal_image_direction(const figure *f)
{
    int32_t dir = f->direction - map_info.orientation;
    figure_image_direction = dir;
    if (dir < 0) {
        dir += 8;
        figure_image_direction = dir;
        if (dir < 0) {
            dir += ((uint32_t) (7 - dir) >> 3) * 8;
            figure_image_direction = dir;
        }
    }
    if (dir >= 8) {
        dir -= ((uint32_t) dir >> 3) * 8;
        figure_image_direction = dir;
    }
    return dir;
}

static int32_t herd_animal_begin(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ANIMAL;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    city_player_current()->animals++;
    f->animal.frame++;
    herd_animal_check_hunter(f);
    if (f->animal.frame_count <= f->animal.frame) {
        f->animal.frame = 0;
        f->animation_frame++;
        return 1;
    }
    return 0;
}

static void herd_animal_rest(figure *f)
{
    random_generate_next();
    f->action_state = (random_generator_state.random1_15bit & 1) ? FIGURE_ACTION_HERD_REST : FIGURE_ACTION_HERD_GRAZE;
    random_generate_next();
    f->wait_ticks = random_rest_ticks(ANIMAL_RULE_HERD_REST_BASE_TICKS, ANIMAL_RULE_HERD_REST_RANDOM_TICKS);
}

void figure_ostrich_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    const formation *herd = formation_get(f->formation_id);
    int32_t frame_advanced = herd_animal_begin(f);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->animal.frame_count = 8;
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            f->animal.frame_count = 7;
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_HERD_CHOOSE:
            random_generate_next();
            f->action_state = (random_generator_state.random1_15bit & 1) ? FIGURE_ACTION_HERD_REST : FIGURE_ACTION_HERD_GRAZE;
            f->animal.frame = 0;
            f->animal.frame_count = 8;
            random_generate_next();
            f->wait_ticks = random_rest_ticks(ANIMAL_RULE_HERD_REST_BASE_TICKS, ANIMAL_RULE_HERD_REST_RANDOM_TICKS);
            return;
        case FIGURE_ACTION_HERD_MOVE:
        case FIGURE_ACTION_HERD_WALK:
            f->animal.frame_count = 12;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->direction = f->previous_tile_direction;
            } else if (f->direction == DIRECTION_LOST) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_CHOOSE;
                f->wait_ticks = (int16_t) (f->index_in_formation * 5);
                f->animal.frame = 0;
                return;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                herd_animal_rest(f);
            }
            if (frame_advanced) {
                herd_animal_react_to_threats(figure_id);
            }
            break;
        case FIGURE_ACTION_HERD_HUNTED:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_DYING_TICKS]) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_HERD_ALERT:
        case FIGURE_ACTION_HERD_ALERTED_BY_MATE:
            f->animal.frame_count = 1;
            herd_animal_react_to_threats(figure_id);
            break;
        case FIGURE_ACTION_HERD_FLEE:
        case FIGURE_ACTION_HERD_STAMPEDE:
            f->animal.frame_count = 12;
            if (++f->animal.frame >= 12) {
                f->animal.frame = 0;
            }
            figure_movement_move_ticks(figure_id, speed * 2);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_ALERT;
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
                return;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_ALERT;
                f->animal.frame = 0;
                return;
            }
            break;
        case FIGURE_ACTION_HERD_REST:
            f->animal.frame_count = 8;
            if (--f->wait_ticks < 0) {
                f->wait_ticks = 0;
            }
            if (f->wait_ticks > 0) {
                if (frame_advanced) {
                    herd_animal_react_to_threats(figure_id);
                }
                break;
            }
            if (!frame_advanced) {
                break;
            }
            random_generate_next();
            {
                int32_t dir = animal_find_free_neighbour_direction(
                    herd->destination_x, herd->destination_y, random_generator_state.random1_15bit % 16, f->x, f->y, 1);
                if (dir >= 0 && dir < HERD_OFFSET_COUNT) {
                    f->destination_x = (int16_t) (herd_roaming_offsets()[dir].x + herd->destination_x);
                    f->destination_y = (int16_t) (herd_roaming_offsets()[dir].y + herd->destination_y);
                    f->action_state = FIGURE_ACTION_HERD_WALK;
                    return;
                }
                f->wait_ticks = animal_rules[ANIMAL_RULE_NO_TILE_WAIT_TICKS];
            }
            herd_animal_react_to_threats(figure_id);
            break;
        case FIGURE_ACTION_HERD_GRAZE:
            f->animal.frame_count = 8;
            if (--f->wait_ticks > 0) {
                if (frame_advanced) {
                    herd_animal_react_to_threats(figure_id);
                }
                break;
            }
            if (!frame_advanced) {
                break;
            }
            f->action_state = FIGURE_ACTION_HERD_REST;
            return;
        case FIGURE_ACTION_HERD_SPAWNED:
            if (--f->wait_ticks <= 0) {
                f->animal.frame = 0;
                f->animal.frame_count = 8;
                f->action_state = FIGURE_ACTION_HERD_CHOOSE;
            }
            f->image_id = 0;
            return;
        default:
            f->action_state = FIGURE_ACTION_HERD_CHOOSE;
            break;
    }
    int32_t dir = animal_image_direction(f);
    if (f->animal.frame > f->animal.frame_count) {
        f->animal.frame -= f->animal.frame_count;
    }
    if (f->animal.frame < 0) {
        f->animal.frame = 0;
    }
    int32_t frame = f->animal.frame;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH_DEATH) + 7;
            break;
        case FIGURE_ACTION_ATTACK:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH_ATTACK) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HERD_CHOOSE:
        case FIGURE_ACTION_HERD_REST:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH_REST) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HERD_HUNTED:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH_DEATH) + (f->wait_ticks < 8 ? f->wait_ticks : 7);
            break;
        case FIGURE_ACTION_HERD_ALERT:
        case FIGURE_ACTION_HERD_ALERTED_BY_MATE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH_GRAZE) + dir;
            break;
        case FIGURE_ACTION_HERD_GRAZE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH_GRAZE) + frame * 8 + dir;
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_OSTRICH) + frame * 8 + dir;
            break;
    }
}

void figure_antelope_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    const formation *herd = formation_get(f->formation_id);
    int32_t frame_advanced = herd_animal_begin(f);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->animal.frame_count = 8;
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            f->animal.frame_count = 7;
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_HERD_CHOOSE:
            random_generate_next();
            f->action_state = (random_generator_state.random1_15bit & 1) ? FIGURE_ACTION_HERD_REST : FIGURE_ACTION_HERD_GRAZE;
            f->animal.frame = 0;
            f->animal.frame_count = 8;
            random_generate_next();
            f->wait_ticks = random_rest_ticks(ANIMAL_RULE_HERD_REST_BASE_TICKS, ANIMAL_RULE_HERD_REST_RANDOM_TICKS);
            return;
        case FIGURE_ACTION_HERD_MOVE:
        case FIGURE_ACTION_HERD_WALK:
            f->animal.frame_count = 12;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_CHOOSE;
                f->wait_ticks = (int16_t) (f->index_in_formation * 5);
                f->animal.frame = 0;
                return;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                herd_animal_rest(f);
                break;
            }
            if (frame_advanced) {
                herd_animal_react_to_threats(figure_id);
            }
            break;
        case FIGURE_ACTION_HERD_HUNTED:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_DYING_TICKS]) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_HERD_ALERT:
        case FIGURE_ACTION_HERD_ALERTED_BY_MATE:
            f->animal.frame_count = 1;
            herd_animal_react_to_threats(figure_id);
            break;
        case FIGURE_ACTION_HERD_FLEE:
        case FIGURE_ACTION_HERD_STAMPEDE:
            f->animal.frame_count = 8;
            if (++f->animal.frame >= 12) {
                f->animal.frame = 0;
            }
            figure_movement_move_ticks(figure_id, speed * 2);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_HERD_ALERT;
                f->animal.frame = 0;
                return;
            }
            break;
        case FIGURE_ACTION_HERD_REST:
            f->animal.frame_count = 8;
            if (--f->wait_ticks < 0) {
                f->wait_ticks = 0;
            }
            if (f->wait_ticks > 0) {
                if (frame_advanced) {
                    herd_animal_react_to_threats(figure_id);
                }
                break;
            }
            if (!frame_advanced) {
                break;
            }
            random_generate_next();
            {
                int32_t dir = animal_find_free_neighbour_direction(
                    herd->destination_x, herd->destination_y, random_generator_state.random1_15bit % 16, f->x, f->y, 1);
                if (dir >= 0 && dir < HERD_OFFSET_COUNT) {
                    f->destination_x = (int16_t) (herd_roaming_offsets()[dir].x + herd->destination_x);
                    f->destination_y = (int16_t) (herd_roaming_offsets()[dir].y + herd->destination_y);
                    f->action_state = FIGURE_ACTION_HERD_WALK;
                    return;
                }
                f->wait_ticks = animal_rules[ANIMAL_RULE_NO_TILE_WAIT_TICKS];
            }
            herd_animal_react_to_threats(figure_id);
            break;
        case FIGURE_ACTION_HERD_GRAZE:
            f->animal.frame_count = 8;
            if (--f->wait_ticks > 0) {
                if (frame_advanced) {
                    herd_animal_react_to_threats(figure_id);
                }
                break;
            }
            if (!frame_advanced) {
                break;
            }
            f->action_state = FIGURE_ACTION_HERD_REST;
            return;
        case FIGURE_ACTION_HERD_SPAWNED:
            if (--f->wait_ticks <= 0) {
                f->animal.frame = 0;
                f->animal.frame_count = 8;
                f->action_state = FIGURE_ACTION_HERD_CHOOSE;
            }
            f->image_id = 0;
            f->is_ghost = 1;
            return;
        default:
            f->action_state = FIGURE_ACTION_HERD_CHOOSE;
            break;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    int32_t dir = figure_image_direction;
    int32_t frame = f->animal.frame;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE_DEATH) + 7;
            break;
        case FIGURE_ACTION_ATTACK:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE_ATTACK) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HERD_CHOOSE:
        case FIGURE_ACTION_HERD_ALERT:
        case FIGURE_ACTION_HERD_ALERTED_BY_MATE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE) + dir;
            break;
        case FIGURE_ACTION_HERD_HUNTED:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE_DEATH) + (f->wait_ticks < 8 ? f->wait_ticks : 7);
            break;
        case FIGURE_ACTION_HERD_FLEE:
        case FIGURE_ACTION_HERD_STAMPEDE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE_FLEE) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HERD_REST:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE_REST) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HERD_GRAZE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE_GRAZE) + frame * 8 + dir;
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ANTELOPE) + frame * 8 + dir;
            break;
    }
}

static int32_t animal_in_deep_water(const figure *f)
{
    static const int32_t offsets[9] = { 0, -GRID_SIZE, 1, GRID_SIZE, -1, -GRID_SIZE + 1, -GRID_SIZE - 1, GRID_SIZE + 1, GRID_SIZE - 1 };
    for (int32_t i = 0; i < 9; i++) {
        if (!(map_grids.terrain[f->grid_offset + offsets[i]] & TERRAIN_WATER)) {
            return 0;
        }
    }
    return 1;
}

static int32_t predator_walk_near(int32_t center_x, int32_t center_y, int32_t random_dir)
{
    figure *f = figure_get(figure_current_id);
    int32_t dir = animal_find_free_neighbour_direction(center_x, center_y, random_dir, f->x, f->y, 0);
    if (dir < 0 || dir >= HERD_OFFSET_COUNT) {
        return 0;
    }
    figure_route_remove(figure_current_id);
    f->destination_x = (int16_t) (herd_spawn_offsets()[dir].x + center_x);
    f->destination_y = (int16_t) (herd_spawn_offsets()[dir].y + center_y);
    f->action_state = FIGURE_ACTION_PREDATOR_WALK;
    return 1;
}

static void predator_end_chase(figure *f)
{
    f->direction = f->previous_tile_direction;
    f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
    f->wait_ticks = (int16_t) (f->index_in_formation * 5);
}

void figure_crocodile_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    f->animal.frame++;
    int32_t in_water = animal_in_deep_water(f);
    if (in_water) {
        f->animal.frame_count = 11;
    }
    int32_t frame_advanced = 0;
    if (f->animal.frame >= f->animal.frame_count) {
        f->animal.frame = 0;
        f->animation_frame++;
        frame_advanced = 1;
    }
    f->terrain_usage = TERRAIN_USAGE_AMPHIBIA;
    city_player_current()->animals++;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    const formation *herd = formation_get(f->formation_id);
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    if (in_water) {
        speed >>= 1;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->animal.frame_count = 8;
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            sound_figure_play(figure_id, ANIMAL_SOUND_CROCODILE_ATTACK, ANIMAL_SOUND_VARIANT);
            f->animal.frame_count = 7;
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHOOSE:
            crocodile_choose_next_action(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
        case FIGURE_ACTION_CROCODILE_GO_TO_WATER:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                crocodile_choose_next_action(figure_id);
            }
            break;
        case FIGURE_ACTION_PREDATOR_DYING:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_DYING_TICKS]) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_PREDATOR_PROWL:
            figure_movement_move_ticks(figure_id, speed);
            predator_find_prey_nearby(figure_id);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                predator_end_chase(f);
            }
            break;
        case FIGURE_ACTION_PREDATOR_REST:
            if (--f->wait_ticks <= 0) {
                random_generate_next();
                if (!predator_walk_near(herd->destination_x, herd->destination_y, random_generator_state.random1_15bit % 16)) {
                    f->wait_ticks = animal_rules[ANIMAL_RULE_NO_TILE_WAIT_TICKS];
                }
            }
            if (frame_advanced) {
                predator_find_prey_nearby(figure_id);
            }
            break;
        case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
            predator_find_prey_nearby(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHASE: {
            int32_t prey_id = f->prey_figure_id;
            if (--f->wait_ticks <= 0) {
                f->hunt_ticks = 0;
                f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
                break;
            }
            figure *prey = figure_get(prey_id);
            if (prey_id && (prey->x != f->destination_x || prey->y != f->destination_y)) {
                figure_route_remove(figure_id);
                f->destination_x = prey->x;
                f->destination_y = prey->y;
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                predator_end_chase(f);
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                if (prey_id && (prey->x != f->destination_x || prey->y != f->destination_y)) {
                    figure_route_remove(figure_id);
                    f->destination_x = prey->x;
                    f->destination_y = prey->y;
                } else {
                    predator_end_chase(f);
                }
            }
            break;
        }
        case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
            random_generate_next();
            if (predator_walk_near(herd->destination_x, herd->destination_y, random_generator_state.random1_15bit % 16)) {
                break;
            }
            random_generate_next();
            if (predator_walk_near(herd->home_x, herd->home_y, random_generator_state.random1_15bit & 15)) {
                break;
            }
            f->state = FIGURE_STATE_DEAD;
            return;
        case FIGURE_ACTION_PREDATOR_SPAWNED:
            if (--f->wait_ticks <= 0) {
                f->animal.frame = 0;
                f->animal.frame_count = 8;
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            }
            f->image_id = 0;
            return;
        default:
            f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            break;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    int32_t dir = figure_image_direction;
    int32_t frame = f->animal.frame;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            if (in_water) {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_DEATH_IN_WATER) + ((f->wait_ticks < 23) ? f->wait_ticks : 22);
            } else {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE_DEATH) + (f->wait_ticks < 8 ? f->wait_ticks : 7);
            }
            break;
        case FIGURE_ACTION_ATTACK:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE_ATTACK) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_PREDATOR_CHOOSE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE) + dir;
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
        case FIGURE_ACTION_CROCODILE_GO_TO_WATER:
        case FIGURE_ACTION_PREDATOR_CHASE:
            f->image_id = image_group(in_water ? IMAGE_GROUP_FIGURE_CROCODILE_SWIM : IMAGE_GROUP_FIGURE_CROCODILE) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE) + frame * 8 + dir;
            break;
        default:
            if (in_water) {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE_IN_WATER) + frame * 8 + dir;
            } else {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE) + dir;
            }
            break;
    }
}

void figure_hippo_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    int32_t frame_advanced = 0;
    if (++f->animal.frame >= f->animal.frame_count) {
        f->animal.frame = 0;
        f->animation_frame++;
        frame_advanced = 1;
    }
    f->terrain_usage = TERRAIN_USAGE_AMPHIBIA;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    const formation *herd = formation_get(f->formation_id);
    int32_t in_water = animal_in_deep_water(f);
    city_player_current()->animals++;
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    int32_t half_speed = in_water ? speed >> 1 : speed;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            f->animal.frame_count = 8;
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            sound_figure_play(figure_id, ANIMAL_SOUND_HIPPO_ATTACK, ANIMAL_SOUND_VARIANT);
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHOOSE:
            hippo_choose_next_action(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
        case FIGURE_ACTION_HIPPO_GO_TO_WATER:
            figure_movement_move_ticks(figure_id, half_speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                hippo_choose_next_action(figure_id);
            }
            break;
        case FIGURE_ACTION_PREDATOR_DYING:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_DYING_TICKS]) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_PREDATOR_PROWL:
            figure_movement_move_ticks(figure_id, half_speed);
            predator_find_prey_nearby(figure_id);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                predator_end_chase(f);
            }
            break;
        case FIGURE_ACTION_PREDATOR_REST:
            if (--f->wait_ticks <= 0) {
                random_generate_next();
                if (!predator_walk_near(herd->destination_x, herd->destination_y, random_generator_state.random1_15bit & 15)) {
                    f->wait_ticks = animal_rules[ANIMAL_RULE_NO_TILE_WAIT_TICKS];
                }
            }
            predator_find_prey_nearby(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
            predator_find_prey_nearby(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHASE: {
            int32_t prey_id = f->prey_figure_id;
            if (--f->wait_ticks <= 0) {
                f->hunt_ticks = 0;
                f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
                break;
            }
            figure *prey = figure_get(prey_id);
            if (frame_advanced && prey_id && (prey->x != f->destination_x || prey->y != f->destination_y)) {
                figure_route_remove(figure_id);
                f->destination_x = prey->x;
                f->destination_y = prey->y;
            }
            figure_movement_move_ticks(figure_id, half_speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                predator_end_chase(f);
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                if (prey_id && (prey->x != f->destination_x || prey->y != f->destination_y)) {
                    figure_route_remove(figure_id);
                    f->destination_x = prey->x;
                    f->destination_y = prey->y;
                } else {
                    predator_end_chase(f);
                }
            }
            break;
        }
        case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
            random_generate_next();
            if (predator_walk_near(herd->destination_x, herd->destination_y, random_generator_state.random1_15bit & 15)) {
                break;
            }
            random_generate_next();
            if (predator_walk_near(herd->home_x, herd->home_y, random_generator_state.random1_15bit & 15)) {
                break;
            }
            f->state = FIGURE_STATE_DEAD;
            return;
        case FIGURE_ACTION_PREDATOR_SPAWNED:
            if (--f->wait_ticks <= 0) {
                f->animal.frame = 0;
                f->animal.frame_count = 8;
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            }
            f->image_id = 0;
            return;
        case FIGURE_ACTION_HIPPO_CHARGE: {
            int32_t prey_id = f->prey_figure_id;
            if (--f->wait_ticks <= 0) {
                f->hunt_ticks = 0;
                f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
                break;
            }
            figure *prey = figure_get(prey_id);
            if (prey_id && prey->action_state != FIGURE_ACTION_CORPSE && prey->state == FIGURE_STATE_ALIVE) {
                if (frame_advanced && (prey->x != f->destination_x || prey->y != f->destination_y)) {
                    figure_route_remove(figure_id);
                    f->destination_x = prey->x;
                    f->destination_y = prey->y;
                }
            } else {
                f->direction = f->previous_tile_direction;
                predator_hunt_or_choose_action(figure_id);
            }
            figure_movement_move_ticks(figure_id, in_water ? speed : speed * 2);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->direction = f->previous_tile_direction;
                f->hunt_ticks = 0;
                f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                if (prey_id && (prey->x != f->destination_x || prey->y != f->destination_y) && prey->action_state != FIGURE_ACTION_CORPSE &&
                    prey->state == FIGURE_STATE_ALIVE) {
                    figure_route_remove(figure_id);
                    f->destination_x = prey->x;
                    f->destination_y = prey->y;
                } else {
                    f->direction = f->previous_tile_direction;
                    predator_hunt_or_choose_action(figure_id);
                }
            }
            break;
        }
        case FIGURE_ACTION_HIPPO_ANIMATE:
            if (f->animal.frame == f->animal.frame_count >> 1) {
                if (--f->wait_ticks > 0) {
                    f->animal.frame = (f->animal.frame_count >> 1) - 1;
                }
            } else if (frame_advanced) {
                hippo_choose_next_action(figure_id);
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            break;
    }
    if (cheat_side_show_enabled) {
        if (f->action_state != FIGURE_ACTION_CORPSE && f->action_state != FIGURE_ACTION_PREDATOR_DYING) {
            f->animal.frame_count = 14;
            f->image_id = f->animal.frame + image_group(IMAGE_GROUP_FIGURE_HIPPO_SIDE_SHOW) + 5;
        } else {
            f->animal.frame_count = 18;
            f->image_id = image_group(IMAGE_GROUP_FIGURE_HIPPO_SIDE_SHOW) + ((f->wait_ticks >= 18) ? 36 : (f->wait_ticks + 19));
        }
        return;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    int32_t dir = figure_image_direction;
    int32_t frame = f->animal.frame;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
        case FIGURE_ACTION_PREDATOR_DYING:
            if (in_water) {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_DEATH_IN_WATER) + ((f->wait_ticks < 23) ? f->wait_ticks : 22);
            } else {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_HIPPO_DEATH) + (f->wait_ticks < 8 ? f->wait_ticks : 7);
            }
            break;
        case FIGURE_ACTION_PREDATOR_CHOOSE:
        case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
            f->image_id = image_group(in_water ? IMAGE_GROUP_FIGURE_HIPPO_IN_WATER : IMAGE_GROUP_FIGURE_HIPPO) + dir;
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
        case FIGURE_ACTION_HIPPO_GO_TO_WATER:
        case FIGURE_ACTION_PREDATOR_CHASE:
            f->image_id = image_group(in_water ? IMAGE_GROUP_FIGURE_HIPPO_SWIM : IMAGE_GROUP_FIGURE_HIPPO) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HIPPO_CHARGE:
            f->image_id = image_group(in_water ? IMAGE_GROUP_FIGURE_HIPPO_SWIM : IMAGE_GROUP_FIGURE_HIPPO_CHARGE) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_HIPPO_ANIMATE:
            if (in_water) {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_HIPPO_ANIMATE_IN_WATER) + frame * 8 + dir;
            } else {
                f->image_id = image_group(IMAGE_GROUP_FIGURE_HIPPO) + dir;
            }
            break;
        default:
            f->image_id = image_group(in_water ? IMAGE_GROUP_FIGURE_HIPPO_IN_WATER : IMAGE_GROUP_FIGURE_HIPPO_REST) + frame * 8 + dir;
            break;
    }
}

static int32_t asp_walk_near(const formation *herd)
{
    random_generate_next();
    figure *f = figure_get(figure_current_id);
    int32_t center_x = herd->destination_x;
    int32_t center_y = herd->destination_y;
    int32_t dir = animal_find_free_neighbour_direction(center_x, center_y, random_generator_state.random1_15bit % 16, f->x, f->y, 0);
    if (dir < 0 || dir >= HERD_OFFSET_COUNT) {
        return 0;
    }
    figure_route_remove(figure_current_id);
    f->destination_x = (int16_t) (herd_roaming_offsets()[dir].x + center_x);
    f->destination_y = (int16_t) (herd_roaming_offsets()[dir].y + center_y);
    f->action_state = FIGURE_ACTION_PREDATOR_WALK;
    return 1;
}

void figure_asp_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    int32_t frame_advanced = 0;
    if (++f->animal.frame >= f->animal.frame_count) {
        f->animal.frame = 0;
        f->animation_frame++;
        frame_advanced = 1;
    }
    f->terrain_usage = TERRAIN_USAGE_ANIMAL;
    city_player_current()->animals++;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    const formation *herd = formation_get(f->formation_id);
    if (++f->animation_frame >= 12) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            f->animal.frame_count = 6;
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            f->animal.frame_count = 6;
            sound_figure_play(figure_id, ANIMAL_SOUND_ASP_ATTACK, ANIMAL_SOUND_VARIANT);
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHOOSE:
            f->animal.frame_count = 0;
            f->direction = f->previous_tile_direction;
            random_generate_next();
            f->action_state = (random_generator_state.random1_15bit & 1) ? FIGURE_ACTION_PREDATOR_REST : FIGURE_ACTION_PREDATOR_IDLE;
            random_generate_next();
            f->wait_ticks = random_rest_ticks(ANIMAL_RULE_ASP_REST_BASE_TICKS, ANIMAL_RULE_ASP_REST_RANDOM_TICKS);
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
            f->animal.frame_count = 12;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
                f->wait_ticks = (int16_t) (f->index_in_formation * 5);
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                random_generate_next();
                f->action_state = (random_generator_state.random1_15bit & 1) ? FIGURE_ACTION_PREDATOR_REST : FIGURE_ACTION_PREDATOR_IDLE;
                random_generate_next();
                f->wait_ticks = (int16_t) (random_generator_state.random1_15bit % animal_rules[ANIMAL_RULE_ASP_REST_RANDOM_TICKS]);
                f->animal.frame = 0;
            }
            break;
        case FIGURE_ACTION_PREDATOR_DYING:
            f->animal.frame_count = 6;
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > animal_rules[ANIMAL_RULE_DYING_TICKS]) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_PREDATOR_PROWL:
            f->animal.frame_count = 12;
            figure_movement_move_ticks(figure_id, speed * 2);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                predator_end_chase(f);
            }
            if (frame_advanced) {
                predator_find_prey_nearby(figure_id);
            }
            break;
        case FIGURE_ACTION_PREDATOR_REST:
            f->animal.frame_count = 6;
            if (--f->wait_ticks <= 0) {
                if (asp_walk_near(herd)) {
                    f->animal.frame = 0;
                } else {
                    random_generate_next();
                    f->wait_ticks = (int16_t) (random_generator_state.random1_15bit % animal_rules[ANIMAL_RULE_ASP_REST_RANDOM_TICKS]);
                }
            }
            predator_find_prey_nearby(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_IDLE:
            f->animal.frame_count = 6;
            if (--f->wait_ticks <= 0) {
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            }
            break;
        case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
            f->animal.frame_count = 1;
            predator_find_prey_nearby(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHASE: {
            f->animal.frame_count = 12;
            int32_t prey_id = f->prey_figure_id;
            if (--f->wait_ticks <= 0 && frame_advanced) {
                f->wait_ticks = 0;
                f->hunt_ticks = 0;
                f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
                break;
            }
            if (prey_id) {
                figure *prey = figure_get(prey_id);
                if ((prey->x != f->destination_x || prey->y != f->destination_y) && frame_advanced) {
                    figure_route_remove(figure_id);
                    f->destination_x = prey->x;
                    f->destination_y = prey->y;
                }
            }
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                predator_end_chase(f);
            }
            break;
        }
        case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
            f->animal.frame_count = 1;
            if (!asp_walk_near(herd)) {
                f->action_state = FIGURE_ACTION_PREDATOR_REST;
                random_generate_next();
                f->wait_ticks = (int16_t) (random_generator_state.random1_15bit % animal_rules[ANIMAL_RULE_ASP_REST_RANDOM_TICKS]);
            }
            break;
        case FIGURE_ACTION_PREDATOR_SPAWNED:
            if (--f->wait_ticks <= 0) {
                f->animal.frame = 0;
                f->animal.frame_count = 8;
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            }
            f->image_id = 0;
            return;
        default:
            f->animal.frame_count = 0;
            f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            break;
    }
    if (f->animal.frame >= f->animal.frame_count || f->animal.frame < 0) {
        f->animal.frame = 0;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += 8;
    }
    int32_t dir = figure_image_direction;
    int32_t frame = f->animal.frame;
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
        case FIGURE_ACTION_PREDATOR_DYING:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ASP_DEATH) + (f->wait_ticks < 6 ? f->wait_ticks : 5);
            break;
        case FIGURE_ACTION_ATTACK:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ASP_ATTACK) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
        case FIGURE_ACTION_PREDATOR_PROWL:
        case FIGURE_ACTION_PREDATOR_CHASE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ASP) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_PREDATOR_REST:
        case FIGURE_ACTION_PREDATOR_IDLE:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ASP_REST) + frame * 8 + dir;
            break;
        case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_ASP_REST) + dir;
            break;
        default:
            break;
    }
}
