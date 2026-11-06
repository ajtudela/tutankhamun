// Trade: the empire trader records of the caravans and trade ships, the storage yard searches of the caravans and
// dockers, the buying and selling at a storage yard (counted in the trader record, the trade route and the city's
// money), and the actions of the trade caravan, its donkeys, the docker and the trade ship.
//
// The trade code has these peculiarities:
// - a negative count of the ship's loads makes the docker's loads that count + DOCKER_LOADS;
// - an export pays the bonus of Ra while the current player, not the paid one, has it;
// - the second round robin of a caravan's sale stores the next resource of the first one;
// - the trade ship leaves through the low byte of the river exit.

#include "figure/trade.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "city/city_data.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "empire/empire.h"
#include "figure/boats.h"
#include "figure/carriers.h"
#include "figure/carriers_internal.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_image_group.h"
#include "figure/figure_type.h"
#include "figure/military.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include <stdint.h>
#include <string.h>

int32_t trade_can_export[36];

int32_t trade_can_import[36];

static empire_trader *trader_get(int32_t id)
{
    return &empire_map_routes.traders[id];
}

static int32_t *trade_route_traded_amount(int32_t route, int32_t resource)
{
    return &trade_route_traded[route][resource];
}

static int32_t trade_price_buy(int32_t resource)
{
    return trade_prices[resource * 2];
}

static int32_t trade_price_sell(int32_t resource)
{
    return trade_prices[(resource * 2) + 1];
}

static int32_t is_storage_yard(const building *b)
{
    return b->type == BUILDING_STORAGE_YARD || b->type == BUILDING_STORAGE_YARD_UP;
}

static int32_t next_resource(int32_t resource)
{
    return (resource + 1 >= RESOURCE_MAX) ? RESOURCE_MIN : (resource + 1);
}

static int32_t storage_result(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->has_road_access == 1) {
        map_result_x = b->road_access_x;
        map_result_y = b->road_access_y;
        return building_id;
    }
    if (!map_has_road_access(b->x, b->y, STORAGE_ROAD_RADIUS)) {
        return 0;
    }
    return building_id;
}

void trader_create(int32_t figure_id)
{
    int32_t id = empire_map_routes.trader_next_index;
    figure_get(figure_id)->trader_id = (uint8_t) id;
    memset(trader_get(id), 0, sizeof(empire_trader));
    empire_map_routes.trader_next_index++;
    if (empire_map_routes.trader_next_index >= EMPIRE_TRADERS) {
        empire_map_routes.trader_next_index = 0;
    }
}

void trader_record_sold_resource(int32_t figure_id, int32_t resource)
{
    int32_t id = figure_get(figure_id)->trader_id;
    empire_trader *trader = trader_get(id);
    trader->sold_amount++;
    trader->sold_resources[resource]++;
    trader->sold_value += trade_price_sell(resource);
}

void trader_record_bought_resource(int32_t figure_id, int32_t resource)
{
    int32_t id = figure_get(figure_id)->trader_id;
    empire_trader *trader = trader_get(id);
    trader->bought_amount++;
    trader->bought_resources[resource]++;
    trader->bought_value += trade_price_buy(resource);
}

int32_t caravan_find_storage(int32_t figure_id, int32_t x, int32_t y, int32_t city_id, int32_t distance_penalty)
{
    figure *f = figure_get(figure_id);
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        trade_can_import[r] = empire_can_export_resource_to_city(city_current_player, city_id, r) ? 1 : 0;
        if (f->trader_amount_bought >= CARAVAN_MAX_LOADS) {
            trade_can_import[r] = 0;
        }
        trade_can_export[r] = (city_id && empire_can_import_resource_from_city(city_current_player, city_id, r)) ? 1 : 0;
        if (f->resource_amount >= CARAVAN_MAX_LOADS) {
            trade_can_export[r] = 0;
        }
    }
    int32_t exports = 0;
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (trade_can_export[r] == 1) {
            exports++;
        }
    }
    int32_t best = 10000;
    int32_t best_id = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !is_storage_yard(b) || !b->has_road_access || b->distance_from_entry <= 0 || b->num_workers == 0) {
            continue;
        }
        int32_t storage_id = b->storage_id;
        if (!figure_trade_caravan_can_sell(figure_id, f->faction_id, id, f->empire_city_id) &&
            !figure_trade_caravan_can_buy(figure_id, f->faction_id, id, f->empire_city_id)) {
            continue;
        }
        int32_t sellable = 0;
        for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            int32_t state = storage_get(storage_id)->resource_state[r];
            if (state != STORAGE_STATE_REFUSE && state != STORAGE_STATE_EMPTY && empire_can_import_resource_from_city(city_current_player, city_id, r)) {
                sellable++;
            }
        }
        int32_t score = STORAGE_SCORE_BASE;
        int32_t room = id;
        for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
            int32_t player = city_current_player;
            room = building_get(room)->next_part_building_id;
            const building *part = building_get(room);
            if (room && trade_can_import[part->subtype]) {
                score -= CARAVAN_SCORE_ROOM_BUYS;
            }
            if (exports && sellable && !storage_get(storage_id)->empty_all) {
                int32_t *counter = &city_player_get(player)->caravan_import_resource;
                for (int32_t n = RESOURCE_MIN; n < RESOURCE_MAX; n++) {
                    *counter = next_resource(*counter);
                    if (storage_get(storage_id)->resource_state[*counter] != STORAGE_STATE_REFUSE) {
                        break;
                    }
                }
                if (storage_get(storage_id)->resource_state[*counter] != STORAGE_STATE_REFUSE) {
                    int32_t resource = part->subtype;
                    if (resource == RESOURCE_NONE) {
                        score -= CARAVAN_SCORE_EMPTY_ROOM;
                    }
                    if (room && trade_can_export[resource] && part->loads_stored < STORAGE_ROOM_CAPACITY && resource == *counter) {
                        score -= CARAVAN_SCORE_ROOM_SELLS;
                    }
                }
            }
        }
        if (score < STORAGE_SCORE_BASE) {
            int32_t distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_penalty, b->distance_from_entry);
            if (score + distance < best) {
                best = score + distance;
                best_id = id;
            }
        }
    }
    return storage_result(best_id);
}

