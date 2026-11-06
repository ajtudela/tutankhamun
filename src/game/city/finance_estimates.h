// City finance and rating helpers outside the tick (city/finance_estimates.c): this year's estimates and totals, the
// construction cost, the palace check, the advisors' causes and the gift to the pharaoh.

#ifndef CITY_FINANCE_ESTIMATES_H
#define CITY_FINANCE_ESTIMATES_H

#include <stdbool.h>
#include <stdint.h>

enum gift_size {
    GIFT_MODEST = 0,
    GIFT_GENEROUS = 1,
    GIFT_LAVISH = 2,
    GIFT_SIZES = 3,
};

enum no_immigration_cause {
    NO_IMMIGRATION_LOW_WAGES = 0,
    NO_IMMIGRATION_NO_JOBS = 1,
    NO_IMMIGRATION_NO_FOOD = 2,
    NO_IMMIGRATION_HIGH_TAXES = 3,
    NO_IMMIGRATION_FOOD_FACTOR = 4,
    NO_IMMIGRATION_LOW_MOOD = 5,
};

enum health_demand {
    HEALTH_DEMAND_NONE = 0,
    HEALTH_DEMAND_COURTHOUSE = 1,
    HEALTH_DEMAND_DENTIST = 2,
    HEALTH_DEMAND_HEALTH = 3,
    HEALTH_DEMAND_COURTHOUSE_NONE = 4,
};

enum education_demand {
    EDUCATION_DEMAND_NONE = 0,
    EDUCATION_DEMAND_MORE = 1,
    EDUCATION_DEMAND_SOME = 2,
    EDUCATION_DEMAND_BOTH = 3,
};

enum entertainment_demand {
    ENTERTAINMENT_DEMAND_NONE = 0,
    ENTERTAINMENT_DEMAND_SOME = 1,
    ENTERTAINMENT_DEMAND_MORE = 2,
};

enum religion_demand {
    RELIGION_DEMAND_NONE = 0,
    RELIGION_DEMAND_FIRST_GOD = 1,
    RELIGION_DEMAND_SECOND_GOD = 2,
    RELIGION_DEMAND_THIRD_GOD = 3,
};

void city_finance_estimate_taxes(void);
void city_finance_estimate_wages(void);
void city_finance_estimate_interest(void);
void city_finance_estimate_salary(void);
void city_finance_calculate_totals(void);
void city_ratings_store_culture_lowest(void);
void city_ratings_update_culture_advisor(void);
void city_ratings_reduce_prosperity_after_bailout(void);
bool city_buildings_has_palace(void);
void city_finance_process_construction(int32_t player_id, int32_t amount);
void city_ratings_advisor_reasons(void);
void city_kingdom_send_gift_to_pharaoh(void);

#endif
