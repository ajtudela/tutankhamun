// Figure movement: walking a route tile by tile (with the checks of the tile ahead, ferries and bridges), roaming the
// roads, following a leader, the cross-country line and the missile line check; the counterpart of Julius'
// figure/movement.c. The tunable values are movement_rules from config/movement_rules.json.
//
// The movement has three rough edges:
// - of the open terrain only plain water blocks a walker (trees and rocks are not checked);
// - roam_ticks walks figure_current_id, not its argument, and never resets its attempt counter within a call;
// - a speed class outside 0..17, a negative one too, walks 3 steps.

#include "figure/movement.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/destroy_enemy.h"
#include "building/ferry_landing.h"
#include "core/calc.h"
#include "core/queue.h"
#include "figure/boats.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "figure/movement_internal.h"
#include "figure/service.h"
#include "game/game_data.h"
#include "map/bridge.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/road_access.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include <stdint.h>

int32_t movement_rules[MOVEMENT_RULE_COUNT] = {
    [MOVEMENT_RULE_BUILDING_MAX_DAMAGE] = 10,
    [MOVEMENT_RULE_WALL_MAX_DAMAGE] = 200,
    [MOVEMENT_RULE_GATEHOUSE_MAX_DAMAGE] = 400,
    [MOVEMENT_RULE_ROAM_START_DISTANCE] = 8,
    [MOVEMENT_RULE_ROAM_ROAD_RADIUS] = 6,
};

static const uint32_t route_tile_terrain = TERRAIN_NOT_CLEAR & ~(TERRAIN_RUBBLE | TERRAIN_GARDEN);

static inline int8_t inc_progress(figure *f)
{
    f->progress_on_tile = (int8_t) (f->progress_on_tile + 1);
    return f->progress_on_tile;
}

void figure_movement_set_next_route_tile_direction(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->routing_path_id <= 0) {
        int8_t direction = (int8_t) calc_general_direction(f->x, f->y, f->destination_x, f->destination_y);
        f->direction = direction;
        if (direction != DIRECTION_AT_DESTINATION) {
            f->direction = DIRECTION_LOST;
        }
    } else if (f->routing_path_current_tile >= f->routing_path_length) {
        figure_route_remove(figure_id);
        f->direction = DIRECTION_AT_DESTINATION;
    } else {
        f->direction = (int8_t) city_figures.route_paths[f->routing_path_id * MAX_PATH_LENGTH + f->routing_path_current_tile];
    }
}

static void move_to_nearest_free_land_tile(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t x = f->x;
    int32_t y = f->y;
    map_find_nearest_free_land_tile(&x, &y);
    map_figure_delete(figure_id);
    f->x = (int16_t) x;
    f->y = (int16_t) y;
    f->grid_offset = map_grid_offset(x, y);
    f->progress_on_tile = MOVEMENT_STEPS_PER_TILE;
    map_figure_add(figure_id);
}

static void attack_tile(figure *f, int8_t direction, int32_t grid_offset, int32_t max_damage)
{
    f->attack_direction = direction;
    f->direction = DIRECTION_ATTACK;
    if ((game_time_tick & 3) == 0) {
        building_destroy_increase_enemy_damage(grid_offset, max_damage);
    }
}

static int32_t uses_ferry(int32_t type)
{
    switch (type) {
        case FIGURE_CART_PUSHER:
        case FIGURE_STORAGE_YARD_CART:
        case FIGURE_ARCHER:
        case FIGURE_INFANTRY:
        case FIGURE_CHARIOTEER:
        case FIGURE_JUGGLER:
        case FIGURE_MUSICIAN:
        case FIGURE_DANCER:
        case FIGURE_SENET_PLAYER:
        case FIGURE_TRADE_CARAVAN:
        case FIGURE_DOCKER:
        case FIGURE_MARKET_BUYER:
        case FIGURE_TOWER_SENTRY:
        case FIGURE_HUNTER:
        case FIGURE_LUMBERJACK:
        case FIGURE_CARPENTER:
        case FIGURE_BRICKLAYER:
        case FIGURE_STONEMASON:
        case FIGURE_LABORER:
        case FIGURE_REED_GATHERER:
        case FIGURE_ZOOKEEPER:
        case FIGURE_TOMB_ARTISAN:
            return 1;
        default:
            return 0;
    }
}

