// The screen as a PNG in memory (screenshot_png.h).

#include "graphics/screenshot_png.h"
#include "graphics/graphics_display.h"
#include "graphics/screen_pixels.h"
#include "window/window_data.h"
#include <stdlib.h>
#include <string.h>

enum png_limits {
    PNG_MAX_PIXEL_BYTES = 3 * 1024 * 1024,
    PNG_STORED_BLOCK = 65535,
    PNG_ADLER_MOD = 65521,
};

static uint32_t crc_table[256];

static void crc_init(void)
{
    if (crc_table[1]) {
        return;
    }
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for (int32_t k = 0; k < 8; k++) {
            c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        }
        crc_table[n] = c;
    }
}

static uint32_t crc32_of(const uint8_t *data, size_t size)
{
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; i++) {
        c = crc_table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFu;
}

static void put32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t) (value >> 24);
    out[1] = (uint8_t) (value >> 16);
    out[2] = (uint8_t) (value >> 8);
    out[3] = (uint8_t) value;
}

// a chunk at out: length, type, data (already in place after the type), crc; returns its whole size
static size_t finish_chunk(uint8_t *out, const char *type, size_t length)
{
    put32(out, (uint32_t) length);
    memcpy(out + 4, type, 4);
    put32(out + 8 + length, crc32_of(out + 4, length + 4));
    return length + 12;
}

static void pixel_rgb(uint16_t pixel, uint8_t *rgb)
{
    if (display_mode == DISPLAY_MODE_RGB565) {
        rgb[0] = (uint8_t) ((pixel >> 8) & 0xF8);
        rgb[1] = (uint8_t) ((pixel >> 3) & 0xFC);
    } else {
        rgb[0] = (uint8_t) ((pixel >> 7) & 0xF8);
        rgb[1] = (uint8_t) ((pixel >> 2) & 0xF8);
    }
    rgb[2] = (uint8_t) (pixel << 3);
}

int32_t graphics_screen_png(uint8_t **png, size_t *size)
{
    *png = 0;
    *size = 0;
    if (!screen_rgb565_ptr || g_screen_width <= 0 || g_screen_height <= 0) {
        return 0;
    }
    int32_t scale = 1;
    while ((size_t) (g_screen_width / scale) * (size_t) (g_screen_height / scale) * 3 > PNG_MAX_PIXEL_BYTES) {
        scale++;
    }
    int32_t width = g_screen_width / scale;
    int32_t height = g_screen_height / scale;
    size_t row = (size_t) width * 3 + 1;
    size_t raw_size = row * (size_t) height;
    size_t blocks = (raw_size + PNG_STORED_BLOCK - 1) / PNG_STORED_BLOCK;
    size_t idat_length = 2 + raw_size + blocks * 5 + 4;
    size_t total = 8 + (12 + 13) + (12 + idat_length) + 12;
    uint8_t *out = malloc(total);
    uint8_t *raw = malloc(raw_size);
    if (!out || !raw) {
        free(out);
        free(raw);
        return 0;
    }
    for (int32_t y = 0; y < height; y++) {
        uint8_t *line = raw + (size_t) y * row;
        line[0] = 0;
        for (int32_t x = 0; x < width; x++) {
            pixel_rgb(pixel_read(screen_pixel_address(x * scale, y * scale)), line + 1 + x * 3);
        }
    }
    crc_init();
    static const uint8_t signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
    size_t at = 0;
    memcpy(out, signature, 8);
    at += 8;

    uint8_t *ihdr = out + at + 8;
    put32(ihdr, (uint32_t) width);
    put32(ihdr + 4, (uint32_t) height);
    ihdr[8] = 8;
    ihdr[9] = 2;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    at += finish_chunk(out + at, "IHDR", 13);

    uint8_t *idat = out + at + 8;
    size_t d = 0;
    idat[d++] = 0x78;
    idat[d++] = 0x01;
    uint32_t a = 1, b = 0;
    for (size_t from = 0; from < raw_size; from += PNG_STORED_BLOCK) {
        size_t length = raw_size - from < PNG_STORED_BLOCK ? raw_size - from : PNG_STORED_BLOCK;
        idat[d++] = from + length >= raw_size ? 1 : 0;
        idat[d++] = (uint8_t) length;
        idat[d++] = (uint8_t) (length >> 8);
        idat[d++] = (uint8_t) ~length;
        idat[d++] = (uint8_t) (~length >> 8);
        memcpy(idat + d, raw + from, length);
        for (size_t i = 0; i < length; i++) {
            a = (a + raw[from + i]) % PNG_ADLER_MOD;
            b = (b + a) % PNG_ADLER_MOD;
        }
        d += length;
    }
    put32(idat + d, (b << 16) | a);
    d += 4;
    at += finish_chunk(out + at, "IDAT", d);
    at += finish_chunk(out + at, "IEND", 0);
    free(raw);
    *png = out;
    *size = at;
    return 1;
}
