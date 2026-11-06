// Warnings of the build tool (building/construction_warnings.c) and their rules: the population the food and job
// warnings start from, the workers and unemployment, the food share and the road access warning limit.

#ifndef BUILDING_CONSTRUCTION_WARNINGS_H
#define BUILDING_CONSTRUCTION_WARNINGS_H

#include <stdint.h>

enum construction_warning_rule {
    CONSTRUCTION_WARNING_RULE_MIN_POPULATION,
    CONSTRUCTION_WARNING_RULE_MIN_WORKERS_NEEDED,
    CONSTRUCTION_WARNING_RULE_MAX_UNEMPLOYMENT_PERCENT,
    CONSTRUCTION_WARNING_RULE_MIN_FOOD_PRODUCED_PERCENT,
    CONSTRUCTION_WARNING_RULE_MAX_PROBLEM_MESSAGES,
    CONSTRUCTION_WARNING_RULE_COUNT,
};

extern int32_t construction_warning_rules[CONSTRUCTION_WARNING_RULE_COUNT];

void construction_warning_check_house_food(int32_t type);
void construction_warning_check_unemployment(int32_t type);
void construction_warning_check_labor_need(int32_t type);
void construction_warning_check_venue_schools(int32_t type);
void construction_warning_check_workers(int32_t type);
void construction_warning_check_well(int32_t type, int32_t x, int32_t y);
void construction_warning_check_tower(int32_t type, int32_t x, int32_t y, int32_t size);
void construction_warning_check_road_access(int32_t type, int32_t x, int32_t y, int32_t size);
void construction_warning_check_granary(int32_t type);
void construction_warning_check_booth(int32_t type);
void construction_warning_check_bandstand(int32_t type);
void construction_warning_check_pavilion(int32_t type);
void construction_warning_check_fort(int32_t type);
void construction_warning_check_military(int32_t type);
void construction_warning_check_recruiter(int32_t type);
void construction_warning_check_brewery(int32_t type);
void construction_warning_check_weaver(int32_t type);
void construction_warning_check_jeweler(int32_t type);
void construction_warning_check_potter(int32_t type);
void construction_warning_check_papyrus(int32_t type);
void construction_warning_check_bricks(int32_t type);
void construction_warning_check_lamps(int32_t type);
void construction_warning_check_paint(int32_t type);
void construction_warning_check_cattle(int32_t type);
void construction_warning_check_senet(int32_t type);
void construction_warning_check_zoo(int32_t type);
void construction_warning_check_papyrus_users(int32_t type);
void construction_warning_check_mortuary(int32_t type);
void construction_warning_check_carpenters(int32_t type);
void construction_warning_check_artisans(int32_t type);
void construction_warning_check_pyramid(int32_t type);
void construction_warning_check_granite(int32_t type);
void construction_warning_check_ptolemaic(int32_t type);

#endif
