// The game's sound wrapper (sound_manager): the list of sound file names (init, add, the entries' destructors) and
// the start of the digital driver (the platform audio, the wave format and the sample handles).

#include "sound/sound_lib.h"

#include "core/static_init.h"
#include "platform/platform_sound.h"
#include "platform/platform_system.h"
#include "sound/sound_data.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum sound_lib_start {
    SOUND_LIB_SAMPLES = 5,
    WAVE_FORMAT_PCM_TAG = 1,
    BITS_PER_BYTE = 8,
};

void *sound_name_entry_construct(sound_name_entry *entry)
{
    entry->owned = 0;
    entry->name = 0;
    return entry;
}

void sound_name_entry_destroy(sound_name_entry *entry)
{
    if (entry->owned && entry->name) {
        free((void *) entry->name);
        entry->name = 0;
    }
}

void sound_lib_name_list_clear(sound_name_list *list)
{
    if (list->entries) {
        for (int32_t i = list->capacity - 1; i >= 0; i--) {
            sound_name_entry_destroy(&list->entries[i]);
        }
        free(list->entries);
        list->entries = 0;
    }
}

int32_t sound_lib_name_list_init(sound_name_list *list, int32_t n)
{
    if (n < 1) {
        return 0;
    }
    sound_lib_name_list_clear(list);
    list->entries = malloc((size_t) n * sizeof(sound_name_entry));
    if (!list->entries) {
        return 0;
    }
    for (int32_t i = 0; i < n; i++) {
        sound_name_entry_construct(&list->entries[i]);
    }
    list->capacity = n;
    list->count = 0;
    return 1;
}

int32_t sound_lib_name_list_add(sound_name_list *list, const char *path, int32_t owned)
{
    int32_t index = list->count;
    if (list->count >= list->capacity) {
        return -1;
    }
    if (!list->entries) {
        return -1;
    }
    for (int32_t i = 0; i < list->count; i++) {
        if (list->entries[i].name && (strcmp(list->entries[i].name, path) == 0)) {
            return i;
        }
    }
    if (owned) {
        list->entries[index].name = _strdup(path);
    } else {
        list->entries[index].name = path;
    }
    list->entries[index].owned = owned;
    list->count++;
    return index;
}

int32_t sound_lib_init_digital(sound_lib_state *self)
{
    platform_audio_startup();
    int32_t bytes_per_sample = self->bits / BITS_PER_BYTE;
    self->format_tag = WAVE_FORMAT_PCM_TAG;
    self->format_channels = (uint16_t) self->channels;
    self->format_rate = self->rate;
    self->format_bytes_per_sec = self->rate * bytes_per_sample * self->channels;
    self->format_block_align = (uint16_t) (bytes_per_sample * self->channels);
    self->format_bits = (uint16_t) self->bits;
    if (platform_audio_open(&self->driver, &self->format_tag)) {
        platform_debug_output(str_wave_out_failed);
        return 0;
    }
    if (!sound_lib_alloc_sample_handles(self, SOUND_LIB_SAMPLES)) {
        return 0;
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        sound_lib_channel_reset(self, i);
    }
    for (int32_t i = 0; i < self->sample_count; i++) {
        self->slots[i].name_index = -1;
        self->slots[i].file = NULL;
        self->slots[i].users = 0;
    }
    return 1;
}
