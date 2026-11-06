// House population (building/house_population.c): the room of the houses, immigrants, emigrants and the homeless,
// eviction from overcrowded houses, the yearly births and deaths and the working people per house level.

#ifndef BUILDING_HOUSE_POPULATION_H
#define BUILDING_HOUSE_POPULATION_H

#include "building/building.h"
#include <stdint.h>

int32_t house_population_max_people(const building *b);
void house_population_fill_building_list_with_houses(void);
void house_population_update_room(void);
void house_population_update_migration(void);
void house_population_calculate_working_people(void);
void house_population_create_immigrants(int32_t num_people);
void house_population_create_emigrants(int32_t num_people);
int32_t house_population_closest_house_with_room(int32_t x, int32_t y);
int32_t house_population_add_to_city(int32_t player, int32_t num_people);
int32_t house_population_remove_from_city(int32_t player, int32_t num_people);
int32_t house_population_calculate_people_per_type(int32_t player);
void house_population_evict_overcrowded(void);

void figure_create_immigrant(int32_t building_id, int32_t num_people);
void figure_create_emigrant(int32_t building_id, int32_t num_people);
void figure_create_homeless(int32_t player, int32_t x, int32_t y, int32_t num_people);

#endif
