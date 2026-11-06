// City labor (city/labor.c): the categories and priorities, the labor steps of the tick and the mothballed check.

#ifndef CITY_LABOR_H
#define CITY_LABOR_H

#include <stdint.h>

enum labor_category_id {
    LABOR_CATEGORY_NONE = -1,
    LABOR_CATEGORY_FOOD_PRODUCTION = 0,
    LABOR_CATEGORY_INDUSTRY_COMMERCE = 1,
    LABOR_CATEGORY_ENTERTAINMENT = 2,
    LABOR_CATEGORY_RELIGION = 3,
    LABOR_CATEGORY_EDUCATION = 4,
    LABOR_CATEGORY_WATER_HEALTH = 5,
    LABOR_CATEGORY_INFRASTRUCTURE = 6,
    LABOR_CATEGORY_GOVERNMENT = 7,
    LABOR_CATEGORY_MILITARY = 8,
    LABOR_CATEGORY_CULTURE = 9,
    LABOR_CATEGORY_COUNT = 10,
};

enum labor_priority {
    LABOR_PRIORITY_CATEGORIES = 9,
    LABOR_PRIORITY_MAX = 9,
};

int32_t city_labor_industry_is_mothballed(int32_t building_id);
void city_labor_calculate_workers_needed_per_category(void);
void city_labor_allocate_workers_to_categories(void);
void city_labor_check_employment(void);
void city_labor_set_building_worker_weight(void);
void city_labor_allocate_workers_to_buildings(void);

#endif
