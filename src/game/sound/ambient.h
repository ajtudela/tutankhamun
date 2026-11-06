// The functions of ambient.c, the ambient streams the city view scores and the terrain slots of the random city
// samples.

#ifndef SOUND_AMBIENT_H
#define SOUND_AMBIENT_H

#include "sound/ambient_types.h"

#include <stdint.h>

enum ambient_stream {
    AMBIENT_HOUSING = 1,
    AMBIENT_INDUSTRIAL = 2,
    AMBIENT_MEADOW = 3,
    AMBIENT_MONUMENT = 4,
    AMBIENT_DESERT = 6,
    AMBIENT_GRASSLAND = 7,
    AMBIENT_FLOODPLAIN = 8,
    AMBIENT_ROCK = 9,
    AMBIENT_WATER = 10,
    AMBIENT_VEGETATION = 11,
};

enum ambient_terrain_slot {
    AMBIENT_TERRAIN_DESERT = 0,
    AMBIENT_TERRAIN_WATER = 1,
    AMBIENT_TERRAIN_ROCK = 2,
    AMBIENT_TERRAIN_FARM = 3,
    AMBIENT_TERRAIN_FLOOD = 4,
    AMBIENT_TERRAIN_FOREST = 5,
};

void sound_ambient_fade_up(int32_t channel);
void sound_ambient_update(void);
void sound_ambient_bind_channel(int32_t slot, const char *path, int32_t mode);
void sound_city_init_ambient_channels(void);
void sound_ambient_clear_scores(void);

#endif
