// The climate of a scenario and the animals of each climate (climate_animals): one prey record and two predator
// records per climate, of which the scenario picks one. A record gives the figure type, the terrain it may stand on,
// the herd's size, spawning, moving and roaming, and for predators the prey search.

#ifndef SCENARIO_SCENARIO_CLIMATE_H
#define SCENARIO_SCENARIO_CLIMATE_H

#include <stdint.h>

enum climate {
    CLIMATE_CENTRAL = 0,
    CLIMATE_NORTHERN = 1,
    CLIMATE_DESERT = 2,
    CLIMATE_COUNT = 3,
};

enum climate_animal_record {
    CLIMATE_ANIMAL_CENTRAL_PREY = 0,
    CLIMATE_ANIMAL_NORTHERN_PREY = 1,
    CLIMATE_ANIMAL_DESERT_PREY = 2,
    CLIMATE_ANIMAL_CENTRAL_PREDATOR_0 = 3,
    CLIMATE_ANIMAL_CENTRAL_PREDATOR_1 = 4,
    CLIMATE_ANIMAL_NORTHERN_PREDATOR_0 = 5,
    CLIMATE_ANIMAL_NORTHERN_PREDATOR_1 = 6,
    CLIMATE_ANIMAL_DESERT_PREDATOR_0 = 7,
    CLIMATE_ANIMAL_DESERT_PREDATOR_1 = 8,
    CLIMATE_ANIMAL_RECORDS = 9,
    CLIMATE_PREDATOR_VARIANTS = 2,
};

typedef struct {
    int32_t figure_type;
    int32_t terrain;
    int32_t max_figures;
    int32_t spawn_only_when_present;
    int32_t spawn_delay;
    int32_t move_delay;
    int32_t roam_radius;
    int32_t amphibious;
    int32_t hunt_delay;
    int32_t hunt_radius;
} climate_animal;

extern climate_animal climate_animals[CLIMATE_ANIMAL_RECORDS];

static inline const climate_animal *climate_prey(int32_t climate)
{
    return &climate_animals[CLIMATE_ANIMAL_CENTRAL_PREY + climate];
}

static inline const climate_animal *climate_predator(int32_t climate, int32_t variant)
{
    return &climate_animals[CLIMATE_ANIMAL_CENTRAL_PREDATOR_0 + (climate * CLIMATE_PREDATOR_VARIANTS) + variant];
}

#endif
