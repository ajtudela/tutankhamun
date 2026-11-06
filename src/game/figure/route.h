// The functions of figure/route.c that other files call.

#ifndef FIGURE_ROUTE_H
#define FIGURE_ROUTE_H

#include <stdint.h>

enum route_search_limit {
    ROUTE_ENEMY_MAX_TILES = 100000,
};

enum route_directions {
    ROUTE_NOT_FOUND = 0,
    ROUTE_FOUR_DIRECTIONS_FIRST = 4,
    ROUTE_EIGHT_DIRECTIONS = 8,
};

void figure_route_add(int32_t figure_id);
void figure_route_clear_all(void);
void figure_route_clean(void);
void figure_route_remove(int32_t figure_id);

#endif
