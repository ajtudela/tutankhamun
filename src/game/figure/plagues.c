// The figures of the city's plagues and curses: the plague citizen that leaves its sick house and roams until it
// dies, the locust swarm that eats a field's crop and flies off, and the mummies of the mummy's revenge, which raise
// new mummies on their victims' tiles.

#include "figure/plagues.h"
#include "building/building.h"
#include "building/building_data.h"
#include "city/city_data.h"
#include "city/locusts.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/image_groups_walkers.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/road_access.h"
#include <stdint.h>

static int32_t image_direction_of(const figure *f)
{
    int32_t direction = f->direction;
    if (direction > DIRECTION_TOP_LEFT) {
        direction = f->previous_tile_direction;
    }
    figure_image_direction = direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    return figure_image_direction;
}

void figure_plague_citizen_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->cart_image_id = image_group(IMAGE_GROUP_FIGURE_PLAGUE_SHADOW);
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    f->use_cross_country = 0;
    f->max_roam_length = f->wait_ticks;
    int32_t home_id = f->building_id;
    const building *home = building_get(home_id);
    if (++f->animation_frame >= PLAGUE_CITIZEN_FRAMES) {
        f->animation_frame = 0;
    }
    f->is_ghost = 0;
    int32_t state = f->action_state;
    if (state == FIGURE_ACTION_PLAGUED_CITIZEN_CREATED) {
        f->is_ghost = 1;
        f->animation_frame = 0;
        if (!map_closest_road_within_radius(home->x, home->y, home->size, PLAGUE_CITIZEN_ROAD_RADIUS)) {
            f->state = FIGURE_STATE_DEAD;
        } else {
            f->action_state = FIGURE_ACTION_PLAGUED_CITIZEN_GOING_TO_ROAD;
            f->destination_x = (int16_t) map_result_x;
            f->destination_y = (int16_t) map_result_y;
            figure_movement_set_cross_country_direction(figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
            f->roam_length = 0;
        }
    } else if (state == FIGURE_ACTION_PLAGUED_CITIZEN_GOING_TO_ROAD) {
        f->use_cross_country = 1;
        f->is_ghost = 1;
        if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
            if (map_grids.building_ids[f->grid_offset] == home_id) {
                f->state = FIGURE_STATE_DEAD;
            } else {
                f->action_state = FIGURE_ACTION_PLAGUED_CITIZEN_ROAMING;
                figure_movement_init_roaming(figure_id);
                f->roam_length = 0;
            }
        }
    } else if (state == FIGURE_ACTION_PLAGUED_CITIZEN_ROAMING) {
        f->is_ghost = 0;
        if (++f->roam_length >= f->max_roam_length) {
            int32_t road = map_closest_road_within_radius(home->x, home->y, home->size, PLAGUE_CITIZEN_ROAD_RADIUS);
            f->state = FIGURE_STATE_DEAD;
            if (road) {
                return;
            }
        }
        figure_movement_roam_ticks(figure_id, 1);
    }
    image_direction_of(f);
    if (f->action_state != FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_PLAGUE_CITIZEN) + f->animation_frame * 8 + figure_image_direction;
        return;
    }
    f->image_id = figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_PLAGUE_CITIZEN_DEATH);
}

