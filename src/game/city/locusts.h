// The locust plague of the farms (city/locusts.c) and the countdown values at which the swarms eat and fly off.

#ifndef CITY_LOCUSTS_H
#define CITY_LOCUSTS_H

enum locust_plague {
    LOCUST_COUNTDOWN_RUN_OUT = -1,
    LOCUST_COUNTDOWN_AFTER_PLAGUE = 2632,
    LOCUST_COUNTDOWN_FLY_OFF = 1000,
    LOCUST_MONTHS_BEFORE_FLOOD = 2,
    LOCUSTS_PER_FARM = 9,
    LOCUST_SQUARE_SIDE = 3,
    LOCUST_MAX_START_DELAY = 300,
};

void city_locusts_update(void);

#endif