int32_t storageyard_import_loads(int32_t building_id, int32_t resource, int32_t amount)
{
    city_player *player = city_player_current();
    if (!is_storage_yard(building_get(building_id))) {
        return 0;
    }
    building_storageyard_get_amount(building_id, resource);
    if (building_storageyard_is_full(building_id, 0, resource)) {
        return 0;
    }
    if (building_storageyard_is_full(building_id, amount, resource)) {
        return 0;
    }
    int32_t stored = 0;
    int32_t room = building_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room = building_get(room)->next_part_building_id;
        const building *part = building_get(room);
        int32_t loads = part->loads_stored;
        stored += (loads > 0) ? loads : 0;
    }
    int32_t free = STORAGE_YARD_CAPACITY - stored;
    int32_t loads;
    if (free > amount) {
        loads = amount / LOAD_AMOUNT;
    } else {
        if (free < LOAD_AMOUNT) {
            return 0;
        }
        loads = free / LOAD_AMOUNT;
    }
    int32_t limit = (player->trade_status[resource] == TRADE_STATUS_IMPORT) ? (player->trading_amount[resource] * LOAD_AMOUNT) : trade_export_keep[resource];
    int32_t stock = player->storage_stored[resource];
    if (stock + (loads * LOAD_AMOUNT) < limit) {
        return loads;
    }
    if (stock >= limit) {
        return 0;
    }
    return (limit - stock) / LOAD_AMOUNT;
}

int32_t figure_docker_get_closest_warehouse_for_import(int32_t x, int32_t y, int32_t city_id, int32_t distance_penalty, int32_t road_network)
{
    city_player *player = city_player_current();
    int32_t *counter = &player->docker_import_resource;
    *counter = next_resource(*counter);
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        trade_can_export[r] = empire_can_import_resource_from_city(city_current_player, city_id, r) != 0;
    }
    counter = &player->docker_import_resource;
    for (int32_t n = RESOURCE_MIN; n < RESOURCE_MAX; n++) {
        if (trade_can_export[*counter]) {
            break;
        }
        *counter = next_resource(*counter);
    }
    if (!trade_can_export[*counter]) {
        return 0;
    }
    int32_t best = 10000;
    int32_t best_id = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !is_storage_yard(b) || !b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != (uint8_t) road_network || b->num_workers <= 0) {
            continue;
        }
        int32_t storage_id = b->storage_id;
        int32_t resource = player->docker_import_resource;
        int32_t state = storage_get(storage_id)->resource_state[resource];
        int32_t limit;
        if (state == STORAGE_STATE_ACCEPT) {
            int32_t amount = building_storageyard_get_amount(id, resource);
            limit = storage_get(storage_id)->accept_amount[resource];
            if (limit <= amount) {
                continue;
            }
        } else if (state == STORAGE_STATE_GET) {
            int32_t amount = building_storageyard_get_amount(id, resource);
            limit = storage_get(storage_id)->get_amount[resource];
            if (limit <= amount) {
                continue;
            }
        } else {
            continue;
        }
        if (!storageyard_import_loads(id, resource, DOCKER_LOADS * LOAD_AMOUNT)) {
            continue;
        }
        int32_t score = STORAGE_SCORE_BASE;
        int32_t room = id;
        for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
            room = building_get(room)->next_part_building_id;
            const building *part = building_get(room);
            if (part->subtype == 0) {
                score -= DOCKER_SCORE_EMPTY_ROOM;
            }
            if (room && part->subtype == player->docker_import_resource && part->loads_stored < STORAGE_ROOM_CAPACITY) {
                score -= DOCKER_SCORE_ROOM;
            }
        }
        if (score < STORAGE_SCORE_BASE) {
            int32_t distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_penalty, b->distance_from_entry);
            if (score + distance < best) {
                best = score + distance;
                best_id = id;
            }
        }
    }
    return storage_result(best_id);
}

int32_t figure_docker_get_closest_warehouse_for_export(int32_t x, int32_t y, int32_t city_id, int32_t distance_penalty, int32_t road_network)
{
    city_player *player = city_player_current();
    int32_t *counter = &player->docker_export_resource;
    *counter = next_resource(*counter);
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        trade_can_import[r] = empire_can_export_resource_to_city(city_current_player, city_id, r) != 0;
    }
    counter = &player->docker_export_resource;
    for (int32_t n = RESOURCE_MIN; n < RESOURCE_MAX; n++) {
        if (trade_can_import[*counter]) {
            break;
        }
        *counter = next_resource(*counter);
    }
    if (!trade_can_import[*counter]) {
        return 0;
    }
    int32_t best = 10000;
    int32_t best_id = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !is_storage_yard(b) || !b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != (uint8_t) road_network) {
            continue;
        }
        if (building_storageyard_get_amount(id, player->docker_export_resource) < LOAD_AMOUNT) {
            continue;
        }
        int32_t score = STORAGE_SCORE_BASE;
        int32_t room = id;
        for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
            room = building_get(room)->next_part_building_id;
            const building *part = building_get(room);
            if (room && part->subtype == player->docker_export_resource && part->loads_stored > 0) {
                score--;
            }
        }
        if (score < STORAGE_SCORE_BASE) {
            int32_t distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_penalty, b->distance_from_entry);
            if (score + distance < best) {
                best = score + distance;
                best_id = id;
            }
        }
    }
    return storage_result(best_id);
}

