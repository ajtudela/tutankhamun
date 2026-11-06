// Stocks of the storage buildings and workshops: the granary list and totals, the road access, room and stock
// of the storage yards, the city's food stocks and supply, and the raw material room and stock of the workshops.
// The tutorial goals of the stocks are checked on the way.
//
// Past the granary list's capacity the last slot keeps the latest granary, and the city stocks are always computed for
// player 1, whatever the current player.

#include "building/building_stocks.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "map/road_access.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t granary_total_stored[36];

int32_t stock_rules[STOCK_RULE_COUNT] = {
    [STOCK_RULE_GRANARY_LIST_MIN_FOOD] = 100,
    [STOCK_RULE_GRANARY_OPERATING_WORKERS_PERCENT] = 50,
    [STOCK_RULE_TUTORIAL_POTTERY_STORED] = 200,
    [STOCK_RULE_TUTORIAL_BEER_STORED] = 300,
    [STOCK_RULE_TUTORIAL_BRICKS_STORED] = 800,
    [STOCK_RULE_TUTORIAL_GRANARY_GAMEMEAT] = 400,
    [STOCK_RULE_TUTORIAL_GRANARY_FIGS] = 800,
    [STOCK_RULE_KINGDOM_GRAIN_BAZAAR_FOOD] = 200,
    [STOCK_RULE_KINGDOM_GRAIN_SUPPLY_MONTHS] = 12,
};

static int32_t tutorial_goal_open(int32_t flag)
{
    return settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[flag].mission && !tutorial_flags_a[flag];
}

void building_granaries_calculate_stocks(void)
{
    for (int32_t i = 0; i < GRANARY_LIST_SLOTS; i++) {
        granary_ids[i] = 0;
    }
    for (int32_t r = 0; r < RESOURCE_MAX; r++) {
        granary_total_stored[r] = 0;
    }
    granary_count = 0;
    int16_t *next = granary_ids;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || (b->type != BUILDING_GRANARY && b->type != BUILDING_GRANARY_UP) || !b->has_road_access ||
            b->distance_from_entry <= 0) {
            continue;
        }
        const building_storage *storage = storage_get(b->storage_id);
        int32_t total = 0;
        for (int32_t r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            if (storage->resource_state[r] != STORAGE_STATE_GET) {
                int32_t amount = b->data.granary.resource_stored[r];
                total += amount;
                granary_total_stored[r] += amount;
            }
        }
        if (total > stock_rules[STOCK_RULE_GRANARY_LIST_MIN_FOOD]) {
            *next = (int16_t) id;
            if (next < granary_ids + GRANARY_LIST_CAPACITY) {
                granary_count++;
                next++;
            }
        }
    }
}

void city_resource_calculate_warehouse_stocks_for_player(int32_t player)
{
    city_player *city = city_player_get(player);
    int32_t *space = city->storage_space;
    int32_t *stored = city->storage_stored;
    for (int32_t r = 0; r < RESOURCE_MAX; r++) {
        space[r] = 0;
        stored[r] = 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && (b->type == BUILDING_STORAGE_YARD || b->type == BUILDING_STORAGE_YARD_UP)) {
            b->has_road_access = STORAGE_YARD_NO_ROAD;
            if (map_has_road_access(b->x, b->y, b->size)) {
                b->has_road_access = STORAGE_YARD_ROAD_ADJACENT;
            } else if (map_has_road_access(b->x, b->y, STORAGE_YARD_SIZE)) {
                b->has_road_access = STORAGE_YARD_ROAD_NEAR;
            }
        }
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_STORAGE_ROOM) {
            continue;
        }
        uint8_t access = building_get(building_main(id))->has_road_access;
        if (!access) {
            continue;
        }
        b->has_road_access = access;
        int32_t resource = b->subtype;
        if (resource == RESOURCE_NONE) {
            space[RESOURCE_NONE] += STORAGE_ROOM_LOADS;
        } else {
            int32_t loads = b->loads_stored;
            space[resource] += STORAGE_ROOM_LOADS - loads;
            stored[resource] += loads;
        }
    }
    if (tutorial_goal_open(TUTORIAL_FLAG_POTTERY_STORED) &&
        !settings_data.is_custom_scenario && stored[RESOURCE_POTTERY] >= stock_rules[STOCK_RULE_TUTORIAL_POTTERY_STORED]) {
        tutorial_set_flag(TUTORIAL_FLAG_POTTERY_STORED);
    }
    if (tutorial_goal_open(TUTORIAL_FLAG_BEER_STORED) &&
        !settings_data.is_custom_scenario && stored[RESOURCE_BEER] >= stock_rules[STOCK_RULE_TUTORIAL_BEER_STORED]) {
        tutorial_set_flag(TUTORIAL_FLAG_BEER_STORED);
    }
    if (tutorial_goal_open(TUTORIAL_FLAG_BRICKS_STORED) &&
        tutorial_flags_a[TUTORIAL_FLAG_SCRIBE] && !settings_data.is_custom_scenario && stored[RESOURCE_BRICKS] >= stock_rules[STOCK_RULE_TUTORIAL_BRICKS_STORED]) {
        tutorial_set_flag(TUTORIAL_FLAG_BRICKS_STORED);
    }
}

void city_resource_calculate_warehouse_stocks(void)
{
    if (city_player_get(1)->in_use) {
        city_resource_calculate_warehouse_stocks_for_player(1);
    }
}

