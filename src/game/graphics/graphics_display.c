// The display set-up, the present of the back buffer, the screen buffers redone after a resolution change, and the
// screen fade (the hailstorm flash and the sidebar darkening on the frame buffer's pixels).
//
// The play mode registers its display with the platform layer and the begin / shutdown go there. Without it there is
// no display: a begin reads the desktop (its colour depth may force full screen), logs that the display could not be
// enabled and leaves the screen refresh off.

#include "graphics/graphics_display.h"
#include "city/message.h"
#include "core/random.h"
#include "core/text.h"
#include "figure/figure_data.h"
#include "game/startup_load.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/screen_fade.h"
#include "graphics/screen_pixels.h"
#include "platform/platform_data.h"
#include "platform/platform_display.h"
#include "platform/platform_system.h"
#include "platform/platform_window.h"
#include "platform/status_log.h"
#include "video/video_data.h"
#include "widget/city_view.h"
#include "widget/city_viewport.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum display_limit {
    DESKTOP_BITS = 16,
    SYSTEM_PALETTE_ENTRIES = 256,
    WARNING_DESKTOP_NOT_16_BIT = 0x39,
    LOG_TEXT_SIZE = 100,
    FADE_TOP_ROWS = 30,
    FADE_SPARKLE_PERIOD = 128,
    FADE_MAX_STEPS = 15,
    FADE_MAX_GREY = 31,
    FADE_JITTER_RANGE = 16,
    FADE_JITTER_OFFSET = 8,
};

enum fade_format {
    FADE_FORMAT_NONE = -1,
    FADE_FORMAT_RGB555 = 0,
    FADE_FORMAT_RGB565 = 1,
};

int32_t graphics_clear_surfaces(void)
{
    platform_window_update(platform_main_window);
    if (platform_surface_fill(display_back_surface, 0) != PLATFORM_SURFACE_OK) {
        log_info(str_dd_fill_failed, 0, 0);
        return 0;
    }
    if (platform_surface_fill(display_front_surface, 0) != PLATFORM_SURFACE_OK) {
        log_info(str_dd_fill_failed, 0, 1);
        return 0;
    }
    return 1;
}

static int32_t read_desktop(void)
{
    settings_data.desktop_color_depth = platform_desktop_color_depth();
    desktop_width = platform_desktop_width();
    desktop_height = platform_desktop_height();
    int32_t palette = 1;
    if (g_screen_width > desktop_width) {
        if (settings_data.fullscreen == 0) {
            city_warning_show(WARNING_DESKTOP_NOT_16_BIT);
        }
        settings_data.fullscreen = 1;
    } else if (settings_data.fullscreen == 0) {
        if (settings_data.desktop_color_depth == DESKTOP_BITS) {
            palette = 0;
        } else {
            city_warning_show(WARNING_DESKTOP_NOT_16_BIT);
            log_info_text(str_dd_256_cols, 0);
            settings_data.fullscreen = 1;
        }
    }
    return palette;
}

int32_t graphics_screen_resize(void)
{
    sidebar_init_positions();
    image_buf_reset(&image_collection.external_buffer, 1);
    if (spare_screen) {
        free(spare_screen);
    }
    int32_t size = (g_screen_height * g_screen_width) * 2;
    if (size <= SPARE_SCREEN_MIN_BYTES) {
        size = SPARE_SCREEN_MIN_BYTES;
    }
    spare_screen = malloc(size);
    if (g_sidebar_collapsed == 1) {
        city_view_set_viewport_without_sidebar();
    } else {
        city_view_set_viewport_with_sidebar();
    }
    city_view_calculate_map_bounds();
    return city_view_check_camera_boundaries();
}

int32_t graphics_display_shutdown(int32_t release_display)
{
    if (platform_display_replaced()) {
        return platform_display_shutdown(release_display);
    }
    return 0;
}

int32_t graphics_begin(void)
{
    if (platform_display_replaced()) {
        return platform_display_begin();
    }
    g_skip_refresh = 1;
    if (read_desktop()) {
        platform_desktop_palette(system_palette, SYSTEM_PALETTE_ENTRIES);
    }
    log_info_text(str_dd_enable_error, 0);
    log_info_text(str_gfx_not_enabled, 0);
    log_info_text(str_gfx_begin_failed, 0);
    return 0;
}