static void enter_ferry(int32_t figure_id, int32_t building_id)
{
    figure *f = figure_get(figure_id);
    const building *b = building_get(building_id);
    int32_t queue_index = b->queue_index;
    int32_t partner_id = b->linked_building_id;
    int32_t found = figure_route_find_ferry_on_path(figure_id, building_id);
    if (found == FERRY_ON_PATH_STAFFED) {
        ring_queue *queue = &ferry_queue_get(queue_index)->queue;
        if (ring_queue_is_full(queue) == 0 && b->num_workers > 0 && building_get(partner_id)->num_workers > 0) {
            if (f->action_state == FIGURE_ACTION_WAIT_FOR_FERRY) {
                return;
            }
            ring_queue_push(queue, &figure_id);
            f->action_state_before_attack = f->action_state;
            f->action_state = FIGURE_ACTION_WAIT_FOR_FERRY;
            figure_route_remove(figure_id);
            figure_get(b->figure_id)->wait_ticks = 0;
            return;
        }
        figure_route_remove(figure_id);
        move_to_nearest_free_land_tile(figure_id);
        switch (f->type) {
            case FIGURE_CART_PUSHER:
            case FIGURE_STORAGE_YARD_CART:
            case FIGURE_JUGGLER:
            case FIGURE_MUSICIAN:
            case FIGURE_DANCER:
            case FIGURE_SENET_PLAYER:
            case FIGURE_TRADE_CARAVAN:
            case FIGURE_DOCKER:
            case FIGURE_LUMBERJACK:
            case FIGURE_CARPENTER:
            case FIGURE_BRICKLAYER:
            case FIGURE_STONEMASON:
            case FIGURE_LABORER:
            case FIGURE_REED_GATHERER:
            case FIGURE_ZOOKEEPER:
                f->action_state = MOVEMENT_ACTION_START_OVER;
                break;
            case FIGURE_MARKET_BUYER:
                f->action_state = FIGURE_ACTION_MARKET_BUYER_RETURNING;
                break;
            case FIGURE_HUNTER:
                f->action_state = FIGURE_ACTION_HUNTER_CREATED;
                break;
            case FIGURE_ARCHER:
            case FIGURE_INFANTRY:
            case FIGURE_CHARIOTEER:
            case FIGURE_TOWER_SENTRY:
                f->state = FIGURE_STATE_DEAD;
                break;
            default:
                break;
        }
        return;
    }
    if (found == FERRY_NOT_ON_PATH) {
        for (int32_t i = 0; i < MOVEMENT_FERRY_PASS_TILES; i++) {
            figure_movement_set_next_route_tile_direction(figure_id);
            f->previous_tile_direction = f->direction;
            f->routing_path_current_tile++;
            figure_movement_move_to_next_tile(figure_id);
            if (f->direction >= DIRECTION_AT_DESTINATION) {
                return;
            }
        }
        return;
    }
    f->direction = DIRECTION_REROUTE;
}

