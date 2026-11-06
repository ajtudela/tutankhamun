// Julius's sound_device API on SDL_mixer. Source, licence and changes: the header comment of
// sound_device.c (Julius, AGPL-3.0).
#ifndef PLATFORM_SOUND_DEVICE_H
#define PLATFORM_SOUND_DEVICE_H

#include <stdint.h>

#define CHANNEL_FILENAME_MAX 32

void sound_device_open(void);
void sound_device_close(void);

void sound_device_init_channels(int32_t num_channels, char filenames[][CHANNEL_FILENAME_MAX]);
int32_t sound_device_is_channel_playing(int32_t channel);

void sound_device_set_music_volume(int32_t volume_pct);
void sound_device_set_channel_volume(int32_t channel, int32_t volume_pct);

int32_t sound_device_play_music(const char *filename, int32_t volume_pct);
void sound_device_play_file_on_channel(const char *filename, int32_t channel, int32_t volume_pct);
void sound_device_play_channel(int32_t channel, int32_t volume_pct);
void sound_device_play_channel_panned(int32_t channel, int32_t volume_pct, int32_t left_pct, int32_t right_pct);
void sound_device_stop_music(void);
void sound_device_stop_channel(int32_t channel);

#endif
