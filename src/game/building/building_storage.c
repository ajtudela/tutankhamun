// Storage of the granaries, storage yards and bazaars: the destinations of cart pushers and storage carts,
// adding goods to granaries, yards, workshops and the recruiter, taking goods from the yards, and the 200
// storage records with their building ids rebuilt after a load or an undo.
//
// The deliveries keep three oddities of the game:
// - the granary supplier passes the granary's road x as its y and doubles short distances below 401 food;
// - a recruiter delivery past the maximum can push the stock above it;
// - a delivery to building 0 counts as one stored.

#include "building/building_storage.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_stocks.h"
#include "building/building_type.h"
#include "city/finance.h"
#include "city/labor.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tutorial.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "window/window_data.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum storage_input_limit {
    GRANARY_SUPPLIER_LOW_FOOD = 401,
    GRANARY_GETTING_MIN_SPACE = 100,
    TUTORIAL_GOLD_DELIVERED = 500,
    STORE_ANYWHERE_ALL = 1000000,
};

static void add_candidate(int32_t id, int32_t tile)
{
    route_candidates.buildings[route_candidates.count] = id;
    route_candidates.tiles[route_candidates.count] = tile;
    route_candidates.count++;
}

static int32_t is_granary(int32_t type)
{
    return type == BUILDING_GRANARY || type == BUILDING_GRANARY_UP;
}

static int32_t is_fully_staffed(const building *b)
{
    return calc_percentage(b->num_workers, building_model_table[b->type].laborers) >= 100;
}

static int32_t storage_target(const building_storage *s, int32_t resource, int16_t *target)
{
    int32_t state = s->resource_state[resource];
    if (state == STORAGE_STATE_ACCEPT) {
        *target = s->accept_amount[resource];
        return 1;
    }
    if (state == STORAGE_STATE_GET) {
        *target = s->get_amount[resource];
        return 1;
    }
    return 0;
}

int32_t building_granary_for_storing(int32_t source, int32_t figure_id, int32_t resource, int32_t road_network)
{
    uint32_t network = (uint32_t) road_network;
    if (scenario_data.kingdom_supplies_grain != 0 || resource < RESOURCE_MIN_FOOD || resource > RESOURCE_GAMEMEAT) {
        return 0;
    }
    if (city_player_current()->stockpiled[resource] != 0 && source == 0) {
        return 0;
    }
    route_candidates.count = 0;
    if (building_highest_id < 1) {
        return 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !is_granary(b->type) || !b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != network) {
            continue;
        }
        if (!is_fully_staffed(b)) {
            storage_staff_missing++;
            continue;
        }
        int16_t target;
        if (!storage_target(storage_get(b->storage_id), resource, &target)) {
            continue;
        }
        int16_t stock = b->data.granary.resource_stored[resource];
        if (target <= stock) {
            continue;
        }
        if (b->incoming_carts >= 1 && (target - b->incoming_loads - stock) <= 0) {
            continue;
        }
        if (b->data.granary.resource_stored[RESOURCE_NONE] < STORAGE_CART_LOAD) {
            continue;
        }
        route_candidates.buildings[route_candidates.count] = id;
        map_has_road_access_granary(b->x, b->y);
        route_candidates.tiles[route_candidates.count] = map_result_x + (map_result_y * GRID_SIZE) + map_info.grid_start;
        route_candidates.count++;
    }
    if (route_candidates.count == 0) {
        return 0;
    }
    int32_t chosen = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
    if (chosen == 0) {
        return 0;
    }
    int32_t id = route_candidates.buildings[chosen - 1];
    building *b = building_get(id);
    map_result_x = b->road_access_x;
    map_result_y = b->road_access_y;
    if (id != 0) {
        b->incoming_carts++;
        b->incoming_loads += figure_get(figure_id)->resource_amount;
    }
    return id;
}

