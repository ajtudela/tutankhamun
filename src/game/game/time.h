// The game time (game/time.c): the animation timers and the game speed (10 to 100 in steps of 10).

#ifndef GAME_TIME_H
#define GAME_TIME_H

#include <stdint.h>

enum {
    ANIMATION_TIMER_FLASH = 5,
    ANIMATION_TIMERS = 51,
};

enum game_speed {
    GAME_SPEED_MIN = 10,
    GAME_SPEED_MAX = 100,
    GAME_SPEED_STEP = 10,
    GAME_SPEED_BUTTON_UP = 0,
    GAME_SPEED_BUTTON_DOWN = 1,
};

void game_animation_update(void);
void game_time_init(void);
void game_animation_update_timers(void);

#endif
