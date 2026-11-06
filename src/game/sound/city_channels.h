// The binding of the city sound channels (city_channels.c) and the bind modes of its table.

#ifndef SOUND_CITY_CHANNELS_H
#define SOUND_CITY_CHANNELS_H

#include "sound/city_channel_types.h"

#include <stdint.h>

enum city_channel_bind_mode {
    CITY_CHANNEL_ROAM = 0,
    CITY_CHANNEL_VARIANT = 1,
};

void sound_city_bind_channel(int32_t building_type, const char *path, int32_t mode);
void sound_city_init_building_channels(void);

#endif
