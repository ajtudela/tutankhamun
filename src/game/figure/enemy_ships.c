// Enemy ships of a sea invasion: the transports that carry soldiers to a landing point and unload them as formations,
// and the warships that patrol, ram and shoot the player's ships and soldiers; their target search, evasion when
// damaged, formation_create and the count of enemy warships made at load.
//
// An evading ship whose three escape lines find no water looks for a free water tile around itself; the bedouins never
// come by sea.

#include "figure/enemy_ships.h"
#include "building/building_data.h"
#include "building/destroy_enemy.h"
#include "core/calc.h"
#include "core/random.h"
#include "editor/editor_data.h"
#include "figure/enemy.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "figure/military_internal.h"
#include "figure/movement.h"
#include "figure/ships_player.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/routing_water.h"
#include "map/terrain.h"
#include "map/water.h"
#include "scenario/invasion.h"
#include "scenario/scenario_internal.h"
#include "sound/sound_play.h"
#include <float.h>
#include <stdbool.h>
#include <stdint.h>

static int32_t facing_direction(const figure *f)
{
    return f->direction > DIRECTION_TOP_LEFT ? f->previous_tile_direction : f->direction;
}

void enemy_ship_reroute_tick(void)
{
    int32_t id = figure_current_id;
    figure *f = figure_get(id);
    if (++f->reroute_ticks > ENEMY_SHIP_REROUTE_LAST_TICK) {
        f->reroute_ticks = 0;
        figure_route_remove(id);
    }
}

void enemy_ship_sink(int32_t figure_id)
{
    figure_combat_handle_corpse(figure_id);
}

static int32_t open_reached_water(int32_t t)
{
    int32_t water_around = (map_grids.terrain[t] & TERRAIN_WATER) && (map_grids.terrain[t - GRID_SIZE] & TERRAIN_WATER) && (map_grids.terrain[t + GRID_SIZE] & TERRAIN_WATER) &&
        (map_grids.terrain[t + 1] & TERRAIN_WATER) && (map_grids.terrain[t - 1] & TERRAIN_WATER);
    return water_around && routing_grids.water[t] != ROUTING_WATER_BLOCKED && routing_grids.distance[t] > 0;
}

int32_t map_water_farthest_on_line(int32_t x, int32_t y, int32_t dst_x, int32_t dst_y, int32_t *out_x, int32_t *out_y)
{
    if (x == dst_x && y == dst_y) {
        *out_x = x;
        *out_y = y;
        return 0;
    }
    map_routing_calculate_distances_water_boat(x, y);
    int32_t dy = dst_y - y;
    int32_t dx = dst_x - x;
    int32_t step_x = 0;
    int32_t step_y = 0;
    int32_t best = 0;
    int32_t best_x = x;
    int32_t best_y = y;
    const double max = FLT_MAX;
    double slope;
    double inverse;
    if (dx == 0) {
        slope = max;
        inverse = 0.0;
    } else {
        if (dy == 0) {
            slope = 0.0;
            inverse = max;
        } else {
            slope = (double) dy / (double) dx;
            inverse = 1.0 / slope;
        }
        step_x = dx < 0 ? -1 : 1;
    }
    if (dy < 0) {
        step_y = -1;
    } else if (dy > 0) {
        step_y = 1;
    }
    int32_t rows = dy < 0 ? -dy : dy;
    int32_t columns = dx < 0 ? -dx : dx;
    int32_t row_offset = map_info.grid_start + (y * GRID_SIZE);
    int32_t yy = y;
    for (int32_t i = rows; i > 0; i--) {
        if (inverse == max) {
            continue;
        }
        row_offset += (step_y * GRID_SIZE);
        yy += step_y;
        int32_t xx = (int32_t) (int64_t) (((double) (yy - y) * inverse) + (double) x);
        int32_t t = row_offset + xx;
        if (open_reached_water(t)) {
            int32_t d = ((yy - y) * (yy - y)) + ((xx - x) * (xx - x));
            if (d >= best) {
                best = d;
                best_x = xx;
                best_y = yy;
            }
        }
    }
    int32_t xx = x;
    for (int32_t i = columns; i > 0; i--) {
        if (slope == max) {
            continue;
        }
        xx += step_x;
        int32_t yv = (int32_t) (int64_t) (((double) (xx - x) * slope) + (double) y);
        int32_t t = map_info.grid_start + (yv * GRID_SIZE) + xx;
        if (open_reached_water(t)) {
            int32_t d = ((yv - y) * (yv - y)) + ((xx - x) * (xx - x));
            if (d >= best) {
                best = d;
                best_x = xx;
                best_y = yv;
            }
        }
    }
    *out_x = best_x;
    *out_y = best_y;
    return best;
}

