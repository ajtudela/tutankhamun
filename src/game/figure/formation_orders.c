// The legions' orders from the interface (move, attack near a tile, return to the fort, go to the standard), with the
// checks that refuse them (morale, a ferry, the game paused, an unreachable tile, Seth's curse) and the warnings they
// give, and the military queries behind the interface: the legion at a fort, the fighter on a tile, the n-th legion or
// warship, a warship's return to its dock.
//
// A charging soldier that takes an order resets the animation frame of figure_current_id, not its own, and
// figure_warship_return_to_dock removes the route of figure_current_id.

#include "figure/formation_orders.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "city/message.h"
#include "figure/boats.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_citizen.h"
#include "map/routing_land.h"
#include "map/water.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t figure_soldier_refuses_orders(int32_t figure_id)
{
    uint8_t action = city_figures.records[figure_id].action_state;
    int32_t at_ferry = (action == FIGURE_ACTION_ON_FERRY) || (action == FIGURE_ACTION_WAIT_FOR_FERRY);
    int32_t going_to_fort = (action == FIGURE_ACTION_SOLDIER_TRAINED_GOING_TO_FORT) || (action == FIGURE_ACTION_SOLDIER_RECRUIT_GOING_TO_FORT);
    return at_ferry || going_to_fort;
}

static int32_t soldier_takes_order(int32_t formation_id, int32_t figure_id, int32_t refuse_attacking)
{
    const formation *legion = formation_get(formation_id);
    uint8_t action = city_figures.records[figure_id].action_state;
    if (action == FIGURE_ACTION_CORPSE || (refuse_attacking && action == FIGURE_ACTION_ATTACK) || legion->months_very_low_morale != 0 ||
        legion->months_low_morale > 1 || figure_soldier_refuses_orders(figure_id)) {
        return 0;
    }
    if (legion->months_low_morale == 1) {
        formation_change_morale(formation_id, LEGION_ORDER_LOW_MORALE_CHANGE);
    }
    return 1;
}

static int32_t legion_is_charging(int32_t formation_id)
{
    const formation *legion = formation_get(formation_id);
    return legion->figure_type == FIGURE_CHARIOTEER && city_figures.records[legion->figures[0]].action_state == FIGURE_ACTION_SOLDIER_CHARGE && button_parameter != 1;
}

int32_t formation_legion_move_to(int32_t x, int32_t y, int32_t formation_id, bool quiet)
{
    formation *legion = formation_get(formation_id);
    if (settings_data.paused != 0) {
        return 0;
    }
    const figure *bearer = figure_get(legion->standard_figure_id);
    map_routing_citizen_can_travel_over_land(legion->x, legion->y, -2, -2);
    int32_t ok;
    if (legion->ship_figure_id != 0) {
        ok = map_routing_citizen_can_travel_no_ferry(legion->x, legion->y, x, y);
    } else {
        ok = map_routing_citizen_can_travel_no_ferry(bearer->x, bearer->y, x, y);
    }
    if (ok == 0) {
        city_warning_show(WARNING_COMPANY_CANNOT_REACH_DESTINATION);
        return 0;
    }
    if (routing_grids.distance[map_info.grid_start + y * GRID_SIZE + x] <= 0) {
        return 0;
    }
    if (x == legion->x && y == legion->y) {
        return 0;
    }
    if (legion->cursed_by_seth != 0) {
        return 0;
    }
    legion->order_x = (int16_t) x;
    legion->order_y = (int16_t) y;
    legion->is_at_fort = 0;
    if (quiet) {
        return 1;
    }
    if (legion->morale <= LEGION_MORALE_WARNING) {
        city_warning_show(WARNING_COMPANY_MORALE_TOO_LOW);
    }
    if (legion_is_charging(formation_id)) {
        for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
            int32_t id = legion->figures[i];
            if (id == 0) {
                break;
            }
            if (!soldier_takes_order(formation_id, id, 0)) {
                continue;
            }
            figure *soldier = figure_get(id);
            soldier->alternative_location_index = 0;
            soldier->charge.steps = 0;
            legion->layout = FORMATION_CHARGE;
            soldier->charge.tile_x = soldier->x;
            soldier->charge.tile_y = soldier->y;
            soldier->action_state = FIGURE_ACTION_SOLDIER_ENGAGING;
            city_figures.records[figure_current_id].animation_frame = 0;
        }
    }
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t id = legion->figures[i];
        if (id == 0) {
            break;
        }
        if (!soldier_takes_order(formation_id, id, 1)) {
            continue;
        }
        figure *soldier = figure_get(id);
        soldier->alternative_location_index = 0;
        soldier->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_STANDARD;
        figure_route_remove(id);
    }
    return 1;
}