int32_t figure_docker_try_import_resource(int32_t building_id, int32_t resource, int32_t city_id, int32_t figure_id)
{
    if (!is_storage_yard(building_get(building_id))) {
        return 0;
    }
    int32_t route = empire_city_get(city_id)->route_id;
    int32_t limit = city_gods_ra_adjust_percentage(trade_route_limits[route][resource]);
    int32_t available = (limit - *trade_route_traded_amount(route, resource)) * LOAD_AMOUNT;
    int32_t max = DOCKER_LOADS * LOAD_AMOUNT;
    if (available < DOCKER_LOADS * LOAD_AMOUNT) {
        max = available < 0 ? 0 : available;
    }
    int32_t loads = storageyard_import_loads(building_id, resource, max);
    figure *f = figure_get(figure_id);
    int16_t count = f->resource_amount;
    if (count < 0) {
        loads = count + DOCKER_LOADS;
    }
    if (loads < DOCKER_LOADS) {
        f->resource_amount = (int16_t) (count - loads + DOCKER_LOADS);
    }
    if (loads == 0) {
        return 0;
    }
    city_player *player = city_player_current();
    for (int32_t i = 0; i < loads; i++) {
        building_storageyard_add_resource(city_current_player, building_id, resource, LOAD_AMOUNT);
        int32_t price = trade_price_buy(resource);
        player->treasury -= price;
        player->imports_this_year += price;
        (*trade_route_traded_amount(empire_city_get(city_id)->route_id, resource))++;
        trader_record_sold_resource(figure_id, resource);
    }
    return 1;
}

static void city_receive_export(int32_t player, int32_t resource)
{
    city_player *owner = city_player_get(player);
    if (city_player_current()->ra_exports_bonus_months > 0) {
        owner->treasury += calc_adjust_with_percentage(trade_price_sell(resource), EXPORT_BONUS_PERCENT);
        owner->exports_this_year += calc_adjust_with_percentage(trade_price_sell(resource), EXPORT_BONUS_PERCENT);
    } else {
        int32_t price = trade_price_sell(resource);
        owner->treasury += price;
        owner->exports_this_year += price;
    }
}

int32_t figure_docker_try_export_resource(int32_t building_id, int32_t resource, int32_t city_id, int32_t figure_id)
{
    if (building_get(building_id)->type != BUILDING_STORAGE_YARD) {
        return 0;
    }
    int32_t amount = building_storageyard_get_amount(building_id, resource);
    uint8_t *bought = &figure_get(figure_id)->trader_amount_bought;
    if (*bought >= SHIP_MAX_LOADS || amount < LOAD_AMOUNT) {
        return 0;
    }
    int32_t loads = (amount < DOCKER_LOADS * LOAD_AMOUNT) ? (amount / LOAD_AMOUNT) : DOCKER_LOADS;
    const city_player *player = city_player_current();
    int32_t stock = player->storage_stored[resource];
    int32_t limit = (player->trade_status[resource] == TRADE_STATUS_EXPORT) ? (player->trading_amount[resource] * LOAD_AMOUNT) : trade_export_keep[resource];
    if (stock - (loads * LOAD_AMOUNT) < limit) {
        loads = (stock - limit) / LOAD_AMOUNT;
    }
    if (loads == 0) {
        return 0;
    }
    for (int32_t i = 0; i < loads; i++) {
        (*bought)++;
        building_storageyard_remove_resource(city_current_player, building_id, resource, LOAD_AMOUNT);
        city_receive_export(city_current_player, resource);
        if (*bought >= SHIP_MAX_LOADS) {
            return 1;
        }
        (*trade_route_traded_amount(empire_city_get(city_id)->route_id, resource))++;
        trader_record_bought_resource(figure_id, resource);
    }
    return 1;
}

int32_t figure_trade_caravan_can_buy(int32_t figure_id, int32_t player, int32_t building_id, int32_t city_id)
{
    if (!is_storage_yard(building_get(building_id)) || figure_get(figure_id)->trader_amount_bought >= CARAVAN_MAX_LOADS) {
        return 0;
    }
    int32_t room = building_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room = building_get(room)->next_part_building_id;
        const building *part = building_get(room);
        if (room > 0 && part->loads_stored > 0 && empire_can_export_resource_to_city(player, city_id, part->subtype) &&
            building_storageyard_get_amount(building_id, part->subtype) >= LOAD_AMOUNT) {
            return 1;
        }
    }
    return 0;
}

int32_t trader_get_buy_resource(int32_t player, int32_t building_id, int32_t city_id)
{
    if (!is_storage_yard(building_get(building_id))) {
        return 0;
    }
    int32_t resource = RESOURCE_NONE;
    int32_t room = building_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room = building_get(room)->next_part_building_id;
        const building *part = building_get(room);
        if (room > 0 && part->loads_stored > 0 && empire_can_export_resource_to_city(player, city_id, part->subtype)) {
            if (building_storageyard_get_amount(building_id, part->subtype) < LOAD_AMOUNT) {
                return 0;
            }
            resource = part->subtype;
        }
    }
    if (resource == RESOURCE_NONE) {
        return 0;
    }
    building_storageyard_remove_resource(player, building_id, resource, LOAD_AMOUNT);
    city_receive_export(player, resource);
    return resource;
}

static int32_t yard_has_room_for_import(int32_t player, int32_t building_id, int32_t city_id)
{
    int32_t room = building_id;
    for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
        room = building_get(room)->next_part_building_id;
        const building *part = building_get(room);
        if (room > 0 && part->loads_stored < STORAGE_ROOM_CAPACITY &&
            (part->loads_stored == 0 || empire_can_import_resource_from_city(player, city_id, part->subtype))) {
            return 1;
        }
    }
    return 0;
}