int32_t building_storageyard_for_storing(int32_t source, int32_t figure_id, int32_t resource, int32_t road_network)
{
    uint32_t network = (uint32_t) road_network;
    route_candidates.count = 0;
    if (resource == RESOURCE_GOLD) {
        return 0;
    }
    for (int32_t room_id = 1; room_id <= building_highest_id; room_id++) {
        building *room = building_get(room_id);
        if (room->state != BUILDING_STATE_IN_USE || room->type != BUILDING_STORAGE_ROOM || !room->has_road_access || room->distance_from_entry <= 0 ||
            room->road_network_id != network) {
            continue;
        }
        int32_t main_id = building_main(room_id);
        building *yard = building_get(main_id);
        const building_storage *s = storage_get(yard->storage_id);
        if (source == main_id && s->resource_state[resource] != STORAGE_STATE_GET) {
            continue;
        }
        int16_t target;
        if (!storage_target(s, resource, &target) || target <= building_storageyard_get_amount(main_id, resource)) {
            continue;
        }
        if (!is_fully_staffed(yard)) {
            storage_staff_missing++;
            continue;
        }
        if (room->subtype != RESOURCE_NONE && (room->subtype != resource || room->loads_stored >= STORAGE_ROOM_LOADS)) {
            continue;
        }
        building *main = building_get(building_main(room_id));
        if (main->has_road_access == 1) {
            add_candidate(room_id, map_grid_offset(main->road_access_x, main->road_access_y));
        } else if (map_has_road_access(main->x, main->y, STORAGE_YARD_SIZE)) {
            add_candidate(room_id, map_grid_offset(map_result_x, map_result_y));
        }
    }
    if (route_candidates.count == 0) {
        return 0;
    }
    int32_t chosen = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
    if (chosen == 0) {
        return 0;
    }
    int32_t room_id = route_candidates.buildings[chosen - 1];
    building *main = building_get(building_main(room_id));
    map_result_x = main->road_access_x;
    map_result_y = main->road_access_y;
    if (main->has_road_access == 1 || map_has_road_access(main->x, main->y, STORAGE_YARD_SIZE)) {
        return room_id;
    }
    return 0;
}

static int16_t storageyard_target(const building *yard, int32_t resource)
{
    const building_storage *s = storage_get(yard->storage_id);
    return (s->resource_state[resource] == STORAGE_STATE_ACCEPT) ? s->accept_amount[resource] : s->get_amount[resource];
}

bool building_storageyard_is_full(int32_t building_id, int32_t amount, int32_t resource)
{
    int16_t target = storageyard_target(building_get(building_id), resource);
    return target < building_storageyard_get_amount(building_id, resource) + amount;
}

int32_t building_storageyard_add_resource(int32_t player, int32_t room_id, int32_t resource, int32_t amount)
{
    int32_t main_id = building_main(room_id);
    building *yard = building_get(main_id);
    if (yard->num_workers <= 0 || yard->fire_duration != 0 || building_storageyard_is_full(main_id, 0, resource)) {
        return 0;
    }
    int16_t target = storageyard_target(yard, resource);
    int32_t excess = 0;
    if (building_storageyard_is_full(main_id, amount, resource)) {
        excess = building_storageyard_get_amount(main_id, resource) + (amount - target);
    }
    amount -= excess;
    if (amount < 1) {
        return amount;
    }
    int32_t stored = 0;
    while (1) {
        int16_t most = -1;
        int32_t best = 0;
        int32_t part = building_main(room_id);
        for (int32_t i = 0; i < STORAGE_YARD_ROOMS; i++) {
            part = building_get(part)->next_part_building_id;
            if (part < 1) {
                return 0;
            }
            building *room = building_get(part);
            if ((room->subtype == RESOURCE_NONE || room->subtype == resource) && room->loads_stored < STORAGE_ROOM_LOADS && most < room->loads_stored) {
                best = part;
                most = room->loads_stored;
            }
        }
        if (best < 1) {
            return 0;
        }
        building *room = building_get(best);
        city_player *city = city_player_get(player);
        city->storage_space[resource]--;
        city->storage_stored[resource]++;
        room->subtype = (int16_t) resource;
        room->loads_stored++;
        map_grids.images[room->grid_offset] = storage_room_image(resource, room->loads_stored);
        stored++;
        room_id = best;
        if (amount <= stored) {
            return amount;
        }
    }
}

