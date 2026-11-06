// Screenshots of the back buffer as 24-bit BMP files, and the BMP helpers of the editor's new map: the pixel
// data offset of a BMP in memory and its rows reversed and mirrored in place.
//
// Notes on the screenshot loop:
// - it returns 1 also when the file cannot be created;
// - the loops re-read the screen size and the BMP's width after every pixel.

#include "graphics/screenshot.h"
#include "building/building_data.h"
#include "core/text.h"
#include "graphics/graphics_data.h"
#include <fcntl.h>
#include <io.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

uint8_t *screenshot_buffer;

int32_t bmp_width;

int32_t bmp_height;

int32_t bmp_image_size;

enum screenshot_limit {
    SCREENSHOT_BUFFER_SIZE = 3100000,
    SCREENSHOT_BYTES_PER_PIXEL = 3,
};

int32_t bmp_data_offset(const uint8_t *data)
{
    bmp_file_header file_header;
    memcpy(&file_header, data, sizeof(file_header));
    return file_header.data_offset;
}

int32_t bmp_flip(uint8_t *data)
{
    bmp_file_header file_header;
    bmp_info_header info_header;
    memcpy(&file_header, data, sizeof(file_header));
    memcpy(&info_header, data + sizeof(file_header), sizeof(info_header));
    bmp_width = info_header.width;
    bmp_height = info_header.height;
    bmp_image_size = info_header.image_size;
    uint8_t *copy = malloc((size_t) bmp_image_size);
    if (!copy) {
        return 0;
    }
    int32_t offset = file_header.data_offset;
    const uint8_t *src = data + bmp_image_size + offset;
    uint8_t *row = copy;
    for (int32_t y = 0; y < bmp_height; y++) {
        uint8_t *dst = row + bmp_width - 1;
        for (int32_t x = 0; x < bmp_width; x++) {
            *dst-- = *--src;
        }
        row += bmp_width;
    }
    memcpy(data + offset, copy, bmp_image_size);
    free(copy);
    return 1;
}

static void write_pixel_rgb(uint8_t *out, uint16_t pixel)
{
    out[0] = (uint8_t) (pixel << 3);
    out[1] = (uint8_t) ((pixel >> 3) & 0xFC);
    out[2] = (uint8_t) ((pixel >> 8) & 0xF8);
}

int32_t graphics_save_screenshot(const char *file_name)
{
    screenshot_buffer = 0;
    screenshot_buffer = malloc(SCREENSHOT_BUFFER_SIZE);
    if (!screenshot_buffer) {
        return 0;
    }
    bmp_file_header file_header;
    bmp_info_header info_header;
    memset(&file_header, 0, sizeof(file_header));
    memset(&info_header, 0, sizeof(info_header));
    memcpy(&file_header.signature, bmp_signature, BMP_SIGNATURE_SIZE);
    int32_t height = g_screen_height;
    int32_t width = g_screen_width;
    int32_t image_size = (height * width) * SCREENSHOT_BYTES_PER_PIXEL;
    info_header.width = width;
    file_header.data_offset = BMP_HEADERS_SIZE;
    info_header.header_size = sizeof(info_header);
    file_header.file_size = (uint32_t) (image_size + BMP_HEADERS_SIZE);
    info_header.height = height;
    info_header.planes = 1;
    info_header.bits_per_pixel = BMP_SCREENSHOT_BITS_PER_PIXEL;
    info_header.image_size = image_size;
    int32_t fd = _open(file_name, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd != -1) {
        memcpy(screenshot_buffer, &file_header, sizeof(file_header));
        memcpy(screenshot_buffer + sizeof(file_header), &info_header, sizeof(info_header));
        height = g_screen_height;
        width = g_screen_width;
        const uint16_t *row_end = screen_rgb565_ptr + (height * width);
        uint8_t *out = screenshot_buffer + BMP_HEADERS_SIZE;
        for (int32_t y = 0; y < height; y++) {
            const uint16_t *src = row_end - width;
            for (int32_t x = 0; x < width; x++) {
                write_pixel_rgb(out, *src++);
                out += SCREENSHOT_BYTES_PER_PIXEL;
                width = g_screen_width;
            }
            height = g_screen_height;
            row_end -= width;
        }
        _write(fd, screenshot_buffer, (uint32_t) (((height * width) * SCREENSHOT_BYTES_PER_PIXEL) + BMP_HEADERS_SIZE));
        _close(fd);
    }
    if (screenshot_buffer) {
        free(screenshot_buffer);
    }
    return 1;
}
