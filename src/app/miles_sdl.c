// The Miles Sound System functions on SDL_mixer for the play mode (media_lib.h's sound library): samples are WAV file
// images decoded into a Mix_Chunk, one mixer channel per sample handle; streams are a Mix_Music for the music and a
// chunk on a channel of its own for speech and ambient; no 3D providers. Volumes and pans are Miles's 0..127. The
// game's own sound wrapper stays as it is; only the library under it changes.
//
// Where Miles and SDL_mixer differ, Miles' behaviour wins:
// - Miles plays MP3 data in a RIFF WAV file (format tag 0x55), which SDL's WAV loader refuses: such a file goes to
//   SDL_mixer as the MP3 frames of its "data" chunk, from the first frame sync on;
// - a pan of 0 is left, 64 the middle and 127 right, with both sides at full volume in the middle.

#include "app/media_lib.h"
#include "app/play.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "platform/platform_file.h"

#define MAX_SAMPLES 48
#define MAX_STREAMS 16
#define MIXER_CHANNELS (MAX_SAMPLES + MAX_STREAMS)

typedef struct file_image {
    struct file_image *next;
    Mix_Chunk *chunk;
    uint32_t size;
    uint32_t magic;
    uint8_t data[4];
} file_image;
#define FILE_IMAGE_MAGIC 0x53534D41u

typedef struct {
    int32_t used;
    file_image *file;
    int32_t volume;
    int32_t pan;
    int32_t loops;
    int32_t started;
} sample;

typedef struct {
    int32_t used;
    char path[PLATFORM_PATH_MAX];
    int32_t is_music;
    Mix_Music *music;
    uint8_t *file;
    Mix_Chunk *chunk;
    int32_t volume;
    int32_t loops;
    int32_t started;
} stream;

static struct {
    int32_t open;
    sample samples[MAX_SAMPLES];
    stream streams[MAX_STREAMS];
    file_image *files;
    stream *music_owner;
} miles;

static int32_t driver_dummy;

static int32_t to_mixer_volume(int32_t volume)
{
    if (volume < 0) {
        volume = 0;
    }
    if (volume > 127) {
        volume = 127;
    }
    return volume * MIX_MAX_VOLUME / 127;
}

static int32_t mixer_open(void)
{
    if (miles.open) {
        return 1;
    }
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        play_log("miles: no audio (%s)", SDL_GetError());
        return 0;
    }
    Mix_Init(MIX_INIT_MP3);
    if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 1024) != 0) {
        play_log("miles: cannot open the audio device (%s)", Mix_GetError());
        return 0;
    }
    Mix_AllocateChannels(MIXER_CHANNELS);
    miles.open = 1;
    play_log("miles: SDL_mixer open, %d channels", MIXER_CHANNELS);
    return 1;
}

static sample *sample_of(int32_t handle)
{
    if (handle < 1 || handle > MAX_SAMPLES) {
        return 0;
    }
    return &miles.samples[handle - 1];
}

static int32_t channel_of(const sample *s)
{
    return (int32_t) (s - miles.samples);
}

static stream *stream_of(int32_t handle)
{
    if (handle < 1 || handle > MAX_STREAMS || !miles.streams[handle - 1].used) {
        return 0;
    }
    return &miles.streams[handle - 1];
}

static int32_t stream_channel(const stream *s)
{
    return MAX_SAMPLES + (int32_t) (s - miles.streams);
}

static SDL_RWops *audio_source(const uint8_t *data, uint32_t size)
{
    if (size >= 12 && memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVE", 4) == 0) {
        int32_t mpeg = 0;
        for (uint32_t at = 12; at + 8 <= size;) {
            uint32_t length;
            memcpy(&length, data + at + 4, 4);
            const uint8_t *body = data + at + 8;
            if (memcmp(data + at, "fmt ", 4) == 0 && length >= 2) {
                mpeg = (body[0] | body[1] << 8) == 0x55;
            } else if (memcmp(data + at, "data", 4) == 0 && mpeg) {
                uint32_t end = at + 8 + length <= size ? length : size - at - 8;
                uint32_t skip = 0;
                while (skip + 1 < end && !(body[skip] == 0xFF && (body[skip + 1] & 0xE0) == 0xE0)) {
                    skip++;
                }
                return SDL_RWFromConstMem(body + skip, (int32_t) (end - skip));
            }
            at += 8 + length + (length & 1);
        }
    }
    return SDL_RWFromConstMem(data, (int32_t) size);
}

