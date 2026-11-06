// Low-level blitters of the RGB565 back buffer: plain and transparent raw copies, RLE-compressed images (plain,
// mirrored, clipped left or right, one colour), the RLE row skip for top clipping, single pixels, the in-place
// 555 to 565 conversion of image data, and the image collection's header and group lookups.
//
// The draw state (position, visible size, skips) comes from the clip helpers of image_draw.c.
//
// The blitters are lenient about their input:
// - every row loop runs at least once, also with no visible rows;
// - the unrolled blitters take any run byte other than 1..14 and 16 as 15 pixels; the clipped ones take 0 as 1;
// - the compressed 555 conversion treats run bytes from 0x80 on as no pixels;
// - an image id not below the image count gives image 0's header.

#include "graphics/image_blit.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/image_collection.h"
#include "graphics/screen_pixels.h"
#include "window/window_data.h"
#include <stdint.h>
#include <string.h>

enum pixel_555_to_565 {
    PIXEL_555_RED_GREEN = 0x7FE0,
    PIXEL_555_RED_GREEN_AND_TOP_BIT = 0xFFE0,
};

static uint8_t *draw_target(void)
{
    return screen_pixel_address(g_draw_x, g_draw_y);
}

void graphics_set_pixel(int32_t x, int32_t y, int32_t color)
{
    pixel_write(screen_pixel_address(x, y), (uint16_t) color);
}

void image_blit_copy(const uint8_t *src)
{
    uint8_t *dst = draw_target();
    uint32_t n = (uint32_t) g_visible_w * 2u;
    int32_t rows = g_visible_h;
    do {
        memcpy(dst, src, n);
        src += n + g_src_skip;
        dst += n + g_dst_skip;
    } while (--rows > 0);
}

void graphics_set_pixel_pair(uint32_t color_pair)
{
    *(uint32_t *) draw_target() = color_pair;
}

const uint8_t *graphics_rle_skip_rows(const uint8_t *p)
{
    int32_t rows = g_clip_top;
    while (rows > 0) {
        rows--;
        int32_t left = g_img_width;
        while (left > 0) {
            uint8_t b = p[0];
            if (b == RLE_SKIP) {
                left -= p[1];
                p += 2;
            } else {
                left -= b;
                p += b * 2 + 1;
            }
        }
    }
    return p;
}

void image_blit_compressed_mirrored(const uint8_t *data)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t row_bytes = g_img_width * 2;
    for (int32_t rows = g_visible_h; rows > 0; rows--) {
        int32_t left = g_img_width;
        d += row_bytes;
        while (left > 0) {
            uint8_t b = *p++;
            if (b == RLE_SKIP) {
                int32_t n = *p++;
                d -= n * 2;
                left -= n;
            } else {
                for (uint8_t c = b; c != 0 && left != 0; c--) {
                    pixel_write(d, pixel_read(p));
                    p += 2;
                    d -= 2;
                    left--;
                }
            }
        }
        d += g_dst_skip + row_bytes;
    }
}

void image_blit_compressed_mirrored_right(const uint8_t *data)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t w2 = g_img_width * 2;
    for (int32_t rows = g_visible_h; rows > 0; rows--) {
        int32_t left = g_img_width;
        int32_t skip = left - g_visible_w;
        d += w2;
        while (left > 0) {
            uint8_t b = p[0];
            if (b == RLE_SKIP) {
                int32_t n = p[1];
                p += 2;
                left -= n;
                skip -= n;
                d -= n * 2;
            } else {
                p++;
                int32_t n = b;
                left -= n;
                for (; n > 0; n--) {
                    if (skip <= 0) {
                        pixel_write(d, pixel_read(p));
                    }
                    d -= 2;
                    p += 2;
                    skip--;
                }
            }
        }
        d += g_screen_pitch;
    }
}

void image_blit_compressed_mirrored_left(const uint8_t *data)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t w2 = g_img_width * 2;
    for (int32_t rows = g_visible_h; rows > 0; rows--) {
        int32_t left = g_img_width;
        int32_t visible = left - (g_src_skip >> 1);
        uint8_t *row = d;
        d += w2 - g_src_skip;
        while (left > 0) {
            uint8_t b = p[0];
            if (b == RLE_SKIP) {
                int32_t n = p[1];
                p += 2;
                visible -= n;
                left -= n;
                d -= n * 2;
            } else {
                p++;
                int32_t n = b;
                left -= n;
                for (; n > 0; n--) {
                    if (visible > 0) {
                        pixel_write(d, pixel_read(p));
                    }
                    d -= 2;
                    p += 2;
                    visible--;
                }
            }
        }
        d = row + g_screen_pitch;
    }
}

