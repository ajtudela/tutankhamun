// The built-in animals of each climate (scenario_climate.h); config/climate_animals.json replaces them when present.

#include "scenario/scenario_climate.h"

#include "figure/figure_type.h"
#include "map/terrain.h"

climate_animal climate_animals[CLIMATE_ANIMAL_RECORDS] = {
    [CLIMATE_ANIMAL_CENTRAL_PREY] = {
        .figure_type = FIGURE_BIRDS, .terrain = TERRAIN_MARSH, .max_figures = 7, .spawn_delay = 4, .move_delay = 8, .roam_radius = 5,
    },
    [CLIMATE_ANIMAL_NORTHERN_PREY] = {
        .figure_type = FIGURE_ANTELOPE, .max_figures = 7, .spawn_delay = 4, .move_delay = 8, .roam_radius = 10,
    },
    [CLIMATE_ANIMAL_DESERT_PREY] = {
        .figure_type = FIGURE_OSTRICH, .max_figures = 7, .spawn_delay = 4, .move_delay = 8, .roam_radius = 8,
    },
    [CLIMATE_ANIMAL_CENTRAL_PREDATOR_0] = {
        .figure_type = FIGURE_HIPPO, .terrain = TERRAIN_MARSH | TERRAIN_WATER, .max_figures = 3, .spawn_delay = 16, .amphibious = 1,
        .hunt_delay = 90, .hunt_radius = 4,
    },
    [CLIMATE_ANIMAL_CENTRAL_PREDATOR_1] = {
        .figure_type = FIGURE_ASP, .max_figures = 6, .spawn_delay = 16, .move_delay = 30, .roam_radius = 2, .hunt_delay = 30, .hunt_radius = 6,
    },
    [CLIMATE_ANIMAL_NORTHERN_PREDATOR_0] = {
        .figure_type = FIGURE_CROCODILE, .terrain = TERRAIN_MARSH | TERRAIN_WATER, .max_figures = 1, .spawn_delay = 32, .amphibious = 1,
        .hunt_delay = 50, .hunt_radius = 2,
    },
    [CLIMATE_ANIMAL_NORTHERN_PREDATOR_1] = {
        .figure_type = FIGURE_LION, .max_figures = 4, .spawn_only_when_present = 1, .spawn_delay = 12, .move_delay = 18, .roam_radius = 10,
        .hunt_delay = 18, .hunt_radius = 14,
    },
    [CLIMATE_ANIMAL_DESERT_PREDATOR_0] = {
        .figure_type = FIGURE_HYENA, .terrain = TERRAIN_DUNE, .max_figures = 7, .spawn_only_when_present = 1, .spawn_delay = 8, .move_delay = 16,
        .roam_radius = 10, .hunt_delay = 16, .hunt_radius = 10,
    },
    [CLIMATE_ANIMAL_DESERT_PREDATOR_1] = {
        .figure_type = FIGURE_SCORPION, .max_figures = 2, .spawn_delay = 20, .move_delay = 20, .roam_radius = 7, .hunt_delay = 20, .hunt_radius = 6,
    },
};
