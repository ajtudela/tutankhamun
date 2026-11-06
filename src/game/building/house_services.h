// House services (building/house_services.c) and their rules: entertainment points from shows and venues, and
// the desirability gained beside water and per elevation level.

#ifndef BUILDING_HOUSE_SERVICES_H
#define BUILDING_HOUSE_SERVICES_H

#include <stdint.h>

enum house_service_rule {
    HOUSE_SERVICE_RULE_COVERAGE_POINTS_DIVISOR,
    HOUSE_SERVICE_RULE_JUGGLER_POINTS,
    HOUSE_SERVICE_RULE_MUSICIAN_POINTS,
    HOUSE_SERVICE_RULE_DANCER_POINTS,
    HOUSE_SERVICE_RULE_SENET_POINTS,
    HOUSE_SERVICE_RULE_ZOO_POINTS,
    HOUSE_SERVICE_RULE_ZOO_FILLS_ABOVE,
    HOUSE_SERVICE_RULE_ENTERTAINMENT_MAX,
    HOUSE_SERVICE_RULE_WATER_DESIRABILITY,
    HOUSE_SERVICE_RULE_ELEVATION_1_DESIRABILITY,
    HOUSE_SERVICE_RULE_ELEVATION_2_DESIRABILITY,
    HOUSE_SERVICE_RULE_ELEVATION_3_DESIRABILITY,
    HOUSE_SERVICE_RULE_ELEVATION_4_DESIRABILITY,
    HOUSE_SERVICE_RULE_ELEVATION_5_DESIRABILITY,
    HOUSE_SERVICE_RULE_COUNT,
};

enum house_tax_income_scale {
    HOUSE_TAX_INCOME_UNIT = 10000,
};

extern int32_t house_service_rules[HOUSE_SERVICE_RULE_COUNT];

int32_t building_house_tax_income(int32_t building_id);
void house_service_decay_houses_covered(void);
void house_service_decay_culture(void);
void house_service_calculate_culture_aggregates(void);
void building_update_desirability(void);
void house_service_decay_tax_collector(void);

#endif
