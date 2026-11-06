// The gameplay config table new_city_rules.json: the records and columns of new_city_rules (city/city_init.h), the
// health, sentiment, wages and tax a new city starts with.

#include "city/city_init.h"
#include "core/config.h"

static const config_name new_city_rule_names[] = {
    { "health", NEW_CITY_RULE_HEALTH },
    { "sentiment", NEW_CITY_RULE_SENTIMENT },
    { "kingdom_wages", NEW_CITY_RULE_KINGDOM_WAGES },
    { "wages", NEW_CITY_RULE_WAGES },
    { "tax_percentage", NEW_CITY_RULE_TAX_PERCENTAGE },
    { 0, 0 },
};

static const config_field new_city_rule_fields[] = {
    { "value", new_city_rules, 1, 1, 0, 100, 0 },
};

static const config_storage new_city_rule_storage[] = {
    { new_city_rules, sizeof(new_city_rules) },
};

const config_table new_city_rule_table = {
    "new_city_rules.json",
    "rule",
    new_city_rule_names,
    NEW_CITY_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(new_city_rule_fields, new_city_rule_storage),
};
