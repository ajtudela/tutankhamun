// Wild animals the ordinary game rarely runs: the predators of the desert and the savanna (hyena, lion, scorpion), one
// action for all of them driven by the wild_predators table, the frogs of the frog plague, the creature, and the
// creation of the herds on the scenario's herd points at the start of a scenario.
//
// The wild animals' setup has three oddities:
// - the herd animals are created for player 1;
// - a predator's tile is searched in herd_spawn_offsets and a prey's in herd_roaming_offsets;
// - the predators wait four times as long as the prey before they start.

#include "figure/animals_wild.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/house_population.h"
#include "city/city_player.h"
#include "core/random.h"
#include "figure/animals_internal.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/image_groups_animals.h"
#include "figure/military.h"
#include "figure/movement_internal.h"
#include "figure/route.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/road_access.h"
#include <stdint.h>

enum wild_animation {
    WILD_ANIMATION_FRAMES = 12,
    FROG_ANIMATION_FRAMES = 9,
    WILD_ARRIVAL_REST_FRAMES = 10,
    WILD_ARRIVAL_IDLE_FRAMES = 4,
    WILD_SPAWNED_FRAMES = 8,
    WILD_CORPSE_LAST_FRAME = 5,
};

enum wild_behaviour {
    WILD_WAIT_TICKS_PER_HERD_INDEX = 5,
    WILD_DYING_TICKS = 500,
    WILD_PROWL_SPEED_FACTOR = 2,
    FROG_ROAD_RADIUS = 2,
    FROG_MOVE_CHANCE_PERCENT = 50,
    HERD_ANIMALS_PLAYER = 1,
    HERD_SPAWN_WAIT_RANGE = 25,
    PREDATOR_SPAWN_WAIT_FACTOR = 4,
    HERD_START_WAIT_TICKS = 24,
};

wild_predator wild_predators[WILD_PREDATOR_KINDS] = {
    [WILD_HYENA] = {
        .frames = { -1, -1, -1, -1, 6, 6, -1, -1, 1, 12, 12, -1, 6, 12, -1, -1, -1, -1, 6, 6, 1, 12, -1, 1, -1 },
        .default_frames = 1,
        .attack_sound = 0,
        .rest_base = 150,
        .rest_range = 100,
        .no_tile_wait = 100,
        .arrival_frames = 0,
        .reset_counter = 0,
    },
    [WILD_LION] = {
        .frames = { -1, -1, -1, -1, 6, 11, -1, -1, 0, -1, -1, -1, 6, 11, -1, -1, -1, -1, 10, 8, 8, 11, -1, 11, -1 },
        .default_frames = 1,
        .attack_sound = 1,
        .rest_base = 150,
        .rest_range = 200,
        .no_tile_wait = 200,
        .arrival_frames = 1,
        .reset_counter = 0,
    },
    [WILD_SCORPION] = {
        .frames = { -1, -1, -1, -1, 6, 6, -1, -1, 0, 12, 12, -1, 6, 12, -1, -1, -1, -1, 6, 6, 1, 12, -1, 1, -1 },
        .default_frames = 0,
        .attack_sound = 2,
        .rest_base = 0,
        .rest_range = 100,
        .no_tile_wait = -1,
        .arrival_frames = 0,
        .reset_counter = 1,
    },
};

static void wild_set_rest_wait(figure *f, const wild_predator *p)
{
    random_generate_next();
    f->wait_ticks = (int16_t) ((random_generator_state.random1_15bit % p->rest_range) + p->rest_base);
}

static void wild_set_no_tile_wait(figure *f, const wild_predator *p)
{
    if (p->no_tile_wait < 0) {
        random_generate_next();
        f->wait_ticks = (int16_t) (random_generator_state.random1_15bit % 100);
    } else {
        f->wait_ticks = (int16_t) p->no_tile_wait;
    }
}

static int32_t wild_walk_near_herd(int32_t formation_id)
{
    figure *f = figure_get(figure_current_id);
    const formation *herd = formation_get(formation_id);
    random_generate_next();
    int32_t dir = animal_find_free_neighbour_direction(
        herd->destination_x, herd->destination_y, random_generator_state.random1_15bit & (HERD_OFFSET_COUNT - 1), f->x, f->y, 0);
    if (dir < 0 || dir >= HERD_OFFSET_COUNT) {
        return 0;
    }
    figure_route_remove(figure_current_id);
    f->destination_x = (int16_t) (herd_roaming_offsets()[dir].x + herd->destination_x);
    f->destination_y = (int16_t) (herd_roaming_offsets()[dir].y + herd->destination_y);
    f->action_state = FIGURE_ACTION_PREDATOR_WALK;
    return 1;
}

static void wild_end_chase(figure *f)
{
    f->direction = f->previous_tile_direction;
    f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
    f->wait_ticks = (int16_t) (f->index_in_formation * WILD_WAIT_TICKS_PER_HERD_INDEX);
}

