/* Adapted from Julius src/platform/sound_device.c (https://github.com/bvschaik/julius, compared at commit
 * 34d1ecd54befb845c0139b371fa8a0438210dac1, 2026-09-12; the API is Julius' src/sound/device.h).
 * Licence: GNU Affero General Public License v3.0 (LICENSES/AGPL-3.0.txt). The upstream file has no copyright
 * line of its own (copyright: the Julius authors).
 * Taken from Julius: the audio constants (22050 Hz, S16, stereo, 1024-sample buffers, 150 channels), the channel
 * table, percentage_to_volume, init_channels, load_channel, the DirectSound-first / other-drivers fallback in
 * sound_device_open, and the structure of sound_device_close, _init_channels, _is_channel_playing,
 * _set_music_volume, _set_channel_volume, _play_file_on_channel, _play_channel, _play_channel_panned,
 * _stop_music and _stop_channel.
 * Changed here: fixed-width integer types, no Julius settings / file / log / Vita calls
 * (plain Mix_LoadWAV / Mix_LoadMUS on the given path), sound_device_play_music written for this project; Julius'
 * custom music player functions are left out (nothing here plays decoded music data). */

#include "platform/sound_device.h"

#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_mixer.h"

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 1024

#define MAX_CHANNELS 150

typedef struct {
    const char *filename;
    Mix_Chunk *chunk;
} sound_channel;

static struct {
    int32_t initialized;
    Mix_Music *music;
    sound_channel channels[MAX_CHANNELS];
} data;

static int32_t percentage_to_volume(int32_t percentage)
{
    return percentage * SDL_MIX_MAXVOLUME / 100;
}

static void init_channels(void)
{
    data.initialized = 1;
    for (int32_t i = 0; i < MAX_CHANNELS; i++) {
        data.channels[i].chunk = 0;
        data.channels[i].filename = 0;
    }
}

void sound_device_open(void)
{
#ifdef _WIN32
    SDL_AudioInit("directsound");
#endif
    if (Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) == 0) {
        init_channels();
        return;
    }
    for (int32_t i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        const char *driver_name = SDL_GetAudioDriver(i);
        if (SDL_strcmp(driver_name, "disk") == 0 || SDL_strcmp(driver_name, "dummy") == 0) {
            continue;
        }
        if (SDL_AudioInit(driver_name) == 0 && Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS) == 0) {
            init_channels();
            return;
        }
    }
}

void sound_device_close(void)
{
    if (data.initialized) {
        for (int32_t i = 0; i < MAX_CHANNELS; i++) {
            sound_device_stop_channel(i);
        }
        sound_device_stop_music();
        Mix_CloseAudio();
        data.initialized = 0;
    }
}

static Mix_Chunk *load_chunk(const char *filename)
{
    if (filename && filename[0]) {
        return Mix_LoadWAV(filename);
    }
    return NULL;
}

static int32_t load_channel(sound_channel *channel)
{
    if (!channel->chunk && channel->filename) {
        channel->chunk = load_chunk(channel->filename);
    }
    return channel->chunk ? 1 : 0;
}

void sound_device_init_channels(int32_t num_channels, char filenames[][CHANNEL_FILENAME_MAX])
{
    if (!data.initialized) {
        return;
    }
    if (num_channels > MAX_CHANNELS) {
        num_channels = MAX_CHANNELS;
    }
    Mix_AllocateChannels(num_channels);
    for (int32_t i = 0; i < num_channels; i++) {
        data.channels[i].chunk = 0;
        data.channels[i].filename = filenames[i][0] ? filenames[i] : 0;
    }
}

int32_t sound_device_is_channel_playing(int32_t channel)
{
    if (channel < 0 || channel >= MAX_CHANNELS) {
        return 0;
    }
    return data.channels[channel].chunk && Mix_Playing(channel);
}

void sound_device_set_music_volume(int32_t volume_pct)
{
    Mix_VolumeMusic(percentage_to_volume(volume_pct));
}

void sound_device_set_channel_volume(int32_t channel, int32_t volume_pct)
{
    if (channel < 0 || channel >= MAX_CHANNELS) {
        return;
    }
    if (data.channels[channel].chunk) {
        Mix_VolumeChunk(data.channels[channel].chunk, percentage_to_volume(volume_pct));
    }
}

int32_t sound_device_play_music(const char *filename, int32_t volume_pct)
{
    if (!data.initialized) {
        return 0;
    }
    sound_device_stop_music();
    if (!filename) {
        return 0;
    }
    data.music = Mix_LoadMUS(filename);
    if (!data.music) {
        return 0;
    }
    if (Mix_PlayMusic(data.music, -1) == -1) {
        Mix_FreeMusic(data.music);
        data.music = 0;
        return 0;
    }
    sound_device_set_music_volume(volume_pct);
    return 1;
}

void sound_device_play_file_on_channel(const char *filename, int32_t channel, int32_t volume_pct)
{
    if (!data.initialized || channel < 0 || channel >= MAX_CHANNELS) {
        return;
    }
    sound_device_stop_channel(channel);
    data.channels[channel].chunk = load_chunk(filename);
    if (data.channels[channel].chunk) {
        sound_device_set_channel_volume(channel, volume_pct);
        Mix_PlayChannel(channel, data.channels[channel].chunk, 0);
    }
}

void sound_device_play_channel(int32_t channel, int32_t volume_pct)
{
    if (!data.initialized || channel < 0 || channel >= MAX_CHANNELS) {
        return;
    }
    sound_channel *ch = &data.channels[channel];
    if (load_channel(ch)) {
        sound_device_set_channel_volume(channel, volume_pct);
        Mix_PlayChannel(channel, ch->chunk, 0);
    }
}

void sound_device_play_channel_panned(int32_t channel, int32_t volume_pct, int32_t left_pct, int32_t right_pct)
{
    if (!data.initialized || channel < 0 || channel >= MAX_CHANNELS) {
        return;
    }
    sound_channel *ch = &data.channels[channel];
    if (load_channel(ch)) {
        Mix_SetPanning(channel, (Uint8) (left_pct * 255 / 100), (Uint8) (right_pct * 255 / 100));
        sound_device_set_channel_volume(channel, volume_pct);
        Mix_PlayChannel(channel, ch->chunk, 0);
    }
}

void sound_device_stop_music(void)
{
    if (data.initialized && data.music) {
        Mix_HaltMusic();
        Mix_FreeMusic(data.music);
        data.music = 0;
    }
}

void sound_device_stop_channel(int32_t channel)
{
    if (!data.initialized || channel < 0 || channel >= MAX_CHANNELS) {
        return;
    }
    sound_channel *ch = &data.channels[channel];
    if (ch->chunk) {
        Mix_HaltChannel(channel);
        Mix_FreeChunk(ch->chunk);
        ch->chunk = 0;
    }
}