static uint8_t *read_whole_file(const char *path, uint32_t *size)
{
    FILE *f = platform_fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int32_t length = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = length > 0 ? malloc((size_t) length) : 0;
    if (data && fread(data, 1, (size_t) length, f) != (size_t) length) {
        free(data);
        data = 0;
    }
    fclose(f);
    *size = (uint32_t) length;
    return data;
}

static int32_t MEDIA_API ail_startup(void)
{
    return 1;
}

static void MEDIA_API ail_shutdown(void)
{
    if (miles.open) {
        Mix_HaltChannel(-1);
        Mix_HaltMusic();
    }
}

static int32_t MEDIA_API ail_set_preference(uint32_t number, int32_t value)
{
    (void) number;
    (void) value;
    return 0;
}

static int32_t MEDIA_API ail_wave_out_open(void **driver, void **wave_out, int32_t device, void *format)
{
    (void) device;
    (void) format;
    if (!mixer_open()) {
        return 1;
    }
    if (driver) {
        *driver = &driver_dummy;
    }
    if (wave_out) {
        *wave_out = 0;
    }
    return 0;
}

static void *MEDIA_API ail_file_read(const char *filename, void *dest)
{
    (void) dest;
    FILE *f = filename ? platform_fopen(filename, "rb") : 0;
    if (!f) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int32_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    file_image *image = size > 0 ? calloc(1, sizeof(file_image) + (size_t) size) : 0;
    if (!image || fread(image->data, 1, (size_t) size, f) != (size_t) size) {
        free(image);
        fclose(f);
        return 0;
    }
    fclose(f);
    image->size = (uint32_t) size;
    image->magic = FILE_IMAGE_MAGIC;
    image->next = miles.files;
    miles.files = image;
    return image->data;
}

static file_image *image_of(const void *data)
{
    if (!data) {
        return 0;
    }
    file_image *image = (file_image *) ((uint8_t *) data - offsetof(file_image, data));
    for (file_image *f = miles.files; f; f = f->next) {
        if (f == image) {
            return image;
        }
    }
    return 0;
}

static void MEDIA_API ail_mem_free_lock(void *data)
{
    file_image *image = image_of(data);
    if (!image) {
        return;
    }
    for (int32_t i = 0; i < MAX_SAMPLES; i++) {
        if (miles.samples[i].file == image) {
            if (miles.open) {
                Mix_HaltChannel(i);
            }
            miles.samples[i].file = 0;
        }
    }
    for (file_image **p = &miles.files; *p; p = &(*p)->next) {
        if (*p == image) {
            *p = image->next;
            break;
        }
    }
    if (image->chunk) {
        Mix_FreeChunk(image->chunk);
    }
    free(image);
}

static int32_t MEDIA_API ail_allocate_sample_handle(void *driver)
{
    (void) driver;
    for (int32_t i = 0; i < MAX_SAMPLES; i++) {
        if (!miles.samples[i].used) {
            memset(&miles.samples[i], 0, sizeof(sample));
            miles.samples[i].used = 1;
            miles.samples[i].volume = 127;
            miles.samples[i].pan = 64;
            miles.samples[i].loops = 1;
            return i + 1;
        }
    }
    play_log("miles: out of sample handles");
    return 0;
}

static void MEDIA_API ail_release_sample_handle(int32_t handle)
{
    sample *s = sample_of(handle);
    if (s) {
        if (miles.open) {
            Mix_HaltChannel(channel_of(s));
        }
        memset(s, 0, sizeof(*s));
    }
}

