// Undo (game/undo.c): whether the undo holds a building.

#ifndef GAME_UNDO_H
#define GAME_UNDO_H

#include <stdint.h>

enum {
    UNDO_MAX_BUILDINGS = 525,
};

int32_t game_undo_contains_building(int32_t building_id);
void game_undo_reduce_time_available(void);
void game_load_restore_buildings_and_figure_lists(void);

#endif
