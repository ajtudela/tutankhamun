// The fishing boats, fishing spots and ferry boats of figure/boats.c, the dock and water tile searches and the ferry
// queues.

#ifndef FIGURE_BOATS_H
#define FIGURE_BOATS_H

#include "building/building_data.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum ferry_on_path {
    FERRY_ON_PATH_UNSTAFFED = 0,
    FERRY_ON_PATH_STAFFED = 1,
    FERRY_NOT_ON_PATH = 2,
};

enum ferry_path_limit {
    FERRY_PATHS = 50,
    FERRY_PASSENGERS = 4,
    FERRY_LANDING_SIZE = 2,
    FERRY_WAIT_TICKS = 100,
    FERRY_TICK_COUNTER_LIMIT = 100,
    FERRY_SLOWNESS_MIN = 2,
    FERRY_SLOWNESS_STEP = 10,
};

enum dock_limit {
    CITY_DOCKS = 10,
    DOCK_QUEUE_NEAR = 2,
    DOCK_QUEUE_FAR = 3,
};

enum dock_water_side {
    DOCK_WATER_ABOVE = 0,
    DOCK_WATER_RIGHT = 1,
    DOCK_WATER_BELOW = 2,
    DOCK_WATER_LEFT = 3,
};

enum boat_limit {
    BOAT_FRAMES = 12,
    BOAT_TICKS_PER_IMAGE = 3,
    BOAT_SINKING_TICKS = 33,
    BOAT_BUBBLES_FRAMES = 23,
    FISHING_BOAT_WAIT_TICKS = 50,
    FISHING_BOAT_FISHING_TICKS = 200,
    FISHING_BOAT_FISHING_IMAGES = 9,
    FISHING_BOAT_FISHING_TURN = 2,
    FISHING_BOAT_DELAY_BASE = 102,
    FISHING_BOAT_DELAY_STEP = 5,
    FISHING_BOAT_FREE_TILE_RADIUS = 5,
    FISHING_SPOT_FRAMES = 24,
    FISHING_SPOT_RANDOM_PAUSE = 100,
    FISHING_SPOT_PAUSE = 25,
};

static inline ferry_path *ferry_path_get(int32_t path)
{
    return &ferry_path_records[path];
}

void ferry_queues_release(int32_t building_id);
void ferry_queue_release(int32_t building_id);
void ferry_load(int32_t figure_id, int32_t building_id, int32_t max);
void ferry_unload(int32_t figure_id, int32_t building_id);
void ferry_move(int32_t figure_id, int32_t slowness);
void ferry_check_passengers(void);
int32_t building_dock_index(int32_t building_id);
int32_t building_dock_get_free_destination(int32_t figure_id);
int32_t building_dock_get_queue_destination(int32_t figure_id);
int32_t scenario_map_closest_fishing_point(int32_t figure_id);
int32_t map_water_find_alternative_fishing_boat_tile(int32_t figure_id);
void figure_fishing_boat_action(void);
void figure_fishing_spot_action(void);
int32_t ferry_passenger_count(int32_t figure_id);
int32_t figure_train_leader(int32_t figure_id);
bool figure_is_queued_for_ferry(int32_t figure_id);
void figure_ferry_boat_action(void);
int32_t figure_route_find_ferry_on_path(int32_t figure_id, int32_t ferry_building_id);

#endif
