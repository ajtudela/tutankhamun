// The tutorial (game/tutorial.c): one entry per flag (its mission, the menus it opens, its message) and the flags
// the simulation sets.

#ifndef GAME_TUTORIAL_H
#define GAME_TUTORIAL_H

#include <stdint.h>

enum tutorial_limit {
    TUTORIAL_COUNT = 26,
    TUTORIAL_MENU_FLAGS = 15,
    TUTORIAL_LAST_MISSION = 10,
    TUTORIAL_FIRST_MISSION_FLAG = 15,
    TUTORIAL_LAST_MISSION_FLAG = 25,
};

typedef struct {
    int32_t mission;
    int32_t building_type_count;
    const int32_t *building_types;
    int32_t menu_flag_count;
    const int32_t *menu_flags;
    int32_t message;
    int32_t reserved_1;
} tutorial_entry;

typedef struct {
    int32_t fire_buildings[1];
    int32_t population_buildings[3];
    int32_t population_menus[2];
    int32_t granary_food_buildings[1];
    int32_t collapse_buildings[1];
    int32_t gold_buildings[4];
    int32_t gold_menus[1];
    int32_t gods_pleased_buildings[2];
    int32_t gods_pleased_menus[1];
    int32_t disease_buildings[2];
    int32_t disease_menus[1];
    int32_t food_stock_buildings[4];
    int32_t food_stock_menus[1];
    int32_t pottery_request_menus[1];
    int32_t pottery_stored_buildings[9];
    int32_t beer_stored_buildings[2];
    int32_t beer_stored_menus[2];
    int32_t houses_done_buildings[3];
    int32_t houses_done_menus[1];
    int32_t scribe_menus[1];
    int32_t bricks_stored_buildings[2];
    int32_t bricks_stored_menus[1];
    int32_t mission4_menus[1];
    int32_t mission5_menus[2];
} tutorial_lists;

enum tutorial_flag {
    TUTORIAL_FLAG_FIRE = 0,
    TUTORIAL_FLAG_POPULATION_150 = 1,
    TUTORIAL_FLAG_GRANARY_FOOD = 2,
    TUTORIAL_FLAG_COLLAPSE = 3,
    TUTORIAL_FLAG_GOLD_DELIVERED = 4,
    TUTORIAL_FLAG_FOOD_STOCK = 7,
    TUTORIAL_FLAG_POTTERY_REQUEST = 8,
    TUTORIAL_FLAG_POTTERY_STORED = 9,
    TUTORIAL_FLAG_BEER_STORED = 10,
    TUTORIAL_FLAG_HOUSES_DONE = 11,
    TUTORIAL_FLAG_SCRIBE = 12,
    TUTORIAL_FLAG_BRICKS_STORED = 13,
    TUTORIAL_FLAG_EMPIRE_OPENED = 14,
    TUTORIAL_FLAG_DISEASE = 6,
    TUTORIAL_FLAG_GODS_PLEASED = 5,
    TUTORIAL_FLAG_ENTERTAINMENT = 5,
};

void tutorial_set_flag(int32_t flag);
void game_animation_init(void);
void tutorial_menu_counts_store(void);
void tutorial_menu_counts_compare(void);
void tutorial_mission_menu_flags(void);
void tutorial_update_mission_messages(void);
void settings_change_game_speed(void);

#endif
