// Playing sound effects (sound/sound_play.c). A figure sound is one of the figure sound groups (figure_sound) in one
// of its five variants (figure_sound_variant: 2 the hit or attack, 3 the death); the citizens and the market traders
// die with one of a run of groups picked at random.

#ifndef SOUND_SOUND_PLAY_H
#define SOUND_SOUND_PLAY_H

#include <stdint.h>

enum sound_effect {
    SOUND_EFFECT_CLEAR_LAND = 2,
    SOUND_EFFECT_MESSAGE = 9,
    SOUND_EFFECT_BUILDING_DESTROYED = 17,
    SOUND_EFFECT_FIRE_SPLASH = 18,
    SOUND_EFFECT_COUNT = 19,
};

enum figure_sound {
    FIGURE_SOUND_HYENA = 0,
    FIGURE_SOUND_LION = 1,
    FIGURE_SOUND_SCORPION = 2,
    FIGURE_SOUND_ASP = 3,
    FIGURE_SOUND_OSTRICH = 4,
    FIGURE_SOUND_ANTELOPE = 5,
    FIGURE_SOUND_HIPPO = 6,
    FIGURE_SOUND_CROCODILE = 7,
    FIGURE_SOUND_BIRDS = 8,
    FIGURE_SOUND_TOWER_SENTRY = 9,
    FIGURE_SOUND_ARCHER = 11,
    FIGURE_SOUND_INFANTRY = 12,
    FIGURE_SOUND_CHARIOT = 13,
    FIGURE_SOUND_SHIP = 17,
    FIGURE_SOUND_CITIZEN = 21,
    FIGURE_SOUND_MARKET_TRADER = 29,
    FIGURE_SOUND_COUNT = 33,
};

enum figure_sound_run {
    FIGURE_SOUND_CITIZEN_COUNT = 8,
    FIGURE_SOUND_MARKET_TRADER_COUNT = 4,
};

enum figure_sound_variant {
    FIGURE_SOUND_VARIANT_HIT = 2,
    FIGURE_SOUND_VARIANT_DIE = 3,
};

void sound_effect_play(int32_t effect);
void sound_figure_play(int32_t figure_id, int32_t sound, int32_t variant);
void sound_stream_fade_down(int32_t stream);
void sound_stream_fade_out(int32_t stream);

#endif
