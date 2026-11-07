// An empire trade route on the map: the distance between its path points, the slot of its drawn path on the heap
// (rebuilt from the way points after every load and save, saved as 0; the original kept the pointer there), its way points, the map objects at its ends, its kind and
// whether it is in use.

#ifndef EMPIRE_EMPIRE_ROUTE_H
#define EMPIRE_EMPIRE_ROUTE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum empire_route_limit {
    EMPIRE_ROUTES = 50,
    EMPIRE_ROUTE_POINTS = 50,
};

typedef struct empire_route_point {
    int16_t x;
    int16_t y;
    uint8_t segment;
    uint8_t pad;
} empire_route_point;

typedef struct empire_route {
    uint8_t step;
    uint8_t reserved_1[3];
    int32_t path_slot;
    empire_route_point points[EMPIRE_ROUTE_POINTS];
    int32_t path_length;
    int32_t path_capacity;
    int16_t first_object;
    int16_t last_object;
    uint8_t kind;
    uint8_t num_points;
    uint8_t in_use;
    uint8_t reserved_2;
} empire_route;

static_assert(sizeof(empire_route) == 0x144, "empire_route is a savegame record of 324 bytes");
static_assert(offsetof(empire_route, path_length) == 0x134 && offsetof(empire_route, kind) == 0x140 && offsetof(empire_route, in_use) == 0x142,
              "empire_route is the savegame's layout");

empire_route_point *empire_route_path(const empire_route *route);
void empire_route_path_set(empire_route *route, empire_route_point *path);

#endif
