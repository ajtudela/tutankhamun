// Screenshots, the BMP helpers and the BMP file and info headers.

#ifndef GRAPHICS_SCREENSHOT_H
#define GRAPHICS_SCREENSHOT_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 2)
typedef struct bmp_file_header {
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved[2];
    int32_t data_offset;
} bmp_file_header;
#pragma pack(pop)

typedef struct bmp_info_header {
    int32_t header_size;
    int32_t width;
    int32_t height;
    int16_t planes;
    int16_t bits_per_pixel;
    int32_t compression;
    int32_t image_size;
    int32_t pixels_per_meter_x;
    int32_t pixels_per_meter_y;
    int32_t colors_used;
    int32_t colors_important;
} bmp_info_header;

static_assert(sizeof(bmp_file_header) == 14, "bmp_file_header is a BMP file header of 14 bytes");
static_assert(offsetof(bmp_file_header, data_offset) == 0xA, "bmp_file_header is the BMP file's layout");
static_assert(sizeof(bmp_info_header) == 0x28, "bmp_info_header is a BMP file header of 40 bytes");
static_assert(offsetof(bmp_info_header, image_size) == 0x14, "bmp_info_header is the BMP file's layout");

enum bmp_layout {
    BMP_HEADERS_SIZE = sizeof(bmp_file_header) + sizeof(bmp_info_header),
    BMP_SIGNATURE_SIZE = 2,
    BMP_SCREENSHOT_BITS_PER_PIXEL = 24,
};

int32_t bmp_data_offset(const uint8_t *data);
int32_t bmp_flip(uint8_t *data);
int32_t graphics_save_screenshot(const char *file_name);

#endif
