// The gameplay config tables of the empire (core/config.h): the trade rules (debt that stops imports, route limits,
// trader delays and loads, stock kept before export) and the city cultures (image offset and luxury goods kind).

#include "core/config.h"
#include "empire/empire.h"
#include "empire/empire_data.h"

static const config_name empire_trade_rule_names[] = {
    { "import_debt_limit", EMPIRE_TRADE_RULE_IMPORT_DEBT_LIMIT },
    { "limit_small", EMPIRE_TRADE_RULE_LIMIT_SMALL },
    { "limit_medium", EMPIRE_TRADE_RULE_LIMIT_MEDIUM },
    { "limit_large", EMPIRE_TRADE_RULE_LIMIT_LARGE },
    { "sea_entry_delay", EMPIRE_TRADE_RULE_SEA_ENTRY_DELAY },
    { "land_entry_delay", EMPIRE_TRADE_RULE_LAND_ENTRY_DELAY },
    { "new_city_entry_delay", EMPIRE_TRADE_RULE_NEW_CITY_ENTRY_DELAY },
    { "no_docks_message_ticks", EMPIRE_TRADE_RULE_NO_DOCKS_MESSAGE_TICKS },
    { "ship_loads", EMPIRE_TRADE_RULE_SHIP_LOADS },
    { "trader_wait_ticks", EMPIRE_TRADE_RULE_TRADER_WAIT_TICKS },
    { "keep_workshop_amount", EMPIRE_TRADE_RULE_KEEP_WORKSHOP_AMOUNT },
    { "keep_monument_amount", EMPIRE_TRADE_RULE_KEEP_MONUMENT_AMOUNT },
    { "keep_other_amount", EMPIRE_TRADE_RULE_KEEP_OTHER_AMOUNT },
    { "food_population_small", EMPIRE_TRADE_RULE_FOOD_POPULATION_SMALL },
    { "food_population_medium", EMPIRE_TRADE_RULE_FOOD_POPULATION_MEDIUM },
    { "food_population_large", EMPIRE_TRADE_RULE_FOOD_POPULATION_LARGE },
    { 0, 0 },
};

static const config_field empire_trade_rule_fields[] = {
    { "value", empire_trade_rules, 1, 1, -100000, 100000, 0 },
};

static const config_storage empire_trade_rule_storage[] = {
    { empire_trade_rules, sizeof(empire_trade_rules) },
};

const config_table empire_trade_rule_table = {
    "empire_trade_rules.json",
    "rule",
    empire_trade_rule_names,
    EMPIRE_TRADE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(empire_trade_rule_fields, empire_trade_rule_storage),
};

static const config_field empire_city_culture_fields[] = {
    { "image_offset", &empire_culture_table[0].image_offset, (int32_t) (sizeof(empire_city_culture) / sizeof(int32_t)), 1, 0, 1000, 0 },
    { "luxury_kind", &empire_culture_table[0].luxury_kind, (int32_t) (sizeof(empire_city_culture) / sizeof(int32_t)), 1, 38, 53, 0 },
};

static const config_storage empire_city_culture_storage[] = {
    { empire_culture_table, sizeof(empire_culture_table) },
};

const config_table empire_city_culture_table = {
    "empire_city_cultures.json",
    "city_name",
    0,
    EMPIRE_CITY_NAMES,
    CONFIG_TABLE_COLUMNS(empire_city_culture_fields, empire_city_culture_storage),
};
