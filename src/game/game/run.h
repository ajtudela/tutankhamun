// The game frame in parts (game/run.c): the tick part, the frame part, the city window's random number, the music
// and ambient update and the city draw dispatchers that advance the animations. A run without a window calls the
// tick part, the animations and the music update instead of the whole frame.

#ifndef GAME_RUN_H
#define GAME_RUN_H

void game_run_tick(void);

void game_run_frame(void);

void game_run_consume_city_rand(void);

void game_run_update_music_and_ambient(void);

void game_run_draw_window(void);

void game_run(void);

#endif
