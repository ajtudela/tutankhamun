// Boats: the fishing boats and the fishing spots, the ferry boats with their passenger queues and path records, and
// the searches for docks and fishing tiles.
//
// The docks, fishing points and ferry queues behave like this, slips included:
// - when no dock is free the last dock of the list is claimed all the same;
// - the closest fishing point's coordinates are cut to their low byte;
// - ferry_queue_release counts against the shrinking queue, so only about half of the waiting figures are released;
// - passengers going ashore set progress_on_tile again for each of their followers.

#include "figure/boats.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/ferry_landing.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "core/crt_compat.h"
#include "core/queue.h"
#include "core/random.h"
#include "figure/carriers.h"
#include "figure/carriers_internal.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/laborer.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "figure/natives.h"
#include "figure/route.h"
#include "figure/trade.h"
#include "graphics/image_collection.h"
#include "map/construction_terrain.h"
#include "map/ferry.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "map/routing_water.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include <stdbool.h>
#include <stdint.h>

int32_t building_dock_index(int32_t building_id)
{
    const city_player *player = city_player_current();
    if (building_count_total[BUILDING_DOCK] > 0) {
        for (int32_t i = 0; i < CITY_DOCKS; i++) {
            if (building_id == player->working_dock_ids[i]) {
                return i;
            }
        }
    }
    return -1;
}

int32_t building_dock_get_free_destination(int32_t figure_id)
{
    const city_player *player = city_player_current();
    if (building_count_total[BUILDING_DOCK] <= 0) {
        return 0;
    }
    int32_t index = figure_get(figure_id)->ship_dock_index;
    if (index >= CITY_DOCKS) {
        return 0;
    }
    int32_t dock_id = 0;
    for (; index < CITY_DOCKS; index++) {
        dock_id = player->working_dock_ids[index];
        if (dock_id > 0) {
            int32_t ship = building_get(dock_id)->data.dock.ship_id;
            if (ship == 0 || ship == figure_id) {
                break;
            }
        }
    }
    if (dock_id <= 0) {
        return 0;
    }
    building *dock = building_get(dock_id);
    switch (dock->data.dock.orientation) {
        case DOCK_WATER_ABOVE:
            map_result_x = dock->x + 1;
            dock->data.dock.ship_id = (int16_t) figure_id;
            map_result_y = dock->y - 1;
            return dock_id;
        case DOCK_WATER_RIGHT:
            map_result_x = dock->x + 3;
            break;
        case DOCK_WATER_BELOW:
            map_result_x = dock->x + 1;
            dock->data.dock.ship_id = (int16_t) figure_id;
            map_result_y = dock->y + 3;
            return dock_id;
        default:
            map_result_x = dock->x - 1;
            break;
    }
    dock->data.dock.ship_id = (int16_t) figure_id;
    map_result_y = dock->y + 1;
    return dock_id;
}

static void dock_queue_tile(const building *dock, int32_t distance)
{
    switch (dock->data.dock.orientation) {
        case DOCK_WATER_ABOVE:
            map_result_x = dock->x + 1;
            map_result_y = dock->y - distance;
            break;
        case DOCK_WATER_RIGHT:
            map_result_x = dock->x + distance + 2;
            map_result_y = dock->y + 1;
            break;
        case DOCK_WATER_BELOW:
            map_result_x = dock->x + 1;
            map_result_y = dock->y + distance + 2;
            break;
        default:
            map_result_x = dock->x - distance;
            map_result_y = dock->y + 1;
            break;
    }
}

int32_t building_dock_get_queue_destination(int32_t figure_id)
{
    const city_player *player = city_player_current();
    if (building_count_total[BUILDING_DOCK] <= 0) {
        return 0;
    }
    int32_t first = figure_get(figure_id)->ship_dock_index;
    for (int32_t distance = DOCK_QUEUE_NEAR; distance <= DOCK_QUEUE_FAR; distance++) {
        for (int32_t index = first; index < CITY_DOCKS; index++) {
            int32_t dock_id = player->working_dock_ids[index];
            if (dock_id > 0) {
                dock_queue_tile(building_get(dock_id), distance);
                if (map_grids.figure_ids[map_grid_offset(map_result_x, map_result_y)] == 0) {
                    return dock_id;
                }
            }
        }
    }
    return 0;
}

