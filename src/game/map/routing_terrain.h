// Rebuilds of the routing grids (map/routing_terrain.c), the ferry lanes, and the values of the citizen, non-citizen, wall and water
// grids.
//
// A water lift's citizen value follows the parity of its orientation minus the view rotation, so a grid rebuilt in a view
// turned a quarter connects the canal across the lift instead of along it.

#ifndef MAP_ROUTING_TERRAIN_H
#define MAP_ROUTING_TERRAIN_H

#include <stdbool.h>
#include <stdint.h>

enum routing_citizen_tile {
    ROUTING_CITIZEN_WATER_LIFT_EVEN = -6,
    ROUTING_CITIZEN_CANAL_ON_FLOODPLAIN = -5,
    ROUTING_CITIZEN_WATER_LIFT_ODD = -4,
    ROUTING_CITIZEN_AQUEDUCT = -3,
    ROUTING_CITIZEN_EXTRA_COST = -2,
    ROUTING_CITIZEN_BLOCKED = -1,
    ROUTING_CITIZEN_ROAD = 0,
    ROUTING_CITIZEN_FERRY_STAFFED = 1,
    ROUTING_CITIZEN_PASSABLE = 2,
    ROUTING_CITIZEN_FERRY_UNSTAFFED = 3,
    ROUTING_CITIZEN_CLEAR = 4,
    ROUTING_CITIZEN_MONUMENT = 5,
    ROUTING_CITIZEN_FORT_GROUND = 6,
};

enum routing_water_tile {
    ROUTING_WATER_LAND = -4,
    ROUTING_WATER_LOW_BRIDGE = -3,
    ROUTING_WATER_MAP_EDGE = -2,
    ROUTING_WATER_BLOCKED = -1,
    ROUTING_WATER_OPEN = 0,
};

enum routing_noncitizen_tile {
    ROUTING_NONCITIZEN_BLOCKED = -1,
    ROUTING_NONCITIZEN_PASSABLE = 0,
    ROUTING_NONCITIZEN_BUILDING = 1,
    ROUTING_NONCITIZEN_ACCESS_RAMP = 2,
    ROUTING_NONCITIZEN_WALL = 3,
    ROUTING_NONCITIZEN_GATEHOUSE = 4,
    ROUTING_NONCITIZEN_FORT = 5,
};

enum routing_wall_tile {
    ROUTING_WALL_BLOCKED = -1,
    ROUTING_WALL_PASSABLE = 0,
};

enum {
    ROUTING_WALL_ACCEPTED_BELOW = 3,
};

enum ferry_route_mark {
    FERRY_ROUTE_REMOVE = 0,
    FERRY_ROUTE_ADD = 1,
};

enum ferry_route_state {
    FERRY_ROUTE_OPEN = 1,
    FERRY_ROUTE_CLOSED = 3,
};

enum ferry_route_directions {
    FERRY_ROUTE_DIRECTIONS = 1000,
    FERRY_ROUTE_DIRECTIONS_CLEARED = 250,
};

void map_routing_update_water_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_routing_update_water(void);
void map_routing_update_walls(void);
void map_routing_update_land_noncitizen_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_routing_update_land_noncitizen(void);
void map_routing_update_land_citizen_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t rebuild_ferries);
void map_routing_update_land_citizen(void);
void map_routing_set_ferry_route(int32_t building_id, bool enable, int32_t type);

#endif