static void MEDIA_API ail_init_sample(int32_t handle)
{
    sample *s = sample_of(handle);
    if (s) {
        if (miles.open) {
            Mix_HaltChannel(channel_of(s));
        }
        s->file = 0;
        s->volume = 127;
        s->pan = 64;
        s->loops = 1;
        s->started = 0;
    }
}

static int32_t MEDIA_API ail_set_sample_file(int32_t handle, void *data, int32_t block)
{
    (void) block;
    sample *s = sample_of(handle);
    file_image *image = image_of(data);
    if (!s || !image) {
        return 0;
    }
    if (!image->chunk && miles.open) {
        image->chunk = Mix_LoadWAV_RW(audio_source(image->data, image->size), 1);
        if (!image->chunk) {
            play_log("miles: cannot decode a sample (%s)", Mix_GetError());
            return 0;
        }
    }
    s->file = image;
    return 1;
}

static void MEDIA_API ail_set_sample_volume(int32_t handle, int32_t volume)
{
    sample *s = sample_of(handle);
    if (s) {
        s->volume = volume;
        if (miles.open) {
            Mix_Volume(channel_of(s), to_mixer_volume(volume));
        }
    }
}

static void apply_pan(int32_t channel, int32_t pan)
{
    if (pan < 0) {
        pan = 0;
    }
    if (pan > 127) {
        pan = 127;
    }
    int32_t right = pan >= 64 ? 255 : pan * 255 / 64;
    int32_t left = pan <= 64 ? 255 : (127 - pan) * 255 / 63;
    Mix_SetPanning(channel, (Uint8) left, (Uint8) right);
}

static void MEDIA_API ail_set_sample_pan(int32_t handle, int32_t pan)
{
    sample *s = sample_of(handle);
    if (s) {
        s->pan = pan;
        if (miles.open) {
            apply_pan(channel_of(s), pan);
        }
    }
}

static void MEDIA_API ail_set_sample_loop_count(int32_t handle, int32_t loops)
{
    sample *s = sample_of(handle);
    if (s) {
        s->loops = loops;
    }
}

static int32_t MEDIA_API ail_sample_loop_count(int32_t handle)
{
    sample *s = sample_of(handle);
    return s ? s->loops : 0;
}

static void MEDIA_API ail_set_sample_reverb(int32_t handle, float room, float time, float level)
{
    (void) handle;
    (void) room;
    (void) time;
    (void) level;
}

static void MEDIA_API ail_start_sample(int32_t handle)
{
    sample *s = sample_of(handle);
    if (!s || !s->file || !s->file->chunk || !miles.open) {
        return;
    }
    int32_t channel = channel_of(s);
    Mix_Volume(channel, to_mixer_volume(s->volume));
    apply_pan(channel, s->pan);
    Mix_PlayChannel(channel, s->file->chunk, s->loops == 0 ? -1 : s->loops - 1);
    s->started = 1;
}

static void MEDIA_API ail_end_sample(int32_t handle)
{
    sample *s = sample_of(handle);
    if (s && miles.open) {
        Mix_HaltChannel(channel_of(s));
    }
}

static int32_t MEDIA_API ail_sample_status(int32_t handle)
{
    sample *s = sample_of(handle);
    if (!s || !miles.open) {
        return SMP_DONE;
    }
    return Mix_Playing(channel_of(s)) ? SMP_PLAYING : SMP_DONE;
}

static int32_t is_music_path(const char *path)
{
    char lower[PLATFORM_PATH_MAX];
    size_t i = 0;
    for (; path[i] && i + 1 < sizeof(lower); i++) {
        lower[i] = (char) tolower((unsigned char) path[i]);
    }
    lower[i] = 0;
    return strstr(lower, "music") != 0;
}

static void stream_stop(stream *s)
{
    if (!miles.open) {
        return;
    }
    if (s->is_music) {
        if (miles.music_owner == s) {
            Mix_HaltMusic();
            miles.music_owner = 0;
        }
    } else {
        Mix_HaltChannel(stream_channel(s));
    }
}

