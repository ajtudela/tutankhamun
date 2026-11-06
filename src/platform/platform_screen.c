// The game's 16-bit screen buffer handed to the display: an external buffer (the play mode's window) when one is set,
// otherwise the game's own, with its width and height.

#include "platform/platform_screen.h"

#include "building/building_data.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "window/window_data.h"

static struct {
    int32_t active;
    uint16_t *buffer;
    int32_t width;
    int32_t height;
} external;

void platform_screen_set_external(uint16_t *buffer, int32_t width, int32_t height)
{
    external.active = 1;
    external.buffer = buffer;
    external.width = width;
    external.height = height;
}

void platform_screen_clear_external(void)
{
    external.active = 0;
    external.buffer = 0;
    external.width = 0;
    external.height = 0;
}

uint16_t *platform_screen_buffer(void)
{
    if (external.active) {
        return external.buffer;
    }
    return screen_rgb565_ptr;
}

int32_t platform_screen_width(void)
{
    if (external.active) {
        return external.width;
    }
    return g_screen_width;
}

int32_t platform_screen_height(void)
{
    if (external.active) {
        return external.height;
    }
    return g_screen_height;
}

int32_t platform_screen_is_565(void)
{
    if (external.active) {
        return 1;
    }
    return display_mode == DISPLAY_MODE_RGB565;
}
