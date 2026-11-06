// The gameplay config tables of the animals (core/config.h): wild_predators.json (per predator kind its frames, sound,
// rests and waits), animal_rules.json (the animals' waits and rests, the hunter's searches, the loads) and
// animal_choices.json (the percentages of the hippo's and crocodile's next action).

#include "core/config.h"
#include "figure/animals.h"
#include "figure/animals_wild.h"
#include "figure/figure_type.h"

static const config_name wild_predator_names[] = {
    { "hyena", WILD_HYENA },
    { "lion", WILD_LION },
    { "scorpion", WILD_SCORPION },
    { 0, 0 },
};

static const config_field wild_predator_fields[] = {
    { "frames", &wild_predators[0].frames[0], (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 25, -1, 100, 0 },
    { "default_frames", &wild_predators[0].default_frames, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, 0, 100, 0 },
    { "attack_sound", &wild_predators[0].attack_sound, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, 0, 2, 0 },
    { "rest_base", &wild_predators[0].rest_base, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, 0, 10000, 0 },
    { "rest_range", &wild_predators[0].rest_range, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, 1, 10000, 0 },
    { "no_tile_wait", &wild_predators[0].no_tile_wait, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, -1, 10000, 0 },
    { "arrival_frames", &wild_predators[0].arrival_frames, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, 0, 1, 0 },
    { "reset_counter", &wild_predators[0].reset_counter, (int32_t) (sizeof(wild_predator) / sizeof(int32_t)), 1, 0, 1, 0 },
};

static const config_storage wild_predator_storage[] = {
    { wild_predators, sizeof(wild_predators) },
};

const config_table wild_predator_table = {
    "wild_predators.json",
    "animal",
    wild_predator_names,
    WILD_PREDATOR_KINDS,
    CONFIG_TABLE_COLUMNS(wild_predator_fields, wild_predator_storage),
};

static const config_name animal_rule_names[] = {
    { "dying_ticks", ANIMAL_RULE_DYING_TICKS },
    { "no_tile_wait_ticks", ANIMAL_RULE_NO_TILE_WAIT_TICKS },
    { "ferry_wait_ticks", ANIMAL_RULE_FERRY_WAIT_TICKS },
    { "herd_alert_ticks", ANIMAL_RULE_HERD_ALERT_TICKS },
    { "herd_rest_base_ticks", ANIMAL_RULE_HERD_REST_BASE_TICKS },
    { "herd_rest_random_ticks", ANIMAL_RULE_HERD_REST_RANDOM_TICKS },
    { "predator_rest_base_ticks", ANIMAL_RULE_PREDATOR_REST_BASE_TICKS },
    { "predator_rest_random_ticks", ANIMAL_RULE_PREDATOR_REST_RANDOM_TICKS },
    { "predator_chase_ticks_per_ring", ANIMAL_RULE_PREDATOR_CHASE_TICKS_PER_RING },
    { "hippo_animate_ticks", ANIMAL_RULE_HIPPO_ANIMATE_TICKS },
    { "hippo_after_hunt_ticks", ANIMAL_RULE_HIPPO_AFTER_HUNT_TICKS },
    { "asp_rest_base_ticks", ANIMAL_RULE_ASP_REST_BASE_TICKS },
    { "asp_rest_random_ticks", ANIMAL_RULE_ASP_REST_RANDOM_TICKS },
    { "gatherer_cut_ticks", ANIMAL_RULE_GATHERER_CUT_TICKS },
    { "hunter_shoot_timeout_ticks", ANIMAL_RULE_HUNTER_SHOOT_TIMEOUT_TICKS },
    { "hunter_retry_ticks", ANIMAL_RULE_HUNTER_RETRY_TICKS },
    { "hunter_take_prey_ticks", ANIMAL_RULE_HUNTER_TAKE_PREY_TICKS },
    { "hunter_shoot_distance", ANIMAL_RULE_HUNTER_SHOOT_DISTANCE },
    { "hunter_search_path_length", ANIMAL_RULE_HUNTER_SEARCH_PATH_LENGTH },
    { "hunter_near_search_path_length", ANIMAL_RULE_HUNTER_NEAR_SEARCH_PATH_LENGTH },
    { "hunter_homeward_search_path_length", ANIMAL_RULE_HUNTER_HOMEWARD_SEARCH_PATH_LENGTH },
    { "hunter_game_load", ANIMAL_RULE_HUNTER_GAME_LOAD },
    { "lumberjack_wood_load", ANIMAL_RULE_LUMBERJACK_WOOD_LOAD },
    { "reed_gatherer_reeds_load", ANIMAL_RULE_REED_GATHERER_REEDS_LOAD },
    { 0, 0 },
};

static const config_field animal_rule_fields[] = {
    { "value", animal_rules, 1, 1, 0, 10000, 0 },
};

static const config_storage animal_rule_storage[] = {
    { animal_rules, sizeof(animal_rules) },
};

const config_table animal_rule_table = {
    "animal_rules.json",
    "rule",
    animal_rule_names,
    ANIMAL_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(animal_rule_fields, animal_rule_storage),
};

static const config_name animal_choice_names[] = {
    { "hippo_idle", ANIMAL_CHOICE_HIPPO_IDLE },
    { "hippo_other", ANIMAL_CHOICE_HIPPO_OTHER },
    { "hippo_in_water", ANIMAL_CHOICE_HIPPO_IN_WATER },
    { "hippo_animating", ANIMAL_CHOICE_HIPPO_ANIMATING },
    { "crocodile_idle", ANIMAL_CHOICE_CROCODILE_IDLE },
    { "crocodile_in_water", ANIMAL_CHOICE_CROCODILE_IN_WATER },
    { "crocodile_other", ANIMAL_CHOICE_CROCODILE_OTHER },
    { 0, 0 },
};

static const config_field animal_choice_fields[] = {
    { "weights", &animal_choices[0].weights[0], ANIMAL_CHOICE_ACTIONS, ANIMAL_CHOICE_ACTIONS, 0, 100, 0 },
};

static const config_storage animal_choice_storage[] = {
    { animal_choices, sizeof(animal_choices) },
};

const config_table animal_choice_table = {
    "animal_choices.json",
    "choice",
    animal_choice_names,
    ANIMAL_CHOICE_COUNT,
    CONFIG_TABLE_COLUMNS(animal_choice_fields, animal_choice_storage),
};
