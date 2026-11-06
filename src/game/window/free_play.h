// The free play window of the front menu (free_play.c) and the flag set while one of its missions runs.

#ifndef WINDOW_FREE_PLAY_H
#define WINDOW_FREE_PLAY_H

#include <stdint.h>

enum free_play_text_id {
    FREE_PLAY_TEXT_BUTTON,
    FREE_PLAY_TEXT_PHARAOH,
    FREE_PLAY_TEXT_CLEOPATRA,
    FREE_PLAY_TEXT_ALTERNATIVE,
    FREE_PLAY_TEXT_COUNT,
};

void free_play_allow(int32_t allowed);
int32_t free_play_allowed(void);
const char *free_play_text(int32_t id);

void window_free_play_show(void);
int32_t window_free_play_shown(void);
void window_free_play_draw(void);
int32_t window_free_play_handle_input(void);

int32_t free_play_running(void);
void free_play_leave(void);
void free_play_forget(void);

#endif
