// City helpers outside the tick (city/population_extra.c): plague spreading, the population graph axis and the gift
// costs.

#ifndef CITY_POPULATION_EXTRA_H
#define CITY_POPULATION_EXTRA_H

#include <stdint.h>

void city_health_plague_spread(int32_t x, int32_t y);
void city_population_graph_axis(int32_t months);
void city_kingdom_gift_costs_update(void);

#endif
