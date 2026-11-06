// The functions of generic_buttons.c that other files call.

#ifndef GRAPHICS_GENERIC_BUTTONS_H
#define GRAPHICS_GENERIC_BUTTONS_H

#include <stdint.h>

int32_t generic_buttons_get_focus(int32_t x, int32_t y, const void *buttons, int32_t count);
int32_t generic_buttons_handle_mouse(int32_t x, int32_t y, const void *buttons, int32_t count);

#endif