static void wild_image(figure *f, int32_t kind)
{
    int32_t dir = figure_image_direction;
    int32_t frame = f->animal.frame;
    int32_t corpse = (f->wait_ticks > WILD_CORPSE_LAST_FRAME) ? WILD_CORPSE_LAST_FRAME : f->wait_ticks;
    switch (kind) {
        case WILD_HYENA:
            switch (f->action_state) {
                case FIGURE_ACTION_CORPSE:
                case FIGURE_ACTION_PREDATOR_DYING:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_DEATH) + corpse;
                    break;
                case FIGURE_ACTION_ATTACK:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_ATTACK) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_CHOOSE:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_WALK) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_WALK:
                case FIGURE_ACTION_PREDATOR_MOVE:
                case FIGURE_ACTION_PREDATOR_PROWL:
                case FIGURE_ACTION_PREDATOR_CHASE:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_WALK) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_REST:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_REST) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_IDLE:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_IDLE) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_HYENA_IDLE) + dir;
                    break;
                default:
                    break;
            }
            break;
        case WILD_LION:
            switch (f->action_state) {
                case FIGURE_ACTION_CORPSE:
                case FIGURE_ACTION_PREDATOR_DYING:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_LION_DEATH) + (corpse * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_ATTACK:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_LION_ATTACK) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_CHOOSE:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_LION_WALK) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_WALK:
                case FIGURE_ACTION_PREDATOR_MOVE:
                case FIGURE_ACTION_PREDATOR_PROWL:
                case FIGURE_ACTION_PREDATOR_CHASE:
                case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_LION_WALK) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_REST:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_LION_REST) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_IDLE:
                case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_LION_IDLE) + ((frame / 2) * DIRECTION_COUNT) + dir;
                    break;
                default:
                    break;
            }
            break;
        default:
            switch (f->action_state) {
                case FIGURE_ACTION_CORPSE:
                case FIGURE_ACTION_PREDATOR_DYING:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_SCORPION_DEATH) + corpse;
                    break;
                case FIGURE_ACTION_ATTACK:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_SCORPION_ATTACK) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_CHOOSE:
                case FIGURE_ACTION_PREDATOR_WALK:
                case FIGURE_ACTION_PREDATOR_MOVE:
                case FIGURE_ACTION_PREDATOR_PROWL:
                case FIGURE_ACTION_PREDATOR_CHASE:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_SCORPION_WALK) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_REST:
                case FIGURE_ACTION_PREDATOR_IDLE:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_SCORPION_IDLE) + (frame * DIRECTION_COUNT) + dir;
                    break;
                case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
                    f->image_id = image_group(IMAGE_GROUP_FIGURE_SCORPION_IDLE) + dir;
                    break;
                default:
                    break;
            }
            break;
    }
}

