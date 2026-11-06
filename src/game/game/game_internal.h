// Shared by the game tick and time modules: the calendar (51 ticks a day, 16 days a month, 12 months a year), the
// "ticks to run" result of the speed gate and the time function type of the timer import.

#ifndef GAME_GAME_INTERNAL_H
#define GAME_GAME_INTERNAL_H

#include "city/city_player.h"
#include "city/gods.h"

#include <stdint.h>

enum {
    TICKS_PER_DAY = 51,
    DAYS_PER_MONTH = 16,
    MONTHS_PER_YEAR = 12,
    GAME_TIME_START_YEAR = 2098,
};

enum {
    GAME_ELAPSED_TICKS_RUN = 10000,
};

#endif
