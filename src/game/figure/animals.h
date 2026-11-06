// The animal, hunter and gatherer actions of figure/animals.c, the searches for the nearest figure of some types or
// the nearest tile of a terrain, and the tunable tables animal_rules (by ANIMAL_RULE_*) and animal_choices.

#ifndef FIGURE_ANIMALS_H
#define FIGURE_ANIMALS_H

#include <stdbool.h>
#include <stdint.h>

enum animal_rule {
    ANIMAL_RULE_DYING_TICKS = 0,
    ANIMAL_RULE_NO_TILE_WAIT_TICKS = 1,
    ANIMAL_RULE_FERRY_WAIT_TICKS = 2,
    ANIMAL_RULE_HERD_ALERT_TICKS = 3,
    ANIMAL_RULE_HERD_REST_BASE_TICKS = 4,
    ANIMAL_RULE_HERD_REST_RANDOM_TICKS = 5,
    ANIMAL_RULE_PREDATOR_REST_BASE_TICKS = 6,
    ANIMAL_RULE_PREDATOR_REST_RANDOM_TICKS = 7,
    ANIMAL_RULE_PREDATOR_CHASE_TICKS_PER_RING = 8,
    ANIMAL_RULE_HIPPO_ANIMATE_TICKS = 9,
    ANIMAL_RULE_HIPPO_AFTER_HUNT_TICKS = 10,
    ANIMAL_RULE_ASP_REST_BASE_TICKS = 11,
    ANIMAL_RULE_ASP_REST_RANDOM_TICKS = 12,
    ANIMAL_RULE_GATHERER_CUT_TICKS = 13,
    ANIMAL_RULE_HUNTER_SHOOT_TIMEOUT_TICKS = 14,
    ANIMAL_RULE_HUNTER_RETRY_TICKS = 15,
    ANIMAL_RULE_HUNTER_TAKE_PREY_TICKS = 16,
    ANIMAL_RULE_HUNTER_SHOOT_DISTANCE = 17,
    ANIMAL_RULE_HUNTER_SEARCH_PATH_LENGTH = 18,
    ANIMAL_RULE_HUNTER_NEAR_SEARCH_PATH_LENGTH = 19,
    ANIMAL_RULE_HUNTER_HOMEWARD_SEARCH_PATH_LENGTH = 20,
    ANIMAL_RULE_HUNTER_GAME_LOAD = 21,
    ANIMAL_RULE_LUMBERJACK_WOOD_LOAD = 22,
    ANIMAL_RULE_REED_GATHERER_REEDS_LOAD = 23,
    ANIMAL_RULE_COUNT = 24,
};

enum animal_choice_kind {
    ANIMAL_CHOICE_HIPPO_IDLE = 0,
    ANIMAL_CHOICE_HIPPO_OTHER = 1,
    ANIMAL_CHOICE_HIPPO_IN_WATER = 2,
    ANIMAL_CHOICE_HIPPO_ANIMATING = 3,
    ANIMAL_CHOICE_CROCODILE_IDLE = 4,
    ANIMAL_CHOICE_CROCODILE_IN_WATER = 5,
    ANIMAL_CHOICE_CROCODILE_OTHER = 6,
    ANIMAL_CHOICE_COUNT = 7,
    ANIMAL_CHOICE_ACTIONS = 4,
    ANIMAL_CHOICE_CROCODILE_ACTIONS = 2,
};

typedef struct {
    int32_t weights[ANIMAL_CHOICE_ACTIONS];
} animal_choice;

extern int32_t animal_rules[ANIMAL_RULE_COUNT];
extern animal_choice animal_choices[ANIMAL_CHOICE_COUNT];

int32_t animal_find_nearest_figure_of_types(
    int32_t x, int32_t y, int32_t num_types, const int32_t *types, uint32_t terrain_mask, int32_t *out_x, int32_t *out_y, int32_t rings, int32_t exclude_id);
void predator_hunt_or_choose_action(int32_t figure_id);
bool animal_terrain_allowed_climate(uint32_t terrain, bool is_prey);
bool figure_is_herd_animal(int32_t type);
bool figure_is_predator_animal(int32_t type);
void figure_seagulls_action(void);
void figure_hunter_spear_action(void);
void figure_hunter_action(void);
void figure_lumberjack_action(void);
void figure_reed_gatherer_action(void);
void figure_birds_action(void);
void figure_ostrich_action(void);
void figure_antelope_action(void);
void figure_crocodile_action(void);
void figure_hippo_action(void);
void figure_asp_action(void);

#endif
