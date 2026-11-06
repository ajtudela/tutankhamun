// The mouse cursors of the play mode (play_cursors.c): the game's cursor handles point at the play build's own three
// cursors (platform/cursor.h).

#ifndef APP_PLAY_CURSORS_H
#define APP_PLAY_CURSORS_H

#include <stdint.h>

void play_cursors_set_scale(int32_t scale_percent);
int32_t play_cursors_install(void);
void play_cursor_show(void *handle);

#endif