void figure_locust_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ANYWHERE;
    f->cart_image_id = 0;
    int32_t field_id = f->immigrant_building_id;
    int32_t speed = figure_properties[f->type].speed;
    building *field = building_get(field_id);
    if (field->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
        return;
    }
    if (++f->animation_frame >= LOCUST_FRAMES) {
        f->animation_frame = 0;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_LOCUST_CREATED:
            f->is_ghost = 1;
            f->animation_frame = 0;
            if (--f->wait_ticks < 1) {
                figure_route_add(figure_id);
                if (f->routing_path_id < 1) {
                    f->state = FIGURE_STATE_DEAD;
                } else {
                    f->action_state = FIGURE_ACTION_LOCUST_GOING_TO_FIELD;
                }
            }
            break;
        case FIGURE_ACTION_LOCUST_GOING_TO_FIELD:
            f->is_ghost = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE) {
                f->action_state = FIGURE_ACTION_LOCUST_CREATED;
                f->wait_ticks = 1;
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                field->immigrant_figure_id = 0;
                field->distance_from_entry = 0;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_LOCUST_EATING;
                f->destination_x = field->x;
                f->destination_y = field->y;
            }
            break;
        case FIGURE_ACTION_LOCUST_EATING: {
            int16_t *eaten = &field->data.industry.progress;
            int32_t countdown = city_locusts_countdown;
            if (*eaten > countdown - LOCUST_COUNTDOWN_FLY_OFF) {
                *eaten = (int16_t) (countdown - LOCUST_COUNTDOWN_FLY_OFF);
            }
            if (*eaten < 0) {
                *eaten = 0;
            }
            if (countdown <= LOCUST_COUNTDOWN_FLY_OFF) {
                f->action_state = FIGURE_ACTION_LOCUST_FLYING_OFF;
                random_generate_next();
                int32_t width = map_info.grid_width;
                int32_t r = random_generator_state.random1_15bit % (width / 4);
                f->destination_x = (int16_t) (((width * 3) / 8) - r + 2);
                f->destination_y = (int16_t) (((width * 7) / 8) - r - 2);
                f->roam_length = 0;
            }
            break;
        }
        case FIGURE_ACTION_LOCUST_FLYING_OFF:
            f->is_ghost = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_LOCUST_CREATED;
            break;
    }
    if (f->action_state == FIGURE_ACTION_LOCUST_EATING) {
        f->image_id = f->animation_frame + image_group(IMAGE_GROUP_FIGURE_LOCUST_EATING);
        return;
    }
    f->image_id = f->animation_frame + image_group(IMAGE_GROUP_FIGURE_LOCUST);
}

void figure_mummy_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_AMPHIBIA;
    f->cart_image_id = 0;
    if (++f->animation_frame >= MUMMY_FRAMES) {
        f->animation_frame = 0;
    }
    int32_t state = f->action_state;
    if (state == FIGURE_ACTION_CORPSE) {
        figure_combat_handle_corpse(figure_id);
    } else if (state == FIGURE_ACTION_ATTACK) {
        const figure *victim = figure_get(f->opponent_id);
        if (victim->state != FIGURE_STATE_ALIVE || victim->action_state == FIGURE_ACTION_CORPSE) {
            int32_t mummies = 0;
            for (int32_t id = 1; id < MAX_FIGURES; id++) {
                const figure *m = figure_get(id);
                if (m->state != FIGURE_STATE_NONE && m->type == FIGURE_MUMMY) {
                    mummies++;
                }
            }
            if (mummies < mummies_max) {
                int32_t mummy_id = figure_create(city_current_player, FIGURE_MUMMY, victim->x, victim->y, 0);
                figure *mummy = figure_get(mummy_id);
                mummy->building_id = f->building_id;
                mummy->action_state = FIGURE_ACTION_MUMMY_ROAMING;
                random_generate_next();
                mummy->wait_ticks = (int16_t) ((random_generator_state.random1_15bit % MUMMY_ROAM_SPREAD) + MUMMY_ROAM_MIN);
                figure_movement_init_roaming(mummy_id);
                mummy->roam_length = 0;
            }
        }
        figure_combat_handle_attack(figure_id);
    } else if (state == FIGURE_ACTION_MUMMY_ROAMING) {
        f->is_ghost = 0;
        if (++f->roam_length >= f->max_roam_length && !map_closest_road_within_radius(f->x, f->y, 1, MUMMY_ROAD_RADIUS)) {
            f->state = FIGURE_STATE_DEAD;
        }
        if ((f->animation_frame & 1) == 0) {
            figure_movement_roam_ticks(figure_id, 1);
        }
    }
    figure_image_direction = f->direction - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE: {
            int32_t index = (f->wait_ticks < (2 * MUMMY_CORPSE_IMAGES)) ? (f->wait_ticks / 2) : (MUMMY_CORPSE_IMAGES - 1);
            f->image_id = image_group(IMAGE_GROUP_FIGURE_MUMMY_DEATH) + index * 8 + figure_image_direction;
            break;
        }
        case FIGURE_ACTION_ATTACK:
            figure_image_direction = f->attack_direction - map_info.orientation;
            f->image_id = image_group(IMAGE_GROUP_FIGURE_MUMMY_ATTACK) + (f->animation_frame >> 1) * 8 + figure_image_direction;
            break;
        case FIGURE_ACTION_MUMMY_ROAMING:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_MUMMY) + (f->animation_frame >> 1) * 8 + figure_image_direction;
            break;
    }
}