static void wild_predator_action(int32_t kind)
{
    const wild_predator *p = &wild_predators[kind];
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    int32_t step_advanced = 0;
    if (++f->animal.frame >= f->animal.frame_count) {
        f->animal.frame = 0;
        f->animation_frame++;
        step_advanced = 1;
    }
    f->terrain_usage = TERRAIN_USAGE_ANIMAL;
    city_player_current()->animals++;
    f->use_cross_country = 0;
    f->is_ghost = 0;
    int32_t formation_id = f->formation_id;
    if (++f->animation_frame >= WILD_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    int32_t state = f->action_state;
    if (state <= FIGURE_ACTION_PREDATOR_SPAWNED && p->frames[state] >= 0) {
        f->animal.frame_count = p->frames[state];
    }
    switch (state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_ATTACK:
            sound_figure_play(figure_id, p->attack_sound, 2);
            figure_combat_handle_attack(figure_current_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHOOSE:
            f->direction = f->previous_tile_direction;
            random_generate_next();
            f->action_state = (random_generator_state.random1_15bit & 1) ? FIGURE_ACTION_PREDATOR_REST : FIGURE_ACTION_PREDATOR_IDLE;
            wild_set_rest_wait(f, p);
            break;
        case FIGURE_ACTION_PREDATOR_WALK:
        case FIGURE_ACTION_PREDATOR_MOVE:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_current_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->direction = f->previous_tile_direction;
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
                f->wait_ticks = (int16_t) (f->index_in_formation * WILD_WAIT_TICKS_PER_HERD_INDEX);
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->direction = f->previous_tile_direction;
                random_generate_next();
                if (random_generator_state.random1_15bit & 1) {
                    f->action_state = FIGURE_ACTION_PREDATOR_REST;
                    if (p->arrival_frames) {
                        f->animal.frame_count = WILD_ARRIVAL_REST_FRAMES;
                    }
                } else {
                    f->action_state = FIGURE_ACTION_PREDATOR_IDLE;
                    if (p->arrival_frames) {
                        f->animal.frame_count = WILD_ARRIVAL_IDLE_FRAMES;
                    }
                }
                wild_set_rest_wait(f, p);
                if (p->reset_counter) {
                    f->animal.frame = 0;
                }
            }
            break;
        case FIGURE_ACTION_PREDATOR_DYING:
            if (f->wait_ticks == 0) {
                figure_play_die_sound(figure_id);
            }
            if (++f->wait_ticks > WILD_DYING_TICKS) {
                f->action_state = FIGURE_ACTION_CORPSE;
            }
            break;
        case FIGURE_ACTION_PREDATOR_PROWL:
            figure_movement_move_ticks(figure_id, speed * WILD_PROWL_SPEED_FACTOR);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_current_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                wild_end_chase(f);
            }
            if (step_advanced) {
                predator_find_prey_nearby(figure_current_id);
            }
            break;
        case FIGURE_ACTION_PREDATOR_REST:
            if (--f->wait_ticks <= 0) {
                if (wild_walk_near_herd(formation_id)) {
                    if (p->reset_counter) {
                        f->animal.frame = 0;
                    }
                } else {
                    wild_set_no_tile_wait(f, p);
                }
            }
            predator_find_prey_nearby(figure_current_id);
            break;
        case FIGURE_ACTION_PREDATOR_IDLE:
            if (--f->wait_ticks <= 0) {
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            }
            break;
        case FIGURE_ACTION_PREDATOR_LOOK_FOR_PREY:
            predator_find_prey_nearby(figure_id);
            break;
        case FIGURE_ACTION_PREDATOR_CHASE: {
            int32_t prey_id = f->prey_figure_id;
            if (--f->wait_ticks <= 0 && step_advanced) {
                f->wait_ticks = 0;
                f->hunt_ticks = 0;
                f->action_state = FIGURE_ACTION_PREDATOR_RETURN_TO_HERD;
                break;
            }
            figure *prey = figure_get(prey_id);
            if (prey_id && (prey->x != f->destination_x || prey->y != f->destination_y) && step_advanced) {
                figure_route_remove(figure_id);
                f->destination_x = prey->x;
                f->destination_y = prey->y;
            }
            figure_movement_move_ticks(figure_current_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_current_id);
            } else if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                wild_end_chase(f);
            }
            break;
        }
        case FIGURE_ACTION_PREDATOR_RETURN_TO_HERD:
            if (!wild_walk_near_herd(formation_id)) {
                f->action_state = FIGURE_ACTION_PREDATOR_REST;
                wild_set_no_tile_wait(f, p);
            }
            break;
        case FIGURE_ACTION_PREDATOR_SPAWNED:
            if (--f->wait_ticks <= 0) {
                f->animal.frame = 0;
                f->animal.frame_count = WILD_SPAWNED_FRAMES;
                f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            }
            f->image_id = 0;
            return;
        default:
            f->animal.frame_count = p->default_frames;
            f->action_state = FIGURE_ACTION_PREDATOR_CHOOSE;
            break;
    }
    if (f->animal.frame >= f->animal.frame_count || f->animal.frame < 0) {
        f->animal.frame = 0;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    wild_image(f, kind);
}

void figure_hyena_action(void)
{
    wild_predator_action(WILD_HYENA);
}

void figure_lion_action(void)
{
    wild_predator_action(WILD_LION);
}

void figure_scorpion_action(void)
{
    wild_predator_action(WILD_SCORPION);
}

void figure_frog_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_AMPHIBIA;
    f->cart_image_id = 0;
    int32_t speed = figure_properties[f->type].speed;
    int32_t house_id = f->immigrant_building_id;
    building *b = building_get(house_id);
    if (b->state != BUILDING_STATE_IN_USE || b->immigrant_figure_id != figure_id || !b->house_size) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    if (++f->animation_frame >= FROG_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_FROG_CREATED:
            f->is_ghost = 1;
            f->animation_frame = 0;
            if (--f->wait_ticks <= 0) {
                if (!map_closest_road_within_radius(b->x, b->y, b->size, FROG_ROAD_RADIUS)) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_FROG_GOING_TO_HOUSE;
                    f->destination_x = map_result_x;
                    f->destination_y = map_result_y;
                    f->roam_length = 0;
                }
            }
            break;
        case FIGURE_ACTION_FROG_GOING_TO_HOUSE:
            f->is_ghost = 0;
            random_generate_next();
            if ((random_generator_state.random1_15bit % 100) < FROG_MOVE_CHANCE_PERCENT) {
                figure_movement_move_ticks(figure_id, speed);
            }
            if (f->direction == DIRECTION_REROUTE) {
                f->action_state = FIGURE_ACTION_FROG_CREATED;
                f->wait_ticks = 1;
                figure_route_remove(figure_current_id);
            } else if (f->direction == DIRECTION_LOST) {
                b->immigrant_figure_id = 0;
                b->distance_from_entry = 0;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_FROG_ENTERING_HOUSE;
                f->destination_x = b->x;
                f->destination_y = b->y;
                figure_movement_set_cross_country_direction(
                    figure_current_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
                f->roam_length = 0;
            }
            break;
        case FIGURE_ACTION_FROG_ENTERING_HOUSE:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                int32_t player = city_current_player;
                f->state = FIGURE_STATE_DEAD;
                figure_create_homeless(player, b->x, b->y, b->house_population);
                b->house_population = 0;
                b->data.house.is_sick = HOUSE_SICK_FROGS;
                int32_t months = (city_frogs_countdown / TICKS_PER_DAY) / DAYS_PER_MONTH;
                b->house_sick_months = (months < 1) ? 1 : months;
                b->immigrant_figure_id = 0;
            }
            f->is_ghost = (f->in_building_wait_ticks != 0);
            break;
        default:
            f->action_state = FIGURE_ACTION_FROG_CREATED;
            break;
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_FROG_DEATH);
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_FROG) + (f->animation_frame * DIRECTION_COUNT) + figure_image_direction;
    }
}

