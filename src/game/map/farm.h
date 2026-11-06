// Farms on the map (map/farm.c): placement, fertility, expected produce, the food warning and the crop drawing.

#ifndef MAP_FARM_H
#define MAP_FARM_H

#include <stdbool.h>
#include <stdint.h>

enum farm_size {
    FARM_SIZE = 3,
    REED_GATHERER_SIZE = 2,
};

enum farm_fertility_bonus {
    FARM_FERTILITY_FLOODPLAIN_WATER_BONUS = 20,
    FARM_FERTILITY_MEADOW_WATER_BONUS = 40,
};

enum farm_food_warning {
    FARM_FOOD_WARNING_PRODUCE_FACTOR = 8,
    FARM_FOOD_WARNING_MIN_PRODUCE = 800,
    FARM_FOOD_WARNING_GRANARY_GAP = 50,
    FARM_FOOD_WARNING_GRANARIES_GAP = 3200,
};

enum farm_progress {
    FARM_PROGRESS_PER_PART = 10,
    FARM_PROGRESS_PARTS = 200,
};

enum farm_worker {
    FARM_WORKER_NONE = 0,
    FARM_WORKER_GROWN = 1,
    FARM_WORKER_SOWING = 2,
    FARM_WORKER_HARVEST = 3,
};

void map_building_place_farm(int32_t building_id, bool extra);
int32_t map_fertility_average(int32_t x, int32_t y, int32_t width, int32_t height);
int32_t building_farm_fertility(int32_t building_id);
int32_t building_farm_expected_produce(int32_t building_id, int32_t use_model);
void map_building_tiles_add_farm(int32_t building_id, int32_t x, int32_t y, bool extra);
void building_farm_food_warning(void);
void map_farm_tiles_meadow(int32_t building_id, int32_t x, int32_t y, int32_t image, int32_t parts, bool extra);
void map_farm_tiles_floodplain(int32_t building_id, int32_t x, int32_t y, int32_t image, int32_t parts, bool extra);

#endif
