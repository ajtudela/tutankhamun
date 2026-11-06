// Industry production and farms (building/building_industry.c): the input codes, farm crops, the per-type
// production table (output, inputs, counted-as resource, input stock) and the industry rules (progress of a
// cycle, input used per cycle, default input stock).

#ifndef BUILDING_BUILDING_INDUSTRY_H
#define BUILDING_BUILDING_INDUSTRY_H

#include "building/building_type.h"
#include <stdbool.h>
#include <stdint.h>

enum industry_input {
    INDUSTRY_INPUT_NONE = 0,
    INDUSTRY_INPUT_FLAX = 1,
    INDUSTRY_INPUT_UNUSED_2 = 2,
    INDUSTRY_INPUT_STONE = 3,
    INDUSTRY_INPUT_TIMBER = 4,
    INDUSTRY_INPUT_CLAY = 5,
    INDUSTRY_INPUT_LIMESTONE = 6,
    INDUSTRY_INPUT_BRICKS = 7,
    INDUSTRY_INPUT_WEAPONS = 8,
    INDUSTRY_INPUT_BARLEY = 9,
    INDUSTRY_INPUT_GEMS = 10,
    INDUSTRY_INPUT_REEDS = 11,
    INDUSTRY_INPUT_PAPYRUS = 12,
    INDUSTRY_INPUT_STRAW = 13,
    INDUSTRY_INPUT_CHARIOTS = 14,
    INDUSTRY_INPUT_GOLD = 15,
    INDUSTRY_INPUT_LINEN = 16,
    INDUSTRY_INPUT_COPPER = 17,
    INDUSTRY_INPUT_SANDSTONE = 18,
    INDUSTRY_INPUT_BEER = 19,
    INDUSTRY_INPUT_HENNA = 20,
    INDUSTRY_INPUT_MEAT = 21,
    INDUSTRY_INPUT_GAMEMEAT = 22,
    INDUSTRY_INPUT_OIL = 23,
    INDUSTRY_INPUT_POTTERY = 24,
    INDUSTRY_INPUT_PAINT = 25,
    INDUSTRY_INPUT_LAMPS = 26,
    INDUSTRY_INPUT_COUNT = 27,
};

enum farm_crop {
    FARM_CROP_NONE = -1,
    FARM_CROP_BARLEY = 0,
    FARM_CROP_FLAX = 1,
    FARM_CROP_GRAIN = 2,
    FARM_CROP_LETTUCE = 3,
    FARM_CROP_POMEGRANATES = 4,
    FARM_CROP_CHICKPEAS = 5,
    FARM_CROP_FIGS = 7,
    FARM_CROP_REEDS = 8,
    FARM_CROP_HENNA = 9,
    FARM_CROP_ROWS = 10,
    FARM_HARVESTS_PER_CROP = 4,
};

typedef struct building_production {
    int32_t output;
    int32_t input;
    int32_t second_input;
    int32_t counted_as;
    int32_t takes_input;
    int32_t input_stock_max;
} building_production;

enum industry_rule {
    INDUSTRY_RULE_PROGRESS_DONE,
    INDUSTRY_RULE_INPUT_LOAD,
    INDUSTRY_RULE_BRICKWORKS_STRAW_LOAD,
    INDUSTRY_RULE_INPUT_STOCK_DEFAULT,
    INDUSTRY_RULE_COUNT,
};

extern building_production building_productions[BUILDING_TYPE_COUNT];
extern int32_t industry_rules[INDUSTRY_RULE_COUNT];

int32_t building_takes_input(int32_t type);
int32_t building_input_stock_max(int32_t type);
int32_t building_input_index(int32_t resource);

bool building_is_crop_farm(int32_t building_id);
bool building_is_farm(int32_t building_id);
bool building_is_floodplain_farm(int32_t building_id);
bool building_is_on_floodplain(int32_t building_id);
bool building_type_is_farm(int32_t type);
int32_t building_farm_crop(int32_t type);
bool building_farm_update_production(int32_t building_id);
void farm_update_harvest_flags(void);
int32_t farm_harvest_loads(int32_t id);
int32_t farm_growth_percentage_of(int32_t building_id);
int32_t farm_has_water_access_of(int32_t building_id);
int32_t building_industry_has_produced_resource(int32_t building_id);
void farm_reduce_fertility_of(int32_t building_id, int32_t percent);
void building_industry_start_new_production(int32_t building_id);

void building_dock_update_open_water_access(void);
void building_industry_update_production(void);

#endif
