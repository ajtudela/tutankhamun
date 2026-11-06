// The platform video players on the media library's Bink and Smacker functions: a movie is a Bink handle, a clip a
// Smack handle, and the info reads the fields of the libraries' handles. The clip format picks Smacker's 555 or 565
// buffer; a clip opened with sound plays its first sound track.

#include "platform/platform_video.h"

#include "app/media_lib.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

media_video_lib media_video;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t frames;
    uint32_t frame_num;
    uint32_t last_frame;
    uint32_t frame_num2;
} bink_handle;

typedef struct {
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t frames;
    uint8_t unread[0x374 - 0x10];
    uint32_t frame_num;
} smack_handle;

static_assert(offsetof(bink_handle, last_frame) == 0x10 && offsetof(bink_handle, frame_num2) == 0x14, "bink_handle is the Bink library's handle layout");
static_assert(offsetof(smack_handle, frame_num) == 0x374, "smack_handle is the Smacker library's handle layout");

enum smack_flag {
    SMACK_OPEN_SOUND_TRACK_1 = 0x2000,
    SMACK_AUTO_EXTRA = -1,
    SMACK_BUFFER_555 = (int32_t) 0x80000000,
    SMACK_BUFFER_565 = (int32_t) 0xC0000000,
};

platform_movie *platform_movie_open(const char *path)
{
    return media_video.bink_open(path, 0);
}

void platform_movie_close(platform_movie *movie)
{
    media_video.bink_close(movie);
}

void platform_movie_info(const platform_movie *movie, platform_video_info *info)
{
    const bink_handle *bink = (const bink_handle *) movie;
    info->width = bink->width;
    info->height = bink->height;
    info->frame = bink->frame_num2;
    info->frames = bink->last_frame;
}

void platform_movie_decode_frame(platform_movie *movie)
{
    media_video.bink_do_frame(movie);
}

void platform_movie_next_frame(platform_movie *movie)
{
    media_video.bink_next_frame(movie);
}

bool platform_movie_wait(platform_movie *movie)
{
    return media_video.bink_wait(movie) != 0;
}

void platform_movie_service(platform_movie *movie)
{
    media_video.bink_service(movie);
}

const char *platform_movie_error(void)
{
    return media_video.bink_get_error();
}

void platform_movie_copy(platform_movie *movie, uint8_t *dest, int32_t pitch)
{
    const bink_handle *bink = (const bink_handle *) movie;
    uint32_t type = media_video.bink_dd_surface_type(NULL);
    media_video.bink_copy_to_buffer(movie, dest, pitch, bink->height, 0, 0, type);
}

void platform_movie_use_sound(void *driver)
{
    media_video.bink_set_sound_system((void *) media_video.bink_open_miles, (uint32_t) (uintptr_t) driver);
}

platform_clip *platform_clip_open(const char *path, bool with_sound)
{
    return media_video.smack_open(path, with_sound ? SMACK_OPEN_SOUND_TRACK_1 : 0, (uint32_t) SMACK_AUTO_EXTRA);
}

void platform_clip_close(platform_clip *clip)
{
    media_video.smack_close(clip);
}

void platform_clip_info(const platform_clip *clip, platform_video_info *info)
{
    const smack_handle *smack = (const smack_handle *) clip;
    info->width = smack->width;
    info->height = smack->height;
    info->frame = smack->frame_num;
    info->frames = smack->frames;
}

bool platform_clip_wait(platform_clip *clip)
{
    return media_video.smack_wait(clip) != 0;
}

void platform_clip_copy(platform_clip *clip, int32_t x, int32_t y, const platform_frame *frame, int32_t format)
{
    const smack_handle *smack = (const smack_handle *) clip;
    uint32_t flags = (uint32_t) (format == PLATFORM_CLIP_RGB565 ? SMACK_BUFFER_565 : SMACK_BUFFER_555);
    media_video.smack_to_buffer(clip, (uint32_t) x, (uint32_t) y, (uint32_t) frame->pitch, smack->height, frame->pixels, flags);
}

void platform_clip_decode_frame(platform_clip *clip)
{
    media_video.smack_do_frame(clip);
}

void platform_clip_next_frame(platform_clip *clip)
{
    media_video.smack_next_frame(clip);
}

void platform_clip_use_sound(uint32_t driver)
{
    media_video.smack_sound_use_mss(driver);
}