void ship_pick_escape_point(int32_t *x, int32_t *y, int32_t dx1, int32_t dy1, int32_t dx2, int32_t dy2, int32_t dx3, int32_t dy3, int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    int32_t ox;
    int32_t oy;
    int32_t best = 0;
    *x = f->x;
    *y = f->y;
    int32_t d = map_water_farthest_on_line(f->x, f->y, f->x + dx1, f->y + dy1, &ox, &oy);
    if (d > -1) {
        *x = ox;
        *y = oy;
        best = d;
    }
    d = map_water_farthest_on_line(f->x, f->y, f->x + dx2, f->y + dy2, &ox, &oy);
    if (best <= d) {
        *x = ox;
        *y = oy;
        best = d;
    }
    d = map_water_farthest_on_line(f->x, f->y, f->x + dx3, f->y + dy3, &ox, &oy);
    if (best <= d) {
        *x = ox;
        *y = oy;
    }
    map_water_find_free_tile(x, y);
}

ship_escape_move ship_escape_moves[DIRECTION_COUNT] = {
    [DIRECTION_TOP] = { .dx1 = 0, .dy1 = -10, .dx2 = 10, .dy2 = -10, .dx3 = -10, .dy3 = -10 },
    [DIRECTION_TOP_RIGHT] = { .dx1 = 0, .dy1 = -10, .dx2 = 10, .dy2 = -10, .dx3 = 10, .dy3 = 0 },
    [DIRECTION_RIGHT] = { .dx1 = 10, .dy1 = -10, .dx2 = 10, .dy2 = 0, .dx3 = 10, .dy3 = 10 },
    [DIRECTION_BOTTOM_RIGHT] = { .dx1 = 10, .dy1 = 0, .dx2 = 10, .dy2 = 10, .dx3 = 0, .dy3 = 10 },
    [DIRECTION_BOTTOM] = { .dx1 = 10, .dy1 = 10, .dx2 = 0, .dy2 = 10, .dx3 = -10, .dy3 = 10 },
    [DIRECTION_BOTTOM_LEFT] = { .dx1 = 0, .dy1 = 10, .dx2 = -10, .dy2 = 0, .dx3 = -10, .dy3 = 10 },
    [DIRECTION_LEFT] = { .dx1 = -10, .dy1 = 0, .dx2 = -10, .dy2 = 10, .dx3 = -10, .dy3 = -10 },
    [DIRECTION_TOP_LEFT] = { .dx1 = -10, .dy1 = 0, .dx2 = -10, .dy2 = -10, .dx3 = 0, .dy3 = -10 },
};

void ship_play_ram_sound(int32_t divisor)
{
    if (divisor >= 1 && divisor <= SHIP_RAM_DIVISOR_MAX) {
        sound_effect_play((SHIP_RAM_SOUND_FIRST - 1) + divisor);
    }
}