int32_t scenario_map_closest_fishing_point(int32_t figure_id)
{
    int32_t count = 0;
    for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
        if (scenario_data.fishing_point_x[i] > 0) {
            count++;
        }
    }
    if (count <= 0) {
        return 0;
    }
    int32_t best = 10000;
    int32_t best_index = 0;
    for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
        int32_t px = scenario_data.fishing_point_x[i];
        if (px <= 0) {
            continue;
        }
        int32_t py = scenario_data.fishing_point_y[i];
        const figure *f = figure_get(figure_id);
        int32_t distance = calc_maximum_distance(f->x, f->y, px, py);
        if (distance < best) {
            best = distance;
            best_index = i;
        }
    }
    if (best >= 10000) {
        return 0;
    }
    map_result_x = (uint8_t) scenario_data.fishing_point_x[best_index];
    map_result_y = (uint8_t) scenario_data.fishing_point_y[best_index];
    return 1;
}

int32_t map_water_find_alternative_fishing_boat_tile(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    if (map_grids.figure_ids[f->grid_offset] == figure_id) {
        return 1;
    }
    for (int32_t radius = 1; radius <= FISHING_BOAT_FREE_TILE_RADIUS; radius++) {
        int32_t x_max = f->x + radius;
        int32_t y_max = f->y + radius;
        int32_t x_min = f->x - radius;
        int32_t y_min = f->y - radius;
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
        int32_t grid_offset = map_grid_offset(x_min, y_min);
        for (; map_result_y <= y_max; map_result_y++) {
            map_result_x = x_min;
            if (x_min <= x_max) {
                do {
                    if ((map_grids.terrain[grid_offset] & TERRAIN_WATER) && map_grids.figure_ids[grid_offset] == 0) {
                        return 0;
                    }
                    map_result_x++;
                    grid_offset++;
                } while (map_result_x <= x_max);
            }
            grid_offset += x_min - x_max + (GRID_SIZE - 1);
        }
    }
    return 1;
}

static void boat_sink(figure *f)
{
    if (f->action_state != FIGURE_ACTION_CORPSE) {
        f->wait_ticks = -1;
    }
    f->action_state = FIGURE_ACTION_CORPSE;
}

static void fishing_boat_go_to_wharf(int32_t wharf_id)
{
    const building *wharf = building_get(wharf_id);
    map_water_tile_next_to(wharf->x, wharf->y, wharf->size);
    figure *f = figure_get(figure_current_id);
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    f->source_x = (int16_t) map_result_x;
    f->source_y = (int16_t) map_result_y;
    figure_route_remove(figure_current_id);
}

