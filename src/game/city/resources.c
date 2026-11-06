// City resources: the monthly food the houses eat, the payment for an import, the lists of resources and foods the
// city can have (those it can produce or import, those offered by the scenario's gifts or made from them, and those it
// had before), the stock caps and the goods the bazaars sell.

#include "city/resources.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_stocks.h"
#include "building/house_evolve.h"
#include "building/temple_complex.h"
#include "city/city_internal.h"
#include "city/resource_type.h"
#include "editor/editor_allowed_buildings.h"
#include "empire/empire.h"
#include "scenario/scenario_internal.h"
#include "window/window_data.h"

#include <stdbool.h>

enum {
    STOCK_CAP_SMALL = 10000,
    STOCK_CAP_DEFAULT = 15000,
    STOCK_CAP_LARGE = 30000,
};

static int32_t house_slot(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return 0;
        case RESOURCE_MEAT:
            return 1;
        case RESOURCE_LETTUCE:
            return 2;
        case RESOURCE_CHICKPEAS:
            return 3;
        case RESOURCE_POMEGRANATES:
            return 4;
        case RESOURCE_FIGS:
            return 5;
        case RESOURCE_FISH:
            return 6;
        case RESOURCE_GAMEMEAT:
            return 7;
        case RESOURCE_POTTERY:
            return INVENTORY_POTTERY;
        case RESOURCE_BEER:
            return INVENTORY_BEER;
        case RESOURCE_LINEN:
            return INVENTORY_LINEN;
        case RESOURCE_LUXURY_GOODS:
            return INVENTORY_JEWELRY;
        default:
            return -1;
    }
}

void city_resource_consume_food(void)
{
    city_player *player = city_player_current();
    city_resource_calculate_food_stocks_for_player(city_current_player);
    player->food_consumption_state = 0;
    int32_t percentage = city_food_consumption_percentage(city_current_player);
    player->food_types_eaten = 1;
    int32_t consumed = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_IN_USE || house->house_size == 0) {
            continue;
        }
        int32_t types = house_models[house->subtype].food_types;
        int32_t amount = calc_adjust_with_percentage(house->house_population, percentage);
        if (types > 1) {
            amount /= types;
        }
        house->data.house.food_types = 0;
        if (scenario_data.kingdom_supplies_grain) {
            house->data.house.inventory[0] = (int16_t) amount;
            house->data.house.food_types = 1;
            player->food_types_available = 1;
            continue;
        }
        if (types <= 0) {
            continue;
        }
        for (int32_t i = 0; i < city_foods.count; i++) {
            int32_t resource = city_foods.list[i];
            if (resource == RESOURCE_NONE) {
                continue;
            }
            int16_t *stock = &house->data.house.inventory[house_slot(resource)];
            if (*stock >= amount) {
                *stock = (int16_t) (*stock - amount);
                consumed += amount;
                house->data.house.food_types++;
            } else if (*stock != 0) {
                *stock = 0;
                consumed += amount;
                house->data.house.food_types++;
            }
            int32_t eaten = house->data.house.food_types;
            if (eaten > player->food_types_eaten) {
                player->food_types_eaten = eaten;
            }
            if (eaten >= types) {
                break;
            }
        }
    }
    player->food_consumed_last_month = consumed;
    player->food_produced_last_month = player->food_produced_this_month;
    player->food_produced_this_month = 0;
}

void city_finance_pay_for_import(int32_t player_id, int32_t resource)
{
    city_player *player = city_player_get(player_id);
    int32_t price = trade_price_get(resource)->buy_price;
    player->treasury -= price;
    player->imports_this_year += price;
}

