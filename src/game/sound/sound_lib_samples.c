// The sound wrapper's samples and streams: sample handles, loaded sample files, playing a sample, stream volumes and
// the shutdown.
//
// A sample whose slot is the reset -1 is left out of the slot users: the original counted it on the integer before
// its slot table, a field of the sound manager, where the write did nothing; here the table is an allocation of its
// own and the same write lands in the allocator's record of it.

#include "sound/sound_lib_samples.h"

#include "platform/platform_sound.h"
#include "sound/sound_lib.h"

#include <stdlib.h>
#include <string.h>

void sound_lib_channel_reset(sound_lib_state *self, int32_t sample)
{
    sound_sample_state *s = &self->states[sample];
    s->request.id = -1;
    s->request.name_index = -1;
    s->request.volume = SOUND_SAMPLE_DEFAULT_VOLUME;
    s->request.pan = SOUND_PAN_CENTRE;
    s->request.loop_count = 1;
    s->slot = -1;
}

int32_t sound_lib_alloc_sample_handles(sound_lib_state *self, int32_t n)
{
    if (n < 1) {
        return 0;
    }
    if (self->handles) {
        sound_lib_stop_all_samples(self);
        for (int32_t i = 0; i < self->sample_count; i++) {
            sound_lib_channel_reset(self, i);
            platform_audio_sample_release(self->handles[i]);
            if (self->slots[i].file) {
                platform_audio_file_free(self->slots[i].file);
                self->slots[i].file = 0;
            }
        }
    }
    if (self->handles) {
        free(self->handles);
        self->handles = 0;
    }
    if (self->spare_handles) {
        free(self->spare_handles);
        self->spare_handles = 0;
    }
    if (self->states) {
        free(self->states);
        self->states = 0;
    }
    if (self->slots) {
        free(self->slots);
        self->slots = 0;
    }
    self->handles = malloc((size_t) n * sizeof(int32_t));
    if (!self->handles) {
        return 0;
    }
    self->spare_handles = malloc((size_t) n * sizeof(int32_t));
    if (!self->spare_handles) {
        return 0;
    }
    self->states = malloc((size_t) n * sizeof(sound_sample_state));
    if (!self->states) {
        return 0;
    }
    self->slots = malloc((size_t) n * sizeof(sound_sample_slot));
    if (!self->slots) {
        return 0;
    }
    self->sample_count = n;
    for (int32_t i = 0; i < self->sample_count; i++) {
        self->handles[i] = platform_audio_sample_allocate(self->driver);
        if (!self->handles[i]) {
            break;
        }
    }
    return 1;
}

int32_t sound_lib_shutdown(sound_lib_state *self)
{
    if (self->disabled) {
        return 1;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        sound_lib_channel_reset(self, i);
        platform_audio_sample_release(self->handles[i]);
        if (self->slots[i].file) {
            platform_audio_file_free(self->slots[i].file);
            self->slots[i].file = 0;
        }
    }
    for (int32_t i = 0; i < SOUND_LIB_STREAMS; i++) {
        if (self->streams[i]) {
            platform_audio_stream_close(self->streams[i]);
            self->streams[i] = 0;
        }
    }
    if (self->provider_3d_kind != SOUND_PROVIDER_3D_NONE) {
        for (int32_t i = 0; i < self->count_3d; i++) {
            platform_audio_3d_sample_release(self->handles_3d[i]);
            if (self->slots_3d[i].file) {
                platform_audio_file_free(self->slots_3d[i].file);
                self->slots_3d[i].file = 0;
            }
        }
        platform_audio_3d_provider_close(self->provider_3d);
    }
    platform_audio_shutdown();
    if (self->slots) {
        free(self->slots);
        self->slots = 0;
    }
    if (self->states) {
        free(self->states);
        self->states = 0;
    }
    if (self->spare_handles) {
        free(self->spare_handles);
        self->spare_handles = 0;
    }
    if (self->handles) {
        free(self->handles);
        self->handles = 0;
    }
    return 1;
}

int32_t sound_lib_stop_sample(sound_lib_state *self, int32_t sample)
{
    if (!self->disabled) {
        platform_audio_sample_end(self->handles[sample]);
    }
    return 1;
}

int32_t sound_lib_stop_3d_sample(sound_lib_state *self, int32_t sample)
{
    if (self->disabled) {
        return 1;
    }
    platform_audio_3d_sample_end(self->handles_3d[sample]);
    return 1;
}

void sound_lib_stop_all_samples(sound_lib_state *self)
{
    if (self->disabled) {
        return;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        sound_lib_stop_sample(self, i);
    }
    for (int32_t i = 0; i < self->count_3d; i++) {
        sound_lib_stop_3d_sample(self, i);
    }
}

void sound_lib_count_slot_users(sound_lib_state *self)
{
    if (self->disabled) {
        return;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        self->slots[i].users = 0;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        if (!platform_audio_sample_playing(self->handles[i])) {
            continue;
        }
        int32_t slot = self->states[i].slot;
        if (slot >= 0 && slot < self->sample_count) {
            self->slots[slot].users += 1;
        }
    }
}