void figure_fishing_boat_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    int32_t home_id = f->building_id;
    building *home = building_get(home_id);
    if (home->state != BUILDING_STATE_IN_USE) {
        boat_sink(f);
    }
    if (f->ship_hit_points <= 0) {
        boat_sink(f);
    }
    if (f->action_state != FIGURE_ACTION_FISHING_BOAT_CREATED && f->action_state != FIGURE_ACTION_CORPSE && home->data.industry.worker_figure_id != figure_id) {
        int32_t wharf_id = map_water_get_wharf_for_new_fishing_boat(figure_id);
        if (wharf_id) {
            building *wharf = building_get(wharf_id);
            wharf->data.industry.worker_figure_id = (int16_t) figure_id;
            f->building_id = (int16_t) wharf_id;
            f->action_state = FIGURE_ACTION_FISHING_BOAT_GOING_TO_WHARF;
            home_id = wharf_id;
            fishing_boat_go_to_wharf(wharf_id);
        } else {
            boat_sink(f);
        }
    }
    f->is_ghost = 0;
    f->is_boat = FIGURE_BOAT_SHIP;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= BOAT_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_FISHING_BOAT_CREATED:
            if (++f->wait_ticks >= FISHING_BOAT_WAIT_TICKS) {
                f->wait_ticks = 0;
                int32_t wharf_id = f->building_id;
                if (wharf_id != 0) {
                    f->action_state = FIGURE_ACTION_FISHING_BOAT_GOING_TO_WHARF;
                    building_get(wharf_id)->data.industry.worker_figure_id = (int16_t) figure_id;
                    fishing_boat_go_to_wharf(home_id);
                }
            }
            break;
        case FIGURE_ACTION_FISHING_BOAT_GOING_TO_FISH:
            figure_movement_move_ticks(figure_id, speed);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIRECTION_AT_DESTINATION) {
                if (!map_water_find_alternative_fishing_boat_tile(figure_id)) {
                    figure_route_remove(figure_id);
                    f->destination_x = (int16_t) map_result_x;
                    f->destination_y = (int16_t) map_result_y;
                    f->direction = f->previous_tile_direction;
                } else {
                    f->action_state = FIGURE_ACTION_FISHING_BOAT_FISHING;
                    f->wait_ticks = 0;
                }
            } else if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                if (f->ship_hit_points > 0) {
                    f->ship_hit_points--;
                }
                f->action_state = FIGURE_ACTION_FISHING_BOAT_AT_WHARF;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
            }
            break;
        case FIGURE_ACTION_FISHING_BOAT_FISHING:
            if (++f->wait_ticks >= FISHING_BOAT_FISHING_TICKS) {
                f->wait_ticks = 0;
                f->action_state = FIGURE_ACTION_FISHING_BOAT_RETURNING_WITH_FISH;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                figure_route_remove(figure_id);
            }
            break;
        case FIGURE_ACTION_FISHING_BOAT_GOING_TO_WHARF:
            figure_movement_move_ticks(figure_id, speed);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_FISHING_BOAT_AT_WHARF;
                f->wait_ticks = 0;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                boat_sink(f);
            }
            break;
        case FIGURE_ACTION_FISHING_BOAT_AT_WHARF: {
            const building *wharf = building_get(home_id);
            int32_t pct = calc_percentage(wharf->num_workers, building_model_table[wharf->type].laborers);
            int32_t delay = (FISHING_BOAT_DELAY_BASE - pct) * FISHING_BOAT_DELAY_STEP;
            if (wharf->data.industry.fish_caught > 0) {
                pct = 0;
            }
            if (map_sweep_countdown != 0 || pct <= 0) {
                break;
            }
            if (++f->wait_ticks < delay) {
                break;
            }
            f->wait_ticks = 0;
            if (scenario_map_closest_fishing_point(figure_id)) {
                f->action_state = FIGURE_ACTION_FISHING_BOAT_GOING_TO_FISH;
                f->destination_x = (int16_t) map_result_x;
                f->destination_y = (int16_t) map_result_y;
                figure_route_remove(figure_id);
            }
            break;
        }
        case FIGURE_ACTION_FISHING_BOAT_RETURNING_WITH_FISH:
            figure_movement_move_ticks(figure_id, speed);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_FISHING_BOAT_AT_WHARF;
                f->wait_ticks = 0;
                building *wharf = building_get(home_id);
                wharf->figure_spawn_delay = 1;
                wharf->data.industry.fish_caught++;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                boat_sink(f);
            }
            break;
        case FIGURE_ACTION_CORPSE:
            break;
        default:
            f->action_state = FIGURE_ACTION_FISHING_BOAT_CREATED;
            break;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        figure_combat_handle_corpse(figure_id);
    }
    int32_t direction = (f->direction >= DIRECTION_COUNT) ? f->previous_tile_direction : f->direction;
    int32_t dir = direction - map_info.orientation;
    figure_image_direction = dir;
    if (f->action_state == FIGURE_ACTION_FISHING_BOAT_FISHING) {
        dir -= FISHING_BOAT_FISHING_TURN;
        figure_image_direction = dir;
    }
    if (dir < 0) {
        dir += DIRECTION_COUNT;
        figure_image_direction = dir;
    }
    switch (f->action_state) {
        case FIGURE_ACTION_FISHING_BOAT_GOING_TO_FISH:
        case FIGURE_ACTION_FISHING_BOAT_GOING_TO_WHARF:
        case FIGURE_ACTION_FISHING_BOAT_RETURNING_WITH_FISH:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_FISHING_BOAT) + (f->animation_frame / BOAT_TICKS_PER_IMAGE) * DIRECTION_COUNT + dir;
            break;
        case FIGURE_ACTION_FISHING_BOAT_FISHING:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_FISHING_BOAT_FISHING) + (f->wait_ticks % FISHING_BOAT_FISHING_IMAGES) * DIRECTION_COUNT + dir;
            break;
        case FIGURE_ACTION_CORPSE:
            if (f->wait_ticks >= BOAT_SINKING_TICKS) {
                int32_t frame_dead = f->wait_ticks - BOAT_SINKING_TICKS;
                if (frame_dead >= BOAT_BUBBLES_FRAMES) {
                    frame_dead = BOAT_BUBBLES_FRAMES - 1;
                }
                f->image_id = image_group(IMAGE_GROUP_FIGURE_BUBBLES) + frame_dead;
            } else {
                f->image_id = (dir / DIRECTION_COUNT) + image_group(IMAGE_GROUP_FIGURE_FISHING_BOAT_SINKING) + (f->wait_ticks / BOAT_TICKS_PER_IMAGE);
            }
            break;
        default:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_FISHING_BOAT_IDLE) + dir;
            break;
    }
}

