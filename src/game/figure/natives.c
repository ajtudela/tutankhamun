// Figures the ordinary game rarely runs: emigrants, the natives of the native huts and their traders, the flotsam on
// the river, and the creation of the flotsam and the fishing points' seagulls at the start of a scenario. The
// flotsam's frames, starts and restart waits come from config files.
//
// The rarely run figures keep these faults:
// - an emigrant without a road nearby is dead but still sets out;
// - a flotsam's restart wait is cut to int16;
// - figure_create_flotsam resets the cross-country direction of the current figure, not of the seagulls.

#include "figure/natives.h"
#include "building/building.h"
#include "building/building_data.h"
#include "city/city_player.h"
#include "core/crt_compat.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/image_groups_boats.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "figure/trade.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/natives.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include <stdint.h>

int32_t emigrant_cart_direction;

int32_t flotsam_bob_frames[FLOTSAM_BOB_FRAMES] = { 0, 1, 2, 3, 4, 4, 4, 3, 2, 1, 0, 0 };
int32_t flotsam_debris_frames[FLOTSAM_DEBRIS_FRAMES] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 3, 2, 1, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0 };
int32_t flotsam_sinking_frames[FLOTSAM_DEBRIS_FRAMES] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

static void advance_frame(figure *f, int32_t frames)
{
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= frames) {
        f->animation_frame = 0;
    }
}

static void emigrant_set_cart_offset(figure *f, int32_t dir)
{
    if (f->action_state == FIGURE_ACTION_EMIGRANT_LEAVING) {
        emigrant_cart_direction = dir + (DIRECTION_COUNT / 2);
        if (emigrant_cart_direction > DIRECTION_TOP_LEFT) {
            emigrant_cart_direction = dir - (DIRECTION_COUNT / 2);
        }
        f->cart_offset_x = figure_cart_offsets[emigrant_cart_direction * 2];
        f->cart_offset_y = figure_cart_offsets[emigrant_cart_direction * 2 + 1];
    }
}

void figure_emigrant_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->terrain_usage = TERRAIN_USAGE_ANY;
    f->cart_image_id = 0;
    int32_t speed = figure_properties[f->type].speed;
    uint32_t terrain = map_grids.terrain[f->grid_offset];
    int32_t on_building = (terrain & TERRAIN_WATER) && map_grids.sprite[f->grid_offset] == 0;
    advance_frame(f, EMIGRANT_FRAMES);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_EMIGRANT_CREATED:
            f->is_ghost = 1;
            f->animation_frame = 0;
            if (++f->wait_ticks > EMIGRANT_WAIT_TICKS) {
                if (map_closest_road_within_radius(f->x, f->y, 1, EMIGRANT_ROAD_RADIUS) == 0) {
                    f->state = FIGURE_STATE_DEAD;
                }
                f->action_state = FIGURE_ACTION_EMIGRANT_EXITING_HOUSE;
                f->destination_x = (int16_t) map_result_x;
                f->destination_y = (int16_t) map_result_y;
                figure_movement_set_cross_country_direction(figure_id, f->cross_country.x, f->cross_country.y, f->destination_x * 15, f->destination_y * 15, 0);
                f->roam_length = 0;
            }
            break;
        case FIGURE_ACTION_EMIGRANT_EXITING_HOUSE:
            f->use_cross_country = 1;
            f->is_ghost = 1;
            if (figure_movement_move_ticks_cross_country(figure_id, 1) == 1) {
                const city_player *player = city_player_current();
                f->action_state = FIGURE_ACTION_EMIGRANT_LEAVING;
                f->destination_x = player->entry_point_x;
                f->destination_y = player->entry_point_y;
                f->roam_length = 0;
                f->progress_on_tile = 15;
            }
            f->is_ghost = f->in_building_wait_ticks ? 1 : 0;
            break;
        case FIGURE_ACTION_EMIGRANT_LEAVING:
            f->use_cross_country = 0;
            f->is_ghost = 0;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_EMIGRANT_CREATED;
            break;
    }

    int32_t dir = f->direction - map_info.orientation;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
    }
    figure_image_direction = dir;
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        if (!on_building) {
            f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_EMIGRANT_DEATH));
            emigrant_set_cart_offset(f, dir);
            return;
        }
        if (f->wait_ticks < EMIGRANT_DROWNING_TICKS) {
            int32_t swept = map_sweep_has_passed(f->x);
            int32_t death = swept ? image_group(IMAGE_GROUP_FIGURE_FERRY_BOAT_SWEPT_SINKING) : image_group(IMAGE_GROUP_FIGURE_FERRY_BOAT_SINKING);
            f->image_id = (figure_image_direction / DIRECTION_COUNT) + death + (f->wait_ticks / 3);
        } else {
            int32_t frame = f->wait_ticks - EMIGRANT_DROWNING_TICKS;
            if (frame > EMIGRANT_BUBBLES_LAST_FRAME) {
                frame = EMIGRANT_BUBBLES_LAST_FRAME;
            }
            f->image_id = frame + image_group(IMAGE_GROUP_FIGURE_BUBBLES);
        }
    } else {
        if (!on_building) {
            f->image_id = (int32_t) (image_group(IMAGE_GROUP_FIGURE_EMIGRANT) + f->animation_frame * 8u + dir);
            emigrant_set_cart_offset(f, dir);
            return;
        }
        int32_t swept = map_sweep_has_passed(f->x);
        int32_t base = swept ? image_group(IMAGE_GROUP_FIGURE_FERRY_BOAT_SWEPT) : image_group(IMAGE_GROUP_FIGURE_FERRY_BOAT);
        f->image_id = (int32_t) (base + (f->animation_frame / 3) * 8u + figure_image_direction);
    }
    f->cart_image_id = 0;
    emigrant_set_cart_offset(f, dir);
}