int32_t formation_legion_attack_near(int32_t x, int32_t y, int32_t formation_id, int32_t target)
{
    formation *legion = formation_get(formation_id);
    static const int8_t offsets[LEGION_ATTACK_NEAR_TILES][2] = {
        { -1, -1 },
        { -1, 0 },
        { -1, 1 },
        { 0, -1 },
        { 0, 0 },
        { 0, 1 },
        { 1, -1 },
        { 1, 0 },
        { 1, 1 },
    };
    const figure *bearer = figure_get(legion->standard_figure_id);
    if (settings_data.paused != 0) {
        return 0;
    }
    map_routing_citizen_can_travel_over_land(legion->x, legion->y, -2, -2);
    int32_t i;
    int32_t ok = 0;
    for (i = 0; i < LEGION_ATTACK_NEAR_TILES; i++) {
        ok = map_routing_citizen_can_travel_no_ferry(bearer->x, bearer->y, x + offsets[i][0], y + offsets[i][1]);
        if (ok != 0) {
            break;
        }
    }
    if (ok == 0) {
        city_warning_show(WARNING_COMPANY_CANNOT_REACH_DESTINATION);
        return 0;
    }
    int32_t tx = x + offsets[i][0];
    int32_t ty = y + offsets[i][1];
    if (routing_grids.distance[tx + ty * GRID_SIZE + map_info.grid_start] <= 0) {
        return 0;
    }
    if (tx == legion->x && ty == legion->y) {
        return 0;
    }
    if (legion->cursed_by_seth != 0) {
        return 0;
    }
    legion->order_x = legion->x;
    legion->order_y = legion->y;
    legion->is_at_fort = 0;
    legion->attack_formation_id = target;
    if (legion->morale <= LEGION_MORALE_WARNING) {
        city_warning_show(WARNING_COMPANY_MORALE_TOO_LOW);
    }
    if (legion_is_charging(formation_id)) {
        for (int32_t n = 0; n < FORMATION_MAX_FIGURES; n++) {
            int32_t id = legion->figures[n];
            if (id == 0) {
                break;
            }
            if (!soldier_takes_order(formation_id, id, 0)) {
                continue;
            }
            figure *soldier = figure_get(id);
            soldier->alternative_location_index = 0;
            soldier->charge.steps = 0;
            soldier->charge.tile_x = soldier->x;
            soldier->charge.tile_y = soldier->y;
            soldier->action_state = FIGURE_ACTION_SOLDIER_ATTACKING_FORMATION;
            soldier->target_figure_id = 0;
            city_figures.records[figure_current_id].animation_frame = 0;
        }
    }
    for (int32_t n = 0; n < FORMATION_MAX_FIGURES; n++) {
        int32_t id = legion->figures[n];
        if (id == 0) {
            break;
        }
        if (!soldier_takes_order(formation_id, id, 1)) {
            continue;
        }
        figure *soldier = figure_get(id);
        soldier->alternative_location_index = 0;
        soldier->action_state = FIGURE_ACTION_SOLDIER_ATTACKING_FORMATION;
        soldier->target_figure_id = 0;
        figure_route_remove(id);
    }
    return 1;
}

int32_t formation_legion_return_to_fort(int32_t x, int32_t y, int32_t formation_id)
{
    formation *legion = formation_get(formation_id);
    const figure *bearer = figure_get(legion->standard_figure_id);
    if (settings_data.paused != 0) {
        return 0;
    }
    map_routing_citizen_can_travel_over_land(legion->x, legion->y, -2, -2);
    if (map_routing_citizen_can_travel_no_ferry(bearer->x, bearer->y, x, y) == 0) {
        city_warning_show(WARNING_COMPANY_CANNOT_REACH_DESTINATION);
        return 0;
    }
    if (routing_grids.distance[map_info.grid_start + y * GRID_SIZE + x] <= 0) {
        return 0;
    }
    if (x == legion->x && y == legion->y) {
        return 0;
    }
    if (legion->cursed_by_seth != 0) {
        return 0;
    }
    legion->order_x = (int16_t) x;
    legion->order_y = (int16_t) y;
    legion->has_move_order = 1;
    legion->move_order_ticks = 0;
    legion->is_at_fort = 0;
    if (legion->morale <= LEGION_MORALE_WARNING) {
        city_warning_show(WARNING_COMPANY_MORALE_TOO_LOW);
    }
    legion->layout = FORMATION_COLUMN;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t id = legion->figures[i];
        if (id == 0) {
            break;
        }
        if (!soldier_takes_order(formation_id, id, 1)) {
            continue;
        }
        figure *soldier = figure_get(id);
        soldier->alternative_location_index = 0;
        soldier->charge.steps = 0;
        soldier->charge.tile_x = soldier->x;
        soldier->charge.tile_y = soldier->y;
        soldier->action_state = FIGURE_ACTION_SOLDIER_CHARGE;
        figure_route_remove(id);
    }
    return 1;
}

