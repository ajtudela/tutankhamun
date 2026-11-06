// The tile pair of the construction ghosts' tables and the record of sphinx_ghost_half_sizes (per sphinx part slot its
// half-tile offset), apart so that the global can be declared with its type.

#ifndef WIDGET_GHOST_TILE_TYPES_H
#define WIDGET_GHOST_TILE_TYPES_H

#include <stdint.h>

enum ghost_sphinx_parts {
    GHOST_SPHINX_PART_SLOTS = 8,
};

typedef struct ghost_tile_pair {
    int32_t x;
    int32_t y;
} ghost_tile_pair;

typedef struct sphinx_ghost_half_size_table {
    ghost_tile_pair part_half_tiles[GHOST_SPHINX_PART_SLOTS];
    int32_t unused;
} sphinx_ghost_half_size_table;

#endif
