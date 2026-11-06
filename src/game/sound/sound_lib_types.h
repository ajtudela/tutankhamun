// The records of the sound wrapper: the manager (sound_manager), the sample requests, the state of each Miles sample,
// the loaded sample files, the name list and the 3D samples.

#ifndef SOUND_SOUND_LIB_TYPES_H
#define SOUND_SOUND_LIB_TYPES_H

#include <stdint.h>

typedef struct sound_lib_state sound_lib_state;

typedef struct {
    int32_t id;
    int32_t name_index;
    int32_t volume;
    int32_t pan;
    int32_t loop_count;
} sound_request;

typedef struct {
    sound_request request;
    int32_t slot;
} sound_sample_state;

typedef struct {
    int32_t name_index;
    void *file;
    int32_t users;
} sound_sample_slot;

typedef struct {
    const char *name;
    int32_t owned;
} sound_name_entry;

typedef struct {
    int32_t id;
    int32_t name_index;
    int32_t volume;
    int32_t pan;
    int32_t loop_count;
    int32_t reserved_1;
    int32_t slot;
    int32_t position[3];
} sound_3d_state;

typedef struct {
    int32_t capacity;
    int32_t count;
    sound_name_entry *entries;
} sound_name_list;

struct sound_lib_state {
    sound_name_list name_list;
    float environment_reverb[26][3];
    int32_t reserved_1;
    int32_t reserved_2;
    int32_t stream0_volume;
    int32_t spare_volume;
    int32_t stream1_volume;
    int32_t stream2_volume;
    uint8_t reserved_3[8];
    int32_t provider_3d_kind;
    int32_t reverb_on;
    float reverb[3];
    void *driver;
    uint16_t format_tag;
    uint16_t format_channels;
    int32_t format_rate;
    int32_t format_bytes_per_sec;
    uint16_t format_block_align;
    uint16_t format_bits;
    int32_t rate;
    int32_t bits;
    int32_t channels;
    int32_t sample_count;
    int32_t *handles;
    int32_t *spare_handles;
    uint8_t reserved_4[4];
    sound_sample_state *states;
    sound_sample_slot *slots;
    sound_request request;
    sound_request spare_request;
    int32_t streams[3];
    int32_t count_3d;
    int32_t provider_3d;
    int32_t eax_available;
    int32_t *handles_3d;
    sound_3d_state *states_3d;
    sound_sample_slot *slots_3d;
    int32_t eax_environment;
    int32_t disabled;
};

#endif
