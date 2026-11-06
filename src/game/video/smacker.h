// The Smacker videos inside windows (video/smacker.c).

#ifndef VIDEO_SMACKER_H
#define VIDEO_SMACKER_H

#include <stddef.h>
#include <stdint.h>

enum video_system_mode {
    VIDEO_SYSTEM_MODE_NONE = 0,
    VIDEO_SYSTEM_MODE_MESSAGE = 1,
    VIDEO_SYSTEM_MODE_FULL_SCREEN = 2,
};

enum victory_video {
    VICTORY_VIDEO_WIDTH = 400,
    VICTORY_VIDEO_HEIGHT = 292,
    FIRST_MISSION_WITH_WIN_GAME_VIDEO = 10,
};

int32_t video_start(const char *file, int32_t x, int32_t y, int32_t window);
int32_t video_stop(void);
void video_shutdown(void);
int32_t video_open(const char *file, int32_t x, int32_t y);
int32_t video_draw(void);
void video_end(void);

#endif