int32_t building_granary_add_resource(int32_t id, int32_t resource, int32_t count, int32_t requested_amount)
{
    building *b = building_get(id);
    int32_t storage = b->storage_id;
    if (id < 1) {
        return 1;
    }
    if (!is_granary(b->type) || resource <= RESOURCE_NONE || resource >= RESOURCE_MAX_FOOD) {
        return 0;
    }
    int16_t space = b->data.granary.resource_stored[RESOURCE_NONE];
    if (space <= 0) {
        return 0;
    }
    int16_t target;
    if (storage_target(storage_get(storage), resource, &target) && b->data.granary.resource_stored[resource] >= target) {
        return 0;
    }
    int32_t amount = requested_amount;
    if (count != 0) {
        city_player_current()->food_produced_this_month += amount;
    }
    if (amount < space) {
        b->data.granary.resource_stored[resource] += (int16_t) amount;
        b->data.granary.resource_stored[RESOURCE_NONE] -= (int16_t) amount;
        return amount;
    }
    b->data.granary.resource_stored[resource] += space;
    int16_t left = b->data.granary.resource_stored[RESOURCE_NONE];
    b->data.granary.resource_stored[RESOURCE_NONE] = 0;
    return left;
}

int32_t building_granary_getting_food(int32_t figure_id, int32_t resource, int32_t road_network)
{
    uint32_t network = (uint32_t) road_network;
    if (scenario_data.kingdom_supplies_grain != 0 || resource <= RESOURCE_NONE || resource >= RESOURCE_MAX_FOOD) {
        return 0;
    }
    if (city_player_current()->stockpiled[resource] != 0) {
        return 0;
    }
    route_candidates.count = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !is_granary(b->type) || !b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != network) {
            continue;
        }
        if (!is_fully_staffed(b)) {
            continue;
        }
        const building_storage *s = storage_get(b->storage_id);
        if (s->resource_state[resource] == STORAGE_STATE_GET && b->data.granary.resource_stored[resource] < s->get_amount[resource] &&
            b->data.granary.resource_stored[RESOURCE_NONE] > GRANARY_GETTING_MIN_SPACE) {
            add_candidate(id, map_info.grid_start + b->x + GRID_SIZE + 1 + (b->y * GRID_SIZE));
        }
    }
    if (route_candidates.count == 0) {
        return 0;
    }
    int32_t chosen = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
    if (chosen == 0) {
        return 0;
    }
    building *b = building_get(route_candidates.buildings[chosen - 1]);
    map_result_y = b->y + 1;
    map_result_x = b->x + 1;
    return route_candidates.buildings[chosen - 1];
}

int32_t building_granary_supplier(int32_t id)
{
    building *b = building_get(id);
    const building_storage *s = storage_get(b->storage_id);
    int32_t getting = 0;
    for (int32_t resource = RESOURCE_MIN_FOOD; resource <= RESOURCE_GAMEMEAT; resource++) {
        if (s->resource_state[resource] == STORAGE_STATE_GET) {
            getting++;
        }
    }
    if (scenario_data.kingdom_supplies_grain != 0 || getting < 1) {
        return 0;
    }
    int32_t best_distance = 10000;
    int32_t best = 0;
    for (int32_t i = 0; i < granary_count; i++) {
        int32_t other_id = granary_ids[i];
        building *other = building_get(other_id);
        if (other->road_network_id != b->road_network_id) {
            continue;
        }
        const building_storage *other_storage = storage_get(other->storage_id);
        int32_t amount = 0;
        for (int32_t resource = RESOURCE_MIN_FOOD; resource <= RESOURCE_GAMEMEAT; resource++) {
            if (s->resource_state[resource] == STORAGE_STATE_GET && other_storage->resource_state[resource] != STORAGE_STATE_GET) {
                amount += other->data.granary.resource_stored[resource];
            }
        }
        if (amount <= 0) {
            continue;
        }
        int32_t distance = calc_distance_with_penalty(
            other->road_access_x, other->road_access_y, b->road_access_x, b->road_access_x, b->distance_from_entry, other->distance_from_entry);
        if (amount < GRANARY_SUPPLIER_LOW_FOOD) {
            distance *= 2;
        }
        if (distance < best_distance) {
            best_distance = distance;
            best = other_id;
        }
    }
    building *supplier = building_get(best);
    map_result_y = supplier->road_access_y;
    map_result_x = supplier->road_access_x;
    return best;
}

