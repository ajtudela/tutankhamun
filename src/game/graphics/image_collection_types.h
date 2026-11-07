// Record types of the image collection (group table entry, SG file slot, image header, the collection).

#ifndef GRAPHICS_IMAGE_COLLECTION_TYPES_H
#define GRAPHICS_IMAGE_COLLECTION_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum image_collection_limit {
    IMAGE_HEADERS = 25500,
    IMAGE_GROUPS = 1000,
    IMAGE_SG_SLOTS = 30,
    IMAGE_SG_BITMAPS = 200,
    IMAGE_SG_NAME_SIZE = 260,
    IMAGE_SG_PIXELS_START = 4,
};

typedef struct image_header {
    int32_t data;
    int32_t data_length;
    int32_t isometric_top_offset;
    int32_t file_slot;
    int32_t mirror_source_offset;
    int16_t width;
    int16_t height;
    int16_t reserved_1[2];
    int16_t group_id;
    int16_t animation_frames;
    int16_t reserved_2;
    int16_t sprite_offset_x;
    int16_t sprite_offset_y;
    uint8_t reserved_3[10];
    uint8_t animation_can_reverse;
    uint8_t reserved_4;
    uint8_t draw_type;
    uint8_t is_compressed;
    uint8_t is_external;
    uint8_t has_isometric_top;
    uint8_t reserved_5[2];
    uint8_t bitmap_name_index;
    uint8_t reserved_6;
    uint8_t animation_speed_id;
    uint8_t reserved_7;
    uint8_t animation_frame;
    uint8_t reserved_8;
    uint8_t is_mirrored;
    uint8_t reserved_9;
} image_header;

static_assert(sizeof(image_header) == 0x40, "image_header is an SG file record of 64 bytes");
static_assert(offsetof(image_header, width) == 0x14 && offsetof(image_header, animation_frames) == 0x1E && offsetof(image_header, sprite_offset_x) == 0x22 &&
                  offsetof(image_header, animation_can_reverse) == 0x30 && offsetof(image_header, draw_type) == 0x32 &&
                  offsetof(image_header, animation_speed_id) == 0x3A && offsetof(image_header, animation_frame) == 0x3C &&
                  offsetof(image_header, is_mirrored) == 0x3E, "image_header is the SG file's layout");

typedef struct image_group_record {
    int32_t file_slot;
    int32_t first_image;
} image_group_record;

typedef struct image_sg_slot {
    char file_name[IMAGE_SG_NAME_SIZE];
    char bitmap_names[IMAGE_SG_BITMAPS][IMAGE_SG_NAME_SIZE];
    uint8_t *pixels;
    int32_t data_size;
    int32_t image_count;
    int32_t group_count;
    int32_t start_image;
    int32_t start_group;
} image_sg_slot;

typedef struct image_collection_object {
    image_group_record groups[IMAGE_GROUPS];
    image_sg_slot slots[IMAGE_SG_SLOTS];
    image_header headers[IMAGE_HEADERS];
    int32_t sg_group_ends[IMAGE_SG_SLOTS];
    uint8_t *external_buffer;
    uint8_t *second_buffer;
    int32_t next_group;
    int32_t image_count;
    int32_t data_total;
    int32_t sg_count;
} image_collection_object;

static_assert(offsetof(image_header, group_id) == 0x1C && offsetof(image_header, bitmap_name_index) == 0x38, "image_header is the SG file's layout");

static_assert(sizeof(void *) != 4 || (sizeof(image_sg_slot) == 0xCC3C && sizeof(image_collection_object) == 0x30F5D8),
              "image_collection_object has the original program's size on 32 bits");
static_assert(sizeof(void *) != 4 || (offsetof(image_collection_object, slots) == 0x1F40 && offsetof(image_collection_object, headers) == 0x180E48 &&
                                         offsetof(image_collection_object, external_buffer) == 0x30F5C0 &&
                                         offsetof(image_collection_object, image_count) == 0x30F5CC),
              "image_collection_object is the original program's layout on 32 bits");

#endif
