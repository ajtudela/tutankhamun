// The wild predators, frogs and herd creation of figure/animals_wild.c: the predator kinds and the per-kind animation
// and rest table that config/wild_predators.json loads.

#ifndef FIGURE_ANIMALS_WILD_H
#define FIGURE_ANIMALS_WILD_H

#include "scenario/scenario_types.h"
#include <stdint.h>

enum wild_kind {
    WILD_HYENA = 0,
    WILD_LION = 1,
    WILD_SCORPION = 2,
    WILD_PREDATOR_KINDS = 3,
    WILD_FROG = WILD_PREDATOR_KINDS,
    WILD_KINDS = 4,
};

enum wild_animation_states {
    WILD_ACTION_STATES = 25,
};

typedef struct {
    int32_t frames[WILD_ACTION_STATES];
    int32_t default_frames;
    int32_t attack_sound;
    int32_t rest_base;
    int32_t rest_range;
    int32_t no_tile_wait;
    int32_t arrival_frames;
    int32_t reset_counter;
} wild_predator;

extern wild_predator wild_predators[WILD_PREDATOR_KINDS];

void figure_create_herds(void);
void figure_hyena_action(void);
void figure_lion_action(void);
void figure_scorpion_action(void);
void figure_frog_action(void);
void figure_creature_action(void);

#endif
