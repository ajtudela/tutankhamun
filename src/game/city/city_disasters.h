// The disasters of the gods' curses and of the scenario events (city/city_disasters.c), with their tunable values.

#ifndef CITY_CITY_DISASTERS_H
#define CITY_CITY_DISASTERS_H

#include <stdbool.h>
#include <stdint.h>

enum disaster_effects {
    DISASTER_HAPPINESS_LOSS = 15,
    DISASTER_TICKS_PER_AMOUNT = 816,
};

enum plague_rules {
    PLAGUE_FIGURE_WAIT_TICKS = 500,
};

enum hailstorm_rules {
    HAILSTORM_COUNTDOWN = 408,
    HAILSTORM_ACTION_CREATED = 8,
    HAILSTORM_CHANCE_CREATED = 25,
    HAILSTORM_CHANCE_FIGHTER = 50,
    HAILSTORM_CHANCE_OTHER = 75,
};

enum frog_rules {
    FROG_HOUSE_CHANCE = 50,
    FROGS_PEOPLE_PERCENT = 65,
    FROG_MAX_START_DELAY = 150,
};

enum blood_river_rules {
    BLOOD_RIVER_MIN_AMOUNT = 3,
    BLOOD_RIVER_MAX_AMOUNT = 9,
    BLOOD_RIVER_RISK = 5000,
    BLOOD_RIVER_WATER_SUPPLY_PER_DISTANCE = 3,
};

enum mummy_rules {
    MUMMY_ROAD_RADIUS = 6,
};

void city_health_change(int32_t amount);
void city_locusts_start(void);
void figure_sink_all_ships(void);
int32_t city_health_find_plague_house(void);
bool city_health_start_plague_at_house(int32_t building_id);
bool city_health_start_plague(bool random_house);
void city_hailstorm_start(void);
int32_t city_frogs_find_water(int32_t building_id, int32_t *x, int32_t *y);
void city_frogs_create(int32_t building_id, int32_t x, int32_t y);
void city_frogs_infest_house(int32_t building_id);
void city_frogs_start(int32_t amount);
void city_blood_river_start(int32_t amount);
int32_t building_destroy_first_of_type(int32_t type);
bool city_mummies_is_source_building(int32_t building_id);
void city_mummies_start(int32_t amount);

#endif