int32_t sound_lib_load_sample_file(sound_lib_state *self, const sound_request *request, int32_t sample)
{
    int32_t slot = -1;
    if (self->disabled) {
        return 1;
    }
    const char *name = self->name_list.entries[request->name_index].name;
    if (!name) {
        return -1;
    }
    if (*name == 0) {
        return -1;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        if (self->slots[i].users == 0) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        return -1;
    }
    if (self->slots[slot].file) {
        platform_audio_file_free(self->slots[slot].file);
        sound_lib_channel_reset(self, sample);
    }
    self->states[sample].request = *request;
    self->slots[slot].file = platform_audio_file_read(self->name_list.entries[request->name_index].name);
    if (!self->slots[slot].file) {
        return -1;
    }
    self->slots[slot].name_index = request->name_index;
    platform_audio_sample_init(self->handles[sample]);
    platform_audio_sample_set_file(self->handles[sample], self->slots[slot].file);
    return slot;
}

int32_t sound_lib_find_free_sample(sound_lib_state *self)
{
    for (int32_t i = 0; i < self->sample_count; i++) {
        if (!platform_audio_sample_playing(self->handles[i])) {
            return i;
        }
    }
    return -1;
}

int32_t sound_lib_find_loaded_slot(sound_lib_state *self, const sound_request *request)
{
    for (int32_t i = 0; i < self->sample_count; i++) {
        if (self->slots[i].name_index == request->name_index && self->slots[i].file) {
            return i;
        }
    }
    return -1;
}

int32_t sound_lib_play_sample(sound_lib_state *self, const sound_request *request)
{
    if (self->disabled) {
        return -1;
    }
    if (request->name_index < 0 || request->name_index > self->name_list.count - 1) {
        return -1;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        if (platform_audio_sample_playing(self->handles[i]) && request->id == self->states[i].request.id &&
            request->name_index == self->states[i].request.name_index) {
            return -1;
        }
    }
    int8_t sample = (int8_t) sound_lib_find_free_sample(self);
    if (sample == -1) {
        return -1;
    }
    sound_lib_count_slot_users(self);
    int8_t slot = (int8_t) sound_lib_find_loaded_slot(self, request);
    if (slot == -1) {
        int32_t loaded = sound_lib_load_sample_file(self, request, sample);
        if (loaded == -1) {
            return -1;
        }
        slot = (int8_t) loaded;
    } else {
        platform_audio_sample_set_file(self->handles[sample], self->slots[slot].file);
    }
    int32_t handle = self->handles[sample];
    platform_audio_sample_set_loop_count(handle, request->loop_count);
    platform_audio_sample_set_pan(self->handles[sample], request->pan);
    platform_audio_sample_set_volume(self->handles[sample], request->volume);
    self->states[sample].slot = slot;
    if (self->reverb_on) {
        platform_audio_sample_set_reverb(self->handles[sample], self->reverb[0], self->reverb[1], self->reverb[2]);
    } else {
        platform_audio_sample_set_reverb(self->handles[sample], 0.0f, 0.0f, 0.0f);
    }
    platform_audio_sample_start(self->handles[sample]);
    return sample;
}

static void set_stream_n_volume(sound_lib_state *self, int32_t stream, int32_t *stored_volume, int32_t volume)
{
    *stored_volume = volume;
    if (self->disabled) {
        return;
    }
    if (!self->streams[stream]) {
        return;
    }
    if (platform_audio_stream_playing(self->streams[stream])) {
        platform_audio_stream_set_volume(self->streams[stream], volume);
    }
}

void sound_lib_set_stream0_volume(sound_lib_state *self, int32_t volume)
{
    set_stream_n_volume(self, 0, &self->stream0_volume, volume);
}

void sound_lib_set_stream1_volume(sound_lib_state *self, int32_t volume)
{
    set_stream_n_volume(self, 1, &self->stream1_volume, volume);
}

void sound_lib_set_stream2_volume(sound_lib_state *self, int32_t volume)
{
    set_stream_n_volume(self, 2, &self->stream2_volume, volume);
}

int32_t sound_lib_get_stream_volume(sound_lib_state *self, int32_t stream)
{
    if (self->disabled) {
        return 0;
    }
    if (!self->streams[stream]) {
        return 0;
    }
    return platform_audio_stream_volume(self->streams[stream]);
}

void sound_lib_set_stream_volume(sound_lib_state *self, int32_t stream, int32_t volume)
{
    if (!self->disabled && self->streams[stream]) {
        platform_audio_stream_set_volume(self->streams[stream], volume);
    }
}

int32_t sound_lib_play_stream(sound_lib_state *self, const char *path, int32_t stream, int32_t loop_count)
{
    if (stream > 2) {
        return -1;
    }
    if (self->disabled) {
        return 1;
    }
    if (self->streams[stream]) {
        platform_audio_stream_close(self->streams[stream]);
        self->streams[stream] = 0;
    }
    self->streams[stream] = platform_audio_stream_open(self->driver, path);
    if (self->streams[stream]) {
        int32_t volume;
        if (stream == 1) {
            volume = self->stream1_volume;
        } else if (stream == 2) {
            volume = self->stream2_volume;
        } else {
            volume = self->stream0_volume;
        }
        platform_audio_stream_set_loop_count(self->streams[stream], loop_count);
        platform_audio_stream_set_volume(self->streams[stream], volume);
        platform_audio_stream_start(self->streams[stream]);
    }
    return 1;
}

void sound_lib_stop_stream(sound_lib_state *self, int32_t stream)
{
    if (self->disabled) {
        return;
    }
    platform_audio_stream_close(self->streams[stream]);
    self->streams[stream] = 0;
}

int32_t sound_lib_stream_is_playing(sound_lib_state *self, int32_t stream)
{
    if (self->disabled) {
        return 0;
    }
    if (!self->streams[stream]) {
        return 0;
    }
    return platform_audio_stream_playing(self->streams[stream]) ? 1 : 0;
}

