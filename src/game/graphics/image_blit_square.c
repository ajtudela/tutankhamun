// Blitters of the square raw images (16, 24 and 32 pixels): unrolled whole-image copies.
//
// They are called only for images clipped left or right, and copy the whole image from the clamped x.

#include "graphics/image_blit_square.h"
#include "graphics/graphics_data.h"
#include "graphics/screen_pixels.h"
#include <stdint.h>

static void blit_square(const uint32_t *src, int32_t size)
{
    uint8_t *dst = screen_pixel_address(g_draw_x, g_draw_y);
    int32_t pitch = g_screen_pitch;
    int32_t row = 0;
    for (int32_t y = 0; y < size; y++) {
        for (int32_t x = 0; x < size / 2; x++) {
            *(uint32_t *) (dst + row + x * 4) = *src++;
        }
        row += pitch;
    }
}

void image_blit_square16(const uint32_t *data)
{
    blit_square(data, 16);
}

void image_blit_square24(const uint32_t *data)
{
    blit_square(data, 24);
}

void image_blit_square32(const uint32_t *data)
{
    blit_square(data, 32);
}
