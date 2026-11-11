// The screen as a PNG in memory (screenshot_png.c), for the bug report: 24-bit RGB, the image data in stored
// (uncompressed) deflate blocks, the screen halved, thirded... until the pixels fit in 3 MB (the report service takes a
// PNG of up to 4 MB).

#ifndef GRAPHICS_SCREENSHOT_PNG_H
#define GRAPHICS_SCREENSHOT_PNG_H

#include <stddef.h>
#include <stdint.h>

// a malloc'ed PNG in *png, 0 when there is no screen or no memory
int32_t graphics_screen_png(uint8_t **png, size_t *size);

#endif