void figure_movement_advance_route_tile(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int8_t direction = f->direction;
    if (direction >= DIRECTION_AT_DESTINATION) {
        return;
    }
    if (f->terrain_usage == TERRAIN_USAGE_ANYWHERE) {
        return;
    }
    int32_t target = f->grid_offset + map_grid_direction_delta[direction];
    int32_t building_id = map_grids.building_ids[target];
    const building *b = building_get(building_id);
    uint32_t terrain = map_grids.terrain[target];
    uint32_t masked = terrain & route_tile_terrain;
    if (f->is_boat) {
        if ((masked & TERRAIN_WATER) && routing_grids.water[target] >= 0) {
            return;
        }
        f->direction = DIRECTION_REROUTE;
        return;
    }
    if (f->terrain_usage == TERRAIN_USAGE_ENEMY) {
        int32_t value = routing_grids.land_noncitizen[target];
        if (value <= ROUTING_NONCITIZEN_BLOCKED) {
            f->direction = DIRECTION_REROUTE;
        } else if (value == ROUTING_NONCITIZEN_BUILDING) {
            attack_tile(f, direction, target, movement_rules[MOVEMENT_RULE_BUILDING_MAX_DAMAGE]);
        } else if (value == ROUTING_NONCITIZEN_ACCESS_RAMP) {
            if (!(terrain & TERRAIN_ACCESS_RAMP)) {
                attack_tile(f, direction, target, movement_rules[MOVEMENT_RULE_BUILDING_MAX_DAMAGE]);
            }
        } else if (value != ROUTING_NONCITIZEN_PASSABLE && value != ROUTING_NONCITIZEN_FORT) {
            int32_t is_wall = value == ROUTING_NONCITIZEN_WALL;
            attack_tile(f, direction, target, movement_rules[is_wall ? MOVEMENT_RULE_WALL_MAX_DAMAGE : MOVEMENT_RULE_GATEHOUSE_MAX_DAMAGE]);
        }
        return;
    }
    if (f->terrain_usage == TERRAIN_USAGE_WALLS) {
        if (routing_grids.walls[target] < ROUTING_WALL_PASSABLE) {
            f->direction = DIRECTION_REROUTE;
        }
        return;
    }
    int8_t citizen = routing_grids.land_citizen[target];
    if (f->terrain_usage == TERRAIN_USAGE_AMPHIBIA) {
        if (citizen == ROUTING_CITIZEN_BLOCKED && !(terrain & (TERRAIN_WATER | TERRAIN_FLOODPLAIN))) {
            f->direction = DIRECTION_REROUTE;
        }
        return;
    }
    if (citizen == ROUTING_CITIZEN_FERRY_STAFFED || citizen == ROUTING_CITIZEN_FERRY_UNSTAFFED) {
        int32_t type = f->type;
        if (type == FIGURE_NONE || (type > FIGURE_HOMELESS && type != FIGURE_FUNERAL_WALKER)) {
            move_to_nearest_free_land_tile(figure_id);
            figure_route_remove(figure_id);
            f->direction = DIRECTION_REROUTE;
        }
    }
    if ((masked & TERRAIN_ROAD) || (masked & (TERRAIN_WALKABLE_BUILDING | TERRAIN_ACCESS_RAMP))) {
        if (!figure_roaming_enabled || !(masked & TERRAIN_BUILDING)) {
            return;
        }
        switch (b->type) {
            case BUILDING_MUD_GATEHOUSE:
            case BUILDING_BRICK_GATEHOUSE:
            case BUILDING_GATEHOUSE:
            case BUILDING_MUD_GATEHOUSE_UP:
            case BUILDING_BRICK_GATEHOUSE_UP:
            case BUILDING_GATEHOUSE_UP:
            case BUILDING_ROADBLOCK:
                f->direction = DIRECTION_REROUTE;
                break;
            default:
                break;
        }
        return;
    }
    if (masked & (TERRAIN_WALL | TERRAIN_GATEHOUSE)) {
        f->direction = DIRECTION_REROUTE;
        return;
    }
    if (!(masked & TERRAIN_BUILDING)) {
        if (masked & TERRAIN_WATER) {
            f->direction = DIRECTION_REROUTE;
        }
        return;
    }
    if (map_grids.terrain[target] & TERRAIN_MONUMENT) {
        return;
    }
    int32_t building_type = b->type;
    if (building_type == BUILDING_TRIUMPHAL_ARCH) {
        return;
    }
    if (building_type == BUILDING_FORT_GROUND && f->terrain_usage != TERRAIN_USAGE_ANY) {
        return;
    }
    if (building_type == BUILDING_PYRAMID && f->terrain_usage == TERRAIN_USAGE_ROADS_OR_MONUMENT) {
        return;
    }
    if (building_type == BUILDING_FESTIVAL_SQUARE) {
        return;
    }
    if (building_type != BUILDING_FERRY_LANDING || figure_roaming_enabled) {
        f->direction = DIRECTION_REROUTE;
        return;
    }
    int32_t type = f->type;
    if (type == FIGURE_IMMIGRANT || type == FIGURE_EMIGRANT || type == FIGURE_HOMELESS || type == FIGURE_FUNERAL_WALKER) {
        return;
    }
    if (!uses_ferry(type)) {
        f->direction = DIRECTION_REROUTE;
        return;
    }
    enter_ferry(figure_id, building_id);
}

static int32_t fights_on_the_way(int32_t type)
{
    switch (type) {
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
        case FIGURE_ENEMY_TRANSPORT_SHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
            return 0;
        default:
            return 1;
    }
}

int32_t figure_bridge_height(int32_t sprite)
{
    switch (sprite) {
        case BRIDGE_SPRITE_NONE:
            return BRIDGE_HEIGHT_GROUND;
        case BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT:
        case BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM:
            return BRIDGE_HEIGHT_RAMP_FRONT;
        case BRIDGE_SPRITE_LOW_SPAN_ALONG_X:
            return BRIDGE_HEIGHT_LOW_SPAN_ALONG_X;
        case BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT:
        case BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP:
        case BRIDGE_SPRITE_SHIP_RAMP_DOWN_BOTTOM:
        case BRIDGE_SPRITE_SHIP_RAMP_DOWN_LEFT:
        case BRIDGE_SPRITE_SHIP_RAMP_DOWN_TOP:
        case BRIDGE_SPRITE_SHIP_RAMP_DOWN_RIGHT:
            return BRIDGE_HEIGHT_RAMP_BACK;
        case BRIDGE_SPRITE_LOW_SPAN_ALONG_Y:
        case BRIDGE_SPRITE_SHIP_SECOND_TILE:
            return BRIDGE_HEIGHT_LOW_SPAN_ALONG_Y;
        default:
            return BRIDGE_HEIGHT_SHIP_SPAN;
    }
}

