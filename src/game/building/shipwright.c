// The shipwrights, docks and boats: the dock repair queues rebuilt after a load or an undo, the boat type the
// city lacks and the wharf a new boat goes to, the boat a dock or wharf ordered, the setup of a new warship or
// transport ship, and the free ferry path records.

#include "building/shipwright.h"
#include "building/building.h"
#include "building/building_type.h"
#include "core/calc.h"
#include "core/queue.h"
#include "figure/boats.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "figure/ships_player.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void building_repair_slots_update(void)
{
    for (int32_t slot = 0; slot < DOCK_QUEUES; slot++) {
        dock_queue_get(slot)->building_id = 0;
    }
    int32_t last = building_highest_id;
    for (int32_t id = 1; id <= last; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_UNUSED && b->type == BUILDING_SHIPWRIGHT && b->queue_slot != 0) {
            dock_queue_get(b->queue_slot - 1)->queue.flag = 1;
        }
    }
    last = building_highest_id;
    for (int32_t id = 1; id <= last; id++) {
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_UNUSED || b->type != BUILDING_SHIPWRIGHT) {
            continue;
        }
        uint8_t slot = b->queue_slot;
        if (slot == 0 || dock_queue_get(slot - 1)->building_id != 0) {
            building_repair_queue_add(id);
        } else {
            dock_queue_get(slot - 1)->building_id = id;
        }
    }
}

int32_t ferry_path_alloc(int32_t figure_id)
{
    (void) figure_id;
    for (int32_t index = 1; index < FERRY_PATHS; index++) {
        ferry_path *path = ferry_path_get(index);
        if (path->in_use == 0) {
            memset(path, 0, sizeof(ferry_path));
            path->in_use = 1;
            return index;
        }
    }
    return 0;
}

bool shipwright_boat_ordered_by(int32_t building_id, int32_t *type)
{
    uint8_t slot = building_get(building_id)->queue_slot;
    if (!slot) {
        return 0;
    }
    *type = dyn_queue_pop(&dock_queue_get(slot - 1)->queue);
    return *type != 0;
}

static int32_t wharf_boat_alive(int32_t id, building *b, int32_t boat_type)
{
    int32_t figure_id = b->data.dock.docked_boat_id;
    if (figure_id <= 0) {
        return 0;
    }
    figure *f = figure_get(figure_id);
    if (f->state == FIGURE_STATE_NONE || (f->type != boat_type && f->type != FIGURE_NONE) || f->building_id != id) {
        b->data.dock.docked_boat_id = 0;
        return 0;
    }
    return 1;
}

static int32_t city_boat_order(int32_t boat_type)
{
    return BOAT_ORDER_FOR_CITY | boat_type;
}

bool shipwright_boat_needed(int32_t *type)
{
    int32_t fishing_wharves = 0, fishing_boats = 0, fishing_orders = 0;
    int32_t transport_wharves = 0, transport_ships = 0, transport_orders = 0;
    int32_t warship_wharves = 0, warships = 0, warship_orders = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->num_workers == 0) {
            continue;
        }
        if (b->type == BUILDING_FISHING_WHARF) {
            fishing_wharves++;
            fishing_boats += wharf_boat_alive(id, b, FIGURE_FISHING_BOAT);
        }
        if (b->type == BUILDING_TRANSPORT_WHARF) {
            transport_wharves++;
            transport_ships += wharf_boat_alive(id, b, FIGURE_TRANSPORT_SHIP);
        }
        if (b->type == BUILDING_WARSHIP_WHARF) {
            warship_wharves++;
            warships += wharf_boat_alive(id, b, FIGURE_WARSHIP);
        }
        if (b->type == BUILDING_SHIPWRIGHT && b->data.shipyard.boat_order != 0) {
            int32_t order = (int16_t) b->data.shipyard.boat_order;
            fishing_orders += (order == city_boat_order(FIGURE_FISHING_BOAT));
            transport_orders += (order == city_boat_order(FIGURE_TRANSPORT_SHIP));
            warship_orders += (order == city_boat_order(FIGURE_WARSHIP));
        }
    }
    if ((fishing_orders + fishing_boats) < fishing_wharves) {
        *type = FIGURE_FISHING_BOAT;
        return 1;
    }
    if ((transport_orders + transport_ships) < transport_wharves) {
        *type = FIGURE_TRANSPORT_SHIP;
        return 1;
    }
    if ((warship_orders + warships) < warship_wharves) {
        *type = FIGURE_WARSHIP;
        return 1;
    }
    return 0;
}

static int32_t wharf_type_of_boat(int32_t boat_type)
{
    if (boat_type == FIGURE_FISHING_BOAT) {
        return BUILDING_FISHING_WHARF;
    }
    return (boat_type == FIGURE_TRANSPORT_SHIP) ? BUILDING_TRANSPORT_WHARF : BUILDING_WARSHIP_WHARF;
}

int32_t shipwright_boat_destination(int32_t boat_type)
{
    int32_t wharf_type = wharf_type_of_boat(boat_type);
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_UNUSED || b->type != wharf_type) {
            continue;
        }
        if (!wharf_boat_alive(id, b, boat_type)) {
            return id;
        }
    }
    return 0;
}

void boat_init_warship(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    const figure_type_properties *properties = &figure_properties[f->type];
    f->reserved_9 = 0;
    f->ship_hit_points = properties->max_damage;
    f->ship_max_hit_points = properties->max_damage;
    f->has_troops_on_board = 0;
    f->ship.moves = 0;
    f->ship.steps = 0;
    f->ship.speed = properties->speed;
    f->ship.max_straight_steps = properties->ship_steps;
    f->ship.model_column_m = properties->column_m;
    f->ship.model_column_n = properties->column_n;
    f->ram_damage_slow = properties->attack;
    f->ship_move_mode = 0;
    f->ram_damage_fast = calc_adjust_with_percentage(properties->attack, WARSHIP_FAST_RAM_DAMAGE_PERCENT);
}

void boat_init_transport(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    const figure_type_properties *properties = &figure_properties[f->type];
    f->reserved_9 = 0;
    f->ship_hit_points = properties->max_damage;
    f->ship_max_hit_points = properties->max_damage;
    f->has_troops_on_board = 0;
    f->ship.speed = properties->speed;
    f->ship_move_mode = 0;
}
