// The land searches for the remaining travel modes of a figure's route (any, fort, no ferry, road and garden, animals,
// amphibia, anywhere) and the multi-target searches for the closest of several destinations, including the
// prefer-roads one that seeds every target.
//
// Where these searches are uneven:
// - the animal search asks the fighting check only for the left neighbour;
// - the multi-target searches look up the targets from the last, so with duplicate offsets the highest index wins;
// - in the prefer-roads search a target reached from the left does not get the new distance before the return.

#include "map/routing_land.h"

#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"

static int32_t is_land_for_any_mode(int8_t value, int32_t offset)
{
    return value >= ROUTING_CITIZEN_ROAD || value == ROUTING_CITIZEN_AQUEDUCT || (value == ROUTING_CITIZEN_BLOCKED && (map_grids.terrain[offset] & TERRAIN_RUBBLE));
}

static int32_t is_ferry_lane(int8_t value)
{
    return value == ROUTING_CITIZEN_FERRY_STAFFED || value == ROUTING_CITIZEN_FERRY_UNSTAFFED;
}

static int32_t is_below_monument_or_fort(int8_t value)
{
    return value < ROUTING_CITIZEN_MONUMENT || value == ROUTING_CITIZEN_FORT_GROUND;
}

static int32_t can_enter_prefer_roads(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return is_land_for_any_mode(value, next) && value < ROUTING_CITIZEN_MONUMENT && value != ROUTING_CITIZEN_FERRY_UNSTAFFED;
}

static int32_t can_enter_animal(int32_t next, int32_t current)
{
    if (next == current - 1 && !map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return (value >= ROUTING_CITIZEN_ROAD || value == ROUTING_CITIZEN_AQUEDUCT) && !is_ferry_lane(value);
}

static int32_t is_water_without_building(int32_t offset)
{
    return (map_grids.terrain[offset] & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) && map_grids.building_ids[offset] == 0;
}

static int32_t can_enter_amphibia(int32_t next, int32_t current)
{
    (void) current;
    int8_t value = routing_grids.land_citizen[next];
    return value >= ROUTING_CITIZEN_ROAD || (value == ROUTING_CITIZEN_BLOCKED && is_water_without_building(next));
}

static int32_t can_enter_road_garden_ext(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return value >= ROUTING_CITIZEN_ROAD && (value < ROUTING_CITIZEN_FERRY_UNSTAFFED || value == ROUTING_CITIZEN_MONUMENT);
}

static int32_t can_enter_no_unstaffed_ferry(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return value >= ROUTING_CITIZEN_ROAD && value != ROUTING_CITIZEN_FERRY_UNSTAFFED;
}

static int32_t can_enter_no_ferry(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return is_land_for_any_mode(value, next) && !is_ferry_lane(value) && is_below_monument_or_fort(value);
}

static int32_t can_enter_fort(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return is_land_for_any_mode(value, next) && is_below_monument_or_fort(value);
}

static int32_t can_enter_any(int32_t next, int32_t current)
{
    (void) current;
    if (!map_routing_no_fighting_friendly(next)) {
        return 0;
    }
    int8_t value = routing_grids.land_citizen[next];
    return is_land_for_any_mode(value, next) && value < ROUTING_CITIZEN_MONUMENT;
}

static int32_t can_enter_road_garden(int32_t next, int32_t current)
{
    (void) current;
    int8_t value = routing_grids.land_citizen[next];
    return value >= ROUTING_CITIZEN_ROAD && value < ROUTING_CITIZEN_FERRY_UNSTAFFED;
}

static int32_t can_enter_anywhere(int32_t next, int32_t current)
{
    (void) next;
    (void) current;
    return 1;
}

static int32_t travel(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y, int32_t (*can_enter)(int32_t next, int32_t current))
{
    int32_t dst_offset = map_grid_offset(dst_x, dst_y);
    routing_route_queue(src_x, src_y, dst_offset, can_enter);
    return routing_grids.distance[dst_offset] != 0;
}

int32_t map_routing_citizen_can_travel_over_land_prefer_roads(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_prefer_roads);
}

int32_t map_routing_animal_can_travel(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_animal);
}

int32_t map_routing_amphibia_can_travel(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_amphibia);
}

int32_t map_routing_citizen_can_travel_over_road_garden_ext(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_road_garden_ext);
}

int32_t map_routing_citizen_can_travel_over_land_no_unstaffed_ferry(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_no_unstaffed_ferry);
}

int32_t map_routing_citizen_can_travel_no_ferry(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_no_ferry);
}

int32_t map_routing_citizen_can_travel_over_land_fort(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_fort);
}

int32_t map_routing_can_travel_anywhere(int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y)
{
    return travel(src_x, src_y, dst_x, dst_y, can_enter_anywhere);
}

int32_t map_routing_citizen_find_first_of_any(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_enter_any);
}

int32_t map_routing_citizen_find_first_of_no_ferry(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_enter_no_ferry);
}

int32_t map_routing_citizen_find_first_of_road_garden(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_enter_road_garden);
}

int32_t map_routing_citizen_find_first_of_fort(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    return routing_route_queue_first_of(src_x, src_y, offsets, count, can_enter_fort);
}

int32_t map_routing_citizen_find_first_of_prefer_roads(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count)
{
    routing_clear_distances();
    if (count <= 0) {
        return 0;
    }
    for (int32_t i = 0; i < count; i++) {
        routing_grids.distance[offsets[i]] = -1 - i;
    }
    routing_queue_set_source(src_x, src_y);
    do {
        int32_t offset = routing_grids.queue_items[routing_queue_head];
        int32_t distance = routing_grids.distance[offset] + 1;
        for (int32_t n = 0; n < ROUTING_NEIGHBOUR_COUNT; n++) {
            int32_t next = offset + routing_neighbour_offsets[n];
            if (routing_grids.distance[next] < 1 && can_enter_prefer_roads(next, offset)) {
                int32_t target = -1 - routing_grids.distance[next];
                if (target >= 0) {
                    if (n != ROUTING_NEIGHBOUR_LEFT) {
                        routing_grids.distance[next] = distance;
                    }
                    return target + 1;
                }
                routing_enqueue(next, distance);
            }
        }
        routing_queue_advance();
    } while (routing_queue_head != routing_queue_tail);
    return 0;
}
