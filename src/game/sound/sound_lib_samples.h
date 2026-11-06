// The sound wrapper's samples and streams (sound_lib_samples.c): the manager's methods.

#ifndef SOUND_SOUND_LIB_SAMPLES_H
#define SOUND_SOUND_LIB_SAMPLES_H

#include "sound/sound_lib_types.h"

#include <stdint.h>

void sound_lib_channel_reset(sound_lib_state *self, int32_t sample);
int32_t sound_lib_alloc_sample_handles(sound_lib_state *self, int32_t n);
int32_t sound_lib_shutdown(sound_lib_state *self);
int32_t sound_lib_stop_sample(sound_lib_state *self, int32_t sample);
int32_t sound_lib_stop_3d_sample(sound_lib_state *self, int32_t sample);
void sound_lib_stop_all_samples(sound_lib_state *self);
void sound_lib_count_slot_users(sound_lib_state *self);
int32_t sound_lib_load_sample_file(sound_lib_state *self, const sound_request *request, int32_t sample);
int32_t sound_lib_find_free_sample(sound_lib_state *self);
int32_t sound_lib_find_loaded_slot(sound_lib_state *self, const sound_request *request);
int32_t sound_lib_play_sample(sound_lib_state *self, const sound_request *request);
void sound_lib_set_stream0_volume(sound_lib_state *self, int32_t volume);
void sound_lib_set_stream1_volume(sound_lib_state *self, int32_t volume);
void sound_lib_set_stream2_volume(sound_lib_state *self, int32_t volume);
int32_t sound_lib_get_stream_volume(sound_lib_state *self, int32_t stream);
void sound_lib_set_stream_volume(sound_lib_state *self, int32_t stream, int32_t volume);
int32_t sound_lib_play_stream(sound_lib_state *self, const char *path, int32_t stream, int32_t loop_count);
void sound_lib_stop_stream(sound_lib_state *self, int32_t stream);
int32_t sound_lib_stream_is_playing(sound_lib_state *self, int32_t stream);

#endif
