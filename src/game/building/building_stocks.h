// Building stocks (building/building_stocks.c) and the stock rules: the food a granary needs to be listed, the
// workers it needs to operate, the tutorial goal stocks and what the kingdom's grain gives.

#ifndef BUILDING_BUILDING_STOCKS_H
#define BUILDING_BUILDING_STOCKS_H

#include <stdint.h>

enum stock_rule {
    STOCK_RULE_GRANARY_LIST_MIN_FOOD,
    STOCK_RULE_GRANARY_OPERATING_WORKERS_PERCENT,
    STOCK_RULE_TUTORIAL_POTTERY_STORED,
    STOCK_RULE_TUTORIAL_BEER_STORED,
    STOCK_RULE_TUTORIAL_BRICKS_STORED,
    STOCK_RULE_TUTORIAL_GRANARY_GAMEMEAT,
    STOCK_RULE_TUTORIAL_GRANARY_FIGS,
    STOCK_RULE_KINGDOM_GRAIN_BAZAAR_FOOD,
    STOCK_RULE_KINGDOM_GRAIN_SUPPLY_MONTHS,
    STOCK_RULE_COUNT,
};

enum stock_list_limit {
    GRANARY_LIST_SLOTS = 100,
    GRANARY_LIST_CAPACITY = 98,
    WORKSHOP_STOCK_RESOURCES = 27,
    GRANARY_SIZE = 4,
};

enum storage_yard_road_access {
    STORAGE_YARD_NO_ROAD = 0,
    STORAGE_YARD_ROAD_ADJACENT = 1,
    STORAGE_YARD_ROAD_NEAR = 2,
};

extern int32_t stock_rules[STOCK_RULE_COUNT];

void city_resource_calculate_food_stocks_for_player(int32_t player);
void building_granaries_calculate_stocks(void);
void city_resource_calculate_warehouse_stocks(void);
void city_resource_calculate_food_stocks_and_supply(void);
void city_resource_calculate_workshop_stocks(void);

#endif