void figure_movement_move_to_next_tile(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int16_t old_x = f->x;
    int16_t old_y = f->y;
    if (f->direction >= DIRECTION_AT_DESTINATION) {
        return;
    }
    map_figure_delete(figure_id);
    int32_t direction = f->direction;
    int32_t offset;
    switch (direction) {
        case DIRECTION_TOP:
            f->y--;
            offset = f->grid_offset - GRID_SIZE;
            break;
        case DIRECTION_TOP_RIGHT:
            f->y--;
            f->x++;
            offset = f->grid_offset - GRID_SIZE + 1;
            break;
        case DIRECTION_RIGHT:
            f->x++;
            offset = f->grid_offset + 1;
            break;
        case DIRECTION_BOTTOM_RIGHT:
            f->y++;
            f->x++;
            offset = f->grid_offset + GRID_SIZE + 1;
            break;
        case DIRECTION_BOTTOM:
            f->y++;
            offset = f->grid_offset + GRID_SIZE;
            break;
        case DIRECTION_BOTTOM_LEFT:
            f->y++;
            f->x--;
            offset = f->grid_offset + GRID_SIZE - 1;
            break;
        case DIRECTION_LEFT:
            f->x--;
            offset = f->grid_offset - 1;
            break;
        case DIRECTION_TOP_LEFT:
            f->y--;
            f->x--;
            offset = f->grid_offset - GRID_SIZE - 1;
            break;
        default:
            return;
    }
    f->grid_offset = offset;
    map_figure_add(figure_id);
    uint32_t terrain = map_grids.terrain[f->grid_offset];
    if ((terrain & TERRAIN_ROAD) || (terrain & TERRAIN_WALKABLE_BUILDING)) {
        f->is_on_road = 1;
        uint8_t sprite;
        if ((terrain & TERRAIN_WATER) && (sprite = map_grids.sprite[f->grid_offset]) != 0) {
            f->height_adjusted_ticks = MOVEMENT_BRIDGE_CLIMB_TICKS;
            f->target_height = (uint8_t) figure_bridge_height(sprite);
        }
    } else {
        f->is_on_road = 0;
    }
    if (fights_on_the_way(f->type)) {
        figure_combat_attack_figure_at(figure_id, map_grids.figure_ids[f->grid_offset]);
    }
    f->previous_tile_x = old_x;
    f->previous_tile_y = old_y;
}

void figure_movement_advance_tick(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    switch (f->direction) {
        case DIRECTION_TOP:
            f->cross_country.y--;
            break;
        case DIRECTION_TOP_RIGHT:
            f->cross_country.y--;
            f->cross_country.x++;
            break;
        case DIRECTION_RIGHT:
            f->cross_country.x++;
            break;
        case DIRECTION_BOTTOM_RIGHT:
            f->cross_country.y++;
            f->cross_country.x++;
            break;
        case DIRECTION_BOTTOM:
            f->cross_country.y++;
            break;
        case DIRECTION_BOTTOM_LEFT:
            f->cross_country.y++;
            f->cross_country.x--;
            break;
        case DIRECTION_LEFT:
            f->cross_country.x--;
            break;
        case DIRECTION_TOP_LEFT:
            f->cross_country.y--;
            f->cross_country.x--;
            break;
        default:
            break;
    }
    if (f->height_adjusted_ticks) {
        f->height_adjusted_ticks--;
        if (f->height_adjusted_ticks == 0) {
            f->is_ghost = 0;
            return;
        }
        f->is_ghost = 1;
        if (f->current_height < f->target_height) {
            f->current_height++;
        }
        if (f->current_height > f->target_height) {
            f->current_height--;
        }
    } else if (f->current_height) {
        f->current_height--;
    }
}

void figure_movement_walk_ticks(int32_t figure_id, int32_t num_ticks)
{
    figure *f = figure_get(figure_id);
    while (num_ticks > 0) {
        num_ticks--;
        if (inc_progress(f) >= MOVEMENT_STEPS_PER_TILE) {
            figure_service_provide_coverage(figure_id);
            f->progress_on_tile = MOVEMENT_STEPS_PER_TILE;
            if (f->routing_path_id <= 0) {
                figure_route_add(figure_id);
            }
            figure_movement_set_next_route_tile_direction(figure_id);
            figure_movement_advance_route_tile(figure_id);
            if (f->direction >= DIRECTION_AT_DESTINATION) {
                break;
            }
            f->routing_path_current_tile++;
            f->previous_tile_direction = f->direction;
            f->progress_on_tile = 0;
        }
        if (f->progress_on_tile == MOVEMENT_TILE_CHANGE_STEP) {
            figure_movement_move_to_next_tile(figure_id);
        }
        figure_movement_advance_tick(figure_id);
    }
    figure_roaming_enabled = 0;
}

