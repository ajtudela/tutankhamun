// The game's 16-bit screen handed to the display (platform/screen.c): its pixel format, and the update of the window's
// texture from the game's buffer.

#ifndef APP_APP_SCREEN_H
#define APP_APP_SCREEN_H

#include <stdint.h>

int32_t app_screen_is_565(void);
void app_screen_update(void);

#endif
