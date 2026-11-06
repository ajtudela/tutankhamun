// The play mode (play.c), with SDL2 and SDL2_mixer linked in statically: running the game in its window until it is
// closed, the platform and media functions it answers, full screen around a video and the start-up display mode. What
// it needs from the rest of the program comes in one structure, which keeps it apart from the game's headers.

#ifndef APP_PLAY_H
#define APP_PLAY_H

#include "app/image.h"

typedef struct {
    void (*log)(const char *format, ...);
    void (*run_tick)(void);
    void (*run_frame)(void);
} play_host;

void play_bind(const play_host *host);

typedef struct {
    int32_t width;
    int32_t height;
    int32_t fullscreen;
} play_display_request;

int32_t play_display_prepare(const play_display_request *command_line);
int32_t play_run(const char *save);
void play_load_save(const char *path);

int32_t play_fullscreen_for_video(void);
void play_fullscreen_after_video(void);

extern void (*play_log)(const char *format, ...);

#endif
