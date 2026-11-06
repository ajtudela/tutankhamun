// The functions of arrow_buttons.c that other files call.

#ifndef GRAPHICS_ARROW_BUTTONS_H
#define GRAPHICS_ARROW_BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

int32_t arrow_buttons_handle_mouse(int32_t x, int32_t y, void *buttons, int32_t count);
void arrow_buttons_draw(int32_t x, int32_t y, const void *buttons, int32_t count);
void image_buttons_set_tooltip(int32_t x, int32_t y, const void *buttons, int32_t count, int32_t base);
int32_t arrow_buttons_get_focus(int32_t x, int32_t y, const void *buttons, int32_t count, int32_t base);
bool rotate_button_handle_mouse(void);

#endif
