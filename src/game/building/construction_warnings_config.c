// The gameplay config table of the build tool's warnings (core/config.h): their thresholds of population,
// workers, unemployment, food share and road access messages.

#include "building/construction_warnings.h"
#include "core/config.h"

static const config_name construction_warning_rule_names[] = {
    { "min_population", CONSTRUCTION_WARNING_RULE_MIN_POPULATION },
    { "min_workers_needed", CONSTRUCTION_WARNING_RULE_MIN_WORKERS_NEEDED },
    { "max_unemployment_percent", CONSTRUCTION_WARNING_RULE_MAX_UNEMPLOYMENT_PERCENT },
    { "min_food_produced_percent", CONSTRUCTION_WARNING_RULE_MIN_FOOD_PRODUCED_PERCENT },
    { "max_problem_messages", CONSTRUCTION_WARNING_RULE_MAX_PROBLEM_MESSAGES },
    { 0, 0 },
};

static const config_field construction_warning_rule_fields[] = {
    { "value", construction_warning_rules, 1, 1, 0, 30000, 0 },
};

static const config_storage construction_warning_rule_storage[] = {
    { construction_warning_rules, sizeof(construction_warning_rules) },
};

const config_table construction_warning_rule_table = {
    "construction_warning_rules.json",
    "rule",
    construction_warning_rule_names,
    CONSTRUCTION_WARNING_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(construction_warning_rule_fields, construction_warning_rule_storage),
};