static void create_herd_animals(int32_t formation_id, int32_t type, int32_t count, int32_t x, int32_t y, bool is_prey)
{
    const formation_layout_spot *offsets = is_prey ? herd_roaming_offsets() : herd_spawn_offsets();
    for (int32_t i = 0; i < count; i++) {
        random_generate_next();
        int32_t dir = i;
        for (int32_t tries = 0; tries < HERD_OFFSET_COUNT; tries++) {
            if (animal_terrain_allowed_climate(map_grids.terrain[x + map_info.grid_start + offsets[dir].x + (offsets[dir].y + y) * GRID_SIZE], is_prey)) {
                break;
            }
            if (++dir >= HERD_OFFSET_COUNT) {
                dir = 0;
            }
        }
        random_generate_next();
        int32_t figure_id = figure_create(HERD_ANIMALS_PLAYER, type, offsets[dir].x + x, offsets[dir].y + y, random_generator_state.random1_15bit & 7);
        figure *f = figure_get(figure_id);
        f->action_state = FIGURE_ACTION_HERD_SPAWNED;
        f->formation_id = (int16_t) formation_id;
        f->wait_ticks = 0;
        random_generate_next();
        int32_t spawn_wait = random_generator_state.random1_15bit % HERD_SPAWN_WAIT_RANGE;
        f->wait_ticks = (int16_t) (is_prey ? spawn_wait : spawn_wait * PREDATOR_SPAWN_WAIT_FACTOR);
        f->index_in_formation = (uint8_t) i;
    }
}

static void create_herd_formation(int32_t formation_id, int32_t x, int32_t y, int32_t kind, int32_t count)
{
    formation *herd = formation_get(formation_id);
    herd->destination_x = (int16_t) x;
    herd->destination_y = (int16_t) y;
    herd->herd_state = HERD_STATE_ROAMING;
    herd->herd_kind = (uint8_t) kind;
    herd->wait.ticks = HERD_START_WAIT_TICKS;
    herd->max_figures = (uint8_t) count;
}

void figure_create_herds(void)
{
    const climate_animal *prey = scenario_prey();
    int32_t prey_type = prey->figure_type;
    int32_t prey_count = prey->max_figures;
    const climate_animal *predator = scenario_predator();
    int32_t predator_type = predator->figure_type;
    int32_t predator_count = predator->max_figures;
    if (editor_active) {
        return;
    }
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        int32_t x = scenario_data.predator_herd_x[i];
        if (x <= 0) {
            continue;
        }
        int32_t y = scenario_data.predator_herd_y[i];
        int32_t formation_id = formation_create(predator_type, FORMATION_HERD, 0, x, y);
        if (formation_id <= 0) {
            continue;
        }
        create_herd_formation(formation_id, x, y, HERD_KIND_PREDATORS, predator_count);
        create_herd_animals(formation_id, predator_type, predator_count, x, y, 0);
    }
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        int32_t x = scenario_data.prey_herd_x[i];
        if (x <= 0) {
            continue;
        }
        int32_t y = scenario_data.prey_herd_y[i];
        int32_t formation_id = formation_create(prey_type, FORMATION_HERD, 0, x, y);
        if (formation_id <= 0) {
            continue;
        }
        create_herd_formation(formation_id, x, y, HERD_KIND_PREY, prey_count);
        create_herd_animals(formation_id, prey_type, prey_count, x, y, 1);
    }
}

void figure_creature_action(void)
{
    figure_get(figure_current_id)->image_id = image_group(IMAGE_GROUP_FIGURE_CROCODILE);
}