void figure_fishing_spot_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->is_ghost = 0;
    f->is_boat = FIGURE_BOAT_SHIP;
    f->cart_image_id = 0;
    if (f->animation_frame < FISHING_SPOT_FRAMES) {
        f->animation_frame++;
        random_generate_next();
        int32_t r = crt_rand();
        f->spot_wait_ticks = (uint8_t) (r % FISHING_SPOT_RANDOM_PAUSE);
    } else {
        f->animation_frame = FISHING_SPOT_FRAMES;
        f->wait_ticks++;
        if (f->wait_ticks >= f->spot_wait_ticks + FISHING_SPOT_PAUSE) {
            f->wait_ticks = 0;
            f->animation_frame = 0;
        }
    }
    f->image_id = f->animation_frame + image_group(IMAGE_GROUP_FIGURE_FISHING_SPOT);
}

int32_t ferry_passenger_count(int32_t figure_id)
{
    const int32_t *passengers = ferry_path_get(figure_get(figure_id)->ferry_path_index)->passengers;
    int32_t count = 0;
    for (int32_t i = 0; i < FERRY_PASSENGERS; i++) {
        if (passengers[i] > 0) {
            count++;
        }
    }
    return count;
}

int32_t figure_train_leader(int32_t figure_id)
{
    int32_t steps = figure_id;
    return figure_find_train_leader(figure_id, &steps);
}

bool figure_is_queued_for_ferry(int32_t figure_id)
{
    int32_t state = figure_get(figure_id)->action_state;
    if (state == FIGURE_ACTION_ON_FERRY || state != FIGURE_ACTION_WAIT_FOR_FERRY) {
        return 1;
    }
    for (int32_t i = 0; i < FERRY_QUEUES; i++) {
        ring_queue *queue = &ferry_queue_get(i)->queue;
        if (queue->in_use && ring_queue_contains(queue, figure_id)) {
            return 1;
        }
    }
    return 0;
}

void ferry_check_passengers(void)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_NONE || f->action_state != FIGURE_ACTION_ON_FERRY) {
            continue;
        }
        int32_t path = 0;
        int32_t found = 0;
        for (; path < FERRY_PATHS && !found; path++) {
            if (ferry_path_get(path)->in_use) {
                for (int32_t i = 0; i < FERRY_PASSENGERS; i++) {
                    if (ferry_path_get(path)->passengers[i] == id) {
                        found = 1;
                        break;
                    }
                }
            }
        }
        if (found) {
            path--;
            int32_t boat_alive = 0;
            for (int32_t boat_id = 1; boat_id < MAX_FIGURES; boat_id++) {
                const figure *boat = figure_get(boat_id);
                if (boat->state != FIGURE_STATE_NONE && boat->type == FIGURE_FERRY_BOAT && boat->ferry_path_index == path) {
                    boat_alive = 1;
                    break;
                }
            }
            if (boat_alive) {
                continue;
            }
        }
        f->state = FIGURE_STATE_DEAD;
    }
}

void ferry_queue_release(int32_t building_id)
{
    ring_queue *queue = &ferry_queue_get(building_get(building_id)->queue_index)->queue;
    if (ring_queue_size(queue) == 0) {
        return;
    }
    for (int32_t i = 0; i < ring_queue_size(queue); i++) {
        int32_t id = ring_queue_pop(queue);
        figure *f = figure_get(id);
        f->action_state = f->action_state_before_attack;
        figure_route_remove(id);
    }
    ring_queue_clear(queue);
}

