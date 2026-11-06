// The effect and figure sound channels (effect.c) and the sizes of their tables.

#ifndef SOUND_EFFECT_H
#define SOUND_EFFECT_H

#include <stdint.h>

enum sound_channel_count {
    EFFECT_CHANNEL_COUNT = 100,
    FIGURE_CHANNEL_COUNT = 165,
    FIGURE_SOUND_VARIANTS = 5,
};

int32_t sound_channel_load_file(const char *path);
void sound_effect_bind_channel(int32_t id, const char *path);
void sound_figure_bind_channel(int32_t type, const char *path, int32_t variant);
void sound_effects_init(void);
void sound_figure_init_channels(void);

#endif