void figure_movement_move_ticks(int32_t figure_id, int32_t speed)
{
    uint8_t *counter = &figure_get(figure_id)->move_speed_counter;
    switch (speed) {
        case 0:
            figure_movement_walk_ticks(figure_id, 0);
            break;
        case 1:
            if (*counter > 2) {
                figure_movement_walk_ticks(figure_id, 1);
                *counter = 0;
            } else {
                (*counter)++;
            }
            break;
        case 2:
            if (*counter > 1) {
                *counter = 0;
                figure_movement_walk_ticks(figure_id, 1);
            } else {
                (*counter)++;
            }
            break;
        case 3:
            if (*counter == 0) {
                figure_movement_walk_ticks(figure_id, 1);
                *counter = 1;
            } else {
                *counter = 0;
            }
            break;
        case 4:
            if (*counter <= 1) {
                figure_movement_walk_ticks(figure_id, 1);
                (*counter)++;
            } else {
                *counter = 0;
            }
            break;
        case 5:
            if (*counter <= 2) {
                figure_movement_walk_ticks(figure_id, 1);
                (*counter)++;
            } else {
                *counter = 0;
            }
            break;
        case 6:
            figure_movement_walk_ticks(figure_id, 1);
            break;
        case 7:
            if (*counter <= 2) {
                figure_movement_walk_ticks(figure_id, 1);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 2);
                *counter = 0;
            }
            break;
        case 8:
            if (*counter <= 1) {
                figure_movement_walk_ticks(figure_id, 1);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 2);
                *counter = 0;
            }
            break;
        case 9:
            if (*counter == 0) {
                figure_movement_walk_ticks(figure_id, 1);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 2);
                *counter = 0;
            }
            break;
        case 10:
            if (*counter <= 1) {
                figure_movement_walk_ticks(figure_id, 2);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 1);
                *counter = 0;
            }
            break;
        case 11:
            if (*counter <= 2) {
                figure_movement_walk_ticks(figure_id, 2);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 1);
                *counter = 0;
            }
            break;
        case 12:
            figure_movement_walk_ticks(figure_id, 2);
            break;
        case 13:
            if (*counter <= 2) {
                figure_movement_walk_ticks(figure_id, 2);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 3);
                *counter = 0;
            }
            break;
        case 14:
            if (*counter <= 1) {
                figure_movement_walk_ticks(figure_id, 2);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 3);
                *counter = 0;
            }
            break;
        case 15:
            if (*counter == 0) {
                figure_movement_walk_ticks(figure_id, 3);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 2);
                *counter = 0;
            }
            break;
        case 16:
            if (*counter <= 1) {
                figure_movement_walk_ticks(figure_id, 3);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 2);
                *counter = 0;
            }
            break;
        case 17:
            if (*counter <= 2) {
                figure_movement_walk_ticks(figure_id, 3);
                (*counter)++;
            } else {
                figure_movement_walk_ticks(figure_id, 2);
                *counter = 0;
            }
            break;
        default:
            figure_movement_walk_ticks(figure_id, 3);
            break;
    }
}

void figure_movement_init_roaming(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    building *b = building_get(f->building_id);
    f->progress_on_tile = MOVEMENT_STEPS_PER_TILE;
    f->roam_choose_destination = 0;
    f->roam_ticks_until_next_turn = MOVEMENT_ROAM_NO_TURN_YET;
    f->roam_turn_direction = MOVEMENT_ROAM_TURN_CLOCKWISE;
    int32_t roam_direction = b->figure_roam_direction;
    b->figure_roam_direction = (uint8_t) (roam_direction + MOVEMENT_ROAM_TURN_CLOCKWISE);
    if (b->figure_roam_direction > DIRECTION_LEFT) {
        b->figure_roam_direction = DIRECTION_TOP;
    }
    int32_t start_distance = movement_rules[MOVEMENT_RULE_ROAM_START_DISTANCE];
    int32_t x = b->x;
    int32_t y = b->y;
    if (roam_direction == DIRECTION_TOP) {
        y -= start_distance;
    } else if (roam_direction == DIRECTION_RIGHT) {
        x += start_distance;
    } else if (roam_direction == DIRECTION_BOTTOM) {
        y += start_distance;
    } else if (roam_direction == DIRECTION_LEFT) {
        x -= start_distance;
    }
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (x >= map_info.grid_width) {
        x = map_info.grid_width - 1;
    }
    if (y >= map_info.grid_height) {
        y = map_info.grid_height - 1;
    }
    if (map_closest_road_within_radius(x, y, 1, movement_rules[MOVEMENT_RULE_ROAM_ROAD_RADIUS])) {
        f->destination_x = map_result_x;
        f->destination_y = map_result_y;
    } else {
        f->roam_choose_destination = 1;
    }
}

