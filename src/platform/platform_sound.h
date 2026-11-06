// The platform's audio: the digital driver (opened with the wave format the game asks for), samples played from files
// read into memory (loop count, pan, volume 0..127, reverb), and streams for music, speech and ambience with their
// volume and loop count. Samples and streams are integer handles. The play mode plays them with SDL_mixer.

#ifndef PLATFORM_PLATFORM_SOUND_H
#define PLATFORM_PLATFORM_SOUND_H

#include <stdbool.h>
#include <stdint.h>

void platform_audio_startup(void);
void platform_audio_shutdown(void);
int32_t platform_audio_open(void **driver, const void *wave_format);

void *platform_audio_file_read(const char *path);
void platform_audio_file_free(void *file);

int32_t platform_audio_sample_allocate(void *driver);
void platform_audio_sample_release(int32_t sample);
void platform_audio_sample_init(int32_t sample);
int32_t platform_audio_sample_set_file(int32_t sample, void *file);
void platform_audio_sample_set_loop_count(int32_t sample, int32_t count);
void platform_audio_sample_set_pan(int32_t sample, int32_t pan);
void platform_audio_sample_set_volume(int32_t sample, int32_t volume);
void platform_audio_sample_set_reverb(int32_t sample, float room, float time, float mix);
void platform_audio_sample_start(int32_t sample);
void platform_audio_sample_end(int32_t sample);
bool platform_audio_sample_playing(int32_t sample);

int32_t platform_audio_stream_open(void *driver, const char *path);
void platform_audio_stream_close(int32_t stream);
void platform_audio_stream_start(int32_t stream);
void platform_audio_stream_set_loop_count(int32_t stream, int32_t count);
void platform_audio_stream_set_volume(int32_t stream, int32_t volume);
int32_t platform_audio_stream_volume(int32_t stream);
bool platform_audio_stream_playing(int32_t stream);

void platform_audio_3d_sample_release(int32_t sample);
void platform_audio_3d_sample_end(int32_t sample);
void platform_audio_3d_provider_close(int32_t provider);

#endif