int32_t figure_trade_caravan_can_sell(int32_t figure_id, int32_t player, int32_t building_id, int32_t city_id)
{
    building *b = building_get(building_id);
    if (!is_storage_yard(b)) {
        return 0;
    }
    calc_percentage(b->num_workers, building_model_table[b->type].laborers);
    if (figure_get(figure_id)->resource_amount >= CARAVAN_MAX_LOADS) {
        return 0;
    }
    int32_t storage_id = b->storage_id;
    city_player *owner = city_player_get(player);
    const city_player *current = city_player_current();
    int32_t count = 0;
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        int32_t state = storage_get(storage_id)->resource_state[r];
        if (state != STORAGE_STATE_REFUSE && state != STORAGE_STATE_EMPTY && !building_storageyard_is_full(building_id, 0, r) &&
            !building_storageyard_is_full(building_id, LOAD_AMOUNT, r) && empire_can_import_resource_from_city(player, city_id, r)) {
            count++;
        }
    }
    if (count <= 0) {
        return 0;
    }
    int32_t sells = empire_can_import_resource_from_city(player, city_id, owner->caravan_import_resource);
    if (storage_get(storage_id)->resource_state[current->caravan_import_resource] != STORAGE_STATE_REFUSE && sells) {
        return yard_has_room_for_import(player, building_id, city_id);
    }
    for (int32_t n = RESOURCE_MIN; n < RESOURCE_MAX; n++) {
        owner->caravan_import_resource = next_resource(owner->caravan_import_resource);
        if (storage_get(storage_id)->resource_state[current->caravan_import_resource] != STORAGE_STATE_REFUSE &&
            empire_can_import_resource_from_city(player, city_id, owner->caravan_import_resource)) {
            return yard_has_room_for_import(player, building_id, city_id);
        }
    }
    return 0;
}

int32_t trader_get_sell_resource(int32_t player, int32_t building_id, int32_t city_id)
{
    city_player *owner = city_player_get(player);
    if (!is_storage_yard(building_get(building_id))) {
        return 0;
    }
    for (int32_t i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        if (empire_can_import_resource_from_city(player, city_id, owner->caravan_import_resource)) {
            int32_t resource = owner->caravan_import_resource;
            owner->caravan_import_resource = next_resource(resource);
            if (building_storageyard_add_resource(city_current_player, building_id, resource, LOAD_AMOUNT) == LOAD_AMOUNT) {
                city_finance_pay_for_import(player, resource);
                return resource;
            }
            for (int32_t n = RESOURCE_MIN; n < RESOURCE_MAX; n++) {
                owner->caravan_backup_import_resource = next_resource(owner->caravan_backup_import_resource);
                if (empire_can_import_resource_from_city(player, city_id, owner->caravan_backup_import_resource)) {
                    resource = owner->caravan_import_resource;
                    owner->caravan_import_resource = next_resource(resource);
                    if (building_storageyard_add_resource(city_current_player, building_id, resource, LOAD_AMOUNT) == LOAD_AMOUNT) {
                        city_finance_pay_for_import(player, resource);
                        return resource;
                    }
                }
            }
            return 0;
        }
        owner->caravan_import_resource = next_resource(owner->caravan_import_resource);
    }
    return 0;
}

static void caravan_leave_city(figure *f)
{
    city_player *player = city_player_current();
    f->action_state = FIGURE_ACTION_TRADE_CARAVAN_LEAVING;
    f->wait_ticks = 0;
    f->destination_x = player->exit_point_x;
    f->destination_y = player->exit_point_y;
}

void figure_trade_caravan_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    int32_t speed = figure_properties[f->type].speed;
    if (frame >= TRADE_FIGURE_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_TRADE_CARAVAN_CREATED:
            if (++f->wait_ticks <= CARAVAN_ENTRY_WAIT_TICKS) {
                f->is_ghost = 1;
                f->animation_frame = 0;
                break;
            } else {
                int32_t dest = caravan_find_storage(figure_id, f->x, f->y, f->empire_city_id, 0);
                if (dest) {
                    building *b = building_get(dest);
                    f->destination_building_id = (int16_t) dest;
                    f->action_state = FIGURE_ACTION_TRADE_CARAVAN_ARRIVING;
                    f->wait_ticks = 0;
                    f->destination_x = b->road_access_x;
                    f->destination_y = b->road_access_y;
                } else {
                    caravan_leave_city(f);
                }
                f->animation_frame = 0;
            }
            break;
        case FIGURE_ACTION_TRADE_CARAVAN_ARRIVING: {
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_TRADE_CARAVAN_TRADING;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                if (++f->lost_ticks >= CARAVAN_LOST_TICKS) {
                    f->state = FIGURE_STATE_DEAD;
                    f->is_ghost = 1;
                }
            } else {
                f->lost_ticks = 0;
            }
            building *b = building_get(f->destination_building_id);
            if (b->state != BUILDING_STATE_IN_USE || b->num_workers == 0) {
                figure_route_remove(figure_id);
                f->action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
                f->wait_ticks = CARAVAN_ENTRY_WAIT_TICKS;
            }
            break;
        }
        case FIGURE_ACTION_TRADE_CARAVAN_TRADING:
            if (++f->wait_ticks > CARAVAN_TRADE_TICKS) {
                f->wait_ticks = 0;
                int32_t failed = 0;
                if (figure_trade_caravan_can_buy(figure_id, f->faction_id, f->destination_building_id, f->empire_city_id)) {
                    int32_t resource = trader_get_buy_resource(f->faction_id, f->destination_building_id, f->empire_city_id);
                    if (resource) {
                        (*trade_route_traded_amount(empire_city_get(f->empire_city_id)->route_id, resource))++;
                        trader_record_bought_resource(figure_id, resource);
                        f->trader_amount_bought++;
                    } else {
                        failed = 1;
                    }
                } else {
                    failed = 1;
                }
                if (figure_trade_caravan_can_sell(figure_id, f->faction_id, f->destination_building_id, f->empire_city_id)) {
                    int32_t resource = trader_get_sell_resource(f->faction_id, f->destination_building_id, f->empire_city_id);
                    if (resource) {
                        (*trade_route_traded_amount(empire_city_get(f->empire_city_id)->route_id, resource))++;
                        trader_record_sold_resource(figure_id, resource);
                        f->resource_amount++;
                    } else {
                        failed++;
                    }
                } else {
                    failed++;
                }
                if (failed == CARAVAN_TRADE_KINDS) {
                    int32_t dest = caravan_find_storage(figure_id, f->x, f->y, f->empire_city_id, -1);
                    if (dest) {
                        f->destination_building_id = (int16_t) dest;
                        f->action_state = FIGURE_ACTION_TRADE_CARAVAN_ARRIVING;
                        f->wait_ticks = 0;
                        f->destination_x = (int16_t) map_result_x;
                        f->destination_y = (int16_t) map_result_y;
                    } else {
                        caravan_leave_city(f);
                    }
                }
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_TRADE_CARAVAN_LEAVING:
            figure_movement_move_ticks(figure_id, speed);
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->animation_frame = 0;
            f->image_id = image_group(IMAGE_GROUP_FIGURE_TRADE_CARAVAN) + 1 + figure_image_direction;
            if (++f->wait_ticks > FERRY_QUEUE_CHECK_TICKS) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
                }
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            break;
        default:
            f->action_state = FIGURE_ACTION_TRADE_CARAVAN_CREATED;
            break;
    }
    int32_t direction = (f->direction >= DIRECTION_COUNT) ? f->previous_tile_direction : f->direction;
    int32_t dir = carrier_image_direction(direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TRADE_CARAVAN_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TRADE_CARAVAN) + (f->animation_frame * DIRECTION_COUNT) + dir;
    }
}