void figure_movement_roam_set_direction(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t grid_offset = map_grid_offset(f->x, f->y);
    int32_t direction = calc_general_direction(f->x, f->y, f->destination_x, f->destination_y) & 0xFF;
    if (direction >= DIRECTION_COUNT) {
        direction = DIRECTION_TOP;
    }
    int32_t direction_cw = DIRECTION_TOP;
    int32_t steps_cw = 0;
    int32_t d = direction;
    int32_t i;
    for (i = 0; i < DIRECTION_COUNT; i++) {
        if (!(d & 1) && (map_grids.terrain[grid_offset + map_grid_direction_delta[d]] & TERRAIN_ROAD)) {
            break;
        }
        d++;
        if (d > DIRECTION_TOP_LEFT) {
            d = DIRECTION_TOP;
        }
        steps_cw++;
    }
    if (i < DIRECTION_COUNT) {
        direction_cw = d;
    }
    int32_t direction_ccw = DIRECTION_BOTTOM;
    int32_t steps_ccw;
    d = direction;
    for (steps_ccw = 0; steps_ccw < DIRECTION_COUNT; steps_ccw++) {
        if (!(d & 1) && (map_grids.terrain[grid_offset + map_grid_direction_delta[d]] & TERRAIN_ROAD)) {
            break;
        }
        d--;
        if (d < DIRECTION_TOP) {
            d = DIRECTION_TOP_LEFT;
        }
    }
    if (steps_ccw < DIRECTION_COUNT) {
        direction_ccw = d;
    }
    if (steps_cw <= steps_ccw) {
        f->direction = (int8_t) direction_cw;
        f->roam_turn_direction = MOVEMENT_ROAM_TURN_CLOCKWISE;
    } else {
        f->direction = (int8_t) direction_ccw;
        f->roam_turn_direction = MOVEMENT_ROAM_TURN_COUNTER_CLOCKWISE;
    }
    f->roam_ticks_until_next_turn = MOVEMENT_ROAM_TURN_TICKS;
}

static void roam_turn_until_road(figure *f, int32_t *attempts, int32_t came_from)
{
    for (;;) {
        int8_t direction = f->direction;
        if (map_adjacent_tiles[direction] != 0 && direction != came_from) {
            return;
        }
        f->direction = (int8_t) (f->roam_turn_direction + direction);
        if (f->direction > DIRECTION_LEFT) {
            f->direction = DIRECTION_TOP;
        }
        if (f->direction < DIRECTION_TOP) {
            f->direction = DIRECTION_LEFT;
        }
        if ((*attempts)++ >= MOVEMENT_ROAM_ROAD_DIRECTIONS) {
            return;
        }
    }
}

void figure_movement_roam_ticks(int32_t figure_id, int32_t num_ticks)
{
    figure *f = figure_get(figure_id);
    int32_t attempts = 0;
    if (!f->roam_choose_destination) {
        figure_roaming_enabled = 1;
        figure_movement_move_ticks(figure_current_id, figure_properties[f->type].speed);
        if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
            f->roam_choose_destination = 1;
        } else if (f->direction == DIRECTION_AT_DESTINATION) {
            f->roam_choose_destination = 1;
            f->roam_length = 0;
        }
        if (!f->roam_choose_destination) {
            return;
        }
        f->roam_ticks_until_next_turn = MOVEMENT_ROAM_STRAIGHT_TICKS;
        f->direction = f->previous_tile_direction;
    }
    while (num_ticks > 0) {
        num_ticks--;
        if (inc_progress(f) >= MOVEMENT_STEPS_PER_TILE) {
            f->progress_on_tile = MOVEMENT_STEPS_PER_TILE;
            int32_t came_from = (f->previous_tile_direction + DIRECTION_BOTTOM) % DIRECTION_COUNT;
            f->roam_random_counter++;
            if (figure_service_provide_coverage(figure_id)) {
                return;
            }
            int32_t roads = map_get_adjacent_road_tiles_for_roaming(f->grid_offset, figure_id);
            if (roads <= 0) {
                figure *current = figure_get(figure_current_id);
                current->roam_length = current->max_roam_length;
                return;
            }
            if (roads == 1) {
                f->direction = (int8_t) (attempts * MOVEMENT_ROAM_TURN_CLOCKWISE);
                if (map_adjacent_tiles[f->direction] == 0) {
                    while (attempts++ < MOVEMENT_ROAM_ROAD_DIRECTIONS) {
                        f->direction = (int8_t) (attempts * MOVEMENT_ROAM_TURN_CLOCKWISE);
                        if (map_adjacent_tiles[f->direction] != 0) {
                            break;
                        }
                    }
                }
            } else if (roads == 2) {
                if (f->roam_ticks_until_next_turn == MOVEMENT_ROAM_NO_TURN_YET) {
                    figure_movement_roam_set_direction(figure_id);
                    came_from = MOVEMENT_ROAM_NO_DIRECTION;
                }
                roam_turn_until_road(f, &attempts, came_from);
            } else {
                f->direction = (int8_t) ((map_grids.random[f->grid_offset] + f->roam_random_counter) & MOVEMENT_ROAM_EVEN_DIRECTION_MASK);
                if (map_adjacent_tiles[f->direction] == 0 || f->direction == came_from) {
                    f->roam_ticks_until_next_turn--;
                    if (f->roam_ticks_until_next_turn <= 0) {
                        figure_movement_roam_set_direction(figure_id);
                        came_from = MOVEMENT_ROAM_NO_DIRECTION;
                    }
                    roam_turn_until_road(f, &attempts, came_from);
                }
            }
            f->routing_path_current_tile++;
            f->previous_tile_direction = f->direction;
            f->progress_on_tile = 0;
        }
        figure_movement_advance_tick(figure_id);
        if (f->progress_on_tile == MOVEMENT_TILE_CHANGE_STEP) {
            figure_movement_move_to_next_tile(figure_id);
        }
    }
}