int32_t building_input_index(int32_t resource)
{
    switch (resource) {
        case RESOURCE_MEAT:
            return INDUSTRY_INPUT_MEAT;
        case RESOURCE_GAMEMEAT:
            return INDUSTRY_INPUT_GAMEMEAT;
        case RESOURCE_STRAW:
            return INDUSTRY_INPUT_STRAW;
        case RESOURCE_WEAPONS:
            return INDUSTRY_INPUT_WEAPONS;
        case RESOURCE_CLAY:
            return INDUSTRY_INPUT_CLAY;
        case RESOURCE_BRICKS:
            return INDUSTRY_INPUT_BRICKS;
        case RESOURCE_POTTERY:
            return INDUSTRY_INPUT_POTTERY;
        case RESOURCE_BARLEY:
            return INDUSTRY_INPUT_BARLEY;
        case RESOURCE_BEER:
            return INDUSTRY_INPUT_BEER;
        case RESOURCE_FLAX:
            return INDUSTRY_INPUT_FLAX;
        case RESOURCE_LINEN:
            return INDUSTRY_INPUT_LINEN;
        case RESOURCE_GEMS:
            return INDUSTRY_INPUT_GEMS;
        case RESOURCE_TIMBER:
            return INDUSTRY_INPUT_TIMBER;
        case RESOURCE_GOLD:
            return INDUSTRY_INPUT_GOLD;
        case RESOURCE_REEDS:
            return INDUSTRY_INPUT_REEDS;
        case RESOURCE_PAPYRUS:
            return INDUSTRY_INPUT_PAPYRUS;
        case RESOURCE_STONE:
            return INDUSTRY_INPUT_STONE;
        case RESOURCE_LIMESTONE:
            return INDUSTRY_INPUT_LIMESTONE;
        case RESOURCE_CHARIOTS:
            return INDUSTRY_INPUT_CHARIOTS;
        case RESOURCE_COPPER:
            return INDUSTRY_INPUT_COPPER;
        case RESOURCE_SANDSTONE:
            return INDUSTRY_INPUT_SANDSTONE;
        case RESOURCE_OIL:
            return INDUSTRY_INPUT_OIL;
        case RESOURCE_HENNA:
            return INDUSTRY_INPUT_HENNA;
        case RESOURCE_PAINT:
            return INDUSTRY_INPUT_PAINT;
        case RESOURCE_LAMPS:
            return INDUSTRY_INPUT_LAMPS;
        default:
            return -1;
    }
}

static int32_t is_upgraded_palace(int32_t type)
{
    return type == BUILDING_VILLAGE_PALACE_UP || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_CITY_PALACE;
}

int32_t building_workshop_for_raw_material(int32_t figure_id, int32_t resource, int32_t road_network)
{
    uint32_t network = (uint32_t) road_network;
    if (city_player_current()->stockpiled[resource] != 0) {
        return 0;
    }
    int32_t index = building_input_index(resource);
    if (index < 0) {
        return 0;
    }
    int32_t min_incoming = 10000;
    int32_t min_stock = 10000;
    route_candidates.count = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || city_labor_industry_is_mothballed(id)) {
            continue;
        }
        int32_t type = b->type;
        if (!building_takes_input(type) || !b->has_road_access || b->distance_from_entry <= 0) {
            continue;
        }
        if (b->subtype != index && b->second_input_resource != index) {
            continue;
        }
        if (b->road_network_id != network || (is_upgraded_palace(type) && b->num_workers == 0)) {
            continue;
        }
        int32_t max = building_input_stock_max(type);
        int32_t stock;
        int32_t incoming;
        if (b->second_input_resource != INDUSTRY_INPUT_NONE && b->second_input_resource == index) {
            stock = b->second_input_stock;
            incoming = b->incoming_loads;
        } else {
            stock = b->loads_stored;
            incoming = b->incoming_carts;
        }
        if (stock + STORAGE_CART_LOAD > max) {
            continue;
        }
        if (incoming > 0 && (max - (incoming * STORAGE_CART_LOAD) - stock) <= 0) {
            continue;
        }
        if (incoming > min_incoming || stock > min_stock) {
            continue;
        }
        if (stock < min_stock) {
            route_candidates.count = 0;
            min_incoming = incoming;
            min_stock = stock;
        }
        add_candidate(id, map_grid_offset(b->road_access_x, b->road_access_y));
    }
    int32_t chosen = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, route_candidates.count);
    int32_t id = chosen ? route_candidates.buildings[chosen - 1] : 0;
    building *b = building_get(id);
    map_result_x = b->road_access_x;
    map_result_y = b->road_access_y;
    if (id == 0) {
        return 0;
    }
    if (b->type == BUILDING_BRICKWORKS && b->second_input_resource == index) {
        b->incoming_loads++;
    } else {
        b->incoming_carts++;
    }
    return id;
}

