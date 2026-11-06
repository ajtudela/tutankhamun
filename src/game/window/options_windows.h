// The options windows (options_windows.c) and the display modes the display options set.

#ifndef WINDOW_OPTIONS_WINDOWS_H
#define WINDOW_OPTIONS_WINDOWS_H

#include <stdint.h>

enum display_resolution {
    DISPLAY_MODE_640X480 = 1,
    DISPLAY_MODE_800X600 = 2,
    DISPLAY_MODE_1024X768 = 3,
};

void window_display_options_draw(void);
void window_game_options_draw(void);
void window_difficulty_options_draw(void);
void window_speed_options_draw(void);
void window_sound_options_draw(void);
void window_3e_draw_background(void);
void window_3e_draw_foreground(void);

#endif
