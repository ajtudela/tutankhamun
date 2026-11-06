// The animal tables of figure/figure_data.c whose record type is in figure/animals_internal.h (the hippo's and
// crocodile's water steps), declared apart because figure/figure_data.h cannot include that header.

#ifndef FIGURE_ANIMALS_INTERNAL_DATA_H
#define FIGURE_ANIMALS_INTERNAL_DATA_H

#include "figure/animals_internal.h"

extern animal_offset hippo_water_steps[8];

extern animal_offset crocodile_water_steps[8];

#endif
