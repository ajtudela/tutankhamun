// The platform audio on the media library's Miles functions: the start-up sets the two Miles preferences the game
// always used (preference 0x28 and no waveOut digital driver), the driver opens on the default wave device, and a
// sample or stream is playing when Miles reports SMP_PLAYING.

#include "platform/platform_sound.h"

#include "app/media_lib.h"

#include <stdint.h>

media_sound_lib media_sound;

enum miles_value {
    MILES_PREFERENCE_0x28 = 0x28,
    MILES_PREFERENCE_0x28_VALUE = 0x40000,
    MILES_DIG_USE_WAVEOUT = 0xF,
    MILES_DEFAULT_DEVICE = -1,
};

void platform_audio_startup(void)
{
    media_sound.ail_startup();
    media_sound.ail_set_preference(MILES_PREFERENCE_0x28, MILES_PREFERENCE_0x28_VALUE);
    media_sound.ail_set_preference(MILES_DIG_USE_WAVEOUT, 0);
}

void platform_audio_shutdown(void)
{
    media_sound.ail_shutdown();
}

int32_t platform_audio_open(void **driver, const void *wave_format)
{
    return media_sound.ail_wave_out_open(driver, 0, MILES_DEFAULT_DEVICE, (void *) wave_format);
}

void *platform_audio_file_read(const char *path)
{
    return media_sound.ail_file_read(path, 0);
}

void platform_audio_file_free(void *file)
{
    media_sound.ail_mem_free_lock(file);
}

int32_t platform_audio_sample_allocate(void *driver)
{
    return media_sound.ail_allocate_sample_handle(driver);
}

void platform_audio_sample_release(int32_t sample)
{
    media_sound.ail_release_sample_handle(sample);
}

void platform_audio_sample_init(int32_t sample)
{
    media_sound.ail_init_sample(sample);
}

int32_t platform_audio_sample_set_file(int32_t sample, void *file)
{
    return media_sound.ail_set_sample_file(sample, file, 0);
}

void platform_audio_sample_set_loop_count(int32_t sample, int32_t count)
{
    media_sound.ail_set_sample_loop_count(sample, count);
}

void platform_audio_sample_set_pan(int32_t sample, int32_t pan)
{
    media_sound.ail_set_sample_pan(sample, pan);
}

void platform_audio_sample_set_volume(int32_t sample, int32_t volume)
{
    media_sound.ail_set_sample_volume(sample, volume);
}

void platform_audio_sample_set_reverb(int32_t sample, float room, float time, float mix)
{
    media_sound.ail_set_sample_reverb(sample, room, time, mix);
}

void platform_audio_sample_start(int32_t sample)
{
    media_sound.ail_start_sample(sample);
}

void platform_audio_sample_end(int32_t sample)
{
    media_sound.ail_end_sample(sample);
}

bool platform_audio_sample_playing(int32_t sample)
{
    return media_sound.ail_sample_status(sample) == SMP_PLAYING;
}

int32_t platform_audio_stream_open(void *driver, const char *path)
{
    return media_sound.ail_open_stream(driver, path, 0);
}

void platform_audio_stream_close(int32_t stream)
{
    media_sound.ail_close_stream(stream);
}

void platform_audio_stream_start(int32_t stream)
{
    media_sound.ail_start_stream(stream);
}

void platform_audio_stream_set_loop_count(int32_t stream, int32_t count)
{
    media_sound.ail_set_stream_loop_count(stream, count);
}

void platform_audio_stream_set_volume(int32_t stream, int32_t volume)
{
    media_sound.ail_set_stream_volume(stream, volume);
}

int32_t platform_audio_stream_volume(int32_t stream)
{
    return media_sound.ail_stream_volume(stream);
}

bool platform_audio_stream_playing(int32_t stream)
{
    return media_sound.ail_stream_status(stream) == SMP_PLAYING;
}

void platform_audio_3d_sample_release(int32_t sample)
{
    media_sound.ail_release_3d_sample_handle(sample);
}

void platform_audio_3d_sample_end(int32_t sample)
{
    media_sound.ail_end_3d_sample(sample);
}

void platform_audio_3d_provider_close(int32_t provider)
{
    media_sound.ail_close_3d_provider(provider);
}
