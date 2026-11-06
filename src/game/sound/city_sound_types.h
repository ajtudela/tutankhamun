// The building sound channels of the city view (city_sounds, saved with the city): per channel whether it was seen
// since the last play tick, its views in all and per direction, its next sample, play count and delay.

#ifndef SOUND_CITY_SOUND_TYPES_H
#define SOUND_CITY_SOUND_TYPES_H

#include <assert.h>
#include <stdint.h>

enum city_sound_size {
    CITY_SOUND_DIRECTIONS = 5,
    CITY_SOUND_SAMPLES = 8,
    CITY_SOUND_CHANNELS = 70,
};

typedef struct {
    int32_t available;
    int32_t total_views;
    int32_t views_threshold;
    int32_t direction_views[CITY_SOUND_DIRECTIONS];
    int32_t current_channel;
    int32_t num_channels;
    int32_t channels[CITY_SOUND_SAMPLES];
    int32_t in_use;
    int32_t times_played;
    uint32_t last_played_time;
    uint32_t delay_millis;
    int32_t should_play;
    int32_t unused[9];
} city_sound_channel;

typedef struct {
    city_sound_channel channels[CITY_SOUND_CHANNELS];
} city_sound_table;

static_assert(sizeof(city_sound_channel) == 0x80, "city_sound_channel is a savegame record of 128 bytes");
static_assert(sizeof(city_sound_table) == 8960, "city_sound_table is a savegame record of 8960 bytes");

#endif
