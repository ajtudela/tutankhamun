// One simulation tick, the dispatcher over the 51 ticks of a day, and the changes of day, month and year.

#ifndef GAME_TICK_H
#define GAME_TICK_H

#include <stdint.h>

void game_tick_run(void);
void advance_tick(void);
void game_tick_advance_day(void);
void game_tick_update_tomb_material_requests(void);

void game_time_init(void);
void game_animation_update_timers(void);
int32_t game_get_elapsed_ticks(void);

#endif