int32_t caravan_of_donkey(int32_t figure_id)
{
    int32_t leader = figure_get(figure_id)->leading_figure_id;
    const figure *leading = figure_get(leader);
    if (leading->type == FIGURE_TRADE_CARAVAN) {
        return leader;
    }
    if (leading->type == FIGURE_TRADE_CARAVAN_DONKEY) {
        return leading->leading_figure_id;
    }
    return 0;
}

void figure_trade_caravan_donkey_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t leader_id = f->leading_figure_id;
    int32_t caravan_id = caravan_of_donkey(figure_id);
    const figure *leader = figure_get(leader_id);
    const figure *caravan = figure_get(caravan_id);
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= TRADE_FIGURE_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    if (leader_id <= 0) {
        f->state = FIGURE_STATE_DEAD;
    } else {
        if (leader->action_state == FIGURE_ACTION_CORPSE || leader->state != FIGURE_STATE_ALIVE ||
            (leader->type != FIGURE_TRADE_CARAVAN && leader->type != FIGURE_TRADE_CARAVAN_DONKEY)) {
            f->state = FIGURE_STATE_DEAD;
        } else {
            if (caravan->action_state != FIGURE_ACTION_ON_FERRY && caravan->action_state != FIGURE_ACTION_WAIT_FOR_FERRY) {
                figure_movement_follow_ticks(figure_id, f->leading_figure_id, 1);
            }
        }
    }
    if (leader->is_ghost) {
        f->is_ghost = 1;
    }
    int32_t direction = (f->direction >= DIRECTION_COUNT) ? f->previous_tile_direction : f->direction;
    int32_t dir = carrier_image_direction(direction);
    int32_t caravan_state = caravan->action_state;
    if (caravan_state == FIGURE_ACTION_ON_FERRY) {
        f->image_id = 0;
        return;
    }
    if (caravan_state == FIGURE_ACTION_WAIT_FOR_FERRY) {
        f->animation_frame = 0;
    }
    if (f->state == FIGURE_STATE_DEAD) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TRADE_CARAVAN_DONKEY_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
    } else {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_TRADE_CARAVAN_DONKEY) + (f->animation_frame * DIRECTION_COUNT) + dir;
    }
    f->is_ghost = 0;
}

int32_t docker_deliver_import_resource(int32_t dock_id)
{
    building *dock = building_get(dock_id);
    int32_t ship_id = dock->data.dock.ship_id;
    if (ship_id == 0) {
        return 0;
    }
    figure *ship = figure_get(ship_id);
    if (ship->action_state != FIGURE_ACTION_TRADE_SHIP_DOCKED || ship->resource_amount < 1 || trader_get(ship->trader_id)->sold_amount >= SHIP_MAX_LOADS) {
        return 0;
    }
    figure *f = figure_get(figure_current_id);
    int32_t dest = figure_docker_get_closest_warehouse_for_import(f->x, f->y, ship->empire_city_id, dock->distance_from_entry, dock->road_network_id);
    if (!dest) {
        return 0;
    }
    f->destination_building_id = (int16_t) dest;
    f->wait_ticks = 0;
    ship->resource_amount = (int16_t) (ship->resource_amount - DOCKER_LOADS);
    f->action_state = FIGURE_ACTION_DOCKER_IMPORT_QUEUE;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    f->resource_id = (uint8_t) city_player_current()->docker_import_resource;
    return 1;
}

int32_t docker_fetch_export_resource(int32_t dock_id)
{
    building *dock = building_get(dock_id);
    int32_t ship_id = dock->data.dock.ship_id;
    if (ship_id == 0) {
        return 0;
    }
    figure *ship = figure_get(ship_id);
    if (ship->action_state != FIGURE_ACTION_TRADE_SHIP_DOCKED || ship->trader_amount_bought >= SHIP_MAX_LOADS) {
        return 0;
    }
    figure *f = figure_get(figure_current_id);
    int32_t dest = figure_docker_get_closest_warehouse_for_export(f->x, f->y, ship->empire_city_id, dock->distance_from_entry, dock->road_network_id);
    if (!dest) {
        return 0;
    }
    f->destination_building_id = (int16_t) dest;
    f->action_state = FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD;
    f->wait_ticks = 0;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    f->resource_id = (uint8_t) city_player_current()->docker_export_resource;
    return 1;
}

int32_t trade_ship_done_trading(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    building *dock = building_get(f->destination_building_id);
    if (dock->state == BUILDING_STATE_IN_USE && (dock->type == BUILDING_DOCK || dock->type == BUILDING_DOCK_UP) && dock->num_workers > 0) {
        for (int32_t i = 0; i < DOCK_DOCKERS; i++) {
            int32_t docker_id = dock->data.dock.docker_ids[i];
            if (docker_id != 0) {
                const figure *docker = figure_get(docker_id);
                if (docker->state == FIGURE_STATE_ALIVE && docker->action_state != FIGURE_ACTION_DOCKER_IDLE) {
                    return 0;
                }
            }
        }
        if (++f->ship_dock_attempts < TRADE_SHIP_DONE_CALLS) {
            return 0;
        }
        f->ship_dock_attempts = TRADE_SHIP_DONE_CALLS + 1;
    }
    return 1;
}

