// The functions of platform_win.c that other files call and the values of game_quit_request (a quit asked: the main loop
// posts the close message; the window closing).

#ifndef PLATFORM_PLATFORM_WIN_H
#define PLATFORM_PLATFORM_WIN_H

#include <stdint.h>

enum game_quit_request {
    GAME_QUIT_NONE = 0,
    GAME_QUIT_REQUESTED = 1,
    GAME_QUIT_CLOSING = 3,
};

void platform_set_display_mode(int32_t mode);
int32_t platform_set_screen_size(int32_t mode);
void platform_pause(void);
void platform_resume(void);
int32_t platform_restore_surfaces(void);

#endif
