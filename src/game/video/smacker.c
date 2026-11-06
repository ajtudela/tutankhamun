// The Smacker videos inside windows (message dialogs with a video, the victory video): video_start opens a video for
// a window, video_draw shows one frame per draw until the last frame, video_stop and video_end go back to the window
// before it. The clips are played by the platform video player; this install has no .smk files, so opening fails
// and the caller goes on in its window.

#include "video/smacker.h"

#include "core/text.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_present.h"
#include "platform/platform_display.h"
#include "platform/platform_system.h"
#include "platform/platform_video.h"
#include "platform/status_log.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "video/video_data.h"
#include "window/window_data.h"
#include "window/window_id.h"

#include <stdint.h>
#include <string.h>

int32_t video_sound_set;

static void log_memory_available(void)
{
    int32_t available = platform_memory_available();
    if (available < 0 || available == INT32_MAX) {
        log_info_text("MEM:Max alloc after = ", 0);
    } else {
        log_info("MEM:Max alloc after = ", 0, available);
    }
}

void video_end(void)
{
    window_current_id = (uint8_t) video_start_window;
    video_smk_playing = 0;
    if (video_system_mode == VIDEO_SYSTEM_MODE_FULL_SCREEN) {
        window_redraw_request = 1;
    }
    video_system_mode = VIDEO_SYSTEM_MODE_NONE;
}

static int32_t window_always_has_video_sound(int32_t window)
{
    return (window == WINDOW_CREDITS_VIDEO) || (window == WINDOW_LOGO);
}

int32_t video_open(const char *file, int32_t x, int32_t y)
{
    video_smk_playing = 0;
    if (video_sound_set == 0) {
        platform_clip_use_sound((uint32_t) sound_lib_manager->driver);
        video_sound_set = 1;
    }
    log_info_text("OK :SMK starting smack ", file);
    video_x = x;
    bool sound = false;
    video_smack = 0;
    video_y = y;
    video_buffer_flags = (display_pixel_format != DISPLAY_PIXEL_FORMAT_565) ? PLATFORM_CLIP_RGB555 : PLATFORM_CLIP_RGB565;
    strncpy(video_file, file, sizeof(video_file) - 1);
    video_file[sizeof(video_file) - 1] = 0;
    if (sound_lib_manager->driver != 0) {
        sound = true;
        if (!window_always_has_video_sound(video_start_window) && (settings_data.sound_effects_enabled == 0)) {
            sound = false;
        }
    }
    video_smack = platform_clip_open(video_file, sound);
    if (video_smack == 0) {
        log_info_text("ERR:SMK Could not open ", video_file);
        log_memory_available();
        return 0;
    }
    graphics_refresh_screen(1);
    video_smk_playing = 1;
    video_draw();
    return 1;
}

static void copy_frame_to(platform_surface *surface, const platform_frame *frame)
{
    platform_clip_copy(video_smack, video_x, video_y, frame, (int32_t) video_buffer_flags);
    platform_clip_decode_frame(video_smack);
    platform_surface_unlock(surface);
}

static void draw_frame_to_back_buffer(void)
{
    platform_frame frame;
    if (platform_surface_lock(display_back_surface, &frame) == PLATFORM_SURFACE_OK) {
        copy_frame_to(display_back_surface, &frame);
    }
}

static void close_smack(void)
{
    if (video_smack) {
        platform_clip_close(video_smack);
        video_smack = 0;
    }
}

int32_t video_draw(void)
{
    if (video_smk_playing == 0) {
        return 0;
    }
    if (platform_clip_wait(video_smack)) {
        return 0;
    }
    draw_frame_to_back_buffer();
    platform_video_info info;
    platform_clip_info(video_smack, &info);
    if (info.frame < (info.frames - 1)) {
        platform_clip_next_frame(video_smack);
        return 1;
    }
    close_smack();
    log_info_text("OK :SMK natural end of ", video_file);
    log_memory_available();
    video_end();
    return 1;
}

int32_t video_stop(void)
{
    if (video_smk_playing == 0) {
        return 0;
    }
    close_smack();
    log_info_text("OK :SMK user ends ", video_file);
    log_memory_available();
    video_end();
    return 0;
}

void video_shutdown(void)
{
    if (video_smk_playing == 0) {
        return;
    }
    close_smack();
}

int32_t video_start(const char *file, int32_t x, int32_t y, int32_t window)
{
    if (video_system_mode == VIDEO_SYSTEM_MODE_NONE) {
        return 0;
    }
    video_start_window = window;
    if (video_open(file, x, y) == 0) {
        window_current_id = (uint8_t) window;
        if (video_system_mode == VIDEO_SYSTEM_MODE_FULL_SCREEN) {
            window_redraw_request = 1;
        }
        return 0;
    }
    window_current_id = WINDOW_MESSAGE_VIDEO;
    return 1;
}
