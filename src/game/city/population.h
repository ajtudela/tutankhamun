// City population (city/population.c, city/population_extra.c): the census, the homeless, the yearly update, the
// population history and the gift costs (gift_cost_rules).

#ifndef CITY_POPULATION_H
#define CITY_POPULATION_H

#include "city/finance_estimates.h"

#include <stdint.h>

typedef struct {
    int32_t savings_divisor;
    int32_t base_cost;
} gift_cost_rule;

extern gift_cost_rule gift_cost_rules[GIFT_SIZES];

enum population_history_size {
    CENSUS_AGES = 100,
    MONTHLY_POPULATION_ENTRIES = 2400,
};

enum census {
    CENSUS_DECENNIA = 10,
    AGES_PER_DECENNIUM = 10,
    OLDEST_AGE = CENSUS_AGES - 1,
};

void city_population_remove_from_census(int32_t player_id, int32_t count);
void city_population_add_to_census(int32_t player_id, int32_t count);
void city_population_add(int32_t player_id, int32_t amount);
void city_population_add_homeless(int32_t player_id, int32_t count);
void city_population_remove_homeless(int32_t player_id, int32_t count);
int32_t city_population_people_in_age_decennium(int32_t player_id, int32_t decennium);
void city_population_remove_from_census_in_decennium(int32_t player_id, int32_t decennium, int32_t count);
int32_t city_population_people_of_working_age(void);
int32_t city_population_request_yearly_update(void);
void city_population_clear_player2_capacity(void);
void city_population_yearly_advance_ages_and_deaths(void);
void city_population_yearly_calculate_births(void);
void city_population_yearly_recalculate(int32_t player_id);
void city_population_record_monthly(void);
void city_population_remove_home_removed(int32_t player_id, int32_t amount);
int32_t city_population_at_month(int32_t months_back, int32_t offset);
void city_population_graph_axis(int32_t months);
void city_health_plague_spread(int32_t x, int32_t y);
void city_kingdom_gift_costs_update(void);
int32_t city_message_get_text_id(int32_t type);

#endif