int32_t graphics_present_frame(void)
{
    if (!display_front_surface || !display_back_surface) {
        return 0;
    }
    return platform_surface_present(display_front_surface, display_back_surface);
}

int32_t calc_bound(int32_t value, int32_t min, int32_t max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

void screen_fade_set_frame(screen_fade *fade, uint16_t *pixels, int32_t width, int32_t height)
{
    fade->pixels[0] = pixels;
    fade->pixels[1] = pixels;
    fade->width = width;
    fade->height = height;
    fade->format = display_mode == DISPLAY_MODE_RGB565 ? FADE_FORMAT_RGB565 : FADE_FORMAT_RGB555;
}

static void fade_to_grey(const screen_fade *fade, uint16_t *pixel, int32_t level, int32_t view_width)
{
    int32_t shift;
    if (fade->format == FADE_FORMAT_RGB565) {
        shift = PIXEL_565_GREEN_SHIFT;
    } else if (fade->format == FADE_FORMAT_RGB555) {
        shift = PIXEL_555_GREEN_SHIFT;
    } else {
        return;
    }
    for (int32_t y = 0; y < fade->height; y++) {
        for (int32_t x = 0; x < fade->width; x++, pixel++) {
            if (x < view_width && y >= FADE_TOP_ROWS) {
                uint32_t value = *pixel;
                int32_t grey = calc_bound((int32_t) (((value >> PIXEL_565_RED_SHIFT) + (value & PIXEL_BLUE_MASK)) >> 1) + level, 0, FADE_MAX_GREY);
                *pixel = (uint16_t) ((((grey << 5) | grey) << shift) | grey);
            }
        }
    }
}

static uint16_t half_bright(const screen_fade *fade, uint32_t value)
{
    if (fade->format == FADE_FORMAT_RGB555) {
        return (uint16_t) (((value >> 1) & 0x3DEF) | (value & 0x8000));
    }
    uint32_t blue_halved = (value & 0xFFE0) | ((value >> 1) & 0xF);
    uint32_t green_halved = ((blue_halved >> 1) & 0x3E0) | (blue_halved & 0xF81F);
    return (uint16_t) (((green_halved >> 1) & 0x7800) | (green_halved & 0x7FF));
}

static void fade_with_sparkles(const screen_fade *fade, uint16_t *pixel, int32_t view_width)
{
    for (int32_t y = 0; y < fade->height; y++) {
        random_generate_next();
        int32_t sparkle = random_generator_state.random1_15bit;
        for (int32_t x = 0; x < fade->width; x++, pixel++) {
            if (x >= view_width || y < FADE_TOP_ROWS) {
                continue;
            }
            if ((x + sparkle) % FADE_SPARKLE_PERIOD == 0) {
                *pixel = 0xFFFF;
                if (fade->format == FADE_FORMAT_RGB555) {
                    *pixel = 0x7FFF;
                }
            } else {
                *pixel = half_bright(fade, *pixel);
            }
        }
    }
}

void screen_fade_step(screen_fade *fade, int32_t index)
{
    if (fade->steps <= 0 || fade->format == FADE_FORMAT_NONE) {
        return;
    }
    int32_t level = fade->level;
    fade->steps--;
    if (fade->jitter) {
        random_generate_next();
        level = level + (random_generator_state.random1_15bit % FADE_JITTER_RANGE) - FADE_JITTER_OFFSET;
    }
    int32_t view_width = g_sidebar_collapsed ? city_view_width_collapsed : city_view_width_expanded;
    if (fade->mode == 0) {
        fade_to_grey(fade, fade->pixels[index], level, view_width);
    } else {
        fade_with_sparkles(fade, fade->pixels[index], view_width);
    }
}

void screen_fade_start(screen_fade *fade, int32_t index, int32_t steps, int32_t level, int32_t jitter, int32_t mode)
{
    fade->steps = calc_bound(steps, 0, FADE_MAX_STEPS);
    fade->level = level;
    fade->jitter = (uint8_t) jitter;
    fade->mode = mode;
    screen_fade_step(fade, index);
}
