// Loading the image collection: the 30 SG file slots, the reset, init and destroy of the collection, one SG
// file and its .555 pixels loaded into a slot (the groups sorted into the group table, the pixels converted for
// the 565 display), the list of the game's files, an image's size and the rows of an external image read from
// its .555 file in the data folder.
//
// Loading has three surprises:
// - a file other than the first that starts with the system bitmap skips the system images;
// - a null or empty name in the file list does not advance the count;
// - image_get_size returns the header's address with only the low byte set to the success flag.

#include "graphics/image_collection.h"
#include "core/static_init.h"
#include "graphics/graphics_data.h"
#include "graphics/image_blit.h"
#include "io/file.h"
#include "io/io_data.h"
#include "platform/log.h"
#include "window/window_data.h"
#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int32_t g_pixel_cursor;

enum sg_file_layout {
    SG_FILE_BUFFER_SIZE = 887368,
    SG_FILE_GROUPS = 300,
    SG_FILE_BITMAP_RECORD = 200,
    SG_SYSTEM_IMAGES = 200,
    SG_SYSTEM_PIXEL_BYTES = 0x992E,
};

typedef struct sg_file_header {
    uint8_t spare_23[16];
    int32_t image_count;
    int32_t bitmap_count;
    uint8_t spare_22[56];
    uint16_t group_first_images[SG_FILE_GROUPS];
    char bitmap_records[IMAGE_SG_BITMAPS][SG_FILE_BITMAP_RECORD];
    uint8_t spare_34[64];
    image_header images[1];
} sg_file_header;

static_assert(offsetof(sg_file_header, group_first_images) == 80 && offsetof(sg_file_header, bitmap_records) == 680 &&
                  offsetof(sg_file_header, images) == 40744, "sg_file_header is the SG file's layout");

int32_t image_sg_expected_group_ends[IMAGE_SG_SLOTS] = { 0x27, 0xFD, 0x10E, 0x1F2, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x240, 0x241, 0x27A, 0x282, 0x293,
    0x2C7, 0x2F0, 0x2FC, 0x2FD, 0x2FE, 0x2FF, 0x300, 0x301, 0x312, 0x317, 0x32C, 0x33C, 0x353, 0x370, 0x393 };

void image_buf_reset(uint8_t **slot, int32_t clear_scratch)
{
    uint8_t *buffer = *slot;
    if (buffer == NULL) {
        return;
    }
    if (buffer == (uint8_t *) spare_screen) {
        if (clear_scratch) {
            *slot = NULL;
        }
        return;
    }
    free(buffer);
    *slot = NULL;
}

static void clear_slot_counts(image_sg_slot *slot)
{
    slot->file_name[0] = 0;
    slot->data_size = 0;
    slot->image_count = 0;
    slot->group_count = 0;
}

void image_sg_slot_ctor(image_sg_slot *slot)
{
    slot->pixels = NULL;
    clear_slot_counts(slot);
}

static void free_slot_pixels(image_sg_slot *slot)
{
    if (slot->pixels != NULL) {
        free(slot->pixels);
        slot->pixels = NULL;
    }
}

static void data_file_path(char *path, const char *name, const char *extension)
{
    memset(path, 0, 260);
    strcpy(path, data_path_prefix);
    strcat(path, name);
    strcat(path, extension);
}

static int32_t load_pixels(const char *name, uint8_t **buffer, int32_t file_offset)
{
    char path[FILE_PATH_SIZE];
    data_file_path(path, name, g_ext_555_dot);
    image_buf_reset(buffer, 0);
    uint32_t size = (uint32_t) file_get_size_from_offset(path, file_offset);
    if ((int32_t) size > 0) {
        *buffer = malloc(size);
        io_read_file_into_buffer(path, *buffer, file_offset);
    }
    return (int32_t) size;
}

static void sort_groups(sorted_list *list, image_sg_slot *slot, const sg_file_header *file, int32_t skip)
{
    for (int32_t group = 0; group < SG_FILE_GROUPS; group++) {
        int32_t first_image_number = (int32_t) file->group_first_images[group] - skip;
        if (first_image_number <= 0) {
            continue;
        }
        sg_group_info *info = malloc(sizeof(sg_group_info));
        sorted_link *node = malloc(sizeof(sorted_link));
        info->first_image_number = first_image_number;
        node->key = group;
        node->item = info;
        sorted_list_insert(list, node);
        info->node = node;
        slot->group_count += 1;
    }
}