void figure_indigenous_native_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    if ((map_grids.terrain[f->grid_offset] & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) == (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) {
        figure_set_state_dead(figure_id);
        return;
    }
    f->terrain_usage = TERRAIN_USAGE_ANY;
    f->use_cross_country = 0;
    f->max_roam_length = NATIVE_MAX_ROAM_LENGTH;
    int32_t home_id = f->building_id;
    int32_t speed = figure_properties[f->type].speed;
    building *home = building_get(home_id);
    city_player *player = city_player_current();
    if (home->state != BUILDING_STATE_IN_USE || home->figure_id != figure_id) {
        f->state = FIGURE_STATE_DEAD;
    }
    advance_frame(f, NATIVE_FRAMES);
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_NATIVE_ATTACKING:
            f->terrain_usage = TERRAIN_USAGE_ENEMY;
            player->security_breach_duration = NATIVE_ATTACK_SECURITY_BREACH;
            player->attacking_natives++;
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_NATIVE_CREATED;
            }
            break;
        case FIGURE_ACTION_NATIVE_CREATED:
            f->animation_frame = 0;
            if (++f->wait_ticks > (int32_t) ((figure_id & NATIVE_WAIT_SPREAD) + NATIVE_WAIT_TICKS)) {
                int32_t target_id = home->subtype;
                f->wait_ticks = 0;
                if (player->natives_hostile == 0) {
                    building *target = building_get(target_id);
                    if (map_native_target_tile(target->x, target->y, target->size)) {
                        f->action_state = FIGURE_ACTION_NATIVE_GOING_TO_MEETING;
                        f->destination_x = (int16_t) map_result_x;
                        f->destination_y = (int16_t) map_result_y;
                    }
                    figure_route_remove(figure_id);
                } else {
                    const formation *native_attack = formation_get(0);
                    f->action_state = FIGURE_ACTION_NATIVE_ATTACKING;
                    f->destination_x = native_attack->destination_x;
                    f->destination_y = native_attack->destination_y;
                    f->destination_building_id = native_attack->destination_building_id;
                    figure_route_remove(figure_id);
                }
            }
            break;
        case FIGURE_ACTION_NATIVE_GOING_TO_MEETING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_NATIVE_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
            }
            break;
        case FIGURE_ACTION_NATIVE_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_NATIVE_CREATED;
            break;
    }

    int32_t dir;
    if (f->action_state == FIGURE_ACTION_ATTACK || f->direction == DIRECTION_ATTACK) {
        dir = f->attack_direction;
    } else if (f->direction < DIRECTION_COUNT) {
        dir = f->direction;
    } else {
        dir = f->previous_tile_direction;
    }
    figure_image_direction = dir - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    int32_t base = image_group(IMAGE_GROUP_FIGURE_NATIVE);
    if (f->action_state == FIGURE_ACTION_ATTACK) {
        if (f->attack_image_offset < NATIVE_FIGHT_WINDUP) {
            f->image_id = figure_image_direction + NATIVE_IMAGE_FIGHT + base;
        } else {
            f->image_id = figure_image_direction + NATIVE_IMAGE_FIGHT + (((int32_t) (f->attack_image_offset - NATIVE_FIGHT_WINDUP) / 2) * 8) + base;
        }
    } else if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + NATIVE_IMAGE_CORPSE + base);
    } else if (f->direction == DIRECTION_ATTACK) {
        f->image_id = (int32_t) (figure_image_direction + NATIVE_IMAGE_FIGHT + (f->animation_frame >> 1) * 8u + base);
    } else if (f->action_state == FIGURE_ACTION_NATIVE_ATTACKING) {
        f->image_id = (int32_t) (figure_image_direction + NATIVE_IMAGE_MARCH + f->animation_frame * 8u + base);
    } else {
        f->image_id = (int32_t) (figure_image_direction + f->animation_frame * 8u + base);
    }
}

