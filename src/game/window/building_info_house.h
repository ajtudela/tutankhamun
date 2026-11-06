// The house panels of the building info window (building_info_house.c) and the sentiment thresholds of the crime
// texts (house_crime_text_rules).

#ifndef WINDOW_BUILDING_INFO_HOUSE_H
#define WINDOW_BUILDING_INFO_HOUSE_H

#include <stdint.h>

enum house_crime_text_rule {
    HOUSE_CRIME_TEXT_RULE_NO_CRIME,
    HOUSE_CRIME_TEXT_RULE_PEACEFUL,
    HOUSE_CRIME_TEXT_RULE_SOME_CRIME,
    HOUSE_CRIME_TEXT_RULE_BECOMING_A_PROBLEM,
    HOUSE_CRIME_TEXT_RULE_HIGH_CRIME,
    HOUSE_CRIME_TEXT_RULE_LAWLESS,
    HOUSE_CRIME_TEXT_RULE_COUNT,
};

extern int32_t house_crime_text_rules[HOUSE_CRIME_TEXT_RULE_COUNT];

void window_building_draw_house(void);
void window_building_draw_house_vacant_lot(void);
void window_building_draw_house_population(int32_t y);
void window_building_draw_house_taxes(int32_t y);
void window_building_draw_house_crime(int32_t y);
void window_building_house_find_worst_desirability(void);
int32_t window_building_house_trade_goods(int32_t *counts, int32_t imports);

#endif
