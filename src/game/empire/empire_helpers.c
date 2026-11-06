// Empire queries of the advisors, the construction warnings and the scenario start: which map objects sell or buy a
// resource, whether a resource can be had at all, the kinds of food on offer, the trade city of a map object, whether
// a route goes by sea, the trade amounts of the map objects, and the trade cities made from the city map objects at
// the start of a scenario.
//
// For our city a raw material, once mapped to a finished good, stays mapped for the next map objects; meat counts as
// available when straw is, and our city counts the foods of its first eight sold slots only.

#include "empire/empire_helpers.h"

#include "city/resource_type.h"
#include "core/calc.h"
#include "editor/editor_allowed_buildings.h"
#include "empire/empire.h"
#include "empire/empire_data.h"
#include "empire/empire_route.h"
#include "empire/trade_status.h"
#include "window/window_data.h"

#include <stdint.h>
#include <string.h>

enum {
    OUR_CITY_FOOD_SLOTS = 8,
    TRADE_AMOUNT_MAX = 40,
    TRADE_AMOUNT_UNSET = 2,
    NEW_CITY_UNREAD_DEFAULT = 10,
    NEW_CITY_TRADER_SLOTS = 3,
};

static int32_t object_is_city(const empire_object *o)
{
    return o->in_use && o->type == EMPIRE_OBJECT_CITY;
}

int32_t empire_our_city_sells_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        const empire_object *o = &empire_map_objects[id];
        if (object_is_city(o) && o->city_type == EMPIRE_CITY_OURS && empire_object_city_sells_resource(id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_can_export_resource_potentially(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && empire_city_type_is_trading(c->type) && c->buys[resource] == 1 && empire_city_has_trade_limit(id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_any_city_sells_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        if (object_is_city(&empire_map_objects[id]) && empire_object_city_sells_resource(id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_any_city_buys_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        if (object_is_city(&empire_map_objects[id]) && empire_object_city_buys_resource(id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_trade_city_sells_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        const empire_object *o = &empire_map_objects[id];
        if (object_is_city(o) && empire_city_type_is_trading(o->city_type) && empire_object_city_sells_resource(id, resource)) {
            return 1;
        }
    }
    return 0;
}

static int32_t our_city_raw_material(int32_t resource)
{
    switch (resource) {
        case RESOURCE_BEER:
            return RESOURCE_BARLEY;
        case RESOURCE_LINEN:
            return RESOURCE_FLAX;
        case RESOURCE_PAINT:
            return RESOURCE_HENNA;
        case RESOURCE_LUXURY_GOODS:
            return RESOURCE_GEMS;
        case RESOURCE_POTTERY:
            return RESOURCE_CLAY;
        case RESOURCE_WEAPONS:
            return RESOURCE_COPPER;
        case RESOURCE_CHARIOTS:
            return RESOURCE_TIMBER;
        case RESOURCE_PAPYRUS:
            return RESOURCE_REEDS;
        default:
            return RESOURCE_NONE;
    }
}

int32_t empire_our_city_can_get_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        const empire_object *o = &empire_map_objects[id];
        if (!object_is_city(o) || o->city_type != EMPIRE_CITY_OURS) {
            continue;
        }
        int32_t raw = our_city_raw_material(resource);
        if (raw) {
            resource = raw;
            if (empire_trade_city_sells_resource(resource)) {
                return 1;
            }
        }
        if (empire_object_city_sells_resource(id, resource)) {
            return 1;
        }
    }
    return 0;
}

static int32_t raw_material(int32_t resource)
{
    switch (resource) {
        case RESOURCE_MEAT:
            return RESOURCE_STRAW;
        case RESOURCE_STRAW:
            return RESOURCE_GRAIN;
        case RESOURCE_WEAPONS:
            return RESOURCE_COPPER;
        case RESOURCE_POTTERY:
            return RESOURCE_CLAY;
        case RESOURCE_BEER:
            return RESOURCE_BARLEY;
        case RESOURCE_LINEN:
            return RESOURCE_FLAX;
        case RESOURCE_LUXURY_GOODS:
            return RESOURCE_GEMS;
        case RESOURCE_PAPYRUS:
            return RESOURCE_REEDS;
        case RESOURCE_CHARIOTS:
            return RESOURCE_TIMBER;
        case RESOURCE_PAINT:
            return RESOURCE_HENNA;
        default:
            return resource;
    }
}

static int32_t raw_material_available(int32_t raw, int16_t scenario_allows)
{
    if (empire_any_city_sells_resource(raw) && scenario_allows != 0) {
        return 1;
    }
    if (!empire_our_city_can_get_resource(raw)) {
        return 0;
    }
    return scenario_allows != 0;
}

int32_t empire_resource_available(int32_t resource)
{
    if (resource == RESOURCE_BRICKS) {
        if (empire_any_city_sells_resource(RESOURCE_BRICKS)) {
            return 1;
        }
        if (!empire_resource_available(RESOURCE_STRAW)) {
            return 0;
        }
        return empire_resource_available(RESOURCE_CLAY) != 0;
    }
    int32_t raw = raw_material(resource);
    if (empire_any_city_sells_resource(resource)) {
        return 1;
    }
    if (resource == RESOURCE_WEAPONS) {
        return raw_material_available(raw, scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH]);
    }
    if (resource == RESOURCE_CHARIOTS) {
        return raw_material_available(raw, scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER]);
    }
    if (resource == RESOURCE_LAMPS) {
        if (empire_any_city_sells_resource(RESOURCE_LAMPS)) {
            return 1;
        }
        if (!empire_resource_available(RESOURCE_OIL)) {
            return 0;
        }
        return empire_resource_available(RESOURCE_POTTERY) != 0;
    }
    if (resource != RESOURCE_STRAW && empire_any_city_sells_resource(raw)) {
        return 1;
    }
    return empire_our_city_can_get_resource(raw) != 0;
}

int32_t empire_count_food_kinds(void)
{
    int32_t counts[RESOURCE_MAX] = { 0 };
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        const empire_object *o = &empire_map_objects[id];
        if (!object_is_city(o)) {
            continue;
        }
        int32_t slots;
        if (o->city_type == EMPIRE_CITY_OURS) {
            slots = OUR_CITY_FOOD_SLOTS;
        } else if (o->city_type > EMPIRE_CITY_FOREIGN || !o->route_id) {
            continue;
        } else {
            slots = EMPIRE_OBJECT_SOLD_RESOURCES;
        }
        for (int32_t i = 0; i < slots; i++) {
            int32_t r = o->sells[i];
            if (r >= RESOURCE_MIN_FOOD && r < RESOURCE_MAX_FOOD) {
                counts[r]++;
            }
        }
    }
    int32_t kinds = 0;
    for (int32_t r = 0; r < RESOURCE_MAX; r++) {
        if (counts[r]) {
            kinds++;
        }
    }
    return kinds;
}

int32_t empire_city_get_for_object(int32_t object_id)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && c->map_object == object_id) {
            return id;
        }
    }
    return 0;
}

