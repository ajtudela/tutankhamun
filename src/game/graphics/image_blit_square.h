// The square raw image blitters.

#ifndef GRAPHICS_IMAGE_BLIT_SQUARE_H
#define GRAPHICS_IMAGE_BLIT_SQUARE_H

#include <stdint.h>

void image_blit_square16(const uint32_t *data);
void image_blit_square24(const uint32_t *data);
void image_blit_square32(const uint32_t *data);

#endif
