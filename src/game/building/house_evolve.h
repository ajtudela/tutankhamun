// House evolution and consumption (building/house_evolve.c, house_evolve_checks.c): house levels, the evolve
// status and texts, the house inventory slots, the needs of a house by size, and the evolve callbacks.

#ifndef BUILDING_HOUSE_EVOLVE_H
#define BUILDING_HOUSE_EVOLVE_H

#include "building/building.h"
#include "building/house_model_types.h"
#include "city/resource_type.h"
#include "game/game_internal.h"
#include "window/window_data.h"
#include <stddef.h>
#include <stdint.h>

enum house_water_need { HOUSE_WATER_NONE = 0, HOUSE_WATER_WELL = 1, HOUSE_WATER_SUPPLY = 2 };

enum house_education {
    HOUSE_EDUCATION_NONE = 0,
    HOUSE_EDUCATION_SCHOOL_OR_LIBRARY = 1,
    HOUSE_EDUCATION_SCHOOL_AND_LIBRARY = 2,
    HOUSE_EDUCATION_HIGHER = 3,
};

enum evolve_text {
    EVOLVE_TEXT_DEVOLVING_DESIRABILITY = 0,
    EVOLVE_TEXT_NO_WELL = 1,
    EVOLVE_TEXT_NO_WATER_SUPPLY = 2,
    EVOLVE_TEXT_NO_ENTERTAINMENT = 3,
    EVOLVE_TEXT_ENTERTAINMENT_BELOW_10 = 4,
    EVOLVE_TEXT_ENTERTAINMENT_BELOW_25 = 5,
    EVOLVE_TEXT_ENTERTAINMENT_BELOW_50 = 6,
    EVOLVE_TEXT_ENTERTAINMENT_BELOW_80 = 7,
    EVOLVE_TEXT_ENTERTAINMENT_HIGH = 8,
    EVOLVE_TEXT_FOOD_TYPES_1 = 9,
    EVOLVE_TEXT_FOOD_TYPES_2 = 10,
    EVOLVE_TEXT_FOOD_TYPES_3 = 11,
    EVOLVE_TEXT_NO_EDUCATION = 14,
    EVOLVE_TEXT_NO_LIBRARY = 15,
    EVOLVE_TEXT_NO_SCHOOL = 16,
    EVOLVE_TEXT_NO_HIGHER_EDUCATION = 17,
    EVOLVE_TEXT_COURTHOUSE = 18,
    EVOLVE_TEXT_POTTERY = 19,
    EVOLVE_TEXT_RELIGION_1 = 20,
    EVOLVE_TEXT_RELIGION_2 = 21,
    EVOLVE_TEXT_RELIGION_3 = 22,
    EVOLVE_TEXT_DENTIST = 23,
    EVOLVE_TEXT_HEALTH_1 = 24,
    EVOLVE_TEXT_HEALTH_2_HAS_PHYSICIAN = 25,
    EVOLVE_TEXT_HEALTH_2 = 26,
    EVOLVE_TEXT_LINEN = 27,
    EVOLVE_TEXT_BEER = 29,
    EVOLVE_TEXT_NEXT_LEVEL = 30,
    EVOLVE_TEXT_NEXT_NO_WELL = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_NO_WELL,
    EVOLVE_TEXT_NEXT_NO_WATER_SUPPLY = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_NO_WATER_SUPPLY,
    EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_NO_ENTERTAINMENT,
    EVOLVE_TEXT_NEXT_FOOD_TYPES_1 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_FOOD_TYPES_1,
    EVOLVE_TEXT_NEXT_FOOD_TYPES_2 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_FOOD_TYPES_2,
    EVOLVE_TEXT_NEXT_FOOD_TYPES_3 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_FOOD_TYPES_3,
    EVOLVE_TEXT_NEXT_NO_EDUCATION = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_NO_EDUCATION,
    EVOLVE_TEXT_NEXT_NO_LIBRARY = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_NO_LIBRARY,
    EVOLVE_TEXT_NEXT_NO_SCHOOL = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_NO_SCHOOL,
    EVOLVE_TEXT_NEXT_COURTHOUSE = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_COURTHOUSE,
    EVOLVE_TEXT_NEXT_POTTERY = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_POTTERY,
    EVOLVE_TEXT_NEXT_RELIGION_1 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_RELIGION_1,
    EVOLVE_TEXT_NEXT_RELIGION_2 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_RELIGION_2,
    EVOLVE_TEXT_NEXT_RELIGION_3 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_RELIGION_3,
    EVOLVE_TEXT_NEXT_DENTIST = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_DENTIST,
    EVOLVE_TEXT_NEXT_HEALTH_1 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_HEALTH_1,
    EVOLVE_TEXT_NEXT_HEALTH_2_HAS_PHYSICIAN = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_HEALTH_2_HAS_PHYSICIAN,
    EVOLVE_TEXT_NEXT_HEALTH_2 = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_HEALTH_2,
    EVOLVE_TEXT_NEXT_LINEN = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_LINEN,
    EVOLVE_TEXT_NEXT_BEER = EVOLVE_TEXT_NEXT_LEVEL + EVOLVE_TEXT_BEER,
    EVOLVE_TEXT_TOP_LEVEL = 60,
    EVOLVE_TEXT_EVOLVING = 61,
    EVOLVE_TEXT_NEXT_DESIRABILITY_E = 62,
    EVOLVE_TEXT_EVOLVING_NO_SPACE = 64,
    EVOLVE_TEXT_NEXT_JEWELRY = 68,
    EVOLVE_TEXT_SECOND_JEWELRY = 70,
    EVOLVE_TEXT_JEWELRY = 72,
    EVOLVE_TEXT_NEXT_SECOND_JEWELRY = 74,
    EVOLVE_TEXT_COUNT = 80,
};