int32_t ship_find_ship_at(int32_t grid_offset, int32_t figure_id)
{
    int32_t id = map_grids.figure_ids[grid_offset];
    while (id) {
        const figure *f = figure_get(id);
        if (id != figure_id) {
            int32_t type = f->type;
            if (type == FIGURE_TRANSPORT_SHIP || type == FIGURE_WARSHIP || type == FIGURE_FISHING_BOAT) {
                return id;
            }
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

int32_t ship_find_ram_target(int32_t figure_id)
{
    static const int32_t ahead[DIRECTION_COUNT] = { -GRID_SIZE, -GRID_SIZE + 1, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };
    const figure *f = figure_get(figure_id);
    int32_t grid_offset = map_info.grid_start + f->x + f->y * GRID_SIZE;
    if ((uint8_t) f->direction > DIRECTION_TOP_LEFT) {
        return 0;
    }
    int32_t id = ship_find_ship_at(grid_offset, figure_id);
    if (id == 0) {
        return ship_find_ship_at(grid_offset + ahead[f->direction], figure_id);
    }
    return id;
}

void enemy_transport_wait(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    f->is_ghost = 0;
    *moving = 0;
    if (--f->wait_ticks >= 1) {
        return;
    }
    f->wait_ticks = 0;
    if (f->has_troops_on_board) {
        f->source_x = f->x;
        f->source_y = f->y;
        int32_t x;
        int32_t y;
        map_water_farthest_on_line(f->x, f->y, f->ship_landing_x, f->ship_landing_y, &x, &y);
        f->destination_x = (int16_t) x;
        f->destination_y = (int16_t) y;
        f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_GOING_TO_LANDING;
        figure_route_remove(figure_id);
        return;
    }
    f->source_x = f->x;
    f->source_y = f->y;
    f->destination_x = scenario_data.river_exit_x;
    f->destination_y = scenario_data.river_exit_y;
    f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING;
}

void enemy_transport_move(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_CORPSE;
    } else if (f->direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
    }
}

static int32_t unload_split(int32_t count, int32_t sizes[4])
{
    if (count <= 0) {
        return 0;
    }
    if (count <= ENEMY_TRANSPORT_FORMATION_SIZE) {
        sizes[0] = count;
        return 1;
    }
    if (count <= 2 * ENEMY_TRANSPORT_FORMATION_SIZE) {
        sizes[1] = count / 2;
        sizes[0] = count - count / 2;
        return 2;
    }
    sizes[1] = sizes[2] = count / 3;
    sizes[0] = count - (2 * (count / 3));
    return 3;
}

void enemy_transport_unload(int32_t figure_id, int32_t x, int32_t y)
{
    figure *ship = figure_get(figure_id);
    int32_t nation = ship->invasion_nation;
    int32_t total = ship->invasion_soldiers;
    int32_t param = ship->invasion_formation_param;
    int32_t event_id = ship->invasion_event_id;
    if (++invasion_last_id > INVASION_LAST_ID_MAX) {
        invasion_last_id = 1;
    }
    int32_t types[3];
    int32_t percentage[3];
    int32_t layout = FORMATION_ENEMY_DOUBLE_LINE;
    if (nation == ENEMY_EGYPTIAN) {
        types[0] = FIGURE_ENEMY_EGYPTIAN_INFANTRY, types[1] = FIGURE_ENEMY_EGYPTIAN_ARCHER, types[2] = FIGURE_ENEMY_EGYPTIAN_CHARIOT;
        percentage[0] = figure_properties[types[0]].invasion_percentage;
        percentage[1] = figure_properties[types[1]].invasion_percentage;
        percentage[2] = figure_properties[types[2]].invasion_percentage;
    } else if (nation == ENEMY_BEDOUIN) {
        types[0] = types[1] = types[2] = FIGURE_BEDOUIN;
        percentage[0] = figure_properties[types[0]].invasion_percentage;
        percentage[1] = 0;
        percentage[2] = 0;
        layout = FORMATION_ENEMY_MOB;
    } else {
        types[0] = FIGURE_ENEMY_INFANTRY, types[1] = FIGURE_ENEMY_ARCHER, types[2] = FIGURE_ENEMY_CHARIOT;
        percentage[0] = figure_properties[types[0]].invasion_percentage;
        percentage[1] = figure_properties[types[1]].invasion_percentage;
        percentage[2] = figure_properties[types[2]].invasion_percentage;
    }
    calc_adjust_with_percentage(total, percentage[0]);
    int32_t count1 = calc_adjust_with_percentage(total, percentage[1]);
    int32_t count2 = calc_adjust_with_percentage(total, percentage[2]);
    int32_t formations[3];
    int32_t sizes[3][4] = { { 0 } };
    formations[0] = unload_split(total - count2 - count1, sizes[0]);
    formations[1] = unload_split(count1, sizes[1]);
    formations[2] = unload_split(count2, sizes[2]);
    int32_t created = 0;
    for (int32_t group = 0; group < 3; group++) {
        int32_t type = types[group];
        for (int32_t k = 0; k < formations[group]; k++) {
            int32_t formation_id = formation_create(type, layout, 0, x, y);
            if (formation_id <= 0) {
                continue;
            }
            formation_get(formation_id)->attack_type = (int16_t) param;
            formation_get(formation_id)->orientation = DIRECTION_TOP;
            formation_get(formation_id)->enemy_type = (uint8_t) nation;
            formation_get(formation_id)->invasion_event_id = (int16_t) event_id;
            formation_get(formation_id)->invasion_sequence = invasion_last_id;
            int32_t i = 0;
            for (; i < sizes[group][k]; i++) {
                int32_t id = figure_create(0, type, x, y, 0);
                figure *f = figure_get(id);
                f->is_friendly = 0;
                f->action_state = FIGURE_ACTION_ENEMY_INITIAL;
                f->wait_ticks = (int16_t) ((i + 1 + (created * ENEMY_TRANSPORT_WAIT_SLOTS)) * ENEMY_TRANSPORT_WAIT_STEP);
                f->formation_id = (int16_t) formation_id;
                figure_name_set(id);
                f->is_ghost = 1;
            }
            created++;
            ship->wait_ticks = (int16_t) ((i + 1 + (created * ENEMY_TRANSPORT_WAIT_SLOTS)) * ENEMY_TRANSPORT_WAIT_STEP);
        }
    }
    ship->has_troops_on_board = 0;
    ship->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_WAITING;
}

void enemy_transport_go_to_landing(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    if (f->direction == DIRECTION_AT_DESTINATION) {
        *moving = 0;
        int32_t x = f->ship_landing_x;
        int32_t y = f->ship_landing_y;
        map_find_nearest_free_land_tile(&x, &y);
        int32_t grid_offset = map_info.grid_start + y * GRID_SIZE + x;
        uint32_t terrain = map_grids.terrain[grid_offset];
        if (terrain & (TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_ELEVATION)) {
            f->wait_ticks = ENEMY_TRANSPORT_LANDING_WAIT;
            f->has_troops_on_board = 0;
            f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_WAITING;
        }
        if (terrain & TERRAIN_WATER) {
            if (!(terrain & TERRAIN_ROAD)) {
                f->wait_ticks = ENEMY_TRANSPORT_LANDING_WAIT;
                f->has_troops_on_board = 0;
                f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_WAITING;
            }
        } else if (terrain & (TERRAIN_BUILDING | TERRAIN_CANAL | TERRAIN_WALL | TERRAIN_GATEHOUSE)) {
            building_destroy_by_enemy(x, y, grid_offset);
        }
        enemy_transport_unload(figure_id, x, y);
        scenario_invasion_landing_message(f->invasion_event_id, f->grid_offset);
    } else if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
}

void enemy_transport_go_away(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    figure_movement_move_ticks(figure_id, figure_properties[f->type].speed);
    if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
        return;
    }
    if (f->direction != DIRECTION_LOST) {
        if (f->direction != DIRECTION_AT_DESTINATION) {
            return;
        }
        if (f->x == scenario_data.river_entry_x && f->y == scenario_data.river_entry_y) {
            f->destination_x = scenario_data.river_exit_x;
            f->destination_y = scenario_data.river_exit_y;
            f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING;
            figure_route_remove(figure_id);
            return;
        }
        if (f->x != scenario_data.river_exit_x || f->y != scenario_data.river_exit_y) {
            f->destination_x = scenario_data.river_entry_x;
            f->destination_y = scenario_data.river_entry_y;
            f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING;
            figure_route_remove(figure_id);
            return;
        }
        if (f->has_troops_on_board) {
            figure_route_remove(figure_id);
            f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_WAITING;
            return;
        }
    }
    f->action_state = FIGURE_ACTION_CORPSE;
}

void enemy_transport_evade(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (f->warship_secondary_action == WARSHIP_MODE_FOLLOW_TARGET) {
        enemy_transport_move(figure_id, moving);
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
    int32_t enemy_id = ship_find_closest(figure_id, WARSHIP_EVADE_RANGE, 1);
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
    f->warship_secondary_action = WARSHIP_MODE_FOLLOW_TARGET;
}

static void ship_set_image(int32_t sinking_group, int32_t moving_group, int32_t still_group, int32_t moving, int32_t keep_direction)
{
    figure *f = figure_get(figure_current_id);
    int32_t direction = facing_direction(f) - map_info.orientation;
    if (direction < 0) {
        direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        if (f->wait_ticks <= ENEMY_SHIP_SINKING_TICKS) {
            f->image_id = (direction / DIRECTION_COUNT) + image_group(sinking_group) + (f->wait_ticks / ENEMY_SHIP_FRAME_TICKS);
        } else {
            int32_t frame = f->wait_ticks - (ENEMY_SHIP_SINKING_TICKS + 1);
            if (frame > ENEMY_SHIP_LAST_BUBBLE_FRAME) {
                frame = ENEMY_SHIP_LAST_BUBBLE_FRAME;
            }
            f->image_id = frame + image_group(IMAGE_GROUP_FIGURE_BUBBLES);
        }
    } else if (moving) {
        f->image_id = image_group(moving_group) + ((f->animation_frame / ENEMY_SHIP_FRAME_TICKS) * DIRECTION_COUNT) + direction;
    } else {
        f->image_id = image_group(still_group) + direction;
    }
    if (keep_direction) {
        figure_image_direction = direction;
    }
}

static void ship_action_start(int32_t damaged_state)
{
    city_player_current()->enemies++;
    figure *f = figure_get(figure_current_id);
    f->is_boat = 1;
    if (++f->animation_frame >= ENEMY_SHIP_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    if (f->ship_hit_points <= 0) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
    int32_t percent = calc_percentage(f->ship_hit_points, f->ship_max_hit_points);
    if (percent <= ENEMY_SHIP_DAMAGED_PERCENT && percent > 0) {
        f->action_state = (uint8_t) damaged_state;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
        if (damaged_state == FIGURE_ACTION_ENEMY_TRANSPORT_EVADING) {
            f->wait_ticks = ENEMY_TRANSPORT_EVADE_WAIT;
        }
    }
}

void figure_enemy_transport_action(void)
{
    int32_t moving = 1;
    ship_action_start(FIGURE_ACTION_ENEMY_TRANSPORT_EVADING);
    enemy_ship_reroute_tick();
    int32_t id = figure_current_id;
    figure *f = figure_get(id);
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            enemy_ship_sink(id);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_WAITING:
            enemy_transport_wait(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_MOVING:
            enemy_transport_move(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_EVADING:
            enemy_transport_evade(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_IDLE:
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_GOING_TO_LANDING:
            enemy_transport_go_to_landing(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING:
            enemy_transport_go_away(id);
            break;
        default:
            f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING;
            break;
    }
    ship_set_image(IMAGE_GROUP_FIGURE_ENEMY_TRANSPORT_SINKING, IMAGE_GROUP_FIGURE_ENEMY_TRANSPORT_MOVING, IMAGE_GROUP_FIGURE_ENEMY_TRANSPORT_STILL, moving, 1);
}

bool figure_military_is_away_on_mission(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    switch (f->type) {
        case FIGURE_ARCHER:
        case FIGURE_INFANTRY:
        case FIGURE_CHARIOTEER:
            if (formation_get(f->formation_id)->request_event_id >= 0) {
                return 1;
            }
            break;
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
            if (f->request_event_id >= 0) {
                return 1;
            }
            break;
    }
    return 0;
}

int32_t enemy_warship_find_target_at(int32_t x, int32_t y, int32_t range)
{
    int32_t best_id = 0;
    int32_t best_score = 0;
    figure_search_best_distance = range;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_ALIVE || figure_military_is_away_on_mission(id) || f->action_state == FIGURE_ACTION_CORPSE) {
            continue;
        }
        int32_t score;
        int32_t distance;
        switch (f->type) {
            case FIGURE_ARCHER:
            case FIGURE_INFANTRY:
            case FIGURE_CHARIOTEER:
                distance = calc_maximum_distance(x, y, f->x, f->y);
                if (distance > figure_properties[figure_get(figure_current_id)->type].missile_range) {
                    continue;
                }
                score = WARSHIP_SCORE_LAND_ENEMY;
                break;
            case FIGURE_TRADE_SHIP:
            case FIGURE_FISHING_BOAT:
                distance = calc_maximum_distance(x, y, f->x, f->y);
                score = WARSHIP_SCORE_BOAT;
                break;
            case FIGURE_TRANSPORT_SHIP:
                score = f->has_troops_on_board ? WARSHIP_SCORE_LOADED_TRANSPORT : WARSHIP_SCORE_TRANSPORT;
                distance = calc_maximum_distance(x, y, f->x, f->y);
                break;
            case FIGURE_WARSHIP:
                distance = calc_maximum_distance(x, y, f->x, f->y);
                score = WARSHIP_SCORE_WARSHIP;
                break;
            default:
                continue;
        }
        if (score - distance > best_score - figure_search_best_distance && figure_movement_can_launch_cross_country_missile(x, y, f->x, f->y)) {
            best_id = id;
            figure_search_best_distance = distance;
            best_score = score;
        }
    }
    if (best_id == 0) {
        return 0;
    }
    const figure *best = figure_get(best_id);
    map_result_y = best->y;
    map_result_x = best->x;
    return best_id;
}

int32_t enemy_warship_find_target(int32_t figure_id, int32_t range)
{
    const figure *f = figure_get(figure_id);
    return enemy_warship_find_target_at(f->x, f->y, range);
}

void enemy_warship_shoot(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    if (++f->wait_ticks_missile > figure_properties[f->type].missile_delay) {
        f->wait_ticks_missile = 0;
        if (enemy_warship_find_target(figure_id, figure_properties[f->type].missile_range) == 0) {
            f->attack_image_offset = 0;
        } else {
            f->attack_image_offset = 1;
            f->direction = (int8_t) calc_missile_shooter_direction(f->x, f->y, map_result_x, map_result_y);
        }
    }
    if (f->attack_image_offset) {
        if (f->attack_image_offset == 1) {
            figure_create_missile(figure_id, f->x, f->y, map_result_x, map_result_y, FIGURE_SPEAR);
        }
        if (++f->attack_image_offset > WARSHIP_SHOOT_TICKS) {
            f->attack_image_offset = 0;
        }
    }
}

void enemy_warship_wait(int32_t figure_id, int32_t *moving)
{
    (void) moving;
    figure *f = figure_get(figure_id);
    if (--f->wait_ticks >= 1) {
        return;
    }
    f->wait_ticks = 0;
    f->is_ghost = 0;
    f->source_x = f->x;
    f->source_y = f->y;
    f->destination_x = scenario_data.river_entry_x;
    f->destination_y = scenario_data.river_entry_y;
    f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING;
    figure_route_remove(figure_id);
}

void enemy_warship_move(int32_t figure_id, int32_t *moving)
{
    ship_move(figure_id, SHIP_MOVE_STRAIGHT);
    *moving = 1;
    figure *f = figure_get(figure_id);
    f->is_ghost = 0;
    if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
    }
}

void enemy_warship_ram(int32_t figure_id, int32_t *moving)
{
    int32_t reduce = 0;
    ship_move(figure_id, SHIP_MOVE_CAUTIOUS);
    *moving = 1;
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
        int32_t target_id = ship_find_ram_target(figure_id);
        if (target_id != 0) {
            figure *target = figure_get(target_id);
            if (target->type == FIGURE_WARSHIP && target->action_state == FIGURE_ACTION_WARSHIP_DOCKED) {
                reduce = ENEMY_WARSHIP_DOCKED_TARGET_REDUCTION;
            }
            int32_t divisor = figure_ship_direction_table[(facing_direction(target) * DIRECTION_COUNT) + f->direction];
            int32_t damage = f->ship.steps < figure_properties[f->type].ship_steps ? f->ram_damage_fast : f->ram_damage_slow;
            damage = (damage - reduce) / divisor;
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
            f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_ATTACKING;
            f->target_figure_id = 0;
            figure_create_explosion_cloud(target->x, target->y, 0, 0);
        }
    }
    if (f->direction == DIRECTION_AT_DESTINATION) {
        f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_ATTACKING;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
        return;
    }
    if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_id);
    } else if (f->direction == DIRECTION_LOST) {
        f->action_state = FIGURE_ACTION_CORPSE;
        figure_route_remove(figure_id);
    }
}

