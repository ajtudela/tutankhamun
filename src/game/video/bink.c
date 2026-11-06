// The Bink player (the intro, the period videos, the grand victory): video_play opens a file of the game directory and
// runs its own modal loop in window WINDOW_VIDEO, switching a full screen to display mode 1 around it; a key, a system key or a left click ends the video. The frames are copied to the primary surface; the movie
// itself is decoded by the platform video player.

#include "video/bink.h"

#include "city/message.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "platform/platform_display.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "platform/platform_system.h"
#include "platform/platform_time.h"
#include "platform/platform_video.h"
#include "platform/platform_win.h"
#include "platform/status_log.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "video/video_data.h"
#include "window/options_buttons.h"
#include "window/window_data.h"
#include "window/window_id.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum bink_player_layout {
    DISPLAY_MODE_VIDEO = 1,
};

video_player *video_bink_init(video_player *self, void *window)
{
    self->window = window;
    self->done = 1;
    self->bink = 0;
    return self;
}

static void close_handles(video_player *self)
{
    if (self->bink) {
        platform_movie_close(self->bink);
        self->bink = 0;
    }
}

void video_bink_close(video_player *self)
{
    close_handles(self);
}

bool video_bink_end(video_player *self)
{
    window_current_id = (uint8_t) self->saved_window;
    self->done = 1;
    window_redraw_request = 1;
    close_handles(self);
    return 1;
}

bool video_bink_blit(video_player *self)
{
    if (!self->bink) {
        return 0;
    }
    if (self->done) {
        return 0;
    }
    platform_frame frame;
    int32_t result = platform_surface_lock(display_front_surface, &frame);
    while (result == PLATFORM_SURFACE_LOST) {
        if (platform_surface_restore(display_front_surface) != PLATFORM_SURFACE_OK) {
            return 1;
        }
        result = platform_surface_lock(display_front_surface, &frame);
    }
    if (result != PLATFORM_SURFACE_OK) {
        log_info("ERR:Bink surface lock error:", 0, result - 1);
        return 1;
    }
    int32_t x = self->x + window_screen_rect.left;
    int32_t y = self->y + window_screen_rect.top;
    platform_movie_copy(self->bink, frame.pixels + (frame.pitch * y) + (x * 2), frame.pitch);
    platform_surface_unlock(display_front_surface);
    return 1;
}

static void video_bink_display_switch(int32_t mode)
{
    g_skip_refresh = 1;
    graphics_display_shutdown(1);
    platform_set_display_mode(mode);
    graphics_screen_resize();
    city_warning_clear_all();
    graphics_begin();
    g_skip_refresh = 0;
    display_mode_confirm_open();
    window_redraw_request = 1;
}

static int32_t message_ends_video(const platform_message *msg)
{
    return (msg->message == PLATFORM_MSG_KEY_DOWN) || (msg->message == PLATFORM_MSG_SYS_KEY_DOWN) || (msg->message == PLATFORM_MSG_LEFT_DOWN);
}

static void finish_video(video_player *self, int32_t fullscreen, int32_t mode)
{
    video_bink_end(self);
    if (fullscreen) {
        video_bink_display_switch(mode);
    }
    platform_cursor_show(1);
}

static bool movie_has_frames_left(const video_player *self)
{
    platform_video_info info;
    platform_movie_info(self->bink, &info);
    return info.frame < info.frames;
}

void video_bink_play_loop(video_player *self)
{
    platform_message msg;
    if (video_player_ptr == 0) {
        return;
    }
    graphics_clear_surfaces();
    int32_t fullscreen = settings_data.fullscreen != 0;
    int32_t mode = settings_data.display_mode;
    if (fullscreen) {
        video_bink_display_switch(DISPLAY_MODE_VIDEO);
    }
    platform_video_info info;
    platform_movie_info(self->bink, &info);
    self->x = (int32_t) ((uint32_t) (g_screen_width - (int32_t) info.width) >> 1);
    self->y = (int32_t) ((uint32_t) (g_screen_height - (int32_t) info.height) >> 1);
    platform_cursor_show(0);
    while (movie_has_frames_left(self)) {
        platform_tick_count();
        platform_movie_decode_frame(self->bink);
        platform_movie_next_frame(self->bink);
        video_bink_blit(self);
        do {
            const char *error = platform_movie_error();
            if (strlen(error) != 0) {
                log_info_text("ERR: Bink error: ", error);
            }
            while (platform_message_peek(&msg, 0, 0, true)) {
                if (message_ends_video(&msg)) {
                    finish_video(self, fullscreen, mode);
                    return;
                }
                platform_message_dispatch(&msg);
                platform_movie_service(self->bink);
            }
            platform_movie_service(self->bink);
        } while (platform_movie_wait(self->bink));
    }
    finish_video(self, fullscreen, mode);
}

static void stop_streams_for_video(void)
{
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_SPEECH);
    platform_movie_use_sound(sound_lib_manager->driver);
}

bool video_play(video_player *self, const char *file, int32_t x, int32_t y)
{
    if (self->bink) {
        platform_movie_close(self->bink);
    }
    if (sound_lib_manager) {
        stop_streams_for_video();
    }
    self->bink = platform_movie_open(file);
    if (!self->bink) {
        platform_debug_output("Unable to load BINK!\n");
        return 0;
    }
    self->saved_window = window_current_id;
    window_current_id = WINDOW_VIDEO;
    self->done = 0;
    window_redraw_request = 0;
    self->y = y;
    self->x = x;
    video_bink_play_loop(self);
    return 1;
}
