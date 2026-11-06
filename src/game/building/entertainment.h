// The shows of the entertainment venues.

#ifndef BUILDING_ENTERTAINMENT_H
#define BUILDING_ENTERTAINMENT_H

#include <stdint.h>

enum venue_show {
    SHOW_JUGGLERS = 0,
    SHOW_MUSICIANS = 1,
    SHOW_DANCERS = 2,
};

enum venue_show_length {
    VENUE_SHOW_DAYS = 32,
};

void figure_entertainer_update_shows(int32_t figure_id);

#endif
