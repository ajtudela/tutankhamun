// The platform sound and video libraries: the Miles and Bink / Smacker functions with the libraries' calling
// convention and arguments, which the platform audio and video call (media_audio.c, media_video.c). The play mode
// fills them with its own implementations before the game runs.

#ifndef APP_MEDIA_LIB_H
#define APP_MEDIA_LIB_H

#include <stdint.h>

enum media_sound_status {
    SMP_DONE = 2,
    SMP_PLAYING = 4,
};

typedef struct {
    void(__stdcall *ail_set_3d_position)(int32_t, int32_t, int32_t, int32_t);
    void(__stdcall *ail_start_3d_sample)(int32_t);
    int32_t(__stdcall *ail_set_3d_provider_preference)(int32_t, const char *, const void *);
    int32_t(__stdcall *ail_set_3d_sample_file)(int32_t, void *);
    void(__stdcall *ail_set_3d_sample_volume)(int32_t, int32_t);
    int32_t(__stdcall *ail_3d_sample_status)(int32_t);
    int32_t(__stdcall *ail_enumerate_3d_providers)(int32_t *, int32_t *, char **);
    int32_t(__stdcall *ail_open_3d_provider)(int32_t);
    void(__stdcall *ail_3d_provider_attribute)(int32_t, const char *, void *);
    int32_t(__stdcall *ail_stream_volume)(int32_t);
    int32_t(__stdcall *ail_open_stream)(void *, const char *, int32_t);
    void(__stdcall *ail_3d_update_position)(int32_t, int32_t);
    int32_t(__stdcall *ail_stream_status)(int32_t);
    void(__stdcall *ail_set_stream_volume)(int32_t, int32_t);
    void(__stdcall *ail_set_sample_loop_count)(int32_t, int32_t);
    void(__stdcall *ail_set_sample_pan)(int32_t, int32_t);
    void(__stdcall *ail_set_3d_orientation)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
    void(__stdcall *ail_set_3d_sample_loop_count)(int32_t, int32_t);
    void(__stdcall *ail_start_sample)(int32_t);
    void *(__stdcall *ail_file_read)(const char *, void *);
    void(__stdcall *ail_init_sample)(int32_t);
    int32_t(__stdcall *ail_set_sample_file)(int32_t, void *, int32_t);
    int32_t(__stdcall *ail_sample_status)(int32_t);
    int32_t(__stdcall *ail_sample_loop_count)(int32_t);
    void(__stdcall *ail_end_3d_sample)(int32_t);
    void(__stdcall *ail_end_sample)(int32_t);
    void(__stdcall *ail_close_stream)(int32_t);
    void(__stdcall *ail_close_3d_provider)(int32_t);
    void(__stdcall *ail_shutdown)(void);
    int32_t(__stdcall *ail_startup)(void);
    int32_t(__stdcall *ail_set_preference)(uint32_t, int32_t);
    int32_t(__stdcall *ail_wave_out_open)(void **, void **, int32_t, void *);
    void(__stdcall *ail_release_3d_sample_handle)(int32_t);
    int32_t(__stdcall *ail_allocate_3d_sample_handle)(int32_t);
    void(__stdcall *ail_release_sample_handle)(int32_t);
    int32_t(__stdcall *ail_allocate_sample_handle)(void *);
    void(__stdcall *ail_set_sample_volume)(int32_t, int32_t);
    void(__stdcall *ail_set_sample_reverb)(int32_t, float, float, float);
    void(__stdcall *ail_set_stream_loop_count)(int32_t, int32_t);
    void(__stdcall *ail_start_stream)(int32_t);
    void(__stdcall *ail_mem_free_lock)(void *);
} media_sound_lib;

extern media_sound_lib media_sound;

typedef struct {
    int32_t(__stdcall *bink_do_frame)(void *);
    int32_t(__stdcall *bink_set_sound_system)(void *, uint32_t);
    void *(__stdcall *bink_open_miles)(uint32_t);
    int32_t(__stdcall *bink_wait)(void *);
    const char *(__stdcall *bink_get_error)(void);
    void(__stdcall *bink_next_frame)(void *);
    void *(__stdcall *bink_open)(const char *, uint32_t);
    int32_t(__stdcall *bink_service)(void *);
    uint32_t(__stdcall *bink_dd_surface_type)(void *);
    int32_t(__stdcall *bink_copy_to_buffer)(void *, void *, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void(__stdcall *bink_close)(void *);
    void(__stdcall *bink_buffer_close)(void *);
    void(__stdcall *smack_to_buffer)(void *, uint32_t, uint32_t, uint32_t, uint32_t, void *, uint32_t);
    int32_t(__stdcall *smack_wait)(void *);
    int32_t(__stdcall *smack_sound_use_mss)(uint32_t);
    void *(__stdcall *smack_open)(const char *, uint32_t, uint32_t);
    void(__stdcall *smack_close)(void *);
    int32_t(__stdcall *smack_do_frame)(void *);
    void(__stdcall *smack_next_frame)(void *);
} media_video_lib;

extern media_video_lib media_video;

extern const media_sound_lib miles_sdl_library;
extern const media_video_lib play_video_library;

void video_play_intro(void);

#endif
