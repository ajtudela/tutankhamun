// The advisors window (advisors.c): the advisor ids, the image groups the advisor windows draw, the help message of
// each advisor and the steps of an advisor's opening animation.

#ifndef WINDOW_ADVISORS_H
#define WINDOW_ADVISORS_H

#include <stdbool.h>
#include <stdint.h>

struct config_name;

enum advisor {
    ADVISOR_NONE = 0,
    ADVISOR_LABOR = 1,
    ADVISOR_MILITARY = 2,
    ADVISOR_FLEET = 3,
    ADVISOR_KINGDOM = 4,
    ADVISOR_RATINGS = 5,
    ADVISOR_TRADE = 6,
    ADVISOR_POPULATION = 7,
    ADVISOR_HEALTH = 8,
    ADVISOR_EDUCATION = 9,
    ADVISOR_ENTERTAINMENT = 10,
    ADVISOR_RELIGION = 11,
    ADVISOR_FINANCIAL = 12,
    ADVISOR_CHIEF = 13,
    ADVISOR_MONUMENTS = 14,
    ADVISOR_COUNT = 15,
};

enum advisor_open_animation {
    ADVISOR_OPEN_LAST_STEP = 5,
    ADVISOR_OPEN_DONE = 6,
};

enum advisor_image_group {
    IMAGE_GROUP_ADVISOR_BACKGROUND = 11,
    IMAGE_GROUP_GOD_HAPPY_BOLT = 278,
    IMAGE_GROUP_LABOR_PRIORITY = 363,
    IMAGE_GROUP_GOD_ANGER_BOLT = 380,
    IMAGE_GROUP_FLEET_SHIP_ROW = 348,
    IMAGE_GROUP_ADVISOR_BAR = 428,
    IMAGE_GROUP_ADVISOR_BAR_BACKGROUND = 429,
    IMAGE_GROUP_TROOP_REQUEST_ICONS = 490,
};

extern int32_t advisor_help_messages[ADVISOR_COUNT];

void window_advisors_show_checked(void);
void city_victory_update_win_houses(void);
void window_advisors_init(void);
void window_advisors_draw_advisor(void);
void advisor_button_help(void);
void button_advisor_bar(void);
void sidebar_button_advisors(void);
bool tutorial_advisor_find_available(int32_t *advisor);
void window_advisors_draw_background(void);
void window_advisors_draw_foreground(void);

extern const struct config_name config_advisor_names[];

#endif
