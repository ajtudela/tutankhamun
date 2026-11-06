// Game time: the calendar reset of a new game, the animation timers advanced every tick, and the speed and pause
// gate that decides whether a frame runs ticks (none at speed 0, while paused, scrolling, dragging or in a window that
// blocks them). A clock that went backwards counts as 10000 ms elapsed.

#include "game/time.h"

#include "building/building_state.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tick.h"
#include "map/map_data.h"
#include "platform/platform_time.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_id.h"

#include <stdint.h>
#include <string.h>

int32_t game_tick_table_a[51];

int32_t game_tick_table_b[51];

int32_t game_cycle_counter_mod768;

int32_t game_cycle_counter_mod384;

int32_t game_cycle_counter_mod256;

int32_t game_cycle_counter_mod128;

int32_t game_cycle_counter_mod64;

int32_t game_cycle_counter_mod32;

int32_t game_cycle_counter_mod24;

int32_t game_cycle_counter_mod16;

int32_t game_cycle_counter_mod8;

int32_t game_cycle_counter_mod4;

uint32_t game_last_tick_millis;

enum game_time_rule {
    ANIMATION_TIMER_STEP_MS = 20,
    GAME_SPEED_STEPS = 10,
    GAME_TICK_MARGIN_MS = 2,
    GAME_CLOCK_WRAP_ELAPSED_MS = 10000,
};

void game_time_init(void)
{
    debug_evolve_error = 0;
    game_time_tick = 0;
    game_time_total_days = 0;
    game_time_day = 0;
    game_time_month = 0;
    game_time_year = GAME_TIME_START_YEAR;
    map_sweep_countdown = 0;
    city_frogs_countdown = 0;
    pyramid_speedup_countdown = 0;
    city_hailstorm_countdown = 0;
    building_corrupt_houses_a = 0;
    building_corrupt_houses_b = 0;
    building_highest_id_ever = 0;
    building_counters_clear();
    for (int32_t i = 0; i < TICKS_PER_DAY; i++) {
        game_tick_table_a[i] = 0;
    }
    for (int32_t i = 0; i < TICKS_PER_DAY; i++) {
        game_tick_table_b[i] = 0;
    }
}

static void cycle(int32_t *counter, int32_t period)
{
    if (++*counter >= period) {
        *counter = 0;
    }
}

void game_animation_update_timers(void)
{
    cycle(&game_cycle_counter_mod4, 4);
    cycle(&game_cycle_counter_mod8, 8);
    cycle(&game_cycle_counter_mod16, 16);
    cycle(&game_cycle_counter_mod24, 24);
    cycle(&game_cycle_counter_mod32, 32);
    cycle(&game_cycle_counter_mod64, 64);
    cycle(&game_cycle_counter_mod128, 128);
    cycle(&game_cycle_counter_mod256, 256);
    cycle(&game_cycle_counter_mod384, 384);
    cycle(&game_cycle_counter_mod768, 768);
}

int32_t game_get_elapsed_ticks(void)
{
    uint32_t now = platform_time_ms();
    int32_t elapsed = (int32_t) (now - game_last_tick_millis);
    if (elapsed < 0) {
        elapsed = GAME_CLOCK_WRAP_ELAPSED_MS;
    }
    int32_t speed_index = (GAME_SPEED_MAX - settings_data.game_speed) / GAME_SPEED_STEP;
    if (speed_index >= GAME_SPEED_STEPS || settings_data.paused) {
        return 0;
    }
    if (window_current_id == WINDOW_TOOLTIP_HOVER || window_current_id == WINDOW_TOOLTIP) {
        if (tooltip_window_id != WINDOW_CITY) {
            return 0;
        }
    } else if (window_blocks_ticks[window_current_id]) {
        return 0;
    }
    if (construction_in_progress || scroll_in_progress) {
        return 0;
    }
    if (speed_index < 0) {
        speed_index = 0;
    }
    if (elapsed < game_millis_per_tick_per_speed[speed_index] + GAME_TICK_MARGIN_MS) {
        return 0;
    }
    game_last_tick_millis = now;
    return GAME_ELAPSED_TICKS_RUN;
}

void game_animation_update(void)
{
    int32_t now = time_current_millis;
    memset(animation_should_advance, 0, ANIMATION_TIMERS);
    for (int32_t i = 0; i < ANIMATION_TIMERS; i++) {
        if (animation_timers[i] + i * ANIMATION_TIMER_STEP_MS <= now) {
            animation_should_advance[i] = 1;
            animation_timers[i] = now;
        }
    }
}
