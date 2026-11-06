// Water supply of the map (map/water_supply.c): the canals, lifts and wells, and the houses with water in a radius.

#ifndef MAP_WATER_SUPPLY_H
#define MAP_WATER_SUPPLY_H

#include <stdint.h>

enum water_lift_water {
    WATER_LIFT_NO_WATER = 0,
    WATER_LIFT_HAS_WATER = 1,
    WATER_LIFT_GETS_WATER = 2,
};

enum houses_water {
    HOUSES_WATER_SOME_WITHOUT = 0,
    HOUSES_WATER_ALL = 1,
    HOUSES_WATER_NO_HOUSES = 2,
};

enum water_supply_range {
    IRRIGATION_RANGE_RADIUS = 2,
    WELL_ACCESS_RADIUS = 2,
    WATER_LIFT_SIZE = 2,
    WELL_RANGE_RADIUS = 2,
};

enum water_building_image {
    WATER_BUILDING_FANCY_DESIRABILITY = 30,
    WATER_BUILDING_FANCY_IMAGE_OFFSET = 2,
};

enum canal_water_image {
    CANAL_NO_WATER_IMAGE_OFFSET = 48,
};

int32_t map_houses_water_in_radius(int32_t building_id, int32_t radius);
int32_t map_water_supply_lift_has_water(int32_t id);
void map_water_supply_mark_well_access(int32_t well_id, int32_t radius);
void map_water_supply_set_all_canals_to_no_water(void);
void map_water_supply_fill_canals_from_offset(int32_t grid_offset);
void map_water_supply_update_reservoir_fountain(void);
void map_water_supply_update_houses(void);

#endif
