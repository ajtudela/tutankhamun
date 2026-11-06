// Legions and warships sent to the kingdom's requests for troops: the strength sent, the dispatch by sea (on the
// transport ships, with the warships) or by land (to the city's exit point), the losses when the request is settled
// (a legion that loses all its soldiers sinks its ship) and the way home.

#include "city/legion_requests.h"

#include "building/building.h"
#include "building/building_type.h"
#include "city/city_internal.h"
#include "empire/empire_data.h"
#include "empire/empire_helpers.h"
#include "figure/boats.h"
#include "figure/enemy_ships.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "io/game_file_internal.h"
#include "map/water.h"
#include "scenario/scenario_internal.h"

static int32_t figure_is_alive_soldier(int32_t figure_id)
{
    if (figure_id <= 0) {
        return 0;
    }
    const figure *soldier = figure_get(figure_id);
    return soldier->state == FIGURE_STATE_ALIVE && soldier->action_state != FIGURE_ACTION_CORPSE;
}

int32_t figure_warship_strength(int32_t figure_id)
{
    return figure_get(figure_id)->ship_hit_points / WARSHIP_STRENGTH_DIVISOR;
}

int32_t formation_request_strength(int32_t request_id, int32_t *warships, int32_t *legions)
{
    if (request_id < 0) {
        return 0;
    }
    int32_t legion_strength = 0;
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *f = formation_get(id);
        if (f->in_use != 1 || !f->is_legion || !f->empire_service || f->num_figures == 0 || f->request_event_id != request_id) {
            continue;
        }
        int32_t base = f->experience + ((f->figure_type == FIGURE_CHARIOTEER) ? CHARIOTEER_STRENGTH_BASE : LEGION_STRENGTH_BASE);
        legion_strength += (base * f->num_figures) / 100;
    }
    if (legions) {
        *legions = legion_strength;
    }
    int32_t total = legion_strength;
    uint32_t faction = (uint32_t) city_current_player;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *warship = figure_get(id);
        if (warship->state && warship->type == FIGURE_WARSHIP && warship->faction_id == faction && warship->request_event_id == request_id) {
            total += figure_warship_strength(id);
        }
    }
    if (warships) {
        *warships = total - legion_strength;
    }
    return total;
}

int32_t figure_find_legion_transport_ship(int32_t formation_id)
{
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        const figure *ship = figure_get(id);
        if (ship->state && ship->type == FIGURE_TRANSPORT_SHIP && ship->ship_formation_id == (uint32_t) formation_id) {
            figure_current_id = id;
            return id;
        }
    }
    figure_current_id = MAX_FIGURES;
    return 0;
}

void formation_legions_dispatch_to_request(int32_t event_id)
{
    if (event_id < 0 || event_id >= scenario_events.count) {
        return;
    }
    int32_t route = scenario_events.events[event_id].route.value;
    if (route < 0 || route >= EMPIRE_ROUTES) {
        return;
    }
    int32_t by_sea = (empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA);
    const city_player *player = city_player_current();
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *f = formation_get(id);
        if (f->in_use != 1 || !f->is_legion || !f->empire_service || f->num_figures == 0 || f->request_event_id >= 0) {
            continue;
        }
        int32_t on_ship = (formation_legion_is_on_ship(id) != 0);
        if (on_ship != by_sea) {
            continue;
        }
        f->request_event_id = (int16_t) event_id;
        f->is_at_fort = 0;
        if (on_ship) {
            figure_current_id = figure_find_legion_transport_ship(id);
            if (figure_current_id != 0) {
                figure_get(figure_current_id)->action_state = FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_REQUEST;
            }
            continue;
        }
        f->order_x = player->exit_point_x;
        f->order_y = player->exit_point_y;
        for (int32_t i = 0; i < f->num_figures; i++) {
            int32_t figure_id = f->figures[i];
            if (figure_is_alive_soldier(figure_id)) {
                figure_get(figure_id)->action_state = FIGURE_ACTION_SOLDIER_GOING_TO_REQUEST;
                figure_route_remove(figure_id);
            }
        }
    }
    if (!by_sea) {
        return;
    }
    uint32_t faction = (uint32_t) city_current_player;
    figure_current_id = MAX_FIGURES;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *warship = figure_get(id);
        if (warship->state && warship->type == FIGURE_WARSHIP && warship->faction_id == faction && warship->answers_requests && warship->request_event_id < 0) {
            warship->request_event_id = (int16_t) event_id;
            warship->action_state = FIGURE_ACTION_WARSHIP_GOING_TO_REQUEST;
        }
    }
}

