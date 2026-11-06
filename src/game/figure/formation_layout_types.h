// The record of enemy_formation_layout_offsets: per formation layout the (x, y) spot of each of its 16 figures, used
// by the enemies, the soldiers and (two layouts) the herds.

#ifndef FIGURE_FORMATION_LAYOUT_TYPES_H
#define FIGURE_FORMATION_LAYOUT_TYPES_H

#include <stdint.h>

enum formation_layout_table {
    FORMATION_LAYOUTS = 17,
    FORMATION_LAYOUT_SPOTS = 16,
    FORMATION_LAYOUT_HERD_SPAWN = 5,
    FORMATION_LAYOUT_HERD = 9,
};

enum enemy_formation_spot_table {
    ENEMY_FORMATION_LAYOUTS = 13,
    ENEMY_FORMATION_SPOT_DIRECTIONS = 4,
    ENEMY_FORMATION_SPOTS = 20,
};

enum explosion_table {
    EXPLOSION_SIZES = 6,
    EXPLOSION_CLOUDS = 16,
};

typedef struct formation_layout_spot {
    int32_t x;
    int32_t y;
} formation_layout_spot;

#endif