static int32_t MEDIA_API ail_open_stream(void *driver, const char *filename, int32_t stream_memory)
{
    (void) driver;
    (void) stream_memory;
    if (!filename || !platform_file_exists(filename)) {
        return 0;
    }
    for (int32_t i = 0; i < MAX_STREAMS; i++) {
        stream *s = &miles.streams[i];
        if (!s->used) {
            memset(s, 0, sizeof(*s));
            s->used = 1;
            snprintf(s->path, sizeof(s->path), "%s", filename);
            s->is_music = is_music_path(filename);
            s->volume = 127;
            s->loops = 1;
            return i + 1;
        }
    }
    play_log("miles: out of streams");
    return 0;
}

static void MEDIA_API ail_close_stream(int32_t handle)
{
    stream *s = stream_of(handle);
    if (!s) {
        return;
    }
    stream_stop(s);
    if (s->music) {
        Mix_FreeMusic(s->music);
    }
    free(s->file);
    if (s->chunk) {
        Mix_FreeChunk(s->chunk);
    }
    memset(s, 0, sizeof(*s));
}

static void MEDIA_API ail_set_stream_volume(int32_t handle, int32_t volume)
{
    stream *s = stream_of(handle);
    if (!s) {
        return;
    }
    s->volume = volume;
    if (!miles.open || !s->started) {
        return;
    }
    if (s->is_music) {
        if (miles.music_owner == s) {
            Mix_VolumeMusic(to_mixer_volume(volume));
        }
    } else {
        Mix_Volume(stream_channel(s), to_mixer_volume(volume));
    }
}

static int32_t MEDIA_API ail_stream_volume(int32_t handle)
{
    stream *s = stream_of(handle);
    return s ? s->volume : 0;
}

static void MEDIA_API ail_set_stream_loop_count(int32_t handle, int32_t loops)
{
    stream *s = stream_of(handle);
    if (s) {
        s->loops = loops;
    }
}

static void MEDIA_API ail_start_stream(int32_t handle)
{
    stream *s = stream_of(handle);
    if (!s || !miles.open) {
        return;
    }
    int32_t loops = s->loops == 0 ? -1 : s->loops - 1;
    if (s->is_music) {
        if (!s->music) {
            uint32_t size = 0;
            s->file = read_whole_file(s->path, &size);
            s->music = s->file ? Mix_LoadMUS_RW(audio_source(s->file, size), 1) : 0;
        }
        if (!s->music) {
            play_log("miles: cannot open music %s (%s)", s->path, Mix_GetError());
            return;
        }
        Mix_VolumeMusic(to_mixer_volume(s->volume));
        Mix_PlayMusic(s->music, s->loops == 0 ? -1 : s->loops);
        miles.music_owner = s;
    } else {
        if (!s->chunk) {
            uint32_t size = 0;
            uint8_t *file = read_whole_file(s->path, &size);
            s->chunk = file ? Mix_LoadWAV_RW(audio_source(file, size), 1) : 0;
            free(file);
        }
        if (!s->chunk) {
            play_log("miles: cannot open stream %s (%s)", s->path, Mix_GetError());
            return;
        }
        Mix_Volume(stream_channel(s), to_mixer_volume(s->volume));
        Mix_PlayChannel(stream_channel(s), s->chunk, loops);
    }
    s->started = 1;
    play_log("miles: stream %s plays (%s, volume %d)", s->path, s->is_music ? "music" : "channel", s->volume);
}

static int32_t MEDIA_API ail_stream_status(int32_t handle)
{
    stream *s = stream_of(handle);
    if (!s || !miles.open || !s->started) {
        return SMP_DONE;
    }
    if (s->is_music) {
        return miles.music_owner == s && Mix_PlayingMusic() ? SMP_PLAYING : SMP_DONE;
    }
    return Mix_Playing(stream_channel(s)) ? SMP_PLAYING : SMP_DONE;
}

static int32_t MEDIA_API ail_enumerate_3d_providers(int32_t *next, int32_t *dest, char **name)
{
    (void) next;
    (void) dest;
    (void) name;
    return 0;
}

static int32_t MEDIA_API ail_open_3d_provider(int32_t provider)
{
    (void) provider;
    return 1;
}

static void MEDIA_API ail_close_3d_provider(int32_t provider)
{
}

