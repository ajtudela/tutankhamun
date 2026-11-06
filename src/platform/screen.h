// The SDL present of the play mode, modelled on Julius. Source, licence and changes: the header comment of
// screen.c (modelled on Julius, AGPL-3.0).
#ifndef PLATFORM_SCREEN_H
#define PLATFORM_SCREEN_H

#include <stdint.h>

int32_t platform_screen_create(const char *title, int32_t width, int32_t height);
void platform_screen_destroy(void);

int32_t platform_screen_resize(int32_t width, int32_t height);

void platform_screen_clear(void);
void platform_screen_update(void);
void platform_screen_render(void);

void *platform_screen_window(void);

#endif
