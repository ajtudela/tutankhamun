// The Bink player object of bink.c: the window before the video, done, the window, the movie and the position on
// the screen.

#ifndef VIDEO_BINK_H
#define VIDEO_BINK_H

#include "platform/platform_video.h"
#include "video/video_player_types.h"

#include <stdbool.h>
#include <stdint.h>

struct video_player {
    uint32_t saved_window;
    uint8_t done;
    uint8_t pad[3];
    void *window;
    uint8_t reserved_1[4];
    platform_movie *bink;
    int32_t x;
    int32_t y;
};

video_player *video_bink_init(video_player *self, void *window);
void video_bink_close(video_player *self);
bool video_bink_end(video_player *self);
bool video_bink_blit(video_player *self);
void video_bink_play_loop(video_player *self);
bool video_play(video_player *self, const char *file, int32_t x, int32_t y);

#endif
