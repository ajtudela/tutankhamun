// Presenting the back buffer at most every REFRESH_PERIOD_MS (unless forced), with the hailstorm flash and the
// sidebar darkening in the city windows, and the line primitives (vertical, horizontal and Bresenham lines,
// clipped to the screen, not to the clip rectangle).

#include "graphics/graphics_present.h"
#include "game/game_data.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/image_blit.h"
#include "graphics/screen_fade.h"
#include "platform/platform_time.h"
#include "widget/sidebar.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>

int32_t line_error;

int32_t line_end_x;

int32_t line_end_y;

int32_t line_start_x;

int32_t line_start_y;

int32_t line_dx;

int32_t line_dy;

int32_t line_x_dir;

int32_t line_y_dir;

uint32_t g_last_present_ms;

enum refresh_timing {
    REFRESH_PERIOD_MS = 50,
    HAILSTORM_FLASH_PERIOD = 136,
    HAILSTORM_FLASH_STEPS = 8,
    HAILSTORM_FLASH_FADE_STEPS = 15,
};

enum line_walk {
    LINE_WALK_X = 0,
    LINE_WALK_Y = 1,
};

static int32_t refresh_sidebar_window(int32_t window_id)
{
    switch (window_id) {
        case WINDOW_CITY:
        case WINDOW_TOP_MENU:
        case WINDOW_CITY_LEGION_SELECTED:
        case WINDOW_CITY_TRANSPORT_SELECTED:
        case WINDOW_CITY_WARSHIP_SELECTED:
        case WINDOW_CITY_LEGION_ORDER:
            return 1;
        default:
            return 0;
    }
}

int32_t graphics_millis_since_last(void)
{
    uint32_t now = platform_time_ms();
    int32_t delta = (int32_t) (now - g_last_present_ms);
    g_last_present_ms = now;
    return delta;
}

void graphics_draw_line_setup(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t dx;
    int32_t dy;
    int32_t start_x;
    int32_t start_y;
    int32_t end_x;
    int32_t end_y;

    line_y_dir = 1;
    line_x_dir = 1;
    if (x1 > x2) {
        dx = x1 - x2;
    } else {
        dx = x2 - x1;
    }
    line_dx = dx;
    if (y1 > y2) {
        dy = y1 - y2;
    } else {
        dy = y2 - y1;
    }
    line_dy = dy;

    if (dy > dx) {
        if (y1 > y2) {
            start_y = y2;
            end_y = y1;
            start_x = x2;
            end_x = x1;
        } else {
            start_y = y1;
            end_y = y2;
            start_x = x1;
            end_x = x2;
        }
    } else {
        if (x1 > x2) {
            start_y = y2;
            end_y = y1;
            start_x = x2;
            end_x = x1;
        } else {
            start_y = y1;
            end_y = y2;
            start_x = x1;
            end_x = x2;
        }
    }

    line_start_y = start_y;
    line_end_y = end_y;
    line_start_x = start_x;
    line_end_x = end_x;
    if (end_x < start_x) {
        line_x_dir = -1;
    }
    if (end_y < start_y) {
        line_y_dir = -1;
    }
}

void graphics_draw_line_step(int32_t walk_y)
{
    int32_t err = line_error;
    int32_t add;
    if (walk_y == LINE_WALK_X) {
        add = line_dy;
        if (err >= 0) {
            add = line_dy - line_dx;
        }
    } else {
        add = line_dx;
        if (err >= 0) {
            add = line_dx - line_dy;
        }
    }
    line_error = err + (add * 2);
}

void graphics_draw_pixel_clipped(int32_t x, int32_t y, int32_t color)
{
    if (x >= 0 && x < display_width && y >= 0 && y < screen_height) {
        graphics_set_pixel(x, y, color);
    }
}

void graphics_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t color)
{
    graphics_draw_line_setup(x1, y1, x2, y2);

    if (x1 == x2) {
        for (int32_t y = line_start_y; y <= line_end_y; y++) {
            graphics_draw_pixel_clipped(x1, y, color);
        }
        return;
    }

    if (y1 == y2) {
        for (int32_t x = line_start_x; x <= line_end_x; x++) {
            graphics_draw_pixel_clipped(x, y1, color);
        }
        return;
    }

    if (line_dx < line_dy) {
        line_error = (line_dx * 2) - line_dy;
        int32_t x = line_start_x;
        for (int32_t y = line_start_y; y <= line_end_y; y++) {
            graphics_draw_pixel_clipped(x, y, color);
            graphics_draw_line_step(LINE_WALK_Y);
            line_dy--;
            if (line_error >= 1) {
                line_dx--;
                x += line_x_dir;
            }
        }
        return;
    }

    line_error = (line_dy * 2) - line_dx;
    int32_t y = line_start_y;
    for (int32_t x = line_start_x; x <= line_end_x; x++) {
        graphics_draw_pixel_clipped(x, y, color);
        graphics_draw_line_step(LINE_WALK_X);
        line_dx--;
        if (line_error >= 1) {
            line_dy--;
            y += line_y_dir;
        }
    }
}

void graphics_refresh_screen(int32_t force_rate)
{
    int32_t delta;
    int32_t i;

    if (g_skip_refresh == 1) {
        return;
    }

    if (window_current_id != WINDOW_MESSAGE_DIALOG && city_hailstorm_countdown > 0 && (city_hailstorm_countdown % HAILSTORM_FLASH_PERIOD) == 0) {
        screen_fade_start(&fade_object, 1, HAILSTORM_FLASH_FADE_STEPS, 0, 1, 0);
        graphics_present_frame();
        for (i = 0; i < HAILSTORM_FLASH_STEPS; i++) {
            screen_fade_step(&fade_object, 1);
            graphics_present_frame();
        }
        return;
    }

    delta = graphics_millis_since_last();
    g_refresh_accum_ms += delta;
    if (force_rate == 1) {
        g_refresh_accum_ms = REFRESH_PERIOD_MS;
    }
    if (g_force_refresh == 0 && g_refresh_accum_ms < REFRESH_PERIOD_MS) {
        return;
    }

    g_refresh_accum_ms = 0;
    g_force_refresh = 0;

    if (window_current_id != WINDOW_MESSAGE_DIALOG && city_hailstorm_countdown > 0 && refresh_sidebar_window(window_current_id)) {
        widget_sidebar_draw_background();
        screen_fade_start(&fade_object, 1, 1, 0, 0, 1);
    }
    graphics_present_frame();
}