bool city_resource_available_from_events(int32_t resource)
{
    for (int32_t i = scenario_events.count - 1; i >= 0; i--) {
        const scenario_event *event = &scenario_events.events[i];
        if (event->type == EVENT_TYPE_GIFT_FROM_PHARAOH && (event->item.fixed == resource || event->item.min == resource || event->item.max == resource)) {
            return 1;
        }
    }
    int32_t raw;
    if (resource == RESOURCE_BRICKS) {
        if ((empire_can_produce_resource(RESOURCE_CLAY) || empire_can_produce_resource_potentially(RESOURCE_CLAY)) &&
            city_resource_available_from_events(RESOURCE_STRAW)) {
            return 1;
        }
        if (city_resource_available_from_events(RESOURCE_CLAY) && empire_can_produce_resource(RESOURCE_GRAIN)) {
            return 1;
        }
        if (empire_can_import_resource_potentially(RESOURCE_CLAY) && city_resource_available_from_events(RESOURCE_STRAW)) {
            return 1;
        }
        if (city_resource_available_from_events(RESOURCE_CLAY) && empire_can_import_resource_potentially(RESOURCE_STRAW)) {
            return 1;
        }
        if (!city_resource_available_from_events(RESOURCE_CLAY)) {
            return 0;
        }
        raw = RESOURCE_STRAW;
    } else if (resource == RESOURCE_LAMPS) {
        if (!empire_can_produce_resource(RESOURCE_CLAY) && !empire_can_produce_resource_potentially(RESOURCE_CLAY) &&
            !city_resource_available_from_events(RESOURCE_CLAY) && !city_resource_available_from_events(RESOURCE_POTTERY)) {
            return 0;
        }
        return empire_can_import_resource_potentially(RESOURCE_OIL) != 0;
    } else {
        switch (resource) {
            case RESOURCE_WEAPONS:
                if (scenario_data.allowed_buildings[ALLOWED_WEAPONSMITH] == 0) {
                    return 0;
                }
                raw = RESOURCE_COPPER;
                break;
            case RESOURCE_POTTERY:
                raw = RESOURCE_CLAY;
                break;
            case RESOURCE_BEER:
                raw = RESOURCE_BARLEY;
                break;
            case RESOURCE_LINEN:
                raw = RESOURCE_FLAX;
                break;
            case RESOURCE_LUXURY_GOODS:
                raw = RESOURCE_GEMS;
                break;
            case RESOURCE_PAPYRUS:
                raw = RESOURCE_REEDS;
                break;
            case RESOURCE_CHARIOTS:
                if (scenario_data.allowed_buildings[ALLOWED_CHARIOT_MAKER] == 0) {
                    return 0;
                }
                raw = RESOURCE_TIMBER;
                break;
            case RESOURCE_PAINT:
                raw = RESOURCE_HENNA;
                break;
            default:
                return 0;
        }
    }
    return city_resource_available_from_events(raw) != 0;
}

void city_resource_determine_available(void)
{
    city_player *player = city_player_current();
    city_resources.count = 0;
    for (int32_t i = 0; i < RESOURCE_MAX; i++) {
        city_resources.list[i] = 0;
    }
    for (int32_t i = 0; i < RESOURCE_MAX; i++) {
        city_foods.list[i] = 0;
    }
    city_foods.count = 0;
    for (int32_t resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (empire_can_produce_resource(resource) || empire_can_import_resource_potentially(resource) || city_resource_available_from_events(resource) ||
            player->resource_known[resource]) {
            city_resources.list[city_resources.count++] = resource;
        }
    }
    for (int32_t food = 1; food < RESOURCE_MAX_FOOD; food++) {
        if (empire_can_produce_resource(food) || empire_can_import_resource_potentially(food) || city_resource_available_from_events(food) ||
            player->food_known[food]) {
            city_foods.list[city_foods.count++] = food;
        }
    }
    for (int32_t i = 0; i < city_resources.count; i++) {
        if (city_resources.list[i] != 0) {
            player->resource_known[city_resources.list[i]] = 1;
        }
    }
    for (int32_t i = 0; i < city_foods.count; i++) {
        if (city_foods.list[i] != 0) {
            player->food_known[city_foods.list[i]] = 1;
        }
    }
}

int32_t city_resource_cap_amount(int32_t resource, int32_t amount)
{
    if (amount < 0) {
        return 0;
    }
    int32_t cap;
    switch (resource) {
        case RESOURCE_WEAPONS:
        case RESOURCE_CHARIOTS:
            cap = STOCK_CAP_SMALL;
            break;
        case RESOURCE_BRICKS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
            cap = STOCK_CAP_LARGE;
            break;
        default:
            cap = STOCK_CAP_DEFAULT;
            break;
    }
    return (amount <= cap) ? amount : cap;
}

int32_t resource_is_bazaar_good(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
        case RESOURCE_MEAT:
        case RESOURCE_LETTUCE:
        case RESOURCE_CHICKPEAS:
        case RESOURCE_POMEGRANATES:
        case RESOURCE_FIGS:
        case RESOURCE_FISH:
        case RESOURCE_GAMEMEAT:
        case RESOURCE_POTTERY:
        case RESOURCE_BEER:
        case RESOURCE_LINEN:
        case RESOURCE_LUXURY_GOODS:
            return 1;
        default:
            return 0;
    }
}