static void fill_group_table(
    sorted_list *list, image_header *images, image_group_record *groups, int32_t slot_index, int32_t start_image, int32_t start_group)
{
    sorted_link *next_node = list->head ? list->head->next : 0;
    sg_group_info *info = list->head ? list->head->item : 0;
    int32_t sequence = 0;
    while (info) {
        sg_group_info *next_info = 0;
        if (next_node) {
            next_info = next_node->item;
            next_node = next_node->next;
        }
        groups->file_slot = slot_index;
        groups->first_image = info->first_image_number - 1 + start_image;
        groups++;
        images[info->first_image_number - 1].group_id = (int16_t) (start_group + sequence);
        sequence++;
        sorted_list_unlink(list, info->node);
        free(info->node);
        free(info);
        info = next_info;
    }
}

static void convert_image(image_header *image)
{
    int32_t data_length = image->data_length;
    uint8_t *data = (uint8_t *) image_pixels_address(image);
    if (image->draw_type == IMAGE_DRAW_ISOMETRIC) {
        if (image->has_isometric_top == 0) {
            image_convert_raw_555(data, data_length);
        } else {
            int32_t raw_length = image->isometric_top_offset;
            image_convert_raw_555(data, raw_length);
            image_convert_compressed_555(data + raw_length, data_length - raw_length);
        }
        return;
    }
    image->has_isometric_top = 0;
    if (image->is_compressed == 0) {
        image_convert_raw_555(data, data_length);
    } else {
        image_convert_compressed_555(data, data_length);
    }
}

static void place_image_data(image_sg_slot *slot, image_header *images, int32_t slot_index)
{
    g_pixel_cursor = IMAGE_SG_PIXELS_START;
    for (int32_t i = 0; i < slot->image_count; i++) {
        image_header *image = &images[i];
        image->file_slot = slot_index;
        if (image->is_external != 0) {
            if (image->data == 0) {
                image->data = 1;
            }
            continue;
        }
        int32_t data_length = image->data_length;
        image->data = g_pixel_cursor;
        g_pixel_cursor += data_length;
        if (data_length > 0) {
            convert_image(image);
        }
    }
}

static int32_t image_sg_load(image_sg_slot *slot, const char *filename, int32_t slot_index, image_header *images,
    int32_t start_image, image_group_record *groups, int32_t start_group)
{
    int32_t previous_count = slot->image_count;
    free_slot_pixels(slot);
    clear_slot_counts(slot);
    strcpy(slot->file_name, filename);
    char path[FILE_PATH_SIZE];
    data_file_path(path, filename, g_ext_sg3);
    slot->start_image = start_image;
    slot->start_group = start_group;
    uint8_t *file_buffer = (uint8_t *) malloc(SG_FILE_BUFFER_SIZE);
    if (!file_buffer) {
        return 0;
    }
    memset(file_buffer, 0, SG_FILE_BUFFER_SIZE);
    const sg_file_header *file = (const sg_file_header *) file_buffer;
    int32_t loaded = 0;
    if (io_read_file_into_buffer(path, file_buffer, 0)) {
        int32_t skip = 0;
        int32_t pixel_offset = 0;
        if (slot_index != 0 && strcmp(file->bitmap_records[0], g_system_bmp) == 0) {
            skip = SG_SYSTEM_IMAGES;
            pixel_offset = SG_SYSTEM_PIXEL_BYTES;
        }
        slot->image_count = file->image_count - skip;
        memcpy(images, &file->images[skip], (size_t) slot->image_count * sizeof(image_header));
        sorted_list list = { 0, 0 };
        sort_groups(&list, slot, file, skip);
        fill_group_table(&list, images, groups, slot_index, start_image, start_group);
        for (int32_t i = 0; i < file->bitmap_count; i++) {
            strcpy(slot->bitmap_names[i], file->bitmap_records[i]);
        }
        slot->data_size = load_pixels(filename, &slot->pixels, pixel_offset);
        loaded = slot->data_size != 0;
        if (loaded) {
            place_image_data(slot, images, slot_index);
        }
    }
    slot->image_count = (slot->image_count > previous_count) ? slot->image_count : previous_count;
    free(file_buffer);
    return loaded;
}

void image_collection_reset(void)
{
    image_collection_object *collection = &image_collection;
    image_buf_reset(&collection->second_buffer, 0);
    image_buf_reset(&collection->external_buffer, 0);
    for (int32_t i = 0; i < IMAGE_SG_SLOTS; i++) {
        image_sg_slot *slot = &collection->slots[i];
        free_slot_pixels(slot);
        clear_slot_counts(slot);
        collection->sg_group_ends[i] = image_sg_expected_group_ends[i];
    }
    collection->sg_count = 0;
    collection->next_group = 1;
    collection->image_count = 1;
    collection->data_total = 0;
}

void image_collection_init(void)
{
    image_collection_object *collection = &image_collection;
    for (int32_t i = 0; i < IMAGE_SG_SLOTS; i++) {
        image_sg_slot_ctor(&collection->slots[i]);
    }
    collection->second_buffer = NULL;
    collection->external_buffer = NULL;
    image_collection_reset();
}

