// The back buffer blitters, the image collection lookups and the RLE run codes the blitters share.

#ifndef GRAPHICS_IMAGE_BLIT_H
#define GRAPHICS_IMAGE_BLIT_H

#include "graphics/image_collection.h"
#include <stdint.h>

enum rle_code {
    RLE_SKIP = 0xFF,
    RLE_UNROLLED_RUN_MAX = 15,
    RLE_UNROLLED_RUN_LONG = 16,
};

static inline int32_t rle_unrolled_run_length(uint8_t b)
{
    if ((b >= 1 && b < RLE_UNROLLED_RUN_MAX) || b == RLE_UNROLLED_RUN_LONG) {
        return b;
    }
    return RLE_UNROLLED_RUN_MAX;
}

void graphics_set_pixel(int32_t x, int32_t y, int32_t color);
void image_blit_copy(const uint8_t *src);
void graphics_set_pixel_pair(uint32_t color_pair);
const uint8_t *graphics_rle_skip_rows(const uint8_t *p);
void image_blit_compressed_mirrored(const uint8_t *data);
void image_blit_compressed_mirrored_right(const uint8_t *data);
void image_blit_compressed_mirrored_left(const uint8_t *data);
void image_blit_raw(const uint8_t *src);
void image_blit_raw_clipped(const uint8_t *src);
void image_blit_compressed(const uint8_t *data);
void image_blit_compressed_left(const uint8_t *data);
void image_blit_compressed_right(const uint8_t *data);
void image_blit_compressed_color(const uint8_t *data);
void image_convert_compressed_555(uint8_t *p, int32_t size);
void image_convert_raw_555(uint8_t *p, int32_t size);
const image_header *image_get(int32_t image_id);
int32_t image_collection_group(int32_t group);

#endif
