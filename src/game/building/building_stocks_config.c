// The gameplay config table of the city's stocks (core/config.h): the granary listing and staffing needs, the
// tutorial goal stocks and what the kingdom's grain gives.

#include "building/building_stocks.h"
#include "core/config.h"

static const config_name stock_rule_names[] = {
    { "granary_list_min_food", STOCK_RULE_GRANARY_LIST_MIN_FOOD },
    { "granary_operating_workers_percent", STOCK_RULE_GRANARY_OPERATING_WORKERS_PERCENT },
    { "tutorial_pottery_stored", STOCK_RULE_TUTORIAL_POTTERY_STORED },
    { "tutorial_beer_stored", STOCK_RULE_TUTORIAL_BEER_STORED },
    { "tutorial_bricks_stored", STOCK_RULE_TUTORIAL_BRICKS_STORED },
    { "tutorial_granary_gamemeat", STOCK_RULE_TUTORIAL_GRANARY_GAMEMEAT },
    { "tutorial_granary_figs", STOCK_RULE_TUTORIAL_GRANARY_FIGS },
    { "kingdom_grain_bazaar_food", STOCK_RULE_KINGDOM_GRAIN_BAZAAR_FOOD },
    { "kingdom_grain_supply_months", STOCK_RULE_KINGDOM_GRAIN_SUPPLY_MONTHS },
    { 0, 0 },
};

static const config_field stock_rule_fields[] = {
    { "value", stock_rules, 1, 1, 0, 30000, 0 },
};

static const config_storage stock_rule_storage[] = {
    { stock_rules, sizeof(stock_rules) },
};

const config_table stock_rule_table = {
    "stock_rules.json",
    "rule",
    stock_rule_names,
    STOCK_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(stock_rule_fields, stock_rule_storage),
};
