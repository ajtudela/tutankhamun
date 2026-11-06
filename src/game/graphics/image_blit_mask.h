// The colour-masked, mirrored raw, one-colour clipped and two-pixel blitters, and image_draw_masked.

#ifndef GRAPHICS_IMAGE_BLIT_MASK_H
#define GRAPHICS_IMAGE_BLIT_MASK_H

#include <stdint.h>

void image_blit_raw_mask(const uint8_t *src, int32_t mask);
void image_blit_raw_mask_clipped(const uint8_t *src, int32_t mask);
void image_blit_compressed_mask(const uint8_t *data, int32_t mask);
void image_blit_compressed_mask_left(const uint8_t *data, int32_t mask);
void image_blit_compressed_mask_right(const uint8_t *data, int32_t mask);
void image_blit_compressed_color_left(const uint8_t *data);
void image_blit_compressed_color_right(const uint8_t *data);
void image_blit_raw_mirrored(const uint8_t *src);
void image_blit_raw_mirrored_clipped(const uint8_t *src);
void image_blit_two_pixels(const uint8_t *src);
int32_t image_draw_masked(int32_t image_id, int32_t x, int32_t y, int32_t mask);

#endif
