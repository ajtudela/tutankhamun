// The game's 16-bit screen handed to the display: the program reads the game's buffer, its size and its pixel format
// and gives them to the SDL present (platform/screen.c), which knows nothing of the game.

#include "app/app_screen.h"

#include "building/building_data.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "platform/screen.h"
#include "window/window_data.h"

int32_t app_screen_is_565(void)
{
    return display_mode == DISPLAY_MODE_RGB565;
}

void app_screen_update(void)
{
    platform_screen_update(screen_rgb565_ptr, g_screen_width, g_screen_height, app_screen_is_565());
}
