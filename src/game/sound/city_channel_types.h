// The city sound channels of the building types (city_channel_base): per building type the name-list indexes of its
// variants, the roaming sample and the number of variants bound.

#ifndef SOUND_CITY_CHANNEL_TYPES_H
#define SOUND_CITY_CHANNEL_TYPES_H

#include <stdint.h>

enum city_channel_size {
    CITY_CHANNEL_VARIANTS = 3,
    CITY_CHANNEL_TYPES = 237,
};

typedef struct {
    int32_t variants[CITY_CHANNEL_VARIANTS];
    int32_t roam;
    int32_t variant_count;
} city_channel;

typedef struct {
    city_channel types[CITY_CHANNEL_TYPES];
} city_channel_table;

#endif