void ferry_queues_release(int32_t building_id)
{
    int32_t other = building_get(building_id)->linked_building_id;
    ferry_queue_release(building_id);
    if (other > 0) {
        ferry_queue_release(other);
    }
}

static int32_t follower_leader(int32_t id, int32_t type)
{
    switch (type) {
        case FIGURE_TRADE_CARAVAN_DONKEY:
            return caravan_of_donkey(id);
        case FIGURE_DELIVERY_BOY:
            return delivery_boy_find_buyer(id);
        case FIGURE_SLED:
        case FIGURE_SLED_PULLER:
            return figure_train_leader(id);
    }
    return -1;
}

void ferry_load(int32_t figure_id, int32_t building_id, int32_t max)
{
    ring_queue *queue = &ferry_queue_get(building_get(building_id)->queue_index)->queue;
    int32_t path = figure_get(figure_id)->ferry_path_index;
    if (max >= ring_queue_size(queue)) {
        max = ring_queue_size(queue);
    }
    for (int32_t i = 0; i < max; i++) {
        int32_t passenger_id = ring_queue_pop(queue);
        figure *p = figure_get(passenger_id);
        if (p->state != FIGURE_STATE_ALIVE || p->action_state != FIGURE_ACTION_WAIT_FOR_FERRY) {
            continue;
        }
        p->action_state = FIGURE_ACTION_ON_FERRY;
        p->image_id = 0;
        ferry_path_get(path)->passengers[i] = passenger_id;
        if (p->cart_image_id != 0) {
            p->cart_image_id = 0;
        }
        figure_route_remove(passenger_id);
        map_figure_delete(passenger_id);
        for (int32_t id = 1; id < MAX_FIGURES; id++) {
            int32_t leader = follower_leader(id, figure_get(id)->type);
            if (leader != -1 && leader == passenger_id) {
                figure_route_remove(id);
                map_figure_delete(id);
            }
        }
    }
}

void ferry_unload(int32_t figure_id, int32_t building_id)
{
    int32_t path = figure_get(figure_id)->ferry_path_index;
    int32_t count = ferry_passenger_count(figure_id);
    if (count <= 0) {
        return;
    }
    int32_t *slot = ferry_path_get(path)->passengers;
    for (; count != 0; count--, slot++) {
        int32_t passenger_id = *slot;
        *slot = -1;
        figure *p = figure_get(passenger_id);
        if (p->state != FIGURE_STATE_ALIVE || p->action_state != FIGURE_ACTION_ON_FERRY) {
            continue;
        }
        const building *landing = building_get(building_id);
        map_has_road_access(landing->x, landing->y, FERRY_LANDING_SIZE);
        int32_t x = map_result_x;
        int32_t y = map_result_y;
        p->previous_tile_x = (int16_t) x;
        p->x = (int16_t) x;
        p->previous_tile_y = (int16_t) y;
        p->y = (int16_t) y;
        p->grid_offset = map_grid_offset(x, y);
        p->cross_country.x = (int16_t) (p->x * 15);
        p->cross_country.y = (int16_t) (p->y * 15);
        p->action_state = p->action_state_before_attack;
        p->progress_on_tile = 15;
        map_figure_add(passenger_id);
        for (int32_t id = 1; id < MAX_FIGURES; id++) {
            figure *follower = figure_get(id);
            int32_t leader = follower_leader(id, follower->type);
            if (leader == -1 || leader != passenger_id) {
                continue;
            }
            follower->x = p->x;
            follower->previous_tile_x = p->x;
            follower->y = p->y;
            follower->previous_tile_y = p->y;
            follower->grid_offset = map_info.grid_start + p->x + (p->y * GRID_SIZE);
            follower->cross_country.x = (int16_t) (follower->x * 15);
            follower->cross_country.y = (int16_t) (follower->y * 15);
            p->progress_on_tile = 15;
            map_figure_add(id);
        }
    }
}

void ferry_move(int32_t figure_id, int32_t slowness)
{
    if (slowness < FERRY_SLOWNESS_MIN) {
        figure_movement_walk_ticks(figure_id, 1);
        return;
    }
    figure *f = figure_get(figure_id);
    int16_t ticks = ++f->wait_ticks;
    if (ticks % slowness != 0) {
        figure_movement_walk_ticks(figure_id, 1);
        return;
    }
    if (ticks > FERRY_TICK_COUNTER_LIMIT) {
        f->wait_ticks = 0;
    }
}