static int32_t native_trader_go_to_storage(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t storage_id = caravan_find_storage(figure_id, f->x, f->y, 0, -1);
    if (storage_id == 0) {
        return 0;
    }
    f->destination_building_id = (int16_t) storage_id;
    f->action_state = FIGURE_ACTION_NATIVE_TRADER_GOING_TO_STORAGE;
    f->wait_ticks = 0;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    f->animation_frame = 0;
    return 1;
}

void figure_native_trader_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_ANY;
    int32_t speed = figure_properties[f->type].speed;
    advance_frame(f, NATIVE_TRADER_FRAMES);
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_NATIVE_TRADER_CREATED:
            f->is_ghost = 1;
            if (++f->wait_ticks > NATIVE_TRADER_WAIT_TICKS) {
                f->wait_ticks = 0;
                if (native_trader_go_to_storage(figure_id)) {
                    break;
                }
                f->state = FIGURE_STATE_DEAD;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_NATIVE_TRADER_GOING_TO_STORAGE:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_NATIVE_TRADER_AT_STORAGE;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
                f->is_ghost = 1;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_NATIVE_TRADER_AT_STORAGE:
            if (++f->wait_ticks > NATIVE_TRADER_WAIT_TICKS) {
                f->wait_ticks = 0;
                if (figure_trade_caravan_can_buy(figure_id, f->faction_id, f->destination_building_id, 0)) {
                    int32_t resource = trader_get_buy_resource(f->faction_id, f->destination_building_id, 0);
                    trader_record_bought_resource(figure_id, resource);
                    f->trader_amount_bought += NATIVE_TRADER_AMOUNT_PER_PURCHASE;
                    f->animation_frame = 0;
                    break;
                }
                if (native_trader_go_to_storage(figure_id)) {
                    break;
                }
                f->action_state = FIGURE_ACTION_NATIVE_TRADER_RETURNING;
                f->wait_ticks = 0;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_NATIVE_TRADER_RETURNING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_NATIVE_TRADER_CREATED;
            break;
    }

    int32_t dir = (f->direction > DIRECTION_TOP_LEFT) ? f->previous_tile_direction : f->direction;
    dir -= map_info.orientation;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
    }
    figure_image_direction = dir;
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = (int32_t) (figure_corpse_offsets[f->wait_ticks / 2] + image_group(IMAGE_GROUP_FIGURE_NATIVE_TRADER_DEATH));
        f->cart_image_id = 0;
    } else {
        f->image_id = (int32_t) (image_group(IMAGE_GROUP_FIGURE_NATIVE_TRADER) + f->animation_frame * 8u + dir);
    }
    int32_t cart = (int32_t) (image_group(IMAGE_GROUP_FIGURE_CARTS) + 8 + f->resource_id * 8u);
    f->cart_image_id = cart;
    if (cart != 0) {
        f->cart_image_id = cart + dir;
        f->cart_offset_x = figure_cart_offsets[dir * 2];
        f->cart_offset_y = figure_cart_offsets[dir * 2 + 1];
    }
}

