// The remaining blitters of image drawing: the colour-masked blits (each pixel ANDed with the mask, used to
// show buildings red), the mirrored raw images, the one-colour RLE blits clipped left or right, the two-pixel
// image, and image_draw_masked.
//
// As in the plain blitters, sizes are not checked:
// - every row loop runs at least once;
// - a raw masked image whose width is a multiple of 4 starts with 4 pixels, so a width of 0 still draws 4;
// - a run byte 0 still takes one pixel in the clipped RLE blits.

#include "graphics/image_blit_mask.h"
#include "graphics/graphics_data.h"
#include "graphics/image_blit.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/screen_pixels.h"
#include "window/window_data.h"
#include <stdint.h>
#include <string.h>

static uint8_t *draw_target(void)
{
    return screen_pixel_address(g_draw_x, g_draw_y);
}

static void and_pixel(uint8_t *pixel, int32_t mask)
{
    pixel_write(pixel, (uint16_t) (pixel_read(pixel) & mask));
}

void image_blit_raw_mask(const uint8_t *src, int32_t mask)
{
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t n = g_img_width;
        if ((n & 3) == 0) {
            for (int32_t i = 0; i < 4; i++) {
                if (pixel_read(src) != PIXEL_TRANSPARENT_KEY) {
                    and_pixel(d, mask);
                }
                src += 2;
                d += 2;
            }
            n -= 4;
            if (n <= 0) {
                d += g_dst_skip;
                continue;
            }
        }
        do {
            if (pixel_read(src) != PIXEL_TRANSPARENT_KEY) {
                and_pixel(d, mask);
            }
            src += 2;
            d += 2;
        } while (--n > 0);
        d += g_dst_skip;
    } while (--rows > 0);
}

void image_blit_raw_mask_clipped(const uint8_t *src, int32_t mask)
{
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t n = g_visible_w;
        do {
            if (pixel_read(src) != PIXEL_TRANSPARENT_KEY) {
                and_pixel(d, mask);
            }
            src += 2;
            d += 2;
        } while (--n > 0);
        src += g_src_skip;
        d += g_dst_skip;
    } while (--rows > 0);
}

void image_blit_compressed_mask(const uint8_t *data, int32_t mask)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t left = g_img_width;
        while (left > 0) {
            uint8_t b = p[0];
            if (b == RLE_SKIP) {
                int32_t n = p[1];
                d += n * 2;
                p += 2;
                left -= n;
            } else {
                int32_t n = rle_unrolled_run_length(b);
                for (int32_t i = 0; i < n; i++) {
                    and_pixel(d + i * 2, mask);
                }
                p += n * 2 + 1;
                d += n * 2;
                left -= n;
            }
        }
        d += g_dst_skip;
    } while (--rows > 0);
}

static void compressed_clipped(const uint8_t *data, int32_t right, int32_t fill, int32_t value)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t left = g_img_width;
        int32_t skip = right ? g_visible_w : (int32_t) ((uint32_t) g_src_skip >> 1);
        uint8_t *q = right ? d : d - skip * 2;
        while (left > 0) {
            uint8_t b = p[0];
            if (b != RLE_SKIP) {
                p++;
                int32_t n = b;
                do {
                    if (right ? skip > 0 : skip <= 0) {
                        if (fill) {
                            pixel_write(q, g_draw_color);
                        } else {
                            and_pixel(q, value);
                        }
                    }
                    p += 2;
                    q += 2;
                    left--;
                    skip--;
                } while (--n > 0);
            } else {
                int32_t n = p[1];
                q += n * 2;
                p += 2;
                skip -= n;
                left -= n;
            }
        }
        d += g_screen_pitch;
    } while (--rows > 0);
}

void image_blit_compressed_mask_left(const uint8_t *data, int32_t mask)
{
    compressed_clipped(data, 0, 0, mask);
}

void image_blit_compressed_mask_right(const uint8_t *data, int32_t mask)
{
    compressed_clipped(data, 1, 0, mask);
}

void image_blit_compressed_color_left(const uint8_t *data)
{
    compressed_clipped(data, 0, 1, 0);
}

void image_blit_compressed_color_right(const uint8_t *data)
{
    compressed_clipped(data, 1, 1, 0);
}

void image_blit_raw_mirrored(const uint8_t *src)
{
    uint8_t *d = draw_target();
    int32_t width2 = g_img_width * 2;
    if (g_visible_h <= 0) {
        return;
    }
    for (int32_t row = 0; row < g_visible_h; row++) {
        d += width2;
        for (int32_t i = 0; i < g_img_width; i++) {
            if (pixel_read(src) != PIXEL_TRANSPARENT_KEY) {
                pixel_write(d, pixel_read(src));
            }
            d -= 2;
            src += 2;
        }
        d += g_dst_skip + width2;
    }
}

void image_blit_raw_mirrored_clipped(const uint8_t *src)
{
    uint8_t *d = draw_target();
    int32_t width2 = g_visible_w * 2;
    for (uint32_t rows = (uint32_t) g_visible_h; rows != 0; rows--) {
        d += width2;
        for (uint32_t n = (uint32_t) g_visible_w; n != 0; n--) {
            if (pixel_read(src) != PIXEL_TRANSPARENT_KEY) {
                pixel_write(d, pixel_read(src));
            }
            d -= 2;
            src += 2;
        }
        d += g_dst_skip + width2;
        src += g_src_skip;
    }
}

void image_blit_two_pixels(const uint8_t *src)
{
    uint8_t *d = draw_target();
    *(uint32_t *) d = *(const uint32_t *) src;
    pixel_write(d + g_screen_pitch, pixel_read(src + 2));
}

int32_t image_draw_masked(int32_t image_id, int32_t x, int32_t y, int32_t mask)
{
    const image_collection_object *collection = &image_collection;
    if (image_id <= 0 || image_id >= IMAGE_HEADERS) {
        return 0;
    }
    if (image_id >= collection->image_count) {
        image_id = 0;
    }
    const image_header *header = &collection->headers[image_id];
    g_data_ptr = header->data;
    if (g_data_ptr <= 0) {
        return 0;
    }
    g_img_width = header->width;
    g_img_height = header->height;
    g_img_type = header->draw_type;
    g_draw_x = x;
    g_draw_y = y;
    g_visible_w = g_img_width;
    graphics_clip_horizontal();
    graphics_clip_vertical();
    if (g_clip_y == IMAGE_CLIP_INVISIBLE) {
        return 0;
    }
    const uint8_t *data = (const uint8_t *) (uintptr_t) g_data_ptr;
    if (header->is_compressed == 0) {
        if (g_clip_x == IMAGE_CLIP_LEFT || g_clip_x == IMAGE_CLIP_RIGHT) {
            image_blit_raw_mask_clipped(data, mask);
        } else {
            image_blit_raw_mask(data, mask);
        }
    } else if (g_clip_x == IMAGE_CLIP_LEFT) {
        image_blit_compressed_mask_left(data, mask);
    } else if (g_clip_x == IMAGE_CLIP_RIGHT) {
        image_blit_compressed_mask_right(data, mask);
    } else {
        image_blit_compressed_mask(data, mask);
    }
    return g_visible_w;
}
