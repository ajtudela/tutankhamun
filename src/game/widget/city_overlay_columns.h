// The overlay columns, one per overlay, called by the overlay drawing's top pass.

#ifndef WIDGET_CITY_OVERLAY_COLUMNS_H
#define WIDGET_CITY_OVERLAY_COLUMNS_H

#include <stdint.h>

void city_overlay_column_fire(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_damage(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_disease(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_malaria(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_crime(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_entertainment(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_education(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_booth(int32_t b, int32_t x, int32_t y);
void city_overlay_column_bandstand(int32_t b, int32_t x, int32_t y);
void city_overlay_column_pavilion(int32_t b, int32_t x, int32_t y);
void city_overlay_column_senet(int32_t b, int32_t x, int32_t y);
void city_overlay_column_zoo(int32_t b, int32_t x, int32_t y);
void city_overlay_column_bazaar_access(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_water(int32_t b, int32_t x, int32_t y);
void city_overlay_column_religion(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_school(int32_t b, int32_t x, int32_t y);
void city_overlay_column_library(int32_t b, int32_t x, int32_t y);
void city_overlay_column_dentist(int32_t b, int32_t x, int32_t y);
void city_overlay_column_physician(int32_t b, int32_t x, int32_t y);
void city_overlay_column_apothecary(int32_t b, int32_t x, int32_t y);
void city_overlay_column_mortuary(int32_t b, int32_t x, int32_t y);
void city_overlay_column_courthouse(int32_t b, int32_t x, int32_t y);
void city_overlay_column_tax_income(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_problems(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_infected_housing(int32_t building_id, int32_t x, int32_t y);
void city_overlay_column_labor(int32_t building_id, int32_t x, int32_t y);

#endif