void image_blit_raw(const uint8_t *src)
{
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t n = g_img_width;
        if ((n & 3) != 0) {
            do {
                uint16_t px = pixel_read(src);
                if (px != PIXEL_TRANSPARENT_KEY) {
                    pixel_write(d, px);
                }
                src += 2;
                d += 2;
            } while (--n > 0);
        } else {
            do {
                for (int32_t i = 0; i < 4; i++) {
                    uint16_t px = pixel_read(src);
                    if (px != PIXEL_TRANSPARENT_KEY) {
                        pixel_write(d, px);
                    }
                    src += 2;
                    d += 2;
                }
            } while ((n -= 4) > 0);
        }
        d += g_dst_skip;
    } while (--rows > 0);
}

void image_blit_raw_clipped(const uint8_t *src)
{
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    int32_t src_skip = g_src_skip;
    do {
        int32_t n = g_visible_w;
        do {
            uint16_t px = pixel_read(src);
            if (px != PIXEL_TRANSPARENT_KEY) {
                pixel_write(d, px);
            }
            src += 2;
            d += 2;
        } while (--n > 0);
        src += src_skip;
        d += g_dst_skip;
    } while (--rows > 0);
}

void image_blit_compressed(const uint8_t *data)
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
                memcpy(d, p + 1, (size_t) n * 2);
                p += n * 2 + 1;
                d += n * 2;
                left -= n;
            }
        }
        d += g_dst_skip;
    } while (--rows > 0);
}

void image_blit_compressed_left(const uint8_t *data)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t left = g_img_width;
        int32_t skip = (int32_t) ((uint32_t) g_src_skip >> 1);
        uint8_t *q = d - skip * 2;
        while (left > 0) {
            uint8_t b = p[0];
            if (b != RLE_SKIP) {
                p++;
                int32_t n = b;
                do {
                    if (skip <= 0) {
                        pixel_write(q, pixel_read(p));
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

void image_blit_compressed_right(const uint8_t *data)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    int32_t rows = g_visible_h;
    do {
        int32_t left = g_img_width;
        int32_t visible = g_visible_w;
        uint8_t *q = d;
        while (left > 0) {
            uint8_t b = p[0];
            if (b != RLE_SKIP) {
                p++;
                int32_t n = b;
                do {
                    if (visible > 0) {
                        pixel_write(q, pixel_read(p));
                    }
                    p += 2;
                    q += 2;
                    visible--;
                    left--;
                } while (--n > 0);
            } else {
                int32_t n = p[1];
                q += n * 2;
                p += 2;
                visible -= n;
                left -= n;
            }
        }
        d += g_screen_pitch;
    } while (--rows > 0);
}

void image_blit_compressed_color(const uint8_t *data)
{
    const uint8_t *p = graphics_rle_skip_rows(data);
    uint8_t *d = draw_target();
    uint16_t color = g_draw_color;
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
                    pixel_write(d + i * 2, color);
                }
                p += n * 2 + 1;
                d += n * 2;
                left -= n;
            }
        }
        d += g_dst_skip;
    } while (--rows > 0);
}

void image_convert_compressed_555(uint8_t *p, int32_t size)
{
    if (display_mode != DISPLAY_MODE_RGB565) {
        return;
    }
    while (size > 0) {
        uint8_t b = p[0];
        if (b == RLE_SKIP) {
            p += 2;
            size -= 2;
            continue;
        }
        p++;
        size--;
        for (int8_t c = (int8_t) b; c > 0; c--) {
            uint16_t v = pixel_read(p);
            pixel_write(p, (uint16_t) (((v & PIXEL_555_RED_GREEN) << 1) | (v & PIXEL_BLUE_MASK)));
            p += 2;
            size -= 2;
        }
    }
}

void image_convert_raw_555(uint8_t *p, int32_t size)
{
    if (display_mode != DISPLAY_MODE_RGB565) {
        return;
    }
    while (size > 0) {
        uint16_t v = pixel_read(p);
        if (v != PIXEL_TRANSPARENT_KEY) {
            pixel_write(p, (uint16_t) (((v & PIXEL_555_RED_GREEN_AND_TOP_BIT) << 1) | (v & PIXEL_BLUE_MASK)));
        }
        p += 2;
        size -= 2;
    }
}

const image_header *image_get(int32_t image_id)
{
    const image_collection_object *collection = &image_collection;
    if (!(image_id < collection->image_count)) {
        image_id = 0;
    }
    return &collection->headers[image_id];
}

int32_t image_collection_group(int32_t group)
{
    const image_collection_object *collection = &image_collection;
    return collection->groups[group].first_image;
}
