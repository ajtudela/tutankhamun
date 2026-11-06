// The food supply percentage the chief and population advisors show and the house construction warnings check: the
// food the city can expect in a month (farms, hunting lodges, fishing wharves, cattle ranches and imports from open
// trade cities) against the food it needs.
//
// A cattle ranch's food is estimated from its model's laborers, not from its workers.

#include "city/food_supply.h"

#include "building/building_data.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "city/city_internal.h"
#include "city/resource_type.h"
#include "empire/empire.h"
#include "map/farm.h"

#include <stdbool.h>

enum food_estimate {
    FLOODPLAIN_FARM_FOOD_FACTOR = 48,
    MEADOW_FARM_FOOD_FACTOR = 64,
    HARVEST_FOOD_DIVISOR = 10,
    INDUSTRY_FOOD_FACTOR = 16,
    INDUSTRY_FOOD_DIVISOR = 4,
    INDUSTRY_FOOD_PERCENTAGE = 80,
    IMPORT_AMOUNT_PER_LOAD = 100,
};

bool building_farm_counts_for_food(int32_t building_id)
{
    int32_t type = building_get(building_id)->type;
    return ((type >= BUILDING_GRAIN_FARM) && (type <= BUILDING_CHICKPEAS_FARM)) || (type == BUILDING_FIG_FARM);
}

int32_t city_resource_food_supply_percentage(void)
{
    const city_player *player = city_player_current();
    int32_t hunting = (player->hunting_lodges_working != 0) ? ((player->game_meat_last_month / player->hunting_lodges_working) * building_count_industry_active[RESOURCE_GAMEMEAT]) : 0;
    int32_t fishing = (player->fishing_wharfs_working != 0) ? ((player->fish_last_month / player->fishing_wharfs_working) * building_count_industry_active[RESOURCE_FISH]) : 0;
    int32_t industries = 0;
    int32_t floodplain = 0;
    int32_t meadow = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        uint8_t is_farm = building_is_farm(id);
        if (!is_farm && b->type != BUILDING_CATTLE_RANCH) {
            continue;
        }
        if (is_farm && !building_farm_counts_for_food(id)) {
            continue;
        }
        if (building_is_floodplain_farm(id)) {
            floodplain += building_farm_expected_produce(id, 1);
        } else if (is_farm) {
            meadow += building_farm_expected_produce(id, 1);
        } else if (b->num_workers > 0 && b->data.industry.progress > 0) {
            industries += (building_model_table[b->type].laborers * INDUSTRY_FOOD_FACTOR) / INDUSTRY_FOOD_DIVISOR;
        }
    }
    int32_t harvests = (floodplain * FLOODPLAIN_FARM_FOOD_FACTOR) + (meadow * MEADOW_FARM_FOOD_FACTOR);
    industries = calc_adjust_with_percentage(industries, INDUSTRY_FOOD_PERCENTAGE);
    int32_t imports = 0;
    for (int32_t id = 0; id < EMPIRE_CITY_COUNT; id++) {
        const empire_city *c = empire_city_get(id);
        int32_t type = c->type;
        if (!c->in_use || (type != EMPIRE_CITY_PHARAOH_TRADING && type != EMPIRE_CITY_EGYPTIAN_TRADING && type != EMPIRE_CITY_FOREIGN_TRADING) || !c->is_open) {
            continue;
        }
        for (int32_t r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            int16_t status = player->trade_status[r];
            if ((status == TRADE_STATUS_IMPORT || status == TRADE_STATUS_IMPORT_AS_NEEDED) && c->sells[r] == 1) {
                imports += (trade_route_limits[c->route_id][r] * IMPORT_AMOUNT_PER_LOAD) / MONTHS_PER_YEAR;
            }
        }
    }
    int32_t supply = crt_ftol_double(((double) harvests / (double) HARVEST_FOOD_DIVISOR) + (double) (imports + industries + fishing + hunting));
    return (supply * 100) / city_food_consumption_percentage(city_current_player);
}
