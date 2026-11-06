// The route candidate list a walker or a cart picks its closest destination from: the count, the tiles and the
// building ids.

#ifndef BUILDING_ROUTE_CANDIDATE_TYPES_H
#define BUILDING_ROUTE_CANDIDATE_TYPES_H

#include <stdint.h>

enum {
    ROUTE_CANDIDATE_TILES = 4000,
    ROUTE_CANDIDATE_BUILDINGS = 4001,
};

typedef struct route_candidate_lists {
    int32_t count;
    int32_t tiles[ROUTE_CANDIDATE_TILES];
    int32_t buildings[ROUTE_CANDIDATE_BUILDINGS];
} route_candidate_lists;

#endif