int32_t building_workshop_for_raw_material_any(int32_t figure_id, int32_t resource, int32_t road_network)
{
    uint32_t network = (uint32_t) road_network;
    if (city_player_current()->stockpiled[resource] != 0) {
        return 0;
    }
    int32_t index = building_input_index(resource);
    if (index < 0) {
        return 0;
    }
    int32_t count = 0;
    route_candidates.count = 0;
    if (building_highest_id < 1) {
        return 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !building_takes_input(b->type)) {
            continue;
        }
        if (b->has_road_access && b->distance_from_entry > 0 && b->subtype == index && b->road_network_id == network) {
            route_candidates.buildings[count] = id;
            route_candidates.tiles[count] = map_grid_offset(b->x, b->y);
            count++;
        }
    }
    route_candidates.count = count;
    if (count == 0) {
        return 0;
    }
    int32_t chosen = figure_route_add_to_closest_of(figure_id, route_candidates.tiles, count);
    if (chosen <= 0) {
        return 0;
    }
    int32_t id = route_candidates.buildings[chosen - 1];
    building *b = building_get(id);
    map_result_x = b->road_access_x;
    map_result_y = b->road_access_y;
    return id;
}

static int32_t input_is_full(int32_t max, int32_t stock, int32_t incoming)
{
    return max <= stock || (incoming > 0 && (max - (incoming * STORAGE_CART_LOAD)) <= stock);
}

int32_t building_recruiter_for_weapons(int32_t source, int32_t figure_id, int32_t resource, int32_t road_network)
{
    uint32_t network = (uint32_t) road_network;
    const city_player *city = city_player_current();
    int32_t id = city->recruiter_building_id;
    int32_t index = building_input_index(resource);
    if (resource != RESOURCE_WEAPONS && resource != RESOURCE_CHARIOTS) {
        return 0;
    }
    if (city->stockpiled[resource] != 0 || building_count_active[BUILDING_RECRUITER] < 1) {
        return 0;
    }
    building *b = building_get(id);
    if (index == b->subtype && input_is_full(building_input_stock_max(b->type), b->loads_stored, b->incoming_carts)) {
        return 0;
    }
    if (index == b->second_input_resource && input_is_full(building_input_stock_max(b->type), b->second_input_stock, b->incoming_loads)) {
        return 0;
    }
    if (formation_num_legions < 1 || !map_has_road_access(b->x, b->y, b->size) || b->road_network_id != network) {
        return 0;
    }
    if (index == b->subtype) {
        b->incoming_carts++;
    }
    if (index == b->second_input_resource) {
        b->incoming_loads++;
    }
    return id;
}

static int32_t add_to_stock(int16_t *stock, int32_t max, int32_t amount)
{
    if (*stock >= max) {
        return 0;
    }
    int32_t excess = 0;
    if (max <= *stock + amount) {
        excess = *stock - max + amount;
    }
    *stock = (int16_t) (*stock - excess + amount);
    return amount - excess;
}