void figure_movement_follow_ticks(int32_t figure_id, int32_t leader_id, int32_t num_ticks)
{
    figure *f = figure_get(figure_id);
    figure *leader = figure_get(leader_id);
    if (f->x == f->source_x && f->y == f->source_y) {
        f->is_ghost = 1;
    }
    while (num_ticks > 0) {
        num_ticks--;
        if (inc_progress(f) >= MOVEMENT_STEPS_PER_TILE) {
            f->progress_on_tile = MOVEMENT_STEPS_PER_TILE;
            int8_t direction = (int8_t) calc_general_direction(f->x, f->y, leader->previous_tile_x, leader->previous_tile_y);
            f->direction = direction;
            if (direction >= DIRECTION_AT_DESTINATION) {
                return;
            }
            f->previous_tile_direction = direction;
            f->progress_on_tile = 0;
        }
        figure_movement_advance_tick(figure_id);
        if (f->progress_on_tile == MOVEMENT_TILE_CHANGE_STEP) {
            figure_movement_move_to_next_tile(figure_id);
        }
    }
}

void figure_movement_set_cross_country_direction(int32_t figure_id, int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst, int32_t is_missile)
{
    figure *f = figure_get(figure_id);
    f->cross_country.destination_x = (int16_t) x_dst;
    f->cross_country.destination_y = (int16_t) y_dst;
    f->cross_country.delta_x = (int16_t) (x_src > x_dst ? x_src - x_dst : x_dst - x_src);
    f->cross_country.delta_y = (int16_t) (y_src > y_dst ? y_src - y_dst : y_dst - y_src);
    if (f->cross_country.delta_x < f->cross_country.delta_y) {
        f->cross_country.delta_xy = (int16_t) ((2 * f->cross_country.delta_x) - f->cross_country.delta_y);
    } else if (f->cross_country.delta_y < f->cross_country.delta_x) {
        f->cross_country.delta_xy = (int16_t) ((2 * f->cross_country.delta_y) - f->cross_country.delta_x);
    } else {
        f->cross_country.delta_xy = 0;
    }
    if (is_missile) {
        f->direction = (int8_t) calc_missile_direction(x_src, y_src, x_dst, y_dst);
    } else {
        int8_t direction = (int8_t) calc_general_direction(x_src, y_src, x_dst, y_dst);
        f->direction = direction;
        if (f->cross_country.delta_y > (2 * f->cross_country.delta_x)) {
            switch (direction) {
                case DIRECTION_TOP_RIGHT:
                case DIRECTION_TOP_LEFT:
                    f->direction = DIRECTION_TOP;
                    break;
                case DIRECTION_BOTTOM_RIGHT:
                case DIRECTION_BOTTOM_LEFT:
                    f->direction = DIRECTION_BOTTOM;
                    break;
            }
        }
        if (f->cross_country.delta_x > (2 * f->cross_country.delta_y)) {
            switch (f->direction) {
                case DIRECTION_TOP_RIGHT:
                case DIRECTION_BOTTOM_RIGHT:
                    f->direction = DIRECTION_RIGHT;
                    break;
                case DIRECTION_BOTTOM_LEFT:
                case DIRECTION_TOP_LEFT:
                    f->direction = DIRECTION_LEFT;
                    break;
            }
        }
    }
    f->cc_direction = (f->cross_country.delta_y > f->cross_country.delta_x) ? CROSS_COUNTRY_ALONG_Y : CROSS_COUNTRY_ALONG_X;
}

void figure_movement_cross_country_update_delta(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->cc_direction == CROSS_COUNTRY_ALONG_X) {
        if (f->cross_country.delta_xy >= 0) {
            f->cross_country.delta_xy = (int16_t) (f->cross_country.delta_xy + (2 * (f->cross_country.delta_y - f->cross_country.delta_x)));
        } else {
            f->cross_country.delta_xy = (int16_t) ((2 * f->cross_country.delta_y) + f->cross_country.delta_xy);
        }
        f->cross_country.delta_x--;
    } else {
        if (f->cross_country.delta_xy >= 0) {
            f->cross_country.delta_xy = (int16_t) (f->cross_country.delta_xy + (2 * (f->cross_country.delta_x - f->cross_country.delta_y)));
        } else {
            f->cross_country.delta_xy = (int16_t) ((2 * f->cross_country.delta_x) + f->cross_country.delta_xy);
        }
        f->cross_country.delta_y--;
    }
}

