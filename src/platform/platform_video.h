// The platform's video players: a movie played full screen frame by frame (the intro, the period videos, the grand
// victory) and the clips shown inside windows (message and victory videos). A movie or clip is opened from a file,
// decoded one frame at a time and copied into a frame buffer; its info gives the size, the current frame and the
// frame count. The play mode plays movies with its Bink decoder and has no clips.

#ifndef PLATFORM_PLATFORM_VIDEO_H
#define PLATFORM_PLATFORM_VIDEO_H

#include "platform/platform_display.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct platform_movie platform_movie;
typedef struct platform_clip platform_clip;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t frame;
    uint32_t frames;
} platform_video_info;

enum platform_clip_format {
    PLATFORM_CLIP_RGB555 = 0,
    PLATFORM_CLIP_RGB565 = 1,
};

platform_movie *platform_movie_open(const char *path);
void platform_movie_close(platform_movie *movie);
void platform_movie_info(const platform_movie *movie, platform_video_info *info);
void platform_movie_decode_frame(platform_movie *movie);
void platform_movie_next_frame(platform_movie *movie);
bool platform_movie_wait(platform_movie *movie);
void platform_movie_service(platform_movie *movie);
const char *platform_movie_error(void);
void platform_movie_copy(platform_movie *movie, uint8_t *dest, int32_t pitch);
void platform_movie_use_sound(void *driver);

platform_clip *platform_clip_open(const char *path, bool with_sound);
void platform_clip_close(platform_clip *clip);
void platform_clip_info(const platform_clip *clip, platform_video_info *info);
bool platform_clip_wait(platform_clip *clip);
void platform_clip_copy(platform_clip *clip, int32_t x, int32_t y, const platform_frame *frame, int32_t format);
void platform_clip_decode_frame(platform_clip *clip);
void platform_clip_next_frame(platform_clip *clip);
void platform_clip_use_sound(uint32_t driver);

#endif