void city_resource_calculate_food_stocks_for_player(int32_t player)
{
    city_player *city = city_player_get(player);
    int32_t house_food[RESOURCE_MAX_FOOD] = { 0 };
    int32_t *granary_food = city->granary_food_stored;
    for (int32_t r = 0; r < RESOURCE_MAX_FOOD; r++) {
        granary_food[r] = 0;
    }
    city->food_stored_total = 0;
    city->food_types_available = 0;
    city->food_supply_months = 0;
    city->granaries_operating = 0;
    city->granaries_understaffed = 0;
    city->granaries_not_operating = 0;
    city->granaries_not_operating_with_food = 0;
    int32_t figs_total = 0;
    int32_t gamemeat_total = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP) {
            b->has_road_access = 0;
            if (!map_has_road_access_granary(b->x, b->y)) {
                continue;
            }
            b->has_road_access = 1;
            int32_t pct_workers = calc_percentage(b->num_workers, building_model_table[b->type].laborers);
            if (pct_workers < 100) {
                city->granaries_understaffed++;
            }
            if (pct_workers < stock_rules[STOCK_RULE_GRANARY_OPERATING_WORKERS_PERCENT]) {
                int32_t amount = 0;
                for (int32_t r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                    amount += b->data.granary.resource_stored[r];
                }
                city->granaries_not_operating++;
                if (amount > 0) {
                    city->granaries_not_operating_with_food++;
                }
            } else {
                const int16_t *stored = b->data.granary.resource_stored;
                figs_total += stored[RESOURCE_FIGS];
                gamemeat_total += stored[RESOURCE_GAMEMEAT];
                for (int32_t r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    granary_food[r] += stored[r];
                }
                city->granaries_operating++;
                if (!settings_data.is_custom_scenario) {
                    if (tutorial_goal_open(TUTORIAL_FLAG_GRANARY_FOOD) &&
                        tutorial_flags_a[TUTORIAL_FLAG_POPULATION_150] && gamemeat_total >= stock_rules[STOCK_RULE_TUTORIAL_GRANARY_GAMEMEAT]) {
                        tutorial_set_flag(TUTORIAL_FLAG_GRANARY_FOOD);
                        if (settings_data.is_custom_scenario) {
                            continue;
                        }
                    }
                    if (tutorial_goal_open(TUTORIAL_FLAG_FOOD_STOCK) &&
                        figs_total >= stock_rules[STOCK_RULE_TUTORIAL_GRANARY_FIGS]) {
                        tutorial_set_flag(TUTORIAL_FLAG_FOOD_STOCK);
                    }
                }
            }
        } else if (b->house_size && b->house_population) {
            for (int32_t slot = INVENTORY_GRAIN; slot <= INVENTORY_GAMEMEAT; slot++) {
                house_food[RESOURCE_MIN_FOOD + slot] += b->data.house.inventory[slot];
            }
        }
    }
    for (int32_t r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        if (granary_food[r] || house_food[r]) {
            city->food_stored_total += granary_food[r];
            city->food_types_available++;
        }
    }
    int32_t needed = calc_adjust_with_percentage(city->population, city_food_consumption_percentage(player));
    city->food_needed_per_month = needed;
    if (needed <= 0) {
        city->food_supply_months = city->food_stored_total > 0;
    } else {
        city->food_supply_months = city->food_stored_total / needed;
    }
    if (scenario_data.kingdom_supplies_grain) {
        city->food_types_available = 1;
        city->food_supply_months = stock_rules[STOCK_RULE_KINGDOM_GRAIN_SUPPLY_MONTHS];
    }
}

void city_resource_calculate_food_stocks_and_supply(void)
{
    if (city_player_get(1)->in_use) {
        city_resource_calculate_food_stocks_for_player(1);
    }
    int32_t supplied = scenario_data.kingdom_supplies_grain;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && (b->type == BUILDING_BAZAAR || b->type == BUILDING_BAZAAR_UP) && supplied) {
            b->data.market.inventory[INVENTORY_GRAIN] = (int16_t) stock_rules[STOCK_RULE_KINGDOM_GRAIN_BAZAAR_FOOD];
        }
    }
}

static void add_workshop_input(int32_t *space, int32_t *stored, int32_t type, int32_t input, int32_t stock)
{
    int32_t room = building_input_stock_max(type) - stock;
    if (room < 0) {
        room = 0;
    }
    space[input] += room;
    stored[input] += stock;
}

void city_resource_calculate_workshop_stocks_for_player(int32_t player)
{
    city_player *city = city_player_get(player);
    int32_t *space = city->workshop_space;
    int32_t *stored = city->workshop_stored;
    for (int32_t r = 0; r < WORKSHOP_STOCK_RESOURCES; r++) {
        stored[r] = 0;
        space[r] = 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || !building_takes_input(b->type)) {
            continue;
        }
        b->has_road_access = 0;
        if (!map_has_road_access(b->x, b->y, b->size)) {
            continue;
        }
        b->has_road_access = 1;
        add_workshop_input(space, stored, b->type, b->subtype, b->loads_stored);
        if (b->second_input_resource) {
            add_workshop_input(space, stored, b->type, b->second_input_resource, b->second_input_stock);
        }
    }
}

void city_resource_calculate_workshop_stocks(void)
{
    if (city_player_get(1)->in_use) {
        city_resource_calculate_workshop_stocks_for_player(1);
    }
}