flotsam_restart_wait flotsam_restart_waits[FLOTSAM_RESTART_STEPS] = {
    [0] = { .floated_at_least = 400, .random_mask = 0x7, .base = 0 },
    [1] = { .floated_at_least = 200, .random_mask = 0xF, .base = 50 },
    [2] = { .floated_at_least = 100, .random_mask = 0x1F, .base = 100 },
    [3] = { .floated_at_least = 50, .random_mask = 0x3F, .base = 200 },
    [4] = { .floated_at_least = 0, .random_mask = 0x7FFF, .base = 300 },
};

static int32_t flotsam_restart_wait_ticks(int16_t floated)
{
    const flotsam_restart_wait *step = &flotsam_restart_waits[FLOTSAM_RESTART_STEPS - 1];
    for (int32_t i = 0; i < FLOTSAM_RESTART_STEPS; i++) {
        if (floated >= flotsam_restart_waits[i].floated_at_least) {
            step = &flotsam_restart_waits[i];
            break;
        }
    }
    return (crt_rand() & step->random_mask) + step->base;
}

void figure_flotsam_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->is_boat = FIGURE_BOAT_FLOTSAM;
    int32_t speed = figure_properties[f->type].speed;
    if (scenario_data.river_exit_x == -1 || scenario_data.river_exit_y == -1) {
        return;
    }
    f->is_ghost = 0;
    f->cart_image_id = 0;
    f->terrain_usage = TERRAIN_USAGE_ANY;
    switch (f->action_state) {
        case FIGURE_ACTION_FLOTSAM_CREATED:
            f->is_ghost = 1;
            if (--f->wait_ticks < 1) {
                f->action_state = FIGURE_ACTION_FLOTSAM_FLOATING;
                f->wait_ticks = 0;
                city_player *player = city_player_current();
                if (player->flotsam_special != 0 && f->flotsam_kind == FLOTSAM_KIND_BOBBING) {
                    f->flotsam_special = 1;
                    player->flotsam_special = 0;
                }
                f->destination_x = (uint8_t) scenario_data.river_exit_x;
                f->destination_y = (uint8_t) scenario_data.river_exit_y;
            }
            break;
        case FIGURE_ACTION_FLOTSAM_FLOATING:
            if (f->flotsam_skip_tick == 0) {
                f->flotsam_skip_tick = 1;
                f->wait_ticks++;
                figure_movement_move_ticks(figure_id, speed);
                f->is_ghost = 0;
                f->height_adjusted_ticks = 0;
                if (f->direction == DIRECTION_AT_DESTINATION || f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                    f->action_state = FIGURE_ACTION_FLOTSAM_OFF_MAP;
                }
            } else {
                f->flotsam_skip_tick = 0;
            }
            break;
        case FIGURE_ACTION_FLOTSAM_OFF_MAP: {
            f->is_ghost = 1;
            f->flotsam_special = 0;
            f->action_state = FIGURE_ACTION_FLOTSAM_CREATED;
            f->wait_ticks = (int16_t) flotsam_restart_wait_ticks(f->wait_ticks);
            map_figure_delete(figure_id);
            f->x = (uint8_t) scenario_data.river_entry_x;
            f->y = (uint8_t) scenario_data.river_entry_y;
            f->grid_offset = map_grid_offset(f->x, f->y);
            f->cross_country.x = (int16_t) (f->x * 15);
            f->cross_country.y = (int16_t) (f->y * 15);
            break;
        }
        default:
            f->action_state = FIGURE_ACTION_FLOTSAM_CREATED;
            break;
    }

    switch (f->flotsam_kind) {
        case FLOTSAM_KIND_BOBBING:
            advance_frame(f, FLOTSAM_BOB_FRAMES);
            f->image_id = flotsam_bob_frames[f->animation_frame] +
                (f->flotsam_special ? image_group(IMAGE_GROUP_FIGURE_FLOTSAM_SPECIAL) : image_group(IMAGE_GROUP_FIGURE_FLOTSAM));
            break;
        case FLOTSAM_KIND_DEBRIS_A:
            advance_frame(f, FLOTSAM_DEBRIS_FRAMES);
            f->image_id = flotsam_debris_frames[f->animation_frame] + image_group(IMAGE_GROUP_FIGURE_FLOTSAM_DEBRIS_A);
            break;
        case FLOTSAM_KIND_DEBRIS_B:
            advance_frame(f, FLOTSAM_DEBRIS_FRAMES);
            f->image_id = flotsam_debris_frames[f->animation_frame] + image_group(IMAGE_GROUP_FIGURE_FLOTSAM_DEBRIS_B);
            break;
        case FLOTSAM_KIND_SINKING: {
            advance_frame(f, FLOTSAM_DEBRIS_FRAMES);
            int32_t frame = flotsam_sinking_frames[f->animation_frame];
            f->image_id = (frame == FLOTSAM_SUNK_FRAME) ? 0 : (frame + image_group(IMAGE_GROUP_FIGURE_FLOTSAM_DEBRIS_C));
            break;
        }
    }
    if (editor_active == 1 && !scenario_data.flotsam_enabled) {
        f->image_id = 0;
    }
}

