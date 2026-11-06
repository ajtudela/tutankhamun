// The gameplay config table of the house services (core/config.h): entertainment points and the desirability of
// water and elevation.

#include "building/house_services.h"
#include "core/config.h"

static const config_name house_service_rule_names[] = {
    { "coverage_points_divisor", HOUSE_SERVICE_RULE_COVERAGE_POINTS_DIVISOR },
    { "juggler_points", HOUSE_SERVICE_RULE_JUGGLER_POINTS },
    { "musician_points", HOUSE_SERVICE_RULE_MUSICIAN_POINTS },
    { "dancer_points", HOUSE_SERVICE_RULE_DANCER_POINTS },
    { "senet_points", HOUSE_SERVICE_RULE_SENET_POINTS },
    { "zoo_points", HOUSE_SERVICE_RULE_ZOO_POINTS },
    { "zoo_fills_above", HOUSE_SERVICE_RULE_ZOO_FILLS_ABOVE },
    { "entertainment_max", HOUSE_SERVICE_RULE_ENTERTAINMENT_MAX },
    { "water_desirability", HOUSE_SERVICE_RULE_WATER_DESIRABILITY },
    { "elevation_1_desirability", HOUSE_SERVICE_RULE_ELEVATION_1_DESIRABILITY },
    { "elevation_2_desirability", HOUSE_SERVICE_RULE_ELEVATION_2_DESIRABILITY },
    { "elevation_3_desirability", HOUSE_SERVICE_RULE_ELEVATION_3_DESIRABILITY },
    { "elevation_4_desirability", HOUSE_SERVICE_RULE_ELEVATION_4_DESIRABILITY },
    { "elevation_5_desirability", HOUSE_SERVICE_RULE_ELEVATION_5_DESIRABILITY },
    { 0, 0 },
};

static const config_field house_service_rule_fields[] = {
    { "value", house_service_rules, 1, 1, 1, 100, 0 },
};

static const config_storage house_service_rule_storage[] = {
    { house_service_rules, sizeof(house_service_rules) },
};

const config_table house_service_rule_table = {
    "house_service_rules.json",
    "rule",
    house_service_rule_names,
    HOUSE_SERVICE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(house_service_rule_fields, house_service_rule_storage),
};
