// The rectangle of the globals that hold one: window_screen_rect, the game window's client rectangle in screen
// coordinates. Its layout is Win32's RECT.

#ifndef PLATFORM_PLATFORM_RECT_H
#define PLATFORM_PLATFORM_RECT_H

#include <stdint.h>

typedef struct {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} platform_rect;

#endif
