// The service coverage of figure/service.c: the coverage of a figure on its tile, the provider of each service, the
// tunable service_rules and the Bast complex upgrade the priests test.

#ifndef FIGURE_SERVICE_H
#define FIGURE_SERVICE_H

#include <stdint.h>

enum service_rule {
    SERVICE_RULE_RADIUS,
    SERVICE_RULE_MISSIONARY_RADIUS,
    SERVICE_RULE_HOUSE_ACCESS,
    SERVICE_RULE_HOUSE_TAX_ACCESS,
    SERVICE_RULE_RISK_SEEN_START,
    SERVICE_RULE_CRIME_RISK_MAX,
    SERVICE_RULE_HOUSES_COVERED_MAX,
    SERVICE_RULE_SEEN_FADE,
    SERVICE_RULE_MARKET_DEMAND,
    SERVICE_RULE_FOOD_PER_PERSON,
    SERVICE_RULE_FOOD_STOCK_PER_PERSON,
    SERVICE_RULE_LARGE_HOUSE_GOODS_FACTOR,
    SERVICE_RULE_GOODS_STOCK_FACTOR,
    SERVICE_RULE_COUNT,
};

enum service_temple_complex {
    SERVICE_BAST_COMPLEX_UPGRADE = 10,
};

extern int32_t service_rules[SERVICE_RULE_COUNT];

int32_t figure_service_provide_coverage(int32_t figure_id);
int32_t service_provide_labor_seeker_coverage(int32_t x, int32_t y);
int32_t service_provide_tax_collector_coverage(int32_t x, int32_t y);
int32_t service_provide_market_goods(int32_t market_id, int32_t x, int32_t y);
int32_t service_provide_water_coverage(int32_t x, int32_t y);
int32_t service_provide_physician_coverage(int32_t x, int32_t y);
void service_provide_physician_health(int32_t x, int32_t y, int32_t amount);
int32_t service_provide_school_coverage(int32_t x, int32_t y);
int32_t service_provide_library_coverage(int32_t x, int32_t y);
int32_t service_provide_dentist_coverage(int32_t x, int32_t y);
int32_t service_provide_apothecary_coverage(int32_t x, int32_t y);
int32_t service_provide_apothecary_health(int32_t x, int32_t y, int32_t amount);
int32_t service_provide_mortuary_coverage(int32_t x, int32_t y);
int32_t service_provide_missionary_coverage(int32_t x, int32_t y);
int32_t service_provide_religion_coverage(int32_t x, int32_t y, int32_t god);
int32_t service_provide_bast_complex_coverage(int32_t x, int32_t y);
int32_t service_provide_engineer_coverage(int32_t x, int32_t y);
int32_t service_provide_fireman_coverage(int32_t x, int32_t y);
void service_provide_crime_coverage(int32_t x, int32_t y, int32_t reduction);
void service_provide_magistrate_coverage(int32_t x, int32_t y);
int32_t service_provide_juggler_coverage(int32_t x, int32_t y);
int32_t service_provide_musician_coverage(int32_t x, int32_t y);
int32_t service_provide_dancer_coverage(int32_t x, int32_t y);
int32_t service_provide_senet_coverage(int32_t x, int32_t y);
int32_t service_provide_zoo_coverage(int32_t x, int32_t y);

#endif
