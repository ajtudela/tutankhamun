// The house model record (one per house level) loaded from the model file; its fields are what a level needs and
// gives.

#ifndef BUILDING_HOUSE_MODEL_TYPES_H
#define BUILDING_HOUSE_MODEL_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum { HOUSE_MODEL_RECORDS = 20 };

typedef struct {
    int32_t devolve_desirability;
    int32_t evolve_desirability;
    int32_t entertainment;
    int32_t water;
    int32_t religion;
    int32_t education;
    int32_t market;
    int32_t dentist;
    int32_t courthouse;
    int32_t health;
    int32_t food_types;
    int32_t pottery;
    int32_t linen;
    int32_t jewelry;
    int32_t beer;
    int32_t crime_growth;
    int32_t crime_base;
    int32_t prosperity;
    int32_t max_people;
    int32_t tax_multiplier;
    int32_t malaria_risk;
    int32_t disease_risk;
} house_model;

static_assert(sizeof(house_model) == 0x58, "house_model is a model file row of 88 bytes");
static_assert(offsetof(house_model, max_people) == 0x48 && offsetof(house_model, disease_risk) == 0x54, "house_model is the model file's layout");

typedef house_model house_model_table[HOUSE_MODEL_RECORDS];

#endif