void figure_movement_cross_country_advance_x(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->cross_country.x < f->cross_country.destination_x) {
        f->cross_country.x++;
    } else if (f->cross_country.x > f->cross_country.destination_x) {
        f->cross_country.x--;
    }
}

void figure_movement_cross_country_advance_y(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (f->cross_country.y < f->cross_country.destination_y) {
        f->cross_country.y++;
    } else if (f->cross_country.y > f->cross_country.destination_y) {
        f->cross_country.y--;
    }
}

static void cross_country_step(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    figure_movement_cross_country_update_delta(figure_id);
    if (f->cc_direction == CROSS_COUNTRY_ALONG_Y) {
        figure_movement_cross_country_advance_y(figure_id);
        if (f->cross_country.delta_xy >= 0) {
            f->cross_country.delta_x--;
            figure_movement_cross_country_advance_x(figure_id);
        }
    } else {
        figure_movement_cross_country_advance_x(figure_id);
        if (f->cross_country.delta_xy >= 0) {
            f->cross_country.delta_y--;
            figure_movement_cross_country_advance_y(figure_id);
        }
    }
}

int32_t figure_movement_move_ticks_cross_country(int32_t figure_id, int32_t num_ticks)
{
    figure *f = figure_get(figure_id);
    int32_t is_at_destination = 0;
    map_figure_delete(figure_id);
    for (int32_t i = 0; i < num_ticks; i++) {
        if (f->missile_damage) {
            f->missile_damage--;
        } else {
            f->missile_damage = 0;
        }
        if (f->cross_country.delta_y + f->cross_country.delta_x <= 0) {
            is_at_destination = 1;
            break;
        }
        cross_country_step(figure_id);
    }
    f->x = (int16_t) (f->cross_country.x / MOVEMENT_STEPS_PER_TILE);
    f->y = (int16_t) (f->cross_country.y / MOVEMENT_STEPS_PER_TILE);
    f->grid_offset = map_info.grid_start + f->x + (f->y * GRID_SIZE);
    if (map_grids.terrain[f->grid_offset] & TERRAIN_BUILDING) {
        f->in_building_wait_ticks = MOVEMENT_IN_BUILDING_WAIT_TICKS;
    } else if (f->in_building_wait_ticks) {
        f->in_building_wait_ticks--;
    }
    map_figure_add(figure_id);
    return is_at_destination;
}

int32_t figure_movement_can_launch_cross_country_missile(int32_t x_src, int32_t y_src, int32_t x_dst, int32_t y_dst)
{
    figure *f = figure_get(0);
    int32_t src_x = MOVEMENT_STEPS_PER_TILE * x_src;
    int32_t src_y = MOVEMENT_STEPS_PER_TILE * y_src;
    f->cross_country.x = (int16_t) src_x;
    f->cross_country.y = (int16_t) src_y;
    int32_t unchecked_tiles = 0;
    if (map_grids.terrain[map_grid_offset(x_src, y_src)] & (TERRAIN_WALL | TERRAIN_GATEHOUSE)) {
        unchecked_tiles = MOVEMENT_MISSILE_WALL_FREE_TILES;
    }
    figure_movement_set_cross_country_direction(0, src_x, src_y, MOVEMENT_STEPS_PER_TILE * x_dst, MOVEMENT_STEPS_PER_TILE * y_dst, 0);
    for (int32_t guard = 0; guard < MOVEMENT_MISSILE_MAX_TILES; guard++) {
        for (int32_t i = 0; i < MOVEMENT_MISSILE_STEPS_PER_TILE; i++) {
            if (f->cross_country.delta_x + f->cross_country.delta_y <= 0) {
                return 1;
            }
            cross_country_step(0);
        }
        f->x = (int16_t) (f->cross_country.x / MOVEMENT_STEPS_PER_TILE);
        f->y = (int16_t) (f->cross_country.y / MOVEMENT_STEPS_PER_TILE);
        if (unchecked_tiles) {
            unchecked_tiles--;
        } else {
            int32_t grid_offset = map_info.grid_start + f->x + (f->y * GRID_SIZE);
            if (map_grids.terrain[grid_offset] & (TERRAIN_WALL | TERRAIN_GATEHOUSE | TERRAIN_TREE)) {
                return 0;
            }
            if ((map_grids.terrain[grid_offset] & TERRAIN_BUILDING) && (map_grids.bitfields[grid_offset] & MAP_BITFIELD_SIZE)) {
                return 0;
            }
        }
    }
    return 0;
}