static void MEDIA_API ail_3d_provider_attribute(int32_t provider, const char *name, void *value)
{
}

static int32_t MEDIA_API ail_set_3d_provider_preference(int32_t provider, const char *name, const void *value)
{
    return 0;
}

static int32_t MEDIA_API ail_allocate_3d_sample_handle(int32_t provider)
{
    return 0;
}

static void MEDIA_API ail_release_3d_sample_handle(int32_t sample)
{
}

static int32_t MEDIA_API ail_set_3d_sample_file(int32_t sample, void *data)
{
    return 0;
}

static void MEDIA_API ail_set_3d_sample_volume(int32_t sample, int32_t volume)
{
}

static void MEDIA_API ail_set_3d_sample_loop_count(int32_t sample, int32_t loops)
{
}

static void MEDIA_API ail_start_3d_sample(int32_t sample)
{
}

static void MEDIA_API ail_end_3d_sample(int32_t sample)
{
}

static int32_t MEDIA_API ail_3d_sample_status(int32_t sample)
{
    return 0;
}

static void MEDIA_API ail_set_3d_position(int32_t sample, int32_t x, int32_t y, int32_t z)
{
}

static void MEDIA_API ail_set_3d_orientation(int32_t sample, int32_t front_x, int32_t front_y, int32_t front_z, int32_t up_x, int32_t up_y, int32_t up_z)
{
}

static void MEDIA_API ail_3d_update_position(int32_t sample, int32_t elapsed)
{
}

const media_sound_lib miles_sdl_library = {
    .ail_set_3d_position = ail_set_3d_position,
    .ail_start_3d_sample = ail_start_3d_sample,
    .ail_set_3d_provider_preference = ail_set_3d_provider_preference,
    .ail_set_3d_sample_file = ail_set_3d_sample_file,
    .ail_set_3d_sample_volume = ail_set_3d_sample_volume,
    .ail_3d_sample_status = ail_3d_sample_status,
    .ail_enumerate_3d_providers = ail_enumerate_3d_providers,
    .ail_open_3d_provider = ail_open_3d_provider,
    .ail_3d_provider_attribute = ail_3d_provider_attribute,
    .ail_stream_volume = ail_stream_volume,
    .ail_open_stream = ail_open_stream,
    .ail_3d_update_position = ail_3d_update_position,
    .ail_stream_status = ail_stream_status,
    .ail_set_stream_volume = ail_set_stream_volume,
    .ail_set_sample_loop_count = ail_set_sample_loop_count,
    .ail_set_sample_pan = ail_set_sample_pan,
    .ail_set_3d_orientation = ail_set_3d_orientation,
    .ail_set_3d_sample_loop_count = ail_set_3d_sample_loop_count,
    .ail_start_sample = ail_start_sample,
    .ail_file_read = ail_file_read,
    .ail_init_sample = ail_init_sample,
    .ail_set_sample_file = ail_set_sample_file,
    .ail_sample_status = ail_sample_status,
    .ail_sample_loop_count = ail_sample_loop_count,
    .ail_end_3d_sample = ail_end_3d_sample,
    .ail_end_sample = ail_end_sample,
    .ail_close_stream = ail_close_stream,
    .ail_close_3d_provider = ail_close_3d_provider,
    .ail_shutdown = ail_shutdown,
    .ail_startup = ail_startup,
    .ail_set_preference = ail_set_preference,
    .ail_wave_out_open = ail_wave_out_open,
    .ail_release_3d_sample_handle = ail_release_3d_sample_handle,
    .ail_allocate_3d_sample_handle = ail_allocate_3d_sample_handle,
    .ail_release_sample_handle = ail_release_sample_handle,
    .ail_allocate_sample_handle = ail_allocate_sample_handle,
    .ail_set_sample_volume = ail_set_sample_volume,
    .ail_set_sample_reverb = ail_set_sample_reverb,
    .ail_set_stream_loop_count = ail_set_stream_loop_count,
    .ail_start_stream = ail_start_stream,
    .ail_mem_free_lock = ail_mem_free_lock,
};