static void deliver_gold(int32_t amount)
{
    city_player *city = city_player_current();
    city->treasury += amount;
    city->gold_delivered += amount;
    city->gold_this_year += amount;
    if (!settings_data.is_custom_scenario && tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_GOLD_DELIVERED].mission == settings_data.mission_id &&
        !tutorial_flags_a[TUTORIAL_FLAG_GOLD_DELIVERED] && city->gold_delivered >= TUTORIAL_GOLD_DELIVERED) {
        tutorial_set_flag(TUTORIAL_FLAG_GOLD_DELIVERED);
    }
}

int32_t building_workshop_add_input(int32_t id, int32_t amount, int32_t resource)
{
    if (id <= 0) {
        return 0;
    }
    building *b = building_get(id);
    int32_t type = b->type;
    if (!building_takes_input(type)) {
        return 0;
    }
    if (resource == RESOURCE_GOLD) {
        if (!building_is_palace(id)) {
            return 0;
        }
        deliver_gold(amount);
        return amount;
    }
    int32_t index = building_input_index(resource);
    int32_t max = building_input_stock_max(type);
    if (b->subtype == index) {
        return add_to_stock(&b->loads_stored, max, amount);
    }
    if (b->second_input_resource == index) {
        return add_to_stock(&b->second_input_stock, max, amount);
    }
    return 0;
}

static int16_t add_past_max(int16_t stock, int32_t max, int32_t *amount)
{
    if (stock + *amount > max) {
        *amount += stock - max;
    }
    return (int16_t) (stock + *amount);
}

int32_t building_recruiter_add_input(int32_t id, int32_t amount, int32_t resource)
{
    if (id < 1) {
        return 1;
    }
    building *b = building_get(id);
    int32_t max = building_input_stock_max(b->type);
    if (resource == RESOURCE_WEAPONS) {
        if (b->loads_stored >= max) {
            return 0;
        }
        b->loads_stored = add_past_max(b->loads_stored, max, &amount);
        b->incoming_carts--;
        return amount;
    }
    if (b->second_input_stock >= max) {
        return 0;
    }
    b->second_input_stock = add_past_max(b->second_input_stock, max, &amount);
    b->incoming_loads--;
    return amount;
}

static int32_t storage_yard_free_space(const building *yard, int32_t resource)
{
    int32_t space = 0;
    for (int32_t room_id = yard->next_part_building_id; room_id != 0; room_id = building_get(room_id)->next_part_building_id) {
        const building *room = building_get(room_id);
        if (room->subtype == RESOURCE_NONE) {
            space += STORAGE_ROOM_LOADS;
        } else if (room->subtype == resource) {
            space += STORAGE_ROOM_LOADS - room->loads_stored;
        }
    }
    return space;
}

int32_t building_storage_store_anywhere(int32_t resource, int32_t remaining, int32_t count_only_flag)
{
    char count_only = (char) count_only_flag;
    int32_t placed = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !b->has_road_access || b->distance_from_entry <= 0 || b->num_workers <= 0) {
            continue;
        }
        const building_storage *s = storage_get(b->storage_id);
        if (b->type == BUILDING_STORAGE_YARD) {
            int16_t target;
            if (!storage_target(s, resource, &target)) {
                continue;
            }
            int32_t space = storage_yard_free_space(b, resource);
            if (target <= space) {
                space = target;
            }
            if (remaining <= space) {
                space = remaining;
            }
            if (!count_only) {
                building_storageyard_add_resource(city_current_player, id, resource, space);
            }
            remaining -= space;
            placed += space;
            if (remaining < 1) {
                return placed;
            }
        } else if (is_granary(b->type) && resource > RESOURCE_NONE && resource < RESOURCE_MAX_FOOD) {
            int16_t target;
            if (!storage_target(s, resource, &target)) {
                continue;
            }
            int32_t space = target - b->data.granary.resource_stored[resource];
            if (b->data.granary.resource_stored[RESOURCE_NONE] <= space) {
                space = b->data.granary.resource_stored[RESOURCE_NONE];
            }
            if (space <= 0) {
                continue;
            }
            if (remaining <= space) {
                space = remaining;
            }
            if (!count_only) {
                building_granary_add_resource(id, resource, 0, space);
            }
            remaining -= space;
            placed += space;
            if (remaining < 1) {
                return placed;
            }
        }
    }
    return placed;
}

