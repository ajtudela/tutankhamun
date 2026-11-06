// The temple complex ghost and the order its decoration tiles are drawn in: per view and complex turn the
// image offset field and the rows of the two passes.

#ifndef WIDGET_CONSTRUCTION_GHOST_TEMPLE_COMPLEX_H
#define WIDGET_CONSTRUCTION_GHOST_TEMPLE_COMPLEX_H

#include <stdint.h>

enum temple_complex_ghost_layout {
    TEMPLE_COMPLEX_GHOST_VIEWS = 4,
    TEMPLE_COMPLEX_GHOST_TURNS = 4,
};

typedef struct temple_complex_ghost_pass {
    int32_t image_field[TEMPLE_COMPLEX_GHOST_TURNS];
    int32_t first_rows_from[TEMPLE_COMPLEX_GHOST_TURNS];
    int32_t first_rows_to[TEMPLE_COMPLEX_GHOST_TURNS];
    int32_t second_rows_from[TEMPLE_COMPLEX_GHOST_TURNS];
    int32_t second_rows_to[TEMPLE_COMPLEX_GHOST_TURNS];
} temple_complex_ghost_pass;

extern temple_complex_ghost_pass temple_complex_ghost_passes[TEMPLE_COMPLEX_GHOST_VIEWS];

void city_building_ghost_draw_temple_complex(int32_t type);

#endif