void formation_legions_return_from_request(int32_t event_id)
{
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *f = formation_get(id);
        if (f->in_use != 1 || !f->is_legion || f->request_event_id != event_id) {
            continue;
        }
        f->request_event_id = -1;
        if (formation_legion_is_on_ship(id)) {
            int32_t ship_id = figure_find_legion_transport_ship(id);
            if (ship_id == 0) {
                continue;
            }
            figure *ship = figure_get(ship_id);
            ship->source_x = ship->x;
            ship->source_y = ship->y;
            int32_t x = 0;
            int32_t y = 0;
            map_water_farthest_on_line(ship->x, ship->y, f->x, f->y, &x, &y);
            ship->destination_x = (int16_t) x;
            ship->destination_y = (int16_t) y;
            ship->ship_landing_x = f->x;
            ship->ship_landing_y = f->y;
            ship->action_state = FIGURE_ACTION_TRANSPORT_SHIP_RETURNING_FROM_REQUEST;
            continue;
        }
        int32_t count = f->num_figures;
        for (int32_t i = 0; i < count; i++) {
            int32_t figure_id = f->figures[i];
            if (figure_is_alive_soldier(figure_id)) {
                figure_get(figure_id)->action_state = FIGURE_ACTION_SOLDIER_RETURNING_FROM_REQUEST;
            }
        }
    }
    int32_t saved_x = map_result_x;
    int32_t saved_y = map_result_y;
    uint32_t faction = (uint32_t) city_current_player;
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *warship = figure_get(id);
        if (!warship->state || warship->type != FIGURE_WARSHIP || warship->request_event_id != event_id || warship->faction_id != faction) {
            continue;
        }
        warship->source_x = warship->x;
        warship->source_y = warship->y;
        const building *dock = building_get(warship->building_id);
        map_water_tile_next_to(dock->x, dock->y, dock->size);
        warship->destination_x = (int16_t) map_result_x;
        warship->destination_y = (int16_t) map_result_y;
        warship->request_event_id = -1;
        map_result_x = saved_x;
        map_result_y = saved_y;
        warship->action_state = FIGURE_ACTION_WARSHIP_RETURNING_FROM_REQUEST;
    }
}

void formation_legions_request_losses(int32_t percentage, int32_t event_id)
{
    int32_t warships = 0;
    int32_t legions = 0;
    formation_request_strength(event_id, &warships, &legions);
    for (int32_t id = 1; id < MAX_FORMATIONS; id++) {
        formation *f = formation_get(id);
        if (f->in_use != 1 || !f->is_legion || f->request_event_id != event_id) {
            continue;
        }
        int32_t alive = 0;
        int32_t count = f->num_figures;
        for (int32_t i = 0; i < count; i++) {
            if (figure_is_alive_soldier(f->figures[i])) {
                alive++;
            }
        }
        int32_t lost = calc_adjust_with_percentage(alive, percentage);
        if (lost >= alive) {
            if (formation_legion_is_on_ship(id)) {
                figure_current_id = figure_find_legion_transport_ship(id);
                if (figure_current_id != 0) {
                    figure *ship = figure_get(figure_current_id);
                    ship->ship_hit_points = 0;
                    ship->state = FIGURE_STATE_SUNK;
                    ship->request_event_id = -1;
                }
            }
            f->is_at_fort = 1;
            f->request_event_id = -1;
        }
        count = f->num_figures;
        for (int32_t i = 0; i < count; i++) {
            int32_t figure_id = f->figures[i];
            if (figure_is_alive_soldier(figure_id) && lost != 0) {
                lost--;
                figure_get(figure_id)->state = FIGURE_STATE_DEAD;
            }
        }
    }
    int32_t damage = calc_adjust_with_percentage(warships * WARSHIP_STRENGTH_DIVISOR, percentage);
    uint32_t faction = (uint32_t) city_current_player;
    int32_t id;
    for (id = 1; id < MAX_FIGURES; id++) {
        figure *warship = figure_get(id);
        if (!warship->state || warship->type != FIGURE_WARSHIP || warship->faction_id != faction || warship->request_event_id != event_id) {
            continue;
        }
        int32_t strength = warship->ship_hit_points;
        if (strength >= damage && percentage != REQUEST_LOSSES_ALL) {
            figure_current_id = id;
            warship->ship_hit_points = strength - damage;
            return;
        }
        warship->ship_hit_points = 0;
        warship->action_state = FIGURE_ACTION_CORPSE;
        damage -= strength;
        warship->request_event_id = -1;
    }
    figure_current_id = id;
}
