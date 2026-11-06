// The cursor checks of a selected transport ship (game/transport_cursor.c).

#ifndef GAME_TRANSPORT_CURSOR_H
#define GAME_TRANSPORT_CURSOR_H

#include <stdint.h>

int32_t transport_cursor_can_embark(int32_t figure_id);
int32_t transport_cursor_can_land(int32_t figure_id);

#endif