void enemy_warship_evade(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (f->warship_secondary_action == WARSHIP_MODE_SAILING) {
        enemy_warship_move(figure_id, moving);
        if (f->direction == DIRECTION_AT_DESTINATION) {
            f->warship_secondary_action = WARSHIP_MODE_WAITING;
        }
        return;
    }
    if (f->warship_secondary_action != WARSHIP_MODE_WAITING) {
        return;
    }
    if (++f->wait_ticks < WARSHIP_EVADE_TICKS) {
        return;
    }
    f->wait_ticks = 0;
    int32_t enemy_id = ship_find_closest(figure_id, WARSHIP_EVADE_RANGE, 1);
    if (enemy_id == 0) {
        f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING;
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

void enemy_warship_attack(int32_t figure_id, int32_t *moving)
{
    figure *f = figure_get(figure_id);
    if (f->warship_secondary_action != WARSHIP_MODE_FOLLOW_TARGET) {
        if (f->warship_secondary_action != WARSHIP_MODE_WAITING) {
            return;
        }
        int32_t target_id = enemy_warship_find_target(figure_id, GRID_SIZE);
        if (target_id == 0) {
            f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING;
            return;
        }
        figure *target = figure_get(target_id);
        int32_t on_land = routing_grids.water[map_grid_offset(target->x, target->y)] == ROUTING_WATER_BLOCKED;
        switch (target->type) {
            case FIGURE_ARCHER:
            case FIGURE_INFANTRY:
            case FIGURE_CHARIOTEER:
            case FIGURE_TRANSPORT_SHIP:
            case FIGURE_WARSHIP:
                if (crt_rand() & 1) {
                    f->ship_leave_month = (uint8_t) ((game_time_month + 1) % MONTHS_PER_YEAR);
                }
                break;
        }
        if (target->type == FIGURE_INFANTRY || target->type == FIGURE_ARCHER || target->type == FIGURE_CHARIOTEER) {
            f->destination_x = f->x;
            f->destination_y = f->y;
        } else if (on_land) {
            int32_t tx = f->x < target->x ? target->x - 1 : target->x;
            int32_t ty = f->y < target->y ? target->y - 1 : target->y;
            int32_t path_id;
            if (!figure_route_get_first_available(target_id, &path_id)) {
                f->destination_x = f->x;
                f->destination_y = f->y;
            } else if (map_routing_get_path_on_water(path_id, f->x, f->y, tx, ty, 0)) {
                f->destination_x = (int16_t) tx;
                f->destination_y = (int16_t) ty;
            } else {
                f->destination_x = f->x;
                f->destination_y = f->y;
            }
        } else {
            f->destination_x = target->x;
            f->destination_y = target->y;
            *moving = 1;
        }
        f->warship_secondary_action = WARSHIP_MODE_FOLLOW_TARGET;
        f->direction = f->previous_tile_direction;
        f->target_figure_id = (int16_t) target_id;
        figure_route_remove(figure_id);
        return;
    }
    int32_t target_id = f->target_figure_id;
    const figure *target = figure_get(target_id);
    int32_t distance = calc_maximum_distance(f->x, f->y, target->x, target->y);
    int32_t in_range = 0;
    if (distance < figure_properties[f->type].missile_range) {
        in_range = 1;
        if (++f->wait_ticks_missile > figure_properties[f->type].missile_delay) {
            f->wait_ticks_missile = 0;
            f->attack_image_offset = target_id != 0;
        } else {
            f->attack_image_offset = 0;
        }
    } else {
        f->attack_image_offset = 0;
    }
    if (f->attack_image_offset) {
        if (f->attack_image_offset == 1) {
            figure_create_missile(figure_id, f->x, f->y, target->x, target->y, FIGURE_SPEAR);
        }
        if (++f->attack_image_offset > WARSHIP_SHOOT_TICKS) {
            f->attack_image_offset = 0;
        }
    }
    int32_t target_type = target->type;
    if (target_type >= FIGURE_ARCHER && target_type <= FIGURE_CHARIOTEER) {
        if (in_range) {
            *moving = 0;
        }
    } else {
        enemy_warship_ram(figure_id, moving);
    }
    f->wait_ticks++;
    if (f->direction == DIRECTION_AT_DESTINATION || f->wait_ticks >= ENEMY_WARSHIP_ATTACK_TICKS) {
        f->wait_ticks = 0;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
        figure_route_remove(figure_id);
    }
}

int32_t enemy_landing_water_tile(int32_t *x, int32_t *y)
{
    for (int32_t i = 0; i < SCENARIO_INVASION_LANDINGS; i++) {
        if (scenario_data.invasion_landing_x[i] != -1 && scenario_data.invasion_landing_y[i] != -1) {
            *x = scenario_data.invasion_landing_x[i];
            *y = scenario_data.invasion_landing_y[i];
            map_water_find_free_tile(x, y);
            return 1;
        }
    }
    return 0;
}

void enemy_warship_patrol(int32_t figure_id, int32_t *moving)
{
    ship_move(figure_id, SHIP_MOVE_STRAIGHT);
    figure *f = figure_get(figure_id);
    *moving = 1;
    f->wait_ticks++;
    if ((crt_rand() % ENEMY_WARSHIP_PATROL_RANDOM_TICKS) + ENEMY_WARSHIP_PATROL_BASE_TICKS <= f->wait_ticks) {
        f->wait_ticks = 0;
        f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_ATTACKING;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
    }
    if (f->direction == DIRECTION_REROUTE || f->direction == DIRECTION_LOST) {
        figure_route_remove(figure_id);
        return;
    }
    if (f->direction != DIRECTION_AT_DESTINATION) {
        return;
    }
    int32_t x = figure_id;
    int32_t y;
    uint8_t found = (uint8_t) enemy_landing_water_tile(&x, &y);
    if (f->x == scenario_data.river_entry_x && f->y == scenario_data.river_entry_y) {
        if (!found) {
            f->destination_x = scenario_data.river_exit_x;
            f->destination_y = scenario_data.river_exit_y;
        } else {
            f->destination_x = (int16_t) x;
            f->destination_y = (int16_t) y;
        }
    } else if (!found || f->x != x || f->y != y) {
        f->destination_x = scenario_data.river_entry_x;
        f->destination_y = scenario_data.river_entry_y;
    } else {
        f->destination_x = scenario_data.river_exit_x;
        f->destination_y = scenario_data.river_exit_y;
    }
    f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING;
    figure_route_remove(figure_id);
}

void enemy_warship_leave(int32_t figure_id, int32_t *moving)
{
    ship_move(figure_id, SHIP_MOVE_STRAIGHT);
    *moving = 1;
    figure *f = figure_get(figure_current_id);
    if (f->direction == DIRECTION_REROUTE) {
        figure_route_remove(figure_current_id);
        return;
    }
    if (f->direction == DIRECTION_LOST || f->direction == DIRECTION_AT_DESTINATION) {
        f->state = FIGURE_STATE_DEAD;
    }
}

void figure_enemy_warship_action(void)
{
    int32_t moving = 0;
    ship_action_start(FIGURE_ACTION_ENEMY_WARSHIP_EVADING);
    int32_t id = figure_current_id;
    figure *f = figure_get(id);
    if ((f->ship_leave_month + ENEMY_WARSHIP_STAY_MONTHS) % MONTHS_PER_YEAR == game_time_month) {
        f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_LEAVING;
        f->destination_x = scenario_data.river_exit_x;
        f->destination_y = scenario_data.river_exit_y;
    }
    enemy_ship_reroute_tick();
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            enemy_ship_sink(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_WAITING:
            enemy_warship_wait(id, &moving);
            enemy_warship_shoot(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_RAMMING:
            enemy_warship_ram(id, &moving);
            enemy_warship_shoot(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_ATTACKING:
            enemy_warship_attack(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_MOVING:
            enemy_warship_move(id, &moving);
            enemy_warship_shoot(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_EVADING:
            enemy_warship_evade(id, &moving);
            enemy_warship_shoot(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING:
            enemy_warship_patrol(id, &moving);
            enemy_warship_shoot(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_LEAVING:
            enemy_warship_leave(id, &moving);
            break;
        default:
            f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING;
            break;
    }
    int32_t moving_image = moving && f->ship_move_mode != SHIP_MOVE_RESTING;
    ship_set_image(IMAGE_GROUP_FIGURE_ENEMY_WARSHIP_SINKING, IMAGE_GROUP_FIGURE_ENEMY_WARSHIP_MOVING, IMAGE_GROUP_FIGURE_ENEMY_WARSHIP_STILL, moving_image, 0);
}

static void egyptian_ship_sinking_image(figure *f, int32_t sinking_base, int32_t direction)
{
    int32_t wait = f->wait_ticks;
    if (wait < ENEMY_SHIP_SINKING_TICKS + 1) {
        f->image_id = (direction / DIRECTION_COUNT) + sinking_base + (wait / ENEMY_SHIP_FRAME_TICKS);
        return;
    }
    int32_t index = wait - (ENEMY_SHIP_SINKING_TICKS + 1);
    if (index > ENEMY_SHIP_LAST_BUBBLE_FRAME) {
        index = ENEMY_SHIP_LAST_BUBBLE_FRAME;
    }
    f->image_id = index + image_group(IMAGE_GROUP_FIGURE_BUBBLES);
}

static int32_t egyptian_ship_start(figure *f)
{
    if (++f->animation_frame >= ENEMY_SHIP_ANIMATION_FRAMES) {
        f->animation_frame = 0;
    }
    if (f->ship_hit_points < 1) {
        f->action_state = FIGURE_ACTION_CORPSE;
    }
    return calc_percentage(f->ship_hit_points, f->ship_max_hit_points);
}

void figure_egyptian_warship_action(void)
{
    int32_t moving = 0;
    int32_t id = figure_current_id;
    figure *f = figure_get(id);
    int32_t health = egyptian_ship_start(f);
    if (health <= ENEMY_SHIP_DAMAGED_PERCENT && health > 0) {
        f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_EVADING;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
    }
    if ((f->ship_leave_month + ENEMY_WARSHIP_STAY_MONTHS) % MONTHS_PER_YEAR == game_time_month) {
        f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_LEAVING;
        f->destination_x = (int16_t) scenario_data.river_exit_x;
        f->destination_y = (int16_t) scenario_data.river_exit_y;
    }
    enemy_ship_reroute_tick();
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            enemy_ship_sink(id);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_WAITING:
            enemy_warship_wait(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_RAMMING:
            enemy_warship_ram(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_ATTACKING:
            enemy_warship_attack(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_MOVING:
            enemy_warship_move(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_EVADING:
            enemy_warship_evade(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING:
            enemy_warship_patrol(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_WARSHIP_LEAVING:
            enemy_warship_leave(id, &moving);
            break;
        default:
            f->action_state = FIGURE_ACTION_ENEMY_WARSHIP_PATROLLING;
            break;
    }
    int32_t direction = facing_direction(f) - map_info.orientation;
    if (direction < 0) {
        direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        egyptian_ship_sinking_image(f, image_group(IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_WARSHIP_SINKING), direction);
        return;
    }
    if (moving && f->ship_move_mode != SHIP_MOVE_RESTING) {
        int32_t frame = f->animation_frame / ENEMY_SHIP_FRAME_TICKS;
        f->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_WARSHIP_MOVING) + (frame * DIRECTION_COUNT) + direction;
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_WARSHIP_STILL) + direction;
}

void figure_egyptian_transport_action(void)
{
    int32_t moving = 1;
    int32_t id = figure_current_id;
    figure *f = figure_get(id);
    int32_t health = egyptian_ship_start(f);
    if (health <= ENEMY_SHIP_DAMAGED_PERCENT && health > 0) {
        f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_EVADING;
        f->warship_secondary_action = WARSHIP_MODE_WAITING;
        f->wait_ticks = ENEMY_TRANSPORT_EVADE_WAIT;
    }
    enemy_ship_reroute_tick();
    switch (f->action_state) {
        case FIGURE_ACTION_CORPSE:
            enemy_ship_sink(id);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_WAITING:
            enemy_transport_wait(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_MOVING:
            enemy_transport_move(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_EVADING:
            enemy_transport_evade(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_IDLE:
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_GOING_TO_LANDING:
            enemy_transport_go_to_landing(id, &moving);
            break;
        case FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING:
            enemy_transport_go_away(id);
            break;
        default:
            f->action_state = FIGURE_ACTION_ENEMY_TRANSPORT_LEAVING;
            break;
    }
    figure_image_direction = facing_direction(f) - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTION_COUNT;
    }
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        egyptian_ship_sinking_image(f, image_group(IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SINKING), figure_image_direction);
        return;
    }
    if (moving) {
        int32_t frame = f->animation_frame / ENEMY_SHIP_FRAME_TICKS;
        f->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_TRANSPORT_MOVING) + (frame * DIRECTION_COUNT) + figure_image_direction;
        return;
    }
    f->image_id = image_group(IMAGE_GROUP_FIGURE_ENEMY_EGYPTIAN_TRANSPORT_STILL) + figure_image_direction;
}

int32_t formation_create(int32_t figure_type, int32_t layout, int32_t orientation, int32_t x, int32_t y)
{
    for (int32_t id = FORMATION_FIRST_CREATED_ID; id < MAX_FORMATIONS; id++) {
        if (formation_get(id)->in_use) {
            continue;
        }
        if (formation_get(id)->herd_kind) {
            formation_get(id)->in_use = 1;
            continue;
        }
        formation_get(id)->home_x = (int16_t) x;
        formation_get(id)->home_y = (int16_t) y;
        formation_get(id)->in_use = 1;
        formation_get(id)->is_legion = 0;
        formation_get(id)->faction = 0;
        formation_get(id)->figure_type = (int16_t) figure_type;
        formation_get(id)->legion_id = (uint8_t) (id - FORMATION_FIRST_CREATED_ID);
        if (layout != FORMATION_ENEMY_DOUBLE_LINE) {
            formation_get(id)->layout = (int16_t) layout;
        } else if (orientation != DIRECTION_TOP && orientation != DIRECTION_BOTTOM) {
            formation_get(id)->layout = FORMATION_DOUBLE_LINE_2;
        } else {
            formation_get(id)->layout = FORMATION_DOUBLE_LINE_1;
        }
        formation_get(id)->morale = FORMATION_CREATED_MORALE;
        return id;
    }
    return 0;
}

void figure_count_traders(void)
{
    city_player *player = city_player_current();
    player->enemy_warships = 0;
    for (int32_t id = 1; id <= MAX_FIGURES - 1; id++) {
        const figure *f = figure_get(id);
        uint8_t type = f->type;
        if ((type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_EGYPTIAN_WARSHIP) && f->state) {
            player->enemy_warships++;
        }
    }
}
