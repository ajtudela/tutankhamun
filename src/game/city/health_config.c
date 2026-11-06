// The gameplay config table health_rules.json: the records and columns of health_rules (city/health.h), the sickness
// months, the monthly health step and the points of the health services, food, malaria and plague.

#include "city/health.h"
#include "core/config.h"

static const config_name health_rule_names[] = {
    { "sick_months", HEALTH_RULE_SICK_MONTHS },
    { "tutorial_health", HEALTH_RULE_TUTORIAL_HEALTH },
    { "monthly_step", HEALTH_RULE_MONTHLY_STEP },
    { "warning_min_population", HEALTH_RULE_WARNING_MIN_POPULATION },
    { "plague_radius_health_step", HEALTH_RULE_PLAGUE_RADIUS_HEALTH_STEP },
    { "plague_max_radius", HEALTH_RULE_PLAGUE_MAX_RADIUS },
    { "bast_altar_percentage", HEALTH_RULE_BAST_ALTAR_PERCENTAGE },
    { "disease_chance", HEALTH_RULE_DISEASE_CHANCE },
    { "mortuary_points", HEALTH_RULE_MORTUARY_POINTS },
    { "physician_points", HEALTH_RULE_PHYSICIAN_POINTS },
    { "food_points", HEALTH_RULE_FOOD_POINTS },
    { "second_food_points", HEALTH_RULE_SECOND_FOOD_POINTS },
    { "third_food_points", HEALTH_RULE_THIRD_FOOD_POINTS },
    { "physician_and_food_points", HEALTH_RULE_PHYSICIAN_AND_FOOD_POINTS },
    { "malaria_health_level", HEALTH_RULE_MALARIA_HEALTH_LEVEL },
    { "malaria_health_weight", HEALTH_RULE_MALARIA_HEALTH_WEIGHT },
    { "plague_marsh_distance", HEALTH_RULE_PLAGUE_MARSH_DISTANCE },
    { "plague_marsh_weight", HEALTH_RULE_PLAGUE_MARSH_WEIGHT },
    { "plague_bonus", HEALTH_RULE_PLAGUE_BONUS },
    { "plague_desert_bonus", HEALTH_RULE_PLAGUE_DESERT_BONUS },
    { "plague_desert_divisor", HEALTH_RULE_PLAGUE_DESERT_DIVISOR },
    { 0, 0 },
};

static const config_field health_rule_fields[] = {
    { "value", health_rules, 1, 1, 1, 100000, 0 },
};

static const config_storage health_rule_storage[] = {
    { health_rules, sizeof(health_rules) },
};

const config_table health_rule_table = {
    "health_rules.json",
    "rule",
    health_rule_names,
    HEALTH_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(health_rule_fields, health_rule_storage),
};
