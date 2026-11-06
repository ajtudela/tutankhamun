// The functions of image_buttons.c that other files call.

#ifndef GRAPHICS_IMAGE_BUTTONS_H
#define GRAPHICS_IMAGE_BUTTONS_H

#include <stdint.h>

int32_t image_buttons_handle_mouse(int32_t x, int32_t y, void *buttons, int32_t count);
int32_t image_buttons_draw(int32_t x, int32_t y, void *buttons, int32_t count);

#endif
