// The kingdom side of the city (city/kingdom.c): the bailout and the kingdom army, the festivals, the victory and
// defeat checks, and the copy of the second player's flag.

#ifndef CITY_KINGDOM_H
#define CITY_KINGDOM_H

#include <stdbool.h>
#include <stdint.h>

enum festival_size {
    FESTIVAL_SMALL = 1,
    FESTIVAL_LARGE = 2,
    FESTIVAL_GRAND = 3,
    FESTIVAL_ALL_GODS = 4,
};

enum victory_state {
    VICTORY_STATE_LOST = -1,
    VICTORY_STATE_NONE = 0,
    VICTORY_STATE_WON = 1,
};

enum festival_celebrants {
    CELEBRANTS_PER_FESTIVAL_SIZE = 10,
};

enum burial_goal {
    BURIAL_GOAL_RESOURCES = 30,
    BURIAL_GOAL_MONUMENT_SLOTS = 3,
};

enum time_limit_extra_years {
    EXTRA_YEARS_VERY_EASY = 7,
    EXTRA_YEARS_EASY = 2,
};

enum lost_population_rule {
    LOST_POPULATION_EXTRA_EMIGRANTS = 2,
    LOST_POPULATION_FRACTION_OF_HIGHEST = 4,
};

void city_emperor_update(void);
void city_festival_update(void);
void city_victory_update_months_to_govern(void);
int32_t city_victory_count_houses_of_level(int32_t min_level);
bool city_victory_goals_reached(void);
void city_victory_check(void);

#endif
