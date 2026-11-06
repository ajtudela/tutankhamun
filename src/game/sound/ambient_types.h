// The ambient terrain channels (ambient_channel_base): per terrain slot the name-list indexes of its variants, the
// roaming sample and the number of variants bound.

#ifndef SOUND_AMBIENT_TYPES_H
#define SOUND_AMBIENT_TYPES_H

#include <stdint.h>

enum ambient_channel_size {
    AMBIENT_CHANNEL_VARIANTS = 10,
    AMBIENT_CHANNEL_SLOTS = 6,
};

typedef struct {
    int32_t variants[AMBIENT_CHANNEL_VARIANTS];
    int32_t roam;
    int32_t variant_count;
} ambient_channel;

typedef struct {
    ambient_channel slots[AMBIENT_CHANNEL_SLOTS];
} ambient_channel_table;

#endif
