// The image collection: the group table, the 30 SG file slots, the image headers (pixel data, size, animation,
// sprite offsets, draw type and flags), the external image buffers and the counters, and the functions that load
// it and look images up.

#ifndef GRAPHICS_IMAGE_COLLECTION_H
#define GRAPHICS_IMAGE_COLLECTION_H

#include "core/sorted_list.h"
#include "graphics/image_collection_types.h"
#include "window/window_data.h"
#include <stddef.h>
#include <stdint.h>

intptr_t image_pixels_address(const image_header *header);
int32_t image_get_size(int32_t image_id, int32_t *out_w, int32_t *out_h);

enum image_draw_type {
    IMAGE_DRAW_PLAIN = 0,
    IMAGE_DRAW_TWO_PIXELS = 2,
    IMAGE_DRAW_SQUARE_16 = 10,
    IMAGE_DRAW_SQUARE_24 = 12,
    IMAGE_DRAW_SQUARE_32 = 13,
    IMAGE_DRAW_ONE_COLOR = 0x14,
    IMAGE_DRAW_ISOMETRIC = 30,
};

typedef struct {
    int32_t first_image_number;
    sorted_link *node;
} sg_group_info;

extern int32_t image_sg_expected_group_ends[IMAGE_SG_SLOTS];

static inline const int32_t *image_group_entry(int32_t group)
{
    return &image_collection.groups[group].first_image;
}

static inline int32_t image_group(int32_t group)
{
    return *image_group_entry(group);
}

static inline int32_t image_known_id(int32_t image_id)
{
    return (image_id >= image_collection.image_count) ? 0 : image_id;
}

static inline const image_header *image_header_get(int32_t image_id)
{
    return &image_collection.headers[image_known_id(image_id)];
}

static inline int16_t image_record_height(int32_t image)
{
    return image_collection.headers[image].height;
}

static inline int16_t image_record_sprite_offset_x(int32_t image)
{
    return image_collection.headers[image].sprite_offset_x;
}

static inline int16_t image_record_sprite_offset_y(int32_t image)
{
    return image_collection.headers[image].sprite_offset_y;
}

static inline int16_t image_record_animation_frames(int32_t image)
{
    return image_collection.headers[image].animation_frames;
}

static inline uint8_t image_record_animation_can_reverse(int32_t image)
{
    return image_collection.headers[image].animation_can_reverse;
}

static inline uint8_t image_record_animation_speed_id(int32_t image)
{
    return image_collection.headers[image].animation_speed_id;
}

void image_buf_reset(uint8_t **slot, int32_t clear_scratch);
void image_sg_slot_ctor(image_sg_slot *slot);
void image_collection_reset(void);
void image_collection_init(void);
int32_t image_collection_load_file(const char *filename, int32_t slot_index);
int32_t image_collection_load_next(const char *filename);
int32_t image_collection_load_one(const char **names);
void image_collection_load_all(const char **names);
int32_t image_load_external_data(char *name, void *buf, int32_t width_bytes, int32_t height, int32_t file_offset);

#endif
