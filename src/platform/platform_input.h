// The platform's mouse cursor and keyboard state. The play mode answers them from SDL.

#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

#include <stdbool.h>
#include <stdint.h>

enum platform_key {
    PLATFORM_KEY_BACKSPACE = 0x0008,
    PLATFORM_KEY_RETURN = 0x000D,
    PLATFORM_KEY_SHIFT = 0x0010,
    PLATFORM_KEY_CONTROL = 0x0011,
    PLATFORM_KEY_ALT = 0x0012,
    PLATFORM_KEY_ESCAPE = 0x001B,
    PLATFORM_KEY_PAGE_UP = 0x0021,
    PLATFORM_KEY_PAGE_DOWN = 0x0022,
    PLATFORM_KEY_END = 0x0023,
    PLATFORM_KEY_HOME = 0x0024,
    PLATFORM_KEY_LEFT = 0x0025,
    PLATFORM_KEY_UP = 0x0026,
    PLATFORM_KEY_RIGHT = 0x0027,
    PLATFORM_KEY_DOWN = 0x0028,
    PLATFORM_KEY_INSERT = 0x002D,
    PLATFORM_KEY_DELETE = 0x002E,
    PLATFORM_KEY_NUMPAD_1 = 0x0061,
    PLATFORM_KEY_NUMPAD_2 = 0x0062,
    PLATFORM_KEY_NUMPAD_3 = 0x0063,
    PLATFORM_KEY_NUMPAD_7 = 0x0067,
    PLATFORM_KEY_NUMPAD_8 = 0x0068,
    PLATFORM_KEY_MULTIPLY = 0x006A,
    PLATFORM_KEY_ADD = 0x006B,
    PLATFORM_KEY_F1 = 0x0070,
    PLATFORM_KEY_F2 = 0x0071,
    PLATFORM_KEY_F3 = 0x0072,
    PLATFORM_KEY_F4 = 0x0073,
    PLATFORM_KEY_F5 = 0x0074,
    PLATFORM_KEY_F6 = 0x0075,
    PLATFORM_KEY_F7 = 0x0076,
    PLATFORM_KEY_F8 = 0x0077,
    PLATFORM_KEY_F9 = 0x0078,
    PLATFORM_KEY_F10 = 0x0079,
    PLATFORM_KEY_F11 = 0x007A,
    PLATFORM_KEY_F12 = 0x007B,
};

int16_t platform_key_state(int32_t key);
bool platform_cursor_position(int32_t *x, int32_t *y);
void *platform_cursor_set(void *cursor);
int32_t platform_cursor_show(bool show);

#endif
