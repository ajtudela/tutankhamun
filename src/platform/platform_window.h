// The platform's window, the desktop's size, colours and palette, and the loading text drawn on the desktop; the play
// mode makes its own SDL window instead.

#ifndef PLATFORM_WINDOW_H
#define PLATFORM_WINDOW_H

#include "platform/platform_rect.h"

#include <stdbool.h>
#include <stdint.h>

void platform_window_update(void *window);
void platform_window_set_focus(void *window);
void platform_window_redraw(void *window);
void platform_window_minimize(void *window);
void platform_window_raise(void *window);
bool platform_window_is_foreground(void *window);
bool platform_window_is_minimized(void *window);
void platform_window_validate(void *window);
void platform_window_screen_rect(void *window, platform_rect *rect);
void platform_window_outer_size(void *window, int32_t width, int32_t height, int32_t *outer_width, int32_t *outer_height);
void platform_window_move(void *window, int32_t x, int32_t y, int32_t width, int32_t height);
void platform_window_resize(void *window, int32_t width, int32_t height);

void *platform_desktop_text_begin(void);
void platform_desktop_text_size(void *dc, const char *text, int32_t length, int32_t *width, int32_t *height);
void platform_desktop_text_draw(void *dc, int32_t x, int32_t y, const char *text, int32_t length, uint32_t background);
void platform_desktop_text_end(void *dc);

int32_t platform_desktop_width(void);
int32_t platform_desktop_height(void);
int32_t platform_desktop_color_depth(void);
void platform_desktop_palette(void *entries, int32_t count);

#endif
