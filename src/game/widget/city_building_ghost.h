// The construction ghost and the pieces its bridge ghosts draw per view-relative direction: the low bridge's
// image offsets and the ship bridge's image offsets and pixel shifts.

#ifndef WIDGET_CITY_BUILDING_GHOST_H
#define WIDGET_CITY_BUILDING_GHOST_H

#include <stdint.h>

enum bridge_ghost_pieces {
    BRIDGE_GHOST_DIRECTIONS = 4,
};

typedef struct bridge_ghost_piece {
    int32_t image;
    int32_t dx;
    int32_t dy;
} bridge_ghost_piece;

typedef struct bridge_ghost_piece_set {
    int32_t low_start;
    int32_t low_end;
    int32_t low_span;
    bridge_ghost_piece ship_start;
    bridge_ghost_piece ship_end;
    bridge_ghost_piece ship_middle;
    bridge_ghost_piece ship_span;
} bridge_ghost_piece_set;

extern bridge_ghost_piece_set bridge_ghost_pieces[BRIDGE_GHOST_DIRECTIONS];

void city_building_ghost_draw(void);
void city_building_ghost_draw_venue(int32_t type);
void city_building_ghost_draw_construction_cost(void);

#endif