static void ferry_sink(int32_t path)
{
    figure *f = figure_get(figure_current_id);
    if (f->action_state != FIGURE_ACTION_CORPSE) {
        f->wait_ticks = -1;
    }
    f->action_state = FIGURE_ACTION_CORPSE;
    ferry_path_get(path)->in_use = 0;
}

static int32_t queue_load_count(ring_queue *queue)
{
    return (ring_queue_size(queue) < FERRY_PASSENGERS) ? ring_queue_size(queue) : FERRY_PASSENGERS;
}

void figure_ferry_boat_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->is_boat = FIGURE_BOAT_SHIP;
    int32_t home_id = f->building_id;
    building *home = building_get(home_id);
    int32_t other_id = home->linked_building_id;
    int32_t home_queue = home->queue_index;
    building *other = building_get(other_id);
    int32_t other_ferry = other->figure_id;
    int32_t other_queue = other->queue_index;
    int32_t pct = calc_percentage(home->num_workers, building_model_table[home->type].laborers);
    int32_t slowness = (100 - pct) / FERRY_SLOWNESS_STEP;
    int32_t path = f->ferry_path_index;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= BOAT_FRAMES) {
        f->animation_frame = 0;
    }
    int32_t state = f->action_state;
    switch (state) {
        case FIGURE_ACTION_CORPSE:
            f->state = FIGURE_STATE_DEAD;
            break;
        case FIGURE_ACTION_FERRY_BOAT_CREATED:
            f->action_state = (ferry_passenger_count(figure_id) > 0) ? FIGURE_ACTION_FERRY_BOAT_CROSSING : FIGURE_ACTION_FERRY_BOAT_CROSSING_BACK;
            if ((home->state == BUILDING_STATE_UNDO && other->state == BUILDING_STATE_UNDO) || home->num_workers == 0 || other->num_workers == 0) {
                ferry_sink(path);
            }
            break;
        case FIGURE_ACTION_FERRY_BOAT_CROSSING:
        case FIGURE_ACTION_FERRY_BOAT_CROSSING_BACK:
            ferry_move(figure_id, slowness);
            if (home->state == BUILDING_STATE_UNDO && other->state == BUILDING_STATE_UNDO) {
                ferry_sink(path);
                break;
            }
            if (f->direction == DIRECTION_AT_DESTINATION) {
                int32_t crossed_to_other = state == FIGURE_ACTION_FERRY_BOAT_CROSSING;
                f->action_state = crossed_to_other ? FIGURE_ACTION_FERRY_BOAT_UNLOAD_AT_OTHER : FIGURE_ACTION_FERRY_BOAT_UNLOAD_AT_HOME;
            } else if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
                f->action_state = FIGURE_ACTION_FERRY_BOAT_CREATED;
            }
            break;
        case FIGURE_ACTION_FERRY_BOAT_WAITING: {
            if (other_id <= 0 || home->state == BUILDING_STATE_UNDO || other->state == BUILDING_STATE_UNDO || home->num_workers == 0 ||
                other->num_workers == 0) {
                ferry_sink(path);
                break;
            }
            int32_t other_crossing = figure_get(other_ferry)->action_state == FIGURE_ACTION_FERRY_BOAT_CROSSING;
            ring_queue *queue = &ferry_queue_get(home_queue)->queue;
            int32_t go;
            if (ring_queue_size(queue) != 0) {
                go = ++f->wait_ticks > FERRY_WAIT_TICKS || ring_queue_size(queue) >= FERRY_PASSENGERS || other_crossing;
            } else {
                go = other_crossing;
            }
            if (!go) {
                break;
            }
            ferry_load(figure_id, home_id, queue_load_count(queue));
            f->destination_building_id = (int16_t) other_id;
            map_water_tile_next_to(other->x, other->y, other->size);
            f->destination_x = (int16_t) map_result_x;
            f->destination_y = (int16_t) map_result_y;
            f->source_x = f->x;
            f->source_y = f->y;
            f->action_state = FIGURE_ACTION_FERRY_BOAT_CROSSING;
            f->wait_ticks = 0;
            break;
        }
        case FIGURE_ACTION_FERRY_BOAT_UNLOAD_AT_HOME:
            ferry_unload(figure_id, home_id);
            f->action_state = FIGURE_ACTION_FERRY_BOAT_WAITING;
            break;
        case FIGURE_ACTION_FERRY_BOAT_LOAD_AT_OTHER: {
            ring_queue *queue = &ferry_queue_get(other_queue)->queue;
            if (ring_queue_size(queue) != 0) {
                ferry_load(figure_id, other_id, queue_load_count(queue));
            }
            f->destination_building_id = f->building_id;
            f->action_state = FIGURE_ACTION_FERRY_BOAT_CROSSING_BACK;
            f->wait_ticks = 0;
            f->destination_x = f->source_x;
            f->destination_y = f->source_y;
            break;
        }
        case FIGURE_ACTION_FERRY_BOAT_UNLOAD_AT_OTHER:
            ferry_unload(figure_id, other_id);
            f->action_state = FIGURE_ACTION_FERRY_BOAT_LOAD_AT_OTHER;
            break;
    }
    int32_t direction = (f->direction >= DIRECTION_COUNT) ? f->previous_tile_direction : f->direction;
    carrier_image_direction(direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->state = FIGURE_STATE_DEAD;
        figure_combat_handle_corpse(figure_id);
        int32_t i = 0;
        int32_t *slot = ferry_path_get(path)->passengers;
        int32_t count = ferry_passenger_count(figure_id);
        while (i < count) {
            if (*slot > 0) {
                int32_t passenger_id = *slot;
                *slot = -1;
                figure_combat_handle_corpse(passenger_id);
            }
            i++;
            slot++;
            count = ferry_passenger_count(figure_id);
        }
    }
    if (f->action_state != FIGURE_ACTION_CORPSE) {
        if (f->action_state > FIGURE_ACTION_FERRY_BOAT_CREATED && f->action_state <= FIGURE_ACTION_FERRY_BOAT_CROSSING_BACK) {
            int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_FERRY_BOAT_SWEPT : IMAGE_GROUP_FIGURE_FERRY_BOAT;
            f->image_id = image_group(group) + (f->animation_frame / BOAT_TICKS_PER_IMAGE) * DIRECTION_COUNT + figure_image_direction;
            return;
        }
        int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_FERRY_BOAT_SWEPT_IDLE : IMAGE_GROUP_FIGURE_FERRY_BOAT_IDLE;
        f->image_id = image_group(group) + figure_image_direction;
        return;
    }
    if (f->wait_ticks >= BOAT_SINKING_TICKS) {
        int32_t frame_dead = f->wait_ticks - BOAT_SINKING_TICKS;
        if (frame_dead >= BOAT_BUBBLES_FRAMES) {
            frame_dead = BOAT_BUBBLES_FRAMES - 1;
        }
        f->image_id = image_group(IMAGE_GROUP_FIGURE_BUBBLES) + frame_dead;
        return;
    }
    int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_FERRY_BOAT_SWEPT_SINKING : IMAGE_GROUP_FIGURE_FERRY_BOAT_SINKING;
    f->image_id = (figure_image_direction / DIRECTION_COUNT) + image_group(group) + (f->wait_ticks / BOAT_TICKS_PER_IMAGE);
}

int32_t figure_route_find_ferry_on_path(int32_t figure_id, int32_t ferry_building_id)
{
    const figure *f = figure_get(figure_id);
    const building *ferry = building_get(ferry_building_id);
    int32_t grid_offset = f->grid_offset;
    for (int32_t i = f->routing_path_current_tile; i < f->routing_path_length; i++) {
        int32_t building_id = map_grids.building_ids[grid_offset];
        if (building_get(building_id)->type == BUILDING_FERRY_LANDING && building_id == ferry->linked_building_id) {
            if (ferry->num_workers > 0 && building_get(ferry->linked_building_id)->num_workers > 0) {
                return FERRY_ON_PATH_STAFFED;
            }
            return FERRY_ON_PATH_UNSTAFFED;
        }
        grid_offset += map_grid_direction_delta[city_figures.route_paths[f->routing_path_id * 500 + i]];
    }
    return FERRY_NOT_ON_PATH;
}
