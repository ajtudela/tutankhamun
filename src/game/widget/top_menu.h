// The top strip and the texts of the top menu's text group it and the city HUD draw.

#ifndef WIDGET_TOP_MENU_H
#define WIDGET_TOP_MENU_H

#include <stdint.h>

enum top_menu_text {
    TOP_MENU_TEXT_FUNDS = 0,
    TOP_MENU_TEXT_POPULATION = 1,
    TOP_MENU_TEXT_MONTHS_TO_COMPLETE_MISSION = 2,
    TOP_MENU_TEXT_MONTHS_UNTIL_VICTORY = 3,
    TOP_MENU_TEXT_MISSION_ALREADY_WON = 5,
    TOP_MENU_TEXT_PLACE_TOMBS_OVER_CLIFFS = 6,
};

void widget_top_menu_draw(int32_t force);
void city_top_menu_hover_update(void);

#endif
