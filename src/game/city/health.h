// City health (city/health.c): the monthly update, the house risks, the outbreaks and their messages; health_rules holds
// the tunable values.

#ifndef CITY_HEALTH_H
#define CITY_HEALTH_H

#include <stdbool.h>
#include <stdint.h>

enum {
    HEALTH_MAX = 100,
    HOUSE_RISK_MAX = 1000,
};

enum plague_spread {
    PLAGUE_SPREAD_RADIUS = 2,
};

enum health_rule {
    HEALTH_RULE_SICK_MONTHS,
    HEALTH_RULE_TUTORIAL_HEALTH,
    HEALTH_RULE_MONTHLY_STEP,
    HEALTH_RULE_WARNING_MIN_POPULATION,
    HEALTH_RULE_PLAGUE_RADIUS_HEALTH_STEP,
    HEALTH_RULE_PLAGUE_MAX_RADIUS,
    HEALTH_RULE_BAST_ALTAR_PERCENTAGE,
    HEALTH_RULE_DISEASE_CHANCE,
    HEALTH_RULE_MORTUARY_POINTS,
    HEALTH_RULE_PHYSICIAN_POINTS,
    HEALTH_RULE_FOOD_POINTS,
    HEALTH_RULE_SECOND_FOOD_POINTS,
    HEALTH_RULE_THIRD_FOOD_POINTS,
    HEALTH_RULE_PHYSICIAN_AND_FOOD_POINTS,
    HEALTH_RULE_MALARIA_HEALTH_LEVEL,
    HEALTH_RULE_MALARIA_HEALTH_WEIGHT,
    HEALTH_RULE_PLAGUE_MARSH_DISTANCE,
    HEALTH_RULE_PLAGUE_MARSH_WEIGHT,
    HEALTH_RULE_PLAGUE_BONUS,
    HEALTH_RULE_PLAGUE_DESERT_BONUS,
    HEALTH_RULE_PLAGUE_DESERT_DIVISOR,
    HEALTH_RULE_COUNT,
};

extern int32_t health_rules[HEALTH_RULE_COUNT];

void city_health_update_house(int32_t building_id, bool recompute);
void city_health_infect_house(int32_t building_id);
void city_health_malaria(int32_t building_id);
bool city_health_spread_plague(int32_t building_id);
void city_health_decrease_plague_distance(void);
void city_health_post_disease_messages(void);
bool city_health_roll_disease_chance(void);
void city_health_update(void);

#endif
