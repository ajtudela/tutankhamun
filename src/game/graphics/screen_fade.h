// The screen fade object: steps left, grey level, frame size, pixel format, mode, jitter, and the pixels of the front
// and back frame.

#ifndef GRAPHICS_SCREEN_FADE_H
#define GRAPHICS_SCREEN_FADE_H

#include <stdint.h>

typedef struct {
    int32_t steps;
    int32_t level;
    int32_t width;
    int32_t height;
    int32_t format;
    int32_t mode;
    uint8_t jitter;
    uint8_t pad[3];
    uint8_t reserved_1[8];
    uint16_t *pixels[2];
    uint8_t reserved_2[4];
} screen_fade;

void screen_fade_set_frame(screen_fade *fade, uint16_t *pixels, int32_t width, int32_t height);
void screen_fade_step(screen_fade *fade, int32_t index);
void screen_fade_start(screen_fade *fade, int32_t index, int32_t steps, int32_t level, int32_t jitter, int32_t mode);

#endif
