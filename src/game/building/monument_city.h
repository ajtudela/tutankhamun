// Scenario and city monument queries (building/monument_city.c).

#ifndef BUILDING_MONUMENT_CITY_H
#define BUILDING_MONUMENT_CITY_H

#include <stdint.h>

int32_t city_victory_score(int32_t months);
int32_t scenario_get_mausoleum_size(void);
int32_t city_has_completed_monument(void);
int32_t quarry_image_group(int32_t resource);
int32_t scenario_count_monuments(void);
int32_t city_count_monuments(void);
int32_t monument_load_extra_scripts(void);

#endif
