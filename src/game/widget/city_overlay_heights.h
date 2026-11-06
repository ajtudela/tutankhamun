// The functions of city_overlay_heights.c that other files call.

#ifndef WIDGET_CITY_OVERLAY_HEIGHTS_H
#define WIDGET_CITY_OVERLAY_HEIGHTS_H

#include <stdint.h>

int32_t city_overlay_farm_hidden(int32_t building_id);
int32_t city_overlay_height_fertility(int32_t grid_offset);
int32_t city_overlay_height_desirability(int32_t grid_offset);
int32_t city_overlay_resource_level(int32_t building_id, int32_t resource, int32_t slot);
int32_t city_overlay_height_native(int32_t building_id);
int32_t city_overlay_height_fire(int32_t building_id);
int32_t city_overlay_height_damage_terrain(int32_t grid_offset);
int32_t city_overlay_height_damage(int32_t building_id);
int32_t city_overlay_height_disease(int32_t building_id);
int32_t city_overlay_height_malaria(int32_t building_id);
int32_t city_overlay_height_crime(int32_t building_id);
int32_t city_overlay_height_entertainment(int32_t building_id);
int32_t city_overlay_height_education(int32_t building_id);
int32_t city_overlay_height_booth(int32_t building_id);
int32_t city_overlay_height_bandstand(int32_t building_id);
int32_t city_overlay_height_pavilion(int32_t building_id);
int32_t city_overlay_height_senet(int32_t building_id);
int32_t city_overlay_height_bazaar_access(int32_t building_id);
int32_t city_overlay_height_water(int32_t building_id);
int32_t city_overlay_height_religion(int32_t building_id);
int32_t city_overlay_height_school(int32_t building_id);
int32_t city_overlay_height_library(int32_t building_id);
int32_t city_overlay_height_dentist(int32_t building_id);
int32_t city_overlay_height_physician(int32_t building_id);
int32_t city_overlay_height_apothecary(int32_t building_id);
int32_t city_overlay_height_mortuary(int32_t building_id);
int32_t city_overlay_height_courthouse(int32_t building_id);
int32_t city_overlay_height_tax_income(int32_t building_id);
int32_t city_overlay_height_problems(int32_t building_id);
int32_t city_overlay_height_labor(int32_t building_id);
int32_t city_overlay_height_city_defenses(int32_t building_id);

#endif