static void check_group_count(const image_collection_object *collection, const image_sg_slot *slot, int32_t slot_index, const char *filename)
{
    int32_t expected_end = collection->sg_group_ends[slot_index] - 1;
    int32_t expected_start = (slot_index > 0) ? (collection->sg_group_ends[slot_index - 1] - 1) : 0;
    int32_t expected_groups = expected_end - expected_start;
    if (expected_groups != slot->group_count) {
        platform_log("image_collection_load_file: tag sync name=%s got=%d expected=%d", filename ? filename : "", slot->group_count, expected_groups);
    }
}

int32_t image_collection_load_file(const char *filename, int32_t slot_index)
{
    image_collection_object *collection = &image_collection;
    if (!filename) {
        return 0;
    }
    if (slot_index == -1) {
        slot_index = collection->sg_count;
    }
    int32_t is_new = 0;
    int32_t start_image = 0;
    int32_t start_group = 0;
    image_sg_slot *slot = &collection->slots[slot_index];
    int32_t result = 1;
    int32_t already_loaded = 0;
    if (slot->image_count == 0) {
        start_group = collection->next_group;
        start_image = collection->image_count;
        is_new = 1;
    } else {
        already_loaded = _stricmp(filename, slot->file_name) == 0;
        start_image = slot->start_image;
        start_group = slot->start_group;
        collection->data_total -= slot->data_size;
    }
    if (!already_loaded) {
        result = image_sg_load(slot, filename, slot_index, &collection->headers[start_image], start_image, &collection->groups[start_group], start_group);
        if (result && is_new) {
            collection->image_count += slot->image_count;
            collection->next_group += slot->group_count;
            collection->sg_count += 1;
        }
    }
    collection->data_total += slot->data_size;
    check_group_count(collection, slot, slot_index, filename);
    return result;
}

int32_t image_collection_load_next(const char *filename)
{
    return image_collection_load_file(filename, -1);
}

int32_t image_collection_load_one(const char **names)
{
    const image_collection_object *collection = &image_collection;
    int32_t loaded = collection->sg_count;
    if (loaded >= IMAGE_SG_SLOTS) {
        return 0;
    }
    const char *name = names[loaded];
    if (!name || !name[0]) {
        return 0;
    }
    return image_collection_load_next(name);
}

void image_collection_load_all(const char **names)
{
    const image_collection_object *collection = &image_collection;
    int32_t ok = 1;
    while (collection->sg_count < IMAGE_SG_SLOTS) {
        const char *name = names[collection->sg_count];
        if (name && name[0]) {
            ok = image_collection_load_next(name);
        }
        if (!ok) {
            break;
        }
    }
}

intptr_t image_pixels_address(const image_header *header)
{
    if (header->data < 1) {
        return 0;
    }
    if (header->is_external != 0) {
        return header->data;
    }
    return (intptr_t) (image_collection.slots[header->file_slot].pixels + header->data);
}

int32_t image_get_size(int32_t image_id, int32_t *out_w, int32_t *out_h)
{
    *out_h = 0;
    *out_w = 0;
    const image_header *header = image_header_get(image_id);
    uint32_t header_address = (uint32_t) (uintptr_t) header;
    if (image_id > 0 && image_id < IMAGE_HEADERS && header != 0) {
        *out_w = header->width;
        *out_h = header->height;
        return (int32_t) ((header_address & ~0xFFu) | 1u);
    }
    return (int32_t) (header_address & ~0xFFu);
}

static int32_t read_rows(int32_t fd, uint8_t *buffer, int32_t width_bytes, int32_t height, int32_t file_offset, int32_t row_skip)
{
    int32_t total = 0;
    if (_lseek(fd, file_offset, SEEK_SET) != -1 && height > 0) {
        uint8_t *row = buffer;
        for (int32_t i = 0; i < height; i++) {
            total += _read(fd, row, (uint32_t) width_bytes);
            _lseek(fd, row_skip, SEEK_CUR);
            row += width_bytes;
        }
    }
    _close(fd);
    return total;
}

int32_t image_load_external_data(char *name, void *buf, int32_t width_bytes, int32_t height, int32_t file_offset)
{
    const image_collection_object *collection = &image_collection;
    char path[FILE_PATH_SIZE];
    memset(path, 0, sizeof(path));
    memcpy(path, data_path_prefix, sizeof(data_path_prefix));
    strcat(path, name);
    file_set_extension(path, file_extension_555);
    int32_t row_skip = ((int32_t) g_img_width * 2) - width_bytes;
    int32_t total = 0;
    int32_t fd = _open(path, _O_RDONLY | _O_BINARY, 0);
    if (fd != -1) {
        total += read_rows(fd, buf, width_bytes, height, file_offset, row_skip);
    }
    if (total == 0) {
        return 0;
    }
    image_convert_raw_555(collection->external_buffer, total);
    return total;
}