enum house_level {
    HOUSE_LEVEL_CRUDE_HUT = 0,
    HOUSE_LEVEL_STURDY_HUT = 1,
    HOUSE_LEVEL_MEAGER_SHANTY = 2,
    HOUSE_LEVEL_COMMON_SHANTY = 3,
    HOUSE_LEVEL_ROUGH_COTTAGE = 4,
    HOUSE_LEVEL_ORDINARY_COTTAGE = 5,
    HOUSE_LEVEL_MODEST_HOMESTEAD = 6,
    HOUSE_LEVEL_SPACIOUS_HOMESTEAD = 7,
    HOUSE_LEVEL_MODEST_APARTMENT = 8,
    HOUSE_LEVEL_SPACIOUS_APARTMENT = 9,
    HOUSE_LEVEL_COMMON_RESIDENCE = 10,
    HOUSE_LEVEL_SPACIOUS_RESIDENCE = 11,
    HOUSE_LEVEL_ELEGANT_RESIDENCE = 12,
    HOUSE_LEVEL_FANCY_RESIDENCE = 13,
    HOUSE_LEVEL_COMMON_MANOR = 14,
    HOUSE_LEVEL_SPACIOUS_MANOR = 15,
    HOUSE_LEVEL_ELEGANT_MANOR = 16,
    HOUSE_LEVEL_STATELY_MANOR = 17,
    HOUSE_LEVEL_MODEST_ESTATE = 18,
    HOUSE_LEVEL_PALATIAL_ESTATE = 19,
    HOUSE_LEVEL_COUNT = 20,
};

enum house_level_range {
    HOUSE_LEVEL_MAX = HOUSE_LEVEL_PALATIAL_ESTATE,
};

enum house_evolve_status {
    HOUSE_EVOLVE_DEVOLVE = -1,
    HOUSE_EVOLVE_STAY = 0,
    HOUSE_EVOLVE_UP = 1,
};

enum house_evolve_limit {
    HOUSE_TOP_LEVEL_EVOLVE_DESIRABILITY = 1000,
    HOUSE_DEVOLVE_DELAY_DAYS = 2,
};

enum house_inventory_slot {
    INVENTORY_NONE = -1,
    INVENTORY_GRAIN = 0,
    INVENTORY_MEAT = 1,
    INVENTORY_LETTUCE = 2,
    INVENTORY_CHICKPEAS = 3,
    INVENTORY_POMEGRANATES = 4,
    INVENTORY_FIGS = 5,
    INVENTORY_FISH = 6,
    INVENTORY_GAMEMEAT = 7,
    INVENTORY_BEER = 8,
    INVENTORY_POTTERY = 9,
    INVENTORY_LINEN = 10,
    INVENTORY_JEWELRY = 11,
};

enum house_inventory_size {
    HOUSE_INVENTORY_SLOTS = 12,
};

static inline int32_t house_good_requirement(int32_t value, int32_t size)
{
    if ((size == 1) && (value > 0)) {
        value = (value + 2) / 4;
        if (value < 1) {
            value = 1;
        }
    }
    return value;
}

static inline int32_t house_food_inventory_index(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return INVENTORY_GRAIN;
        case RESOURCE_MEAT:
            return INVENTORY_MEAT;
        case RESOURCE_LETTUCE:
            return INVENTORY_LETTUCE;
        case RESOURCE_CHICKPEAS:
            return INVENTORY_CHICKPEAS;
        case RESOURCE_POMEGRANATES:
            return INVENTORY_POMEGRANATES;
        case RESOURCE_FIGS:
            return INVENTORY_FIGS;
        case RESOURCE_FISH:
            return INVENTORY_FISH;
        case RESOURCE_GAMEMEAT:
            return INVENTORY_GAMEMEAT;
        case RESOURCE_POTTERY:
            return INVENTORY_POTTERY;
        case RESOURCE_BEER:
            return INVENTORY_BEER;
        case RESOURCE_LINEN:
            return INVENTORY_LINEN;
        case RESOURCE_LUXURY_GOODS:
            return INVENTORY_JEWELRY;
        default:
            return INVENTORY_NONE;
    }
}

static inline int32_t house_food_types_available(building *b)
{
    int32_t available = 0;
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        if ((resource != RESOURCE_NONE) && (b->data.house.inventory[house_food_inventory_index(resource)] != 0)) {
            available++;
        }
    }
    return available;
}
int32_t house_evolve_is_frozen(void);
int32_t house_check_desirability(int32_t building_id);
void house_consume_goods(int32_t building_id);
void city_houses_reset_demands(void);
void building_house_process_evolve_and_consume_goods(void);
void house_evolve_crude_hut(void);
void house_evolve_sturdy_hut(void);
void house_evolve_meager_shanty(void);
void house_evolve_common_shanty(void);
void house_evolve_rough_cottage(void);
void house_evolve_ordinary_cottage(void);
void house_evolve_modest_homestead(void);
void house_evolve_spacious_homestead(void);
void house_evolve_modest_apartment(void);
void house_evolve_spacious_apartment(void);
void house_evolve_common_residence(void);
void house_evolve_spacious_residence(void);
void house_evolve_elegant_residence(void);
void house_evolve_fancy_residence(void);
void house_evolve_common_manor(void);
void house_evolve_spacious_manor(void);
void house_evolve_elegant_manor(void);
void house_evolve_stately_manor(void);
void house_evolve_modest_estate(void);
void house_evolve_palatial_estate(void);

#endif
