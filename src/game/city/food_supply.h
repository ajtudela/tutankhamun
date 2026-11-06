// The food supply percentage (city/food_supply.c) and the farm check it uses.

#ifndef CITY_FOOD_SUPPLY_H
#define CITY_FOOD_SUPPLY_H

#include <stdbool.h>
#include <stdint.h>

bool building_farm_counts_for_food(int32_t building_id);
int32_t city_resource_food_supply_percentage(void);

#endif