void formation_legion_to_standard(int32_t formation_id, int32_t action)
{
    formation *legion = formation_get(formation_id);
    const figure *bearer = figure_get(legion->standard_figure_id);
    int32_t ship;
    if (action != 0 && (ship = legion->ship_figure_id) != 0) {
        figure *ship_figure = figure_get(ship);
        ship_figure->action_state = (uint8_t) action;
        int16_t tx;
        int16_t ty;
        if (action == -1) {
            tx = legion->x;
            ty = legion->y;
        } else {
            tx = legion->home_x;
            ty = legion->home_y;
        }
        int32_t wx;
        int32_t wy;
        map_water_farthest_on_line(ship_figure->x, ship_figure->y, tx, ty, &wx, &wy);
        ship_figure->destination_x = (int16_t) wx;
        ship_figure->destination_y = (int16_t) wy;
        ship_figure->ship_landing_x = tx;
        ship_figure->ship_landing_y = ty;
        figure_route_remove(ship);
        return;
    }
    map_routing_citizen_can_travel_over_land(legion->x, legion->y, -2, -2);
    int16_t sx = legion->home_x;
    int32_t row = map_info.grid_start + legion->home_y * GRID_SIZE;
    if (map_routing_citizen_can_travel_no_ferry(bearer->x, bearer->y, sx, legion->home_y) == 0) {
        city_warning_show(WARNING_COMPANY_CANNOT_REACH_DESTINATION);
        return;
    }
    if (routing_grids.distance[row + sx] <= 0 || legion->cursed_by_seth != 0) {
        return;
    }
    legion->is_at_fort = 1;
    for (int32_t i = 0; i < FORMATION_MAX_FIGURES; i++) {
        int32_t id = legion->figures[i];
        if (id == 0) {
            return;
        }
        if (!soldier_takes_order(formation_id, id, 1)) {
            continue;
        }
        figure *soldier = figure_get(id);
        soldier->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_FORT;
        figure_route_remove(id);
    }
}

int32_t formation_legion_at_building(int32_t grid_offset)
{
    int32_t id = map_grids.building_ids[grid_offset];
    const building *b = building_get(id);
    if (id > 0 && b->state == BUILDING_STATE_IN_USE) {
        int16_t type = b->type;
        if (type == BUILDING_FORT_GROUND || type == BUILDING_FORT) {
            return b->formation_id;
        }
    }
    return 0;
}

int32_t figure_fighter_at_grid_offset(int32_t grid_offset)
{
    int32_t id = (int16_t) map_grids.figure_ids[grid_offset];
    while (id >= 1) {
        const figure *f = figure_get(id);
        if (f->action_state != FIGURE_ACTION_CORPSE) {
            uint8_t type = f->type;
            if ((type > FIGURE_TOWER_SENTRY && type < FIGURE_NATIVE_TRADER) || (type == FIGURE_NATIVE && f->action_state == FIGURE_ACTION_NATIVE_ATTACKING) ||
                type == FIGURE_CROCODILE || type == FIGURE_HYENA || type == FIGURE_SCORPION || type == FIGURE_ASP || type == FIGURE_LION ||
                type == FIGURE_HIPPO || type == FIGURE_MUMMY || type == FIGURE_BEDOUIN || type == FIGURE_ENEMY_EGYPTIAN_WARSHIP ||
                type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP || type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_TRANSPORT_SHIP) {
                return id;
            }
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

int32_t figure_warship_by_index(int32_t index)
{
    int32_t n = 1;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *f = figure_get(id);
        if (f->state != FIGURE_STATE_NONE && f->type == FIGURE_WARSHIP && f->faction_id == (uint8_t) city_current_player) {
            if (n == index) {
                return id;
            }
            n++;
        }
    }
    return 0;
}

void figure_warship_return_to_dock(int32_t figure_id)
{
    int32_t saved_y = map_result_y;
    int32_t saved_x = map_result_x;
    figure *warship = figure_get(figure_id);
    warship->action_state = FIGURE_ACTION_WARSHIP_SAILING;
    const building *dock = building_get(warship->building_id);
    map_water_tile_next_to(dock->x, dock->y, dock->size);
    warship->source_x = warship->x;
    warship->source_y = warship->y;
    warship->destination_x = (int16_t) map_result_x;
    warship->destination_y = (int16_t) map_result_y;
    map_result_x = saved_x;
    map_result_y = saved_y;
    figure_route_remove(figure_current_id);
}

int32_t formation_any_legion_with_soldiers(void)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        const formation *m = formation_get(id);
        if (m->in_use == 1 && m->is_legion != 0 && m->legion_recruit_type > 0) {
            return 1;
        }
    }
    return 0;
}

int32_t formation_for_legion(int32_t index)
{
    int32_t n = 1;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        const formation *m = formation_get(id);
        if (m->in_use == 1 && m->is_legion != 0) {
            if (n == index) {
                return id;
            }
            n++;
        }
    }
    return 0;
}

int32_t formation_grid_offset_for_invasion(int32_t value)
{
    int32_t id;
    for (id = 1; id < MAX_FORMATIONS; id++) {
        const formation *m = formation_get(id);
        if (m->in_use == 1 && m->is_legion == 0 && m->herd_kind == HERD_KIND_NONE && m->invasion_sequence == value) {
            break;
        }
    }
    if (id >= MAX_FORMATIONS) {
        return 0;
    }
    const formation *found = formation_get(id);
    if (found->x < 1 && found->y < 1) {
        return 0;
    }
    return map_info.grid_start + found->x + found->y * GRID_SIZE;
}
