// The game window's display mode: the screen size and the drawing area of a mode, the window resized and centred,
// pause and resume, and the display surfaces restored after they were lost.

#include "platform/platform_win.h"

#include <stdint.h>
#include "graphics/graphics_data.h"
#include "platform/platform_data.h"
#include "platform/platform_display.h"
#include "platform/platform_window.h"
#include "platform/status_log.h"
#include "video/video_data.h"
#include "window/display_resolution_list.h"
#include "window/options_windows.h"
#include "window/window_data.h"

enum platform_layout {
    UI_WIDTH = 640,
    UI_HEIGHT = 480,
    SCREEN_BYTES_PER_PIXEL = 2,
};

int32_t platform_set_screen_size(int32_t mode)
{
    if (mode == DISPLAY_MODE_640X480) {
        g_screen_width = 640;
        g_screen_height = 480;
    } else if (mode == DISPLAY_MODE_800X600) {
        g_screen_width = 800;
        g_screen_height = 600;
    } else if (mode == DISPLAY_MODE_1024X768) {
        g_screen_width = 1024;
        g_screen_height = 768;
    }
    display_resolution_apply_custom();
    return mode;
}

void platform_set_display_mode(int32_t mode)
{
    platform_set_screen_size(mode);
    settings_data.display_mode = mode;
    int32_t width = g_screen_width;
    int32_t height = g_screen_height;
    display_width = width;
    ui_origin_x = (width - UI_WIDTH) / 2;
    ui_origin_y = (height - UI_HEIGHT) / 2;
    g_screen_pitch = width * SCREEN_BYTES_PER_PIXEL;
    screen_height = height;
    g_clip_x0 = 0;
    g_clip_x1 = width;
    g_clip_y0 = 0;
    g_clip_y1 = height;
    int32_t outer_width;
    int32_t outer_height;
    platform_window_outer_size(platform_main_window, width, height, &outer_width, &outer_height);
    window_pos_x = (desktop_width - g_screen_width) / 2;
    window_pos_y = (desktop_height - g_screen_height) / 2;
    platform_window_resize(platform_main_window, outer_width, outer_height);
    platform_window_update(platform_main_window);
    platform_window_set_focus(platform_main_window);
}

void platform_pause(void)
{
    log_info_text("OK :Paused.  ", 0);
    app_paused = 1;
    platform_window_redraw(platform_main_window);
}

void platform_resume(void)
{
    if (platform_window_is_foreground(platform_main_window)) {
        app_paused = 0;
    }
}

int32_t platform_restore_surfaces(void)
{
    if (display_front_surface == 0 || platform_surface_restore(display_front_surface) != PLATFORM_SURFACE_OK) {
        log_info_text("ERR:Can't restore front buffer.  ", 0);
        return 0;
    }
    if (display_back_surface == 0 || platform_surface_restore(display_back_surface) != PLATFORM_SURFACE_OK) {
        log_info_text("ERR:Can't restore back buffer.  ", 0);
        return 0;
    }
    return 1;
}