int32_t building_storage_room_for(int32_t resource)
{
    return building_storage_store_anywhere(resource, STORE_ANYWHERE_ALL, 1);
}

int32_t building_storageyards_remove_resource(int32_t player, int32_t resource, int32_t amount)
{
    city_player *city = city_player_get(player);
    int32_t id = city->last_storage_yard_id;
    int32_t left = amount;
    for (int32_t i = 1; i < MAX_BUILDINGS && left >= 1; i++) {
        if (++id > MAX_BUILDINGS - 1) {
            id = 1;
        }
        const building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && (b->type == BUILDING_STORAGE_YARD || b->type == BUILDING_STORAGE_YARD_UP)) {
            city->last_storage_yard_id = (int16_t) id;
            left = building_storageyard_remove_resource(player, id, resource, left);
        }
    }
    return amount - left;
}

static void storage_refuse_all(building_storage *s)
{
    for (int32_t i = 0; i < STORAGE_RESOURCES; i++) {
        s->resource_state[i] = STORAGE_STATE_REFUSE;
        s->accept_amount[i] = STORAGE_AMOUNT_ALL;
        s->get_amount[i] = STORAGE_AMOUNT_ALL;
    }
}

int32_t building_storage_create(int32_t building_id)
{
    city_resource_determine_available();
    int32_t id;
    for (id = 1; id < STORAGE_RECORDS; id++) {
        if (storage_get(id)->in_use == 0) {
            break;
        }
    }
    if (id >= STORAGE_RECORDS) {
        return 0;
    }
    building_storage *s = storage_get(id);
    memset(s, 0, sizeof(building_storage));
    s->in_use = 1;
    s->building_id = (int16_t) building_id;
    if (building_storage_highest_id <= id) {
        building_storage_highest_id = id;
    }
    switch (building_get(building_id)->type) {
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
            for (int32_t i = 0; i < city_resources.count; i++) {
                int32_t resource = city_resources.list[i];
                if (resource_is_bazaar_good(resource)) {
                    s->resource_state[resource] = STORAGE_STATE_ACCEPT;
                }
            }
            break;
        case BUILDING_GRANARY:
            storage_refuse_all(s);
            for (int32_t i = 0; i < city_foods.count; i++) {
                s->resource_state[city_foods.list[i]] = STORAGE_STATE_ACCEPT;
            }
            break;
        case BUILDING_STORAGE_YARD:
            storage_refuse_all(s);
            for (int32_t i = 0; i < city_resources.count; i++) {
                int32_t resource = city_resources.list[i];
                if (resource < RESOURCE_MIN_FOOD || resource > RESOURCE_GAMEMEAT) {
                    s->resource_state[resource] = STORAGE_STATE_ACCEPT;
                }
            }
            break;
    }
    return id;
}

void building_storage_clear_all(void)
{
    memset(building_storages, 0, STORAGE_RECORDS * sizeof(building_storage));
}

void building_storage_reset_building_ids(void)
{
    for (int32_t i = 1; i < STORAGE_RECORDS; i++) {
        storage_get(i)->building_id = 0;
    }
    int32_t last = building_highest_id;
    for (int32_t id = 1; id <= last; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_UNUSED && building_type_has_storage(b->type) && b->storage_id != 0) {
            storage_get(b->storage_id)->in_use = 1;
        }
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_UNUSED || !building_type_has_storage(b->type)) {
            continue;
        }
        uint8_t slot = b->storage_id;
        if (slot == 0 || storage_get(slot)->building_id != 0) {
            b->storage_id = (uint8_t) building_storage_create(id);
        } else {
            storage_get(slot)->building_id = (int16_t) id;
        }
    }
    building_storage_highest_id = 0;
    for (int32_t i = 1; i < STORAGE_RECORDS; i++) {
        if (storage_get(i)->in_use != 0) {
            building_storage_highest_id = i;
        }
    }
}
