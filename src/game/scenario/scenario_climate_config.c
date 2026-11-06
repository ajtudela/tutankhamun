// The gameplay config table of the animals of each climate (core/config.h): the prey and the two predator records.

#include "core/config.h"
#include "figure/figure_type.h"
#include "scenario/scenario_climate.h"

static const config_name climate_animal_names[] = {
    { "central_prey", CLIMATE_ANIMAL_CENTRAL_PREY },
    { "northern_prey", CLIMATE_ANIMAL_NORTHERN_PREY },
    { "desert_prey", CLIMATE_ANIMAL_DESERT_PREY },
    { "central_predator_0", CLIMATE_ANIMAL_CENTRAL_PREDATOR_0 },
    { "central_predator_1", CLIMATE_ANIMAL_CENTRAL_PREDATOR_1 },
    { "northern_predator_0", CLIMATE_ANIMAL_NORTHERN_PREDATOR_0 },
    { "northern_predator_1", CLIMATE_ANIMAL_NORTHERN_PREDATOR_1 },
    { "desert_predator_0", CLIMATE_ANIMAL_DESERT_PREDATOR_0 },
    { "desert_predator_1", CLIMATE_ANIMAL_DESERT_PREDATOR_1 },
    { 0, 0 },
};

#define CLIMATE_ANIMAL_STRIDE ((int32_t) (sizeof(climate_animal) / sizeof(int32_t)))

static const config_field climate_animal_fields[] = {
    { "figure_type", &climate_animals[0].figure_type, CLIMATE_ANIMAL_STRIDE, 1, 0, 109, config_figure_type_names },
    { "terrain", &climate_animals[0].terrain, CLIMATE_ANIMAL_STRIDE, 1, 0, 0x7FFFFFFF, 0 },
    { "max_figures", &climate_animals[0].max_figures, CLIMATE_ANIMAL_STRIDE, 1, 0, 255, 0 },
    { "spawn_only_when_present", &climate_animals[0].spawn_only_when_present, CLIMATE_ANIMAL_STRIDE, 1, 0, 1, 0 },
    { "spawn_delay", &climate_animals[0].spawn_delay, CLIMATE_ANIMAL_STRIDE, 1, 0, 10000, 0 },
    { "move_delay", &climate_animals[0].move_delay, CLIMATE_ANIMAL_STRIDE, 1, 0, 10000, 0 },
    { "roam_radius", &climate_animals[0].roam_radius, CLIMATE_ANIMAL_STRIDE, 1, 0, 100, 0 },
    { "amphibious", &climate_animals[0].amphibious, CLIMATE_ANIMAL_STRIDE, 1, 0, 1, 0 },
    { "hunt_delay", &climate_animals[0].hunt_delay, CLIMATE_ANIMAL_STRIDE, 1, 0, 255, 0 },
    { "hunt_radius", &climate_animals[0].hunt_radius, CLIMATE_ANIMAL_STRIDE, 1, 0, 100, 0 },
};

static const config_storage climate_animal_storage[] = {
    { climate_animals, sizeof(climate_animals) },
};

const config_table climate_animal_table = {
    "climate_animals.json",
    "record",
    climate_animal_names,
    CLIMATE_ANIMAL_RECORDS,
    CONFIG_TABLE_COLUMNS(climate_animal_fields, climate_animal_storage),
};