int32_t empire_route_is_sea(int32_t route)
{
    if (route >= 0 && route < EMPIRE_ROUTES) {
        return empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA;
    }
    return 0;
}

int32_t empire_object_trade_amount(int32_t object_id, int32_t resource)
{
    const empire_object *o = &empire_map_objects[object_id];
    if (o->type == EMPIRE_OBJECT_CITY && o->city_type != EMPIRE_CITY_OURS && o->trade_amount[resource] <= TRADE_AMOUNT_MAX) {
        return o->trade_amount[resource];
    }
    return 0;
}

void empire_object_set_trade_amount(int32_t object_id, int32_t resource, int32_t amount)
{
    empire_object *o = &empire_map_objects[object_id];
    if (o->type == EMPIRE_OBJECT_CITY && o->city_type != EMPIRE_CITY_OURS && amount >= 0) {
        o->trade_amount[resource] = (uint8_t) amount;
    }
}

void empire_objects_fix_trade_amounts(void)
{
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        if (!object_is_city(&empire_map_objects[id])) {
            continue;
        }
        int32_t total = 0;
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            total += empire_object_trade_amount(id, r);
        }
        if (total == 0) {
            for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                empire_object_set_trade_amount(id, r, TRADE_AMOUNT_UNSET);
            }
        }
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (!empire_object_city_sells_resource(id, r) && !empire_object_city_buys_resource(id, r)) {
                empire_object_set_trade_amount(id, r, 0);
            }
        }
    }
}

static int32_t route_limit_of_amount(int32_t amount)
{
    switch (amount) {
        case 1:
            return empire_trade_rules[EMPIRE_TRADE_RULE_LIMIT_SMALL];
        case 2:
            return empire_trade_rules[EMPIRE_TRADE_RULE_LIMIT_MEDIUM];
        case 3:
            return empire_trade_rules[EMPIRE_TRADE_RULE_LIMIT_LARGE];
        default:
            return 0;
    }
}

void empire_object_init_cities(void)
{
    memset(empire_cities, 0, sizeof(empire_cities));
    empire_city *c = &empire_cities[1];
    for (int32_t id = 0; id < EMPIRE_OBJECT_COUNT; id++) {
        empire_object *o = &empire_map_objects[id];
        if (!object_is_city(o)) {
            continue;
        }
        if (c >= empire_cities + EMPIRE_CITY_COUNT) {
            return;
        }
        c->in_use = 1;
        c->type = o->city_type;
        c->name_id = o->name_id;
        if (o->route_id >= EMPIRE_TRADE_ROUTES) {
            o->route_id = EMPIRE_TRADE_ROUTES - 1;
        }
        c->route_id = o->route_id;
        c->is_open = o->is_open;
        c->cost_to_open = o->cost_to_open;
        c->is_sea_trade = (uint8_t) empire_route_is_sea(c->route_id);
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            c->sells[r] = 0;
            c->buys[r] = 0;
            int32_t type = c->type;
            if (empire_city_type_is_trading(type) || type == EMPIRE_CITY_OURS) {
                if (empire_object_city_sells_resource(id, r)) {
                    c->sells[r] = 1;
                }
                if (empire_object_city_buys_resource(id, r)) {
                    c->buys[r] = 1;
                }
            }
            int32_t amount = empire_object_trade_amount(id, r);
            int32_t route = c->route_id;
            trade_route_limits[route][r] = route_limit_of_amount(amount);
            trade_route_traded[route][r] = 0;
        }
        c->unread_default = NEW_CITY_UNREAD_DEFAULT;
        c->trader_entry_delay = (int16_t) empire_trade_rules[EMPIRE_TRADE_RULE_NEW_CITY_ENTRY_DELAY];
        c->trader_blocked = 0;
        for (int32_t slot = 0; slot < NEW_CITY_TRADER_SLOTS; slot++) {
            c->trader_figures[slot] = 0;
        }
        c->map_object = (int16_t) id;
        c++;
    }
}
