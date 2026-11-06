// Shared by figure/animals.c, herds.c and animals_wild.c: the offsets of the 16 tiles around a herd, the terrain an
// animal may not stand on, the spear's and hunter's timings, the animal sounds, and the searches and helpers the three
// files call.

#ifndef FIGURE_ANIMALS_INTERNAL_H
#define FIGURE_ANIMALS_INTERNAL_H

#include "core/calc.h"
#include "figure/animals.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "scenario/scenario_climate.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t x;
    int32_t y;
} animal_offset;

enum herd_offsets {
    HERD_OFFSET_COUNT = 16,
};

enum animal_terrain {
    ANIMAL_TERRAIN_BLOCKED = TERRAIN_ROCK | TERRAIN_GARDEN | TERRAIN_RUBBLE | TERRAIN_WALL | TERRAIN_GATEHOUSE | TERRAIN_DIKE | TERRAIN_ORE | TERRAIN_GEM_ROCK |
        TERRAIN_MONUMENT | TERRAIN_ROCK_NEAR_MONUMENT,
    ANIMAL_TERRAIN_SPECIAL = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_ELEVATION | TERRAIN_RUBBLE |
        TERRAIN_FLOODPLAIN | TERRAIN_MARSH | TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_DUNE | TERRAIN_DEEPWATER | TERRAIN_ROCK_NEAR_MONUMENT,
};

enum hunter_timing {
    HUNTER_SPEAR_FLIGHT_TICKS = 104,
    HUNTER_SHOT_TICK = 9,
};

enum hunter_search {
    HUNTER_PREY_CANDIDATES = 30,
};

enum animal_sound {
    ANIMAL_SOUND_ASP_ATTACK = 3,
    ANIMAL_SOUND_HIPPO_ATTACK = 6,
    ANIMAL_SOUND_CROCODILE_ATTACK = 7,
    ANIMAL_SOUND_HUNTER_SHOT_NORTHERN = 14,
    ANIMAL_SOUND_HUNTER_SHOT_DESERT = 15,
    ANIMAL_SOUND_HUNTER_SHOT_CENTRAL = 16,
    ANIMAL_SOUND_VARIANT = 2,
};

static inline const climate_animal *scenario_prey(void)
{
    return climate_prey(scenario_data.climate_type);
}

static inline const climate_animal *scenario_predator(void)
{
    return climate_predator(scenario_data.climate_type, scenario_data.predator_variant);
}

typedef int32_t (*tile_callback)(int32_t grid_offset, int32_t arg);

int32_t herd_count_animals_in_state(int32_t formation_id, uint32_t action_state);
bool animal_is_amphibious_climate(bool is_prey);
int32_t animal_find_free_neighbour_direction(int32_t x, int32_t y, int32_t direction, int32_t src_x, int32_t src_y, bool is_prey);
bool animal_find_flee_tile_from(int32_t x, int32_t y, int32_t from_x, int32_t from_y, int32_t *out_x, int32_t *out_y);
bool predator_find_prey_nearby(int32_t figure_id);
bool herd_animal_react_to_threats(int32_t figure_id);
int32_t animal_find_reachable_tile_in_area(int32_t x, int32_t y, int32_t radius, int32_t *out_x, int32_t *out_y, bool is_prey);
int32_t animal_find_nearest_figure_of_types_in_state(int32_t x, int32_t y, int32_t num_types, const int32_t *types, uint32_t terrain_mask, int32_t *out_x,
    int32_t *out_y, int32_t rings, int32_t exclude_id, uint32_t action_state);
int32_t animal_search_rings_for(int32_t x, int32_t y, int32_t *out_x, int32_t *out_y, int32_t rings, int32_t arg, tile_callback callback);
bool animal_terrain_allowed_climate(uint32_t terrain, bool is_prey);
void hippo_start_action(int32_t figure_id, int32_t action_state);
void hippo_choose_next_action(int32_t figure_id);
void crocodile_start_action(int32_t figure_id, int32_t action_state);
void crocodile_choose_next_action(int32_t figure_id);
int32_t hunter_find_prey_target(int32_t x, int32_t y, int32_t max_path_length);

void sound_figure_play(int32_t figure_id, int32_t sound_id, int32_t variant);

#include "figure/animals_internal_data.h"

#endif
