// The tables of video/video_data.c.

#ifndef VIDEO_VIDEO_DATA_H
#define VIDEO_VIDEO_DATA_H

#include "platform/platform_display.h"
#include "platform/platform_video.h"
#include "video/smacker.h"

#include <stdint.h>

extern int32_t intermezzo_type;

extern platform_surface *display_front_surface;

extern uint32_t video_buffer_flags;

extern int32_t video_start_window;

extern int32_t video_y;

extern int32_t video_x;

extern char video_file[64];


extern platform_clip *video_smack;

extern int32_t video_system_mode;

#endif
