// Trade checks used when a trade city changes its status or a route's demand drops: what a map object sells and
// buys, whether an open city buys a resource, whether a route's first city is open, and the reset of an import or
// export setting that is no longer possible.

#include "empire/trade_status.h"

#include "city/city_player.h"
#include "empire/empire.h"
#include "empire/empire_data.h"

#include <stdint.h>

int32_t empire_object_city_sells_resource(int32_t object_id, int32_t resource)
{
    const empire_object *o = &empire_map_objects[object_id];
    for (int32_t i = 0; i < EMPIRE_OBJECT_SOLD_RESOURCES; i++) {
        if (o->sells[i] == resource) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_object_city_buys_resource(int32_t object_id, int32_t resource)
{
    const empire_object *o = &empire_map_objects[object_id];
    for (int32_t i = 0; i < EMPIRE_OBJECT_BOUGHT_RESOURCES; i++) {
        if (o->buys[i] == resource) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_can_export_resource(int32_t resource)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (!c->in_use || !empire_city_type_is_trading(c->type)) {
            continue;
        }
        if (c->is_open && c->buys[resource] == 1 && empire_city_has_trade_limit(id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t empire_city_is_trade_route_open(int32_t route)
{
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = &empire_cities[id];
        if (c->in_use && c->route_id == route) {
            return c->is_open != 0;
        }
    }
    return 0;
}

int32_t city_trade_status_update(int32_t resource)
{
    city_player *player = city_player_current();
    int16_t status = player->trade_status[resource];
    if (status == TRADE_STATUS_IMPORT || status == TRADE_STATUS_IMPORT_AS_NEEDED) {
        if (empire_can_import_resource(resource) == 0) {
            player->trade_status[resource] = TRADE_STATUS_NONE;
            return 1;
        }
    }
    status = player->trade_status[resource];
    if ((status == TRADE_STATUS_EXPORT || status == TRADE_STATUS_EXPORT_SURPLUS) && empire_can_export_resource(resource) == 0) {
        player->trade_status[resource] = TRADE_STATUS_NONE;
        return 1;
    }
    return 0;
}
