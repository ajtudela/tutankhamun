// The texts of the military advisor's group, which the military and fleet advisors draw, and the functions of
// military.c that other files call.

#ifndef WINDOW_ADVISOR_MILITARY_H
#define WINDOW_ADVISOR_MILITARY_H

#include <stdint.h>

enum military_text {
    MILITARY_TEXT_TITLE = 0,
    MILITARY_TEXT_GO_TO = 1,
    MILITARY_TEXT_GO_TO_COMPANY = 2,
    MILITARY_TEXT_RETURN = 3,
    MILITARY_TEXT_RETURN_TO_FORT = 4,
    MILITARY_TEXT_KINGDOM = 5,
    MILITARY_TEXT_KINGDOM_SERVICE = 6,
    MILITARY_TEXT_IN = 7,
    MILITARY_TEXT_NO_THREATS = 8,
    MILITARY_TEXT_ENEMIES_APPROACHING = 9,
    MILITARY_TEXT_ENEMIES_IN_SIGHT = 10,
    MILITARY_TEXT_KINGDOM_TROOPS_AT_GATES = 11,
    MILITARY_TEXT_NO_REQUESTS_ABROAD = 12,
    MILITARY_TEXT_TROOPS_NEEDED_ABROAD = 13,
    MILITARY_TEXT_MARCHING_TO_FIGHT = 14,
    MILITARY_TEXT_MARCHING_BACK = 15,
    MILITARY_TEXT_NO_COMPANIES = 16,
    MILITARY_TEXT_COMPANY = 17,
    MILITARY_TEXT_EXPERIENCE = 18,
    MILITARY_TEXT_NAVY_STATUS = 19,
    MILITARY_TEXT_WARSHIP = 20,
    MILITARY_TEXT_TO_WHARF = 21,
    MILITARY_TEXT_HULL = 22,
    MILITARY_TEXT_STRENGTH = 23,
    MILITARY_TEXT_CREW_STATUS = 24,
    MILITARY_TEXT_NO_WARSHIPS = 25,
    MILITARY_TEXT_NAVY_NEEDED = 26,
    MILITARY_TEXT_SAILING_TO_FIGHT = 27,
    MILITARY_TEXT_SAILING_BACK = 28,
    MILITARY_TEXT_ABROAD = 29,
    MILITARY_TEXT_DISPATCH = 30,
    MILITARY_TEXT_DISPATCH_NOW = 31,
    MILITARY_TEXT_AND = 32,
};

void button_military_send_help(void);
void advisor_military_count_ships(void);
void window_advisor_military_draw_background(void);
void window_advisor_military_draw_foreground(void);
void button_fleet_send_help(void);
void button_military_go_to_legion(void);
void button_military_return_to_fort(void);
void button_military_empire_service(void);
int32_t formation_get_num_legions(void);

#endif
