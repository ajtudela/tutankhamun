// The game's Miles wrapper: the sound manager object (name list, reverb presets, stream volumes, 3D provider, Miles
// driver, sample tables, stream handles), the sample requests and the loaded sample files. The record types are in
// sound/sound_lib_types.h; the sample methods are in their own header, included here.

#ifndef SOUND_SOUND_LIB_H
#define SOUND_SOUND_LIB_H

#include "platform/platform_sound.h"
#include "sound/sound_lib_samples.h"
#include "sound/sound_lib_types.h"

#include <stddef.h>
#include <stdint.h>

enum sound_lib_limits {
    SOUND_LIB_STREAMS = 3,
    SOUND_LIB_3D_SAMPLES = 5,
    SOUND_PROVIDER_3D_KINDS = 5,
    SOUND_PROVIDER_3D_NONE = 5,
    SOUND_PROVIDER_NAME_SIZE = 100,
    SOUND_SAMPLE_DEFAULT_VOLUME = 100,
    SOUND_PAN_CENTRE = 64,
    SOUND_PAN_RANGE = 128,
    EAX_ENVIRONMENT_PADDED_CELL = 1,
};

enum sound_stream {
    SOUND_STREAM_MUSIC = 0,
    SOUND_STREAM_SPEECH = 1,
    SOUND_STREAM_CITY = 2,
};

void *sound_name_entry_construct(sound_name_entry *entry);
void sound_name_entry_destroy(sound_name_entry *entry);
void sound_lib_name_list_clear(sound_name_list *list);
int32_t sound_lib_name_list_init(sound_name_list *list, int32_t n);
int32_t sound_lib_name_list_add(sound_name_list *list, const char *path, int32_t owned);
int32_t sound_lib_init_digital(sound_lib_state *self);

#endif
