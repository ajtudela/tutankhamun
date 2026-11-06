// The properties of the figure types, one record per type filled from the difficulty's figure model file: category,
// hit points, melee and missile attack and defense, missile range and delay, speed, invasion share and the ships'
// steps. enemy_model holds the same records per invading nation; figure/enemy.c copies the scenario's into
// figure_properties.

#ifndef FIGURE_FIGURE_PROPERTIES_H
#define FIGURE_FIGURE_PROPERTIES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum figure_properties_size {
    FIGURE_PROPERTIES_TYPES = 110,
};

typedef struct {
    int32_t category;
    int32_t max_damage;
    int32_t attack;
    int32_t defense;
    int32_t missile_defense;
    int32_t missile_attack;
    int32_t missile_range;
    int32_t missile_delay;
    int32_t speed;
    int32_t invasion_percentage;
    int32_t ship_steps;
    int32_t ship_moves;
    int32_t column_m;
    int32_t column_n;
} figure_type_properties;

typedef figure_type_properties figure_properties_table[FIGURE_PROPERTIES_TYPES];

enum enemy_model_type {
    ENEMY_MODEL_INFANTRY = 0,
    ENEMY_MODEL_ARCHER = 1,
    ENEMY_MODEL_CHARIOT = 2,
    ENEMY_MODEL_WARSHIP = 3,
    ENEMY_MODEL_TRANSPORT_SHIP = 4,
    ENEMY_MODEL_TYPES = 5,
};

enum enemy_model_size {
    ENEMY_MODEL_NATIONS = 12,
};

typedef figure_type_properties enemy_models_table[ENEMY_MODEL_NATIONS][ENEMY_MODEL_TYPES];

static_assert(sizeof(figure_type_properties) == 0x38, "figure_type_properties is a model file row of 56 bytes");
static_assert(offsetof(figure_type_properties, missile_defense) == 0x10 && offsetof(figure_type_properties, speed) == 0x20 &&
                  offsetof(figure_type_properties, ship_steps) == 0x28, "figure_type_properties is the model file's layout");

#endif