flotsam_start flotsam_starts[FLOTSAM_COUNT] = {
    [0] = { .kind = FLOTSAM_KIND_SINKING, .wait = 10 },
    [1] = { .kind = FLOTSAM_KIND_DEBRIS_A, .wait = 50 },
    [2] = { .kind = FLOTSAM_KIND_SINKING, .wait = 100 },
    [3] = { .kind = FLOTSAM_KIND_DEBRIS_B, .wait = 130 },
    [4] = { .kind = FLOTSAM_KIND_DEBRIS_A, .wait = 200 },
    [5] = { .kind = FLOTSAM_KIND_SINKING, .wait = 250 },
    [6] = { .kind = FLOTSAM_KIND_DEBRIS_B, .wait = 400 },
    [7] = { .kind = FLOTSAM_KIND_SINKING, .wait = 430 },
    [8] = { .kind = FLOTSAM_KIND_DEBRIS_B, .wait = 500 },
    [9] = { .kind = FLOTSAM_KIND_DEBRIS_A, .wait = 600 },
    [10] = { .kind = FLOTSAM_KIND_SINKING, .wait = 70 },
    [11] = { .kind = FLOTSAM_KIND_SINKING, .wait = 750 },
    [12] = { .kind = FLOTSAM_KIND_DEBRIS_B, .wait = 820 },
    [13] = { .kind = FLOTSAM_KIND_SINKING, .wait = 830 },
    [14] = { .kind = FLOTSAM_KIND_SINKING, .wait = 900 },
    [15] = { .kind = FLOTSAM_KIND_SINKING, .wait = 980 },
    [16] = { .kind = FLOTSAM_KIND_DEBRIS_A, .wait = 1010 },
    [17] = { .kind = FLOTSAM_KIND_DEBRIS_B, .wait = 1030 },
    [18] = { .kind = FLOTSAM_KIND_BOBBING, .wait = 1200 },
    [19] = { .kind = FLOTSAM_KIND_DEBRIS_A, .wait = 1300 },
};

void figure_create_flotsam(int32_t x, int32_t y, int32_t create)
{
    if (editor_active == 0) {
        for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
            int16_t px = scenario_data.fishing_point_x[i];
            if (px <= 0) {
                continue;
            }
            random_generate_next();
            int32_t point_id = figure_create(1, FIGURE_SEAGULLS, px, scenario_data.fishing_point_y[i], 0);
            figure *point = figure_get(point_id);
            point->animation_frame = (uint8_t) (crt_rand() & 0x1F);
            point->progress_on_tile = (int8_t) (crt_rand() & 7);
            int32_t current = figure_current_id;
            figure *c = figure_get(current);
            figure_movement_set_cross_country_direction(current, c->cross_country.x, c->cross_country.y, c->destination_x * 15, c->destination_y * 15, 0);
        }
    }
    if (create && (editor_active != 0 || scenario_data.flotsam_enabled)) {
        for (int32_t id = 1; id < MAX_FIGURES; id++) {
            figure *f = figure_get(id);
            if (f->state != FIGURE_STATE_NONE && f->type == FIGURE_FLOTSAM) {
                figure_delete(id);
            }
        }
        for (int32_t i = 0; i < FLOTSAM_COUNT; i++) {
            int32_t flotsam_id = figure_create(1, FIGURE_FLOTSAM, x, y, 0);
            figure *f = figure_get(flotsam_id);
            f->action_state = FIGURE_ACTION_FLOTSAM_CREATED;
            f->flotsam_kind = (uint8_t) flotsam_starts[i].kind;
            f->wait_ticks = (int16_t) flotsam_starts[i].wait;
        }
    }
}
