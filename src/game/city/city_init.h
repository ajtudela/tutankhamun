// The city data of a new scenario (city/city_init.c): the player records, the gods, the messages and the start values
// of a new city (new_city_rules).

#ifndef CITY_CITY_INIT_H
#define CITY_CITY_INIT_H

#include <stdint.h>

enum city_player_slot {
    CITY_PLAYER_HUMAN = 1,
};

enum god_start_values {
    GOD_MOOD_NEUTRAL = 50,
    GOD_EFFECT_NONE = -1,
};

enum new_city_rule {
    NEW_CITY_RULE_HEALTH,
    NEW_CITY_RULE_SENTIMENT,
    NEW_CITY_RULE_KINGDOM_WAGES,
    NEW_CITY_RULE_WAGES,
    NEW_CITY_RULE_TAX_PERCENTAGE,
    NEW_CITY_RULE_COUNT,
};

extern int32_t new_city_rules[NEW_CITY_RULE_COUNT];

enum population_shown_clear {
    POPULATION_SHOWN_FIRST_CLEAR = 8,
};

void city_gods_init(void);
void city_data_init(void);
void city_message_init_scenario(void);

#endif
