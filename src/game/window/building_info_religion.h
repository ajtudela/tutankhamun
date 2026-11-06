// The religious panels of the building info window (building_info_religion.c) and the month thresholds of the festival
// square's texts (festival_square_rules).

#ifndef WINDOW_BUILDING_INFO_RELIGION_H
#define WINDOW_BUILDING_INFO_RELIGION_H

#include <stdint.h>

enum festival_square_rule {
    FESTIVAL_SQUARE_RULE_MONTHS_FEW,
    FESTIVAL_SQUARE_RULE_MONTHS_QUITE_SOME_TIME,
    FESTIVAL_SQUARE_RULE_MONTHS_OVER_A_YEAR,
    FESTIVAL_SQUARE_RULE_MONTHS_ALMOST_TWO_YEARS,
    FESTIVAL_SQUARE_RULE_MONTHS_OVER_TWO_YEARS,
    FESTIVAL_SQUARE_RULE_MONTHS_GOOD_OLD_DAYS,
    FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ORDER_NOW,
    FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ABOUT_TIME,
    FESTIVAL_SQUARE_RULE_COUNT,
};

extern int32_t festival_square_rules[FESTIVAL_SQUARE_RULE_COUNT];

void window_building_draw_oracle(void);
void window_building_draw_temple(int32_t god);
void window_building_draw_festival_square(void);
void window_building_festival_button(void);
void window_hold_festival_draw_background(void);
void window_hold_festival_draw_foreground(void);

#endif