int32_t trade_ship_lost_dock(int32_t figure_id)
{
    building *dock = building_get(figure_get(figure_id)->destination_building_id);
    if (dock->state != BUILDING_STATE_IN_USE) {
        return 1;
    }
    if (dock->type != BUILDING_DOCK && dock->type != BUILDING_DOCK_UP) {
        return 1;
    }
    if (dock->num_workers < 1) {
        return 1;
    }
    return dock->data.dock.ship_id != figure_id;
}

static int32_t docker_walk(int32_t speed)
{
    cart_image(figure_current_id);
    figure_movement_move_ticks(figure_current_id, speed);
    return figure_get(figure_current_id)->direction;
}

void figure_docker_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    int32_t speed = figure_properties[f->type].speed;
    if (frame >= TRADE_FIGURE_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    f->is_ghost = 0;
    int32_t dock_id = f->building_id;
    building *dock = building_get(dock_id);
    if (dock->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (dock->type != BUILDING_DOCK && dock->type != BUILDING_FISHING_WHARF && dock->type != BUILDING_DOCK_UP) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (dock->data.dock.ship_wait_days) {
        dock->data.dock.ship_wait_days--;
    }
    if (dock->data.dock.ship_id != 0) {
        const figure *ship = figure_get(dock->data.dock.ship_id);
        if (ship->state != FIGURE_STATE_ALIVE || ship->type != FIGURE_TRADE_SHIP || ship->action_state == FIGURE_ACTION_TRADE_SHIP_LEAVING) {
            dock->data.dock.ship_id = 0;
        }
    }
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_DOCKER_IDLE:
            f->resource_id = RESOURCE_NONE;
            f->cart_image_id = 0;
            if (!docker_deliver_import_resource(dock_id)) {
                docker_fetch_export_resource(dock_id);
            }
            f->animation_frame = 0;
            if (!(map_grids.terrain[f->grid_offset] & TERRAIN_ROAD)) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_DOCKER_IMPORT_QUEUE:
            f->cart_image_id = 0;
            f->animation_frame = 0;
            if (dock->data.dock.unloading_docker_id <= 0) {
                dock->data.dock.unloading_docker_id = (int16_t) figure_id;
                f->wait_ticks = 0;
            }
            if (dock->data.dock.unloading_docker_id == figure_id) {
                f->wait_ticks++;
                dock->data.dock.ship_wait_days = DOCK_SHIP_WAIT_DAYS;
                if (f->wait_ticks >= DOCKER_SHIP_TICKS) {
                    f->action_state = FIGURE_ACTION_DOCKER_IMPORT_TO_STORAGEYARD;
                    f->wait_ticks = 0;
                    cart_image(figure_id);
                    dock->data.dock.unloading_docker_id = 0;
                }
            } else {
                int32_t busy = 0;
                for (int32_t i = 0; i < DOCK_DOCKERS; i++) {
                    int32_t docker_id = dock->data.dock.docker_ids[i];
                    if (docker_id != 0) {
                        const figure *docker = figure_get(docker_id);
                        if (docker->state == FIGURE_STATE_ALIVE &&
                            (docker->action_state == FIGURE_ACTION_DOCKER_IMPORT_QUEUE || docker->action_state == FIGURE_ACTION_DOCKER_EXPORT_QUEUE) &&
                            dock->data.dock.unloading_docker_id == docker_id) {
                            busy = 1;
                        }
                    }
                }
                if (busy != 1) {
                    dock->data.dock.unloading_docker_id = 0;
                }
            }
            break;
        case FIGURE_ACTION_DOCKER_EXPORT_QUEUE:
            cart_image(figure_id);
            if (dock->data.dock.unloading_docker_id <= 0) {
                dock->data.dock.unloading_docker_id = (int16_t) figure_id;
                f->wait_ticks = 0;
            }
            if (dock->data.dock.unloading_docker_id == figure_id) {
                dock->data.dock.ship_wait_days = DOCK_SHIP_WAIT_DAYS;
                if (++f->wait_ticks >= DOCKER_SHIP_TICKS) {
                    f->action_state = FIGURE_ACTION_DOCKER_IDLE;
                    f->wait_ticks = 0;
                    f->image_id = 0;
                    f->cart_image_id = 0;
                    dock->data.dock.unloading_docker_id = 0;
                }
            }
            if (++f->wait_ticks >= DOCKER_EXPORT_QUEUE_TICKS) {
                f->action_state = FIGURE_ACTION_DOCKER_IDLE;
                f->wait_ticks = 0;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_DOCKER_IMPORT_TO_STORAGEYARD:
        case FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD:
        case FIGURE_ACTION_DOCKER_EXPORT_RETURNING: {
            int32_t state = f->action_state;
            int32_t dir = docker_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                if (state == FIGURE_ACTION_DOCKER_IMPORT_TO_STORAGEYARD) {
                    f->action_state = FIGURE_ACTION_DOCKER_IMPORT_AT_STORAGEYARD;
                } else if (state == FIGURE_ACTION_DOCKER_EXPORT_TO_STORAGEYARD) {
                    f->action_state = FIGURE_ACTION_DOCKER_EXPORT_AT_STORAGEYARD;
                } else {
                    f->action_state = FIGURE_ACTION_DOCKER_EXPORT_QUEUE;
                    f->wait_ticks = 0;
                }
            } else if (dir == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_DOCKER_IMPORT_AT_STORAGEYARD:
            cart_image(figure_id);
            if (++f->wait_ticks > DOCKER_STORAGE_TICKS) {
                int32_t ship_id = dock->data.dock.ship_id;
                int32_t city_id = ship_id ? figure_get(ship_id)->empire_city_id : 0;
                int32_t dest = f->destination_building_id;
                building *yard = building_get(dest);
                if ((yard->type == BUILDING_STORAGE_YARD || yard->type == BUILDING_STORAGE_YARD_UP) &&
                    figure_docker_try_import_resource(dest, f->resource_id, city_id, ship_id)) {
                    f->action_state = FIGURE_ACTION_DOCKER_RETURNING;
                    f->wait_ticks = 0;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                    f->resource_id = RESOURCE_NONE;
                    docker_fetch_export_resource(dock_id);
                } else {
                    f->action_state = FIGURE_ACTION_DOCKER_RETURNING;
                    f->wait_ticks = 0;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                }
                f->wait_ticks = 0;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_DOCKER_EXPORT_AT_STORAGEYARD: {
            cart_image(figure_id);
            int32_t ship_id = dock->data.dock.ship_id;
            int32_t city_id = ship_id ? figure_get(ship_id)->empire_city_id : 0;
            if (++f->wait_ticks > DOCKER_STORAGE_TICKS) {
                f->action_state = FIGURE_ACTION_DOCKER_RETURNING;
                f->wait_ticks = 0;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                f->wait_ticks = 0;
                if (figure_docker_try_export_resource(f->destination_building_id, f->resource_id, city_id, ship_id)) {
                    f->action_state = FIGURE_ACTION_DOCKER_EXPORT_RETURNING;
                } else {
                    docker_fetch_export_resource(dock_id);
                }
            }
            f->animation_frame = 0;
            break;
        }
        case FIGURE_ACTION_DOCKER_RETURNING: {
            int32_t dir = docker_walk(speed);
            if (dir == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_DOCKER_IDLE;
            } else if (dir == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (dir == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_WAIT_FOR_FERRY:
            f->image_id = image_group(IMAGE_GROUP_FIGURE_DOCKER) + 1 + figure_image_direction;
            f->animation_frame = 0;
            cart_image(figure_id);
            if (++f->wait_ticks > FERRY_QUEUE_CHECK_TICKS) {
                f->wait_ticks = 0;
                if (!figure_is_queued_for_ferry(figure_id)) {
                    f->action_state = FIGURE_ACTION_DOCKER_IDLE;
                }
            }
            break;
        case FIGURE_ACTION_ON_FERRY:
            f->image_id = 0;
            if (f->cart_image_id != 0) {
                f->cart_image_id = 0;
            }
            break;
        default:
            f->action_state = FIGURE_ACTION_DOCKER_IDLE;
            break;
    }
    int32_t direction = (f->direction >= DIRECTION_COUNT) ? f->previous_tile_direction : f->direction;
    int32_t dir = carrier_image_direction(direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_DOCKER_DEATH) + figure_corpse_offsets[f->wait_ticks / 2];
        f->cart_image_id = 0;
    } else if (f->action_state != FIGURE_ACTION_ON_FERRY) {
        f->image_id = image_group(IMAGE_GROUP_FIGURE_DOCKER) + (f->animation_frame * DIRECTION_COUNT) + dir;
    }
    if (!f->cart_image_id) {
        f->image_id = 0;
        return;
    }
    f->cart_image_id += dir;
    int32_t offset_dir = dir;
    if (carrier_is_big_resource(f->resource_id)) {
        offset_dir = dir + (DIRECTION_COUNT / 2);
        if (offset_dir >= DIRECTION_COUNT) {
            offset_dir = dir - (DIRECTION_COUNT / 2);
        }
    }
    f->cart_offset_x = figure_cart_offsets[offset_dir * 2];
    f->cart_offset_y = figure_cart_offsets[(offset_dir * 2) + 1];
}

static void trade_ship_next_dock(int32_t dock_id, int32_t clear_dock)
{
    uint8_t index = (uint8_t) (building_dock_index(dock_id) + 1);
    figure *f = figure_get(figure_current_id);
    f->ship_dock_index = index;
    if (index < CITY_DOCKS) {
        f->action_state = FIGURE_ACTION_TRADE_SHIP_CREATED;
        f->wait_ticks = 0;
        return;
    }
    f->action_state = FIGURE_ACTION_TRADE_SHIP_LEAVING;
    f->wait_ticks = 0;
    f->destination_x = (uint8_t) scenario_data.river_exit_x;
    f->destination_y = (uint8_t) scenario_data.river_exit_y;
    if (clear_dock) {
        building_get(dock_id)->data.dock.ship_id = 0;
    }
}

void figure_trade_ship_action(void)
{
    int32_t figure_id = figure_current_id;
    figure *f = figure_get(figure_id);
    int32_t speed = figure_properties[f->type].speed;
    int32_t moving = 1;
    int32_t dock_id = f->destination_building_id;
    if (dock_id != 0) {
        building *dock = building_get(dock_id);
        if (dock->data.dock.ship_id == figure_id) {
            map_water_tile_next_to(dock->x, dock->y, dock->size);
            if (map_result_x == f->destination_x && map_result_y == f->destination_y) {
                dock->data.dock.ship_id = (int16_t) figure_id;
            } else {
                dock->data.dock.ship_id = 0;
            }
        }
    }
    f->is_ghost = 0;
    f->is_boat = FIGURE_BOAT_SHIP;
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= TRADE_FIGURE_FRAMES) {
        f->animation_frame = 0;
    }
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_ATTACK:
            figure_combat_handle_attack(figure_id);
            break;
        case FIGURE_ACTION_CORPSE:
            figure_combat_handle_corpse(figure_id);
            break;
        case FIGURE_ACTION_TRADE_SHIP_CREATED:
            moving = 0;
            f->is_ghost = 0;
            if (++f->wait_ticks > TRADE_SHIP_WAIT_TICKS) {
                int32_t dest = building_dock_get_free_destination(figure_id);
                if (dest) {
                    building *dock = building_get(dest);
                    map_water_tile_next_to(dock->x, dock->y, dock->size);
                    f->destination_building_id = (int16_t) dest;
                    f->action_state = FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK;
                    f->destination_x = (int16_t) map_result_x;
                    f->destination_y = (int16_t) map_result_y;
                } else if (building_dock_get_queue_destination(figure_id)) {
                    f->action_state = FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK_QUEUE;
                    f->destination_x = (int16_t) map_result_x;
                    f->destination_y = (int16_t) map_result_y;
                } else {
                    f->action_state = FIGURE_ACTION_TRADE_SHIP_LEAVING;
                    f->wait_ticks = 0;
                    f->destination_x = (uint8_t) scenario_data.river_exit_x;
                    f->destination_y = (uint8_t) scenario_data.river_exit_y;
                }
                f->wait_ticks = 0;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK_QUEUE:
        case FIGURE_ACTION_TRADE_SHIP_LEAVING: {
            int32_t state = f->action_state;
            figure_movement_move_ticks(figure_id, speed);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIRECTION_AT_DESTINATION) {
                if (state == FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK_QUEUE) {
                    f->action_state = FIGURE_ACTION_TRADE_SHIP_ANCHORED;
                } else {
                    f->action_state = FIGURE_ACTION_TRADE_SHIP_CREATED;
                    f->state = FIGURE_STATE_DEAD;
                }
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        }
        case FIGURE_ACTION_TRADE_SHIP_ANCHORED:
            moving = 0;
            if (++f->wait_ticks > TRADE_SHIP_ANCHORED_TICKS) {
                int32_t dest = building_dock_get_free_destination(figure_id);
                if (dest) {
                    building *dock = building_get(dest);
                    map_water_tile_next_to(dock->x, dock->y, dock->size);
                    f->destination_building_id = (int16_t) dest;
                    f->action_state = FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK;
                    f->destination_x = (int16_t) map_result_x;
                    f->destination_y = (int16_t) map_result_y;
                } else {
                    if (map_grids.figure_ids[f->grid_offset] != figure_current_id && building_dock_get_queue_destination(figure_id)) {
                        f->action_state = FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK_QUEUE;
                        f->destination_x = (int16_t) map_result_x;
                        f->destination_y = (int16_t) map_result_y;
                    }
                }
                f->wait_ticks = 0;
            }
            f->animation_frame = 0;
            break;
        case FIGURE_ACTION_TRADE_SHIP_GOING_TO_DOCK:
            figure_movement_move_ticks(figure_id, speed);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIRECTION_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_TRADE_SHIP_DOCKED;
            } else if (f->direction == DIRECTION_REROUTE) {
                figure_route_remove(figure_id);
            } else if (f->direction == DIRECTION_LOST) {
                f->state = FIGURE_STATE_DEAD;
                if (city_message_count[MESSAGE_CATEGORY_SHIP_LOST] == 0) {
                    city_message_use_popup = 1;
                    city_message_post(MESSAGE_NAVIGATION_IMPOSSIBLE, 0, 0, 0, 0);
                    city_message_count[MESSAGE_CATEGORY_SHIP_LOST]++;
                }
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                trade_ship_next_dock(f->destination_building_id, 0);
            }
            break;
        case FIGURE_ACTION_TRADE_SHIP_DOCKED: {
            int32_t ship_dock_id = f->destination_building_id;
            moving = 0;
            if (trade_ship_lost_dock(figure_id)) {
                f->ship_dock_attempts = 0;
                trade_ship_next_dock(ship_dock_id, 1);
            } else if (trade_ship_done_trading(figure_id)) {
                building *dock = building_get(ship_dock_id);
                dock->data.dock.unloading_docker_id = 0;
                dock->data.dock.ship_wait_days = 0;
                f->ship_dock_attempts = 0;
                trade_ship_next_dock(ship_dock_id, 1);
            }
            city_message_count[MESSAGE_CATEGORY_SHIP_LOST] = 0;
            switch (building_get(ship_dock_id)->data.dock.orientation) {
                case DOCK_WATER_ABOVE:
                    f->direction = DIRECTION_RIGHT;
                    break;
                case DOCK_WATER_RIGHT:
                    f->direction = DIRECTION_BOTTOM;
                    break;
                case DOCK_WATER_BELOW:
                    f->direction = DIRECTION_LEFT;
                    break;
                default:
                    f->direction = DIRECTION_TOP;
                    break;
            }
            f->animation_frame = 0;
            break;
        }
        default:
            f->action_state = FIGURE_ACTION_TRADE_SHIP_CREATED;
            moving = 0;
            break;
    }
    int32_t direction = (f->direction >= DIRECTION_COUNT) ? f->previous_tile_direction : f->direction;
    carrier_image_direction(direction);
    if (f->action_state == FIGURE_ACTION_CORPSE) {
        if (f->wait_ticks >= BOAT_SINKING_TICKS) {
            int32_t frame_dead = f->wait_ticks - BOAT_SINKING_TICKS;
            if (frame_dead >= BOAT_BUBBLES_FRAMES) {
                frame_dead = BOAT_BUBBLES_FRAMES - 1;
            }
            f->image_id = image_group(IMAGE_GROUP_FIGURE_BUBBLES) + frame_dead;
            return;
        }
        int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_TRADE_SHIP_SWEPT_SINKING : IMAGE_GROUP_FIGURE_TRADE_SHIP_SINKING;
        f->image_id = (figure_image_direction / DIRECTION_COUNT) + image_group(group) + (f->wait_ticks / BOAT_TICKS_PER_IMAGE);
        return;
    }
    if (moving) {
        int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_TRADE_SHIP_SWEPT : IMAGE_GROUP_FIGURE_TRADE_SHIP;
        f->image_id = image_group(group) + ((f->animation_frame / BOAT_TICKS_PER_IMAGE) * DIRECTION_COUNT) + figure_image_direction;
        return;
    }
    int32_t group = map_sweep_has_passed(f->x) ? IMAGE_GROUP_FIGURE_TRADE_SHIP_SWEPT_IDLE : IMAGE_GROUP_FIGURE_TRADE_SHIP_IDLE;
    f->image_id = image_group(group) + figure_image_direction;
}
