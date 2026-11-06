// The per-tick figure dispatcher: resets the city's counts of enemies, rioters, natives, animals and soldiers, runs
// the action of every figure in use from the figure action table and deletes the figures that died, keeping a timing
// profile per figure type. Also the empty action.

#include "figure/action.h"
#include "city/city_player.h"
#include "figure/figure.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "game/game_internal.h"
#include "platform/platform_time.h"

int32_t figure_profile_current_type;

int32_t figure_profile_start_ticks;

int32_t figure_profile_end_ticks;

figure_profile_clock figure_profile_start_counter;

figure_profile_end_clock figure_profile_end_counter;

static void read_counter(figure_profile_clock *clock)
{
    uint64_t count;
    if (platform_time_counter(&count)) {
        clock->low_part = (uint32_t) count;
        clock->high_part = (int32_t) (count >> 32);
    }
}

void figure_action_profile_begin(int32_t type)
{
    figure_profile_current_type = type;
    figure_profile_start_ticks = (int32_t) platform_tick_count();
    if (figure_profile_qpc_available) {
        read_counter(&figure_profile_start_counter);
    }
}

void figure_action_profile_end(void)
{
    if (figure_profile_qpc_available) {
        read_counter(&figure_profile_end_counter.clock);
    }
    int32_t now = (int32_t) platform_tick_count();
    figure_profile_end_ticks = now;
    int32_t elapsed = now - figure_profile_start_ticks;
    figure_profile_entry *p = &figure_action_profile[figure_profile_current_type];
    p->calls++;
    p->ticks_total += elapsed;
    if (elapsed > p->ticks_max) {
        p->ticks_max = elapsed;
    }
    if (figure_profile_qpc_available) {
        int32_t counts = (int32_t) (figure_profile_end_counter.clock.low_part - figure_profile_start_counter.low_part);
        double seconds = (double) counts / (double) figure_profile_frequency.low_part;
        p->seconds_total = seconds + p->seconds_total;
        if (seconds > p->seconds_max) {
            p->seconds_max = seconds;
        }
    }
}

void figure_action_handle(void)
{
    city_player *player = city_player_current();
    player->enemies = 0;
    player->rioters = 0;
    player->attacking_natives = 0;
    player->animals = 0;
    player->kingdom_soldiers = 0;
    player->soldiers = 0;
    player->senet_house_has_plays = 0;
    figure_count_processed = 0;
    if (player->security_breach_duration > 0) {
        player->security_breach_duration--;
    }
    for (figure_current_id = 1; figure_current_id < MAX_FIGURES; figure_current_id++) {
        int32_t id = figure_current_id;
        figure *f = figure_get(id);
        if (f->state == FIGURE_STATE_NONE) {
            continue;
        }
        int16_t target_id = f->targeted_by_figure_id;
        if (target_id) {
            figure *target = figure_get(target_id);
            if (target->state != FIGURE_STATE_ALIVE) {
                f->targeted_by_figure_id = 0;
            }
            if (target->target_figure_id != id) {
                f->targeted_by_figure_id = 0;
            }
        }
        figure_count_processed++;
        figure_action_profile_begin(f->type);
        figure_action_table[f->type]();
        figure_action_profile_end();
        if (figure_get(figure_current_id)->state == FIGURE_STATE_DEAD) {
            figure_delete(figure_current_id);
        }
    }
}

void figure_action_none(void)
{
}
