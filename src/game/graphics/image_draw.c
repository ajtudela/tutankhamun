// Image drawing on the RGB565 back buffer: image_draw, image_draw_mirrored and an image's isometric top, and
// the clip helpers the other image draws share. By draw type an image goes to the raw, compressed, footprint,
// square, two-pixel or one-colour blitters; external images are read from their .555 file first, and a
// full-screen picture is copied to the screen (centred on screen sizes other than the standard ones, the rest
// filled black).
//
// Not every image reaches the screen as expected:
// - an id not below the image count draws image 0;
// - image_draw_mirrored stores the data address before testing it and draws raw images without the colour mask;
// - one-tile footprints of widths 26 and 10 draw nothing.

#include "graphics/image_draw.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_blit.h"
#include "graphics/image_blit_mask.h"
#include "graphics/image_blit_square.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw_footprint.h"
#include "io/file.h"
#include "io/io_data.h"
#include "widget/screen_layout.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

int32_t g_height_for_clip;

int32_t g_width_for_clip;

int32_t g_clip_left;

char g_ext_path[68];

enum isometric_footprint {
    FOOTPRINT_SMALL_WIDTH = 26,
    FOOTPRINT_SMALL_HEIGHT = 14,
    FOOTPRINT_TINY_WIDTH = 10,
    FOOTPRINT_TINY_HEIGHT = 6,
};

enum fullscreen_picture {
    FULLSCREEN_PICTURE_WIDTH = 1024,
    FULLSCREEN_PICTURE_HEIGHT = 768,
    EXTERNAL_PATH_CLEARED = 0x100,
};

static const uint8_t *pixels(int32_t address)
{
    return (const uint8_t *) (uintptr_t) address;
}

static const image_header *image_entry(const image_collection_object *collection, int32_t image_id)
{
    return &collection->headers[(image_id >= collection->image_count) ? 0 : image_id];
}

static const char *image_bitmap_name(const image_collection_object *collection, int32_t image_id)
{
    const image_header *header = image_entry(collection, image_id);
    return collection->slots[header->file_slot].bitmap_names[header->bitmap_name_index];
}

static void set_horizontally_invisible(void)
{
    g_clip_x = IMAGE_CLIP_INVISIBLE;
    g_visible_w = 0;
}

void graphics_clip_horizontal(void)
{
    int32_t width = (int32_t) g_img_width;
    g_clip_x = IMAGE_CLIP_NONE;
    g_clip_left = 0;
    g_width_for_clip = width;
    if (g_img_width < 1) {
        set_horizontally_invisible();
    } else if (g_draw_x < g_clip_x0) {
        if (width + g_draw_x <= g_clip_x0) {
            set_horizontally_invisible();
        } else {
            g_clip_x = IMAGE_CLIP_LEFT;
            g_clip_left = g_clip_x0 - g_draw_x;
            g_draw_x = g_clip_x0;
            g_visible_w = g_width_for_clip - g_clip_left;
        }
    } else if (g_clip_x1 - width < g_draw_x) {
        if (g_clip_x1 <= g_draw_x) {
            set_horizontally_invisible();
        } else {
            g_width_for_clip = g_clip_x1 - g_draw_x;
            g_clip_x = IMAGE_CLIP_RIGHT;
            g_visible_w = g_width_for_clip - g_clip_left;
        }
    } else {
        g_visible_w = g_width_for_clip - g_clip_left;
    }
    g_src_skip = (width - g_visible_w) * 2;
    g_dst_skip = (display_width - g_visible_w) * 2;
}

static void set_vertically_invisible(void)
{
    g_clip_y = IMAGE_CLIP_INVISIBLE;
    g_visible_h = 0;
}

void graphics_clip_vertical(void)
{
    int32_t y = g_draw_y;
    int32_t height = (int32_t) g_img_height;
    g_height_for_clip = height;
    g_clip_y = IMAGE_CLIP_NONE;
    g_clip_top = 0;
    if (g_img_height < 1) {
        set_vertically_invisible();
    } else if (g_draw_y < g_clip_y0) {
        if (height + g_draw_y <= g_clip_y0) {
            set_vertically_invisible();
        } else {
            g_draw_y = g_clip_y0;
            g_clip_top = g_clip_y0 - y;
            g_clip_y = IMAGE_CLIP_TOP;
            height = (g_clip_y1 - g_clip_y0) + g_clip_top;
            if (height < g_height_for_clip) {
                g_height_for_clip = height;
            }
            g_visible_h = g_height_for_clip - g_clip_top;
        }
    } else if (g_clip_y1 - height < g_draw_y) {
        if (g_draw_y < g_clip_y1) {
            g_clip_y = IMAGE_CLIP_BOTTOM;
            g_height_for_clip = g_clip_y1 - g_draw_y;
            g_visible_h = g_height_for_clip - g_clip_top;
        } else {
            set_vertically_invisible();
        }
    } else {
        g_visible_h = g_height_for_clip - g_clip_top;
    }
    if (g_clip_x == IMAGE_CLIP_INVISIBLE) {
        g_clip_y = IMAGE_CLIP_INVISIBLE;
    }
}

static int32_t clipped_left_or_right(uint8_t clip_x)
{
    return clip_x == IMAGE_CLIP_LEFT || clip_x == IMAGE_CLIP_RIGHT;
}

static void draw_compressed(const uint8_t *data, int32_t mirrored)
{
    uint8_t clip_x = g_clip_x;
    if (mirrored) {
        if (clip_x == IMAGE_CLIP_LEFT) {
            image_blit_compressed_mirrored_left(data);
        } else if (clip_x == IMAGE_CLIP_RIGHT) {
            image_blit_compressed_mirrored_right(data);
        } else {
            image_blit_compressed_mirrored(data);
        }
    } else if (clip_x == IMAGE_CLIP_LEFT) {
        image_blit_compressed_left(data);
    } else if (clip_x == IMAGE_CLIP_RIGHT) {
        image_blit_compressed_right(data);
    } else {
        image_blit_compressed(data);
    }
}

static void draw_compressed_masked(int32_t data)
{
    uint16_t mask = city_view.color_mask;
    if (!mask) {
        return;
    }
    uint8_t clip_x = g_clip_x;
    if (clip_x == IMAGE_CLIP_LEFT) {
        image_blit_compressed_mask_left(pixels(data), mask);
    } else if (clip_x == IMAGE_CLIP_RIGHT) {
        image_blit_compressed_mask_right(pixels(data), mask);
    } else {
        image_blit_compressed_mask(pixels(data), mask);
    }
}

static void draw_raw(const uint8_t *data, int32_t mirrored)
{
    uint8_t clip_x = g_clip_x;
    if (mirrored) {
        if (clipped_left_or_right(clip_x)) {
            image_blit_raw_mirrored_clipped(data);
        } else {
            image_blit_raw_mirrored(data);
        }
    } else if (clipped_left_or_right(clip_x)) {
        image_blit_raw_clipped(data);
    } else {
        image_blit_raw(data);
    }
}

static void draw_raw_masked(int32_t data)
{
    uint16_t mask = city_view.color_mask;
    if (!mask) {
        return;
    }
    if (clipped_left_or_right(g_clip_x)) {
        image_blit_raw_mask_clipped(pixels(data), mask);
    } else {
        image_blit_raw_mask(pixels(data), mask);
    }
}

static int32_t footprint_tiles(int16_t width)
{
    for (int32_t tiles = 1; tiles <= FOOTPRINT_MAX_TILES; tiles++) {
        if (width == (tiles * FOOTPRINT_TILE_WIDTH) - 2) {
            return tiles;
        }
    }
    return 0;
}

static void apply_isometric_top_offsets(int32_t x, int32_t y, int16_t width, int16_t height)
{
    int32_t tiles = footprint_tiles(width);
    if (tiles == 0) {
        return;
    }
    g_draw_x = x - ((tiles - 1) * FOOTPRINT_TILE_HEIGHT);
    g_draw_y = (y - height) + (tiles * FOOTPRINT_TILE_HEIGHT);
    g_img_height = (int16_t) (height - ((tiles * FOOTPRINT_TILE_HALF_HEIGHT) + 1));
}

static void build_external_path(char *path, const image_collection_object *collection, int32_t image_id)
{
    memcpy(path, data_path_prefix, sizeof(data_path_prefix));
    memset(path + sizeof(data_path_prefix), 0, EXTERNAL_PATH_CLEARED - sizeof(data_path_prefix));
    strcat(path, image_bitmap_name(collection, image_id));
    file_set_extension(path, file_extension_555);
}

static int32_t load_external_pixels(image_collection_object *collection, const image_header *header, int32_t image_id)
{
    char path[FILE_PATH_SIZE];
    build_external_path(path, collection, image_id);
    image_buf_reset(&collection->external_buffer, 0);
    collection->external_buffer = (uint8_t *) spare_screen;
    int32_t data_length = header->data_length;
    uint8_t *buffer = collection->external_buffer;
    if (!io_read_file_part(path, buffer, data_length, header->data - 1)) {
        return 0;
    }
    if (header->is_compressed == 0) {
        image_convert_raw_555(buffer, data_length);
    } else {
        image_convert_compressed_555(buffer, data_length);
    }
    return 1;
}

static int32_t draw_external_file(image_collection_object *collection, const image_header *header, int32_t image_id, int32_t mirrored)
{
    if (!load_external_pixels(collection, header, image_id)) {
        return 0;
    }
    graphics_clip_horizontal();
    graphics_clip_vertical();
    int32_t width = (int32_t) g_img_width;
    if (g_clip_y == IMAGE_CLIP_INVISIBLE) {
        return 0;
    }
    const uint8_t *buffer = collection->external_buffer;
    if (g_img_type == IMAGE_DRAW_PLAIN && header->is_compressed != 0) {
        draw_compressed(buffer, mirrored);
        return width;
    }
    g_data_ptr = ((width * g_clip_top) + g_clip_left) * 2;
    draw_raw(buffer + g_data_ptr, mirrored);
    return width;
}

static int32_t draw_external_fullscreen(image_collection_object *collection, int32_t image_id)
{
    int32_t width = g_screen_width;
    int32_t height = g_screen_height;
    image_view_width = width;
    g_visible_w = width;
    image_view_x = (FULLSCREEN_PICTURE_WIDTH - width) / 2;
    image_view_y = (FULLSCREEN_PICTURE_HEIGHT - height) / 2;
    int32_t offset = (((int32_t) g_img_width * image_view_y) + image_view_x) * 2;
    image_view_height = height;
    g_visible_h = height;
    const char *name = image_bitmap_name(collection, image_id);
    memcpy(g_ext_path, name, strlen(name) + 1);
    image_buf_reset(&collection->external_buffer, 0);
    collection->external_buffer = (uint8_t *) spare_screen;
    g_draw_y = 0;
    g_draw_x = 0;
    image_load_external_data(g_ext_path, collection->external_buffer, g_visible_w * 2, g_visible_h, offset);
    g_src_skip = 0;
    g_dst_skip = 0;
    image_blit_copy(collection->external_buffer);
    return g_visible_w;
}

static int32_t smaller(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

static int32_t is_fullscreen_picture_of_screen_layout(const image_header *header)
{
    return !screen_layout_is_standard_size() && header->is_compressed == 0 && (int32_t) g_img_width >= SCREEN_LAYOUT_PICTURE_WIDTH &&
        (int32_t) g_img_height >= SCREEN_LAYOUT_PICTURE_HEIGHT;
}

static void fill_around(int32_t x, int32_t y, int32_t width, int32_t height, int32_t area_x, int32_t area_y, int32_t area_width, int32_t area_height)
{
    graphics_fill_rect(area_x, area_y, area_width, y - area_y, 0);
    graphics_fill_rect(area_x, y + height, area_width, (area_y + area_height) - (y + height), 0);
    graphics_fill_rect(area_x, y, x - area_x, height, 0);
    graphics_fill_rect(x + width, y, (area_x + area_width) - (x + width), height, 0);
}

static int32_t draw_external_centred(image_collection_object *collection, const image_header *header, int32_t image_id)
{
    int32_t picture_width = (int32_t) g_img_width;
    int32_t picture_height = (int32_t) g_img_height;
    int32_t width = smaller(g_screen_width, picture_width);
    int32_t height = smaller(g_screen_height, picture_height);
    image_view_width = width;
    image_view_height = height;
    image_view_x = (picture_width - width) / 2;
    image_view_y = (picture_height - height) / 2;
    int32_t offset = (header->data - 1) + (((picture_width * image_view_y) + image_view_x) * 2);
    g_visible_w = width;
    g_visible_h = height;
    const char *name = image_bitmap_name(collection, image_id);
    memcpy(g_ext_path, name, strlen(name) + 1);
    image_buf_reset(&collection->external_buffer, 0);
    collection->external_buffer = (uint8_t *) spare_screen;
    int32_t x = (g_screen_width - width) / 2;
    int32_t y = (g_screen_height - height) / 2;
    image_load_external_data(g_ext_path, collection->external_buffer, width * 2, height, offset);
    g_draw_x = x;
    g_draw_y = y;
    g_src_skip = 0;
    g_dst_skip = (display_width - width) * 2;
    image_blit_copy(collection->external_buffer);
    g_dst_skip = 0;
    fill_around(x, y, width, height, 0, 0, g_screen_width, g_screen_height);
    return width;
}

static int32_t draw_isometric_clipped(int16_t width)
{
    uint8_t clip_x = g_clip_x;
    uint8_t clip_y = g_clip_y;
    int32_t is_small = width == FOOTPRINT_SMALL_WIDTH || width == FOOTPRINT_TINY_WIDTH;
    if (width == FOOTPRINT_ONE_TILE_WIDTH) {
        if (clip_x == IMAGE_CLIP_LEFT) {
            image_draw_footprint_tile_left(g_data_ptr, clip_y);
        } else if (clip_x == IMAGE_CLIP_RIGHT) {
            image_draw_footprint_tile_right(g_data_ptr, clip_y);
        } else {
            image_draw_footprint_tile((const uint32_t *) (uintptr_t) g_data_ptr, clip_y);
        }
    } else if (is_small) {
        return g_visible_w;
    } else if (clip_x == IMAGE_CLIP_LEFT) {
        image_blit_raw_clipped(pixels(g_data_ptr));
    } else if (clip_x == IMAGE_CLIP_RIGHT) {
        image_blit_raw_mirrored_clipped(pixels(g_data_ptr));
    } else {
        image_blit_raw_mirrored(pixels(g_data_ptr));
    }
    return g_visible_w;
}

static int32_t draw_footprint(int32_t tiles)
{
    switch (tiles) {
        case 2:
            return image_draw_footprint_size1();
        case 3:
            return image_draw_footprint_size2();
        case 4:
            return image_draw_footprint_size3();
        case 5:
            return image_draw_footprint_size4();
        default:
            return image_draw_footprint_size5();
    }
}

static int32_t draw_special_type(uint8_t type)
{
    uint8_t clip_x = g_clip_x;
    if (type == IMAGE_DRAW_ONE_COLOR) {
        if (clip_x == IMAGE_CLIP_LEFT) {
            image_blit_compressed_color_left(pixels(g_data_ptr));
        } else if (clip_x == IMAGE_CLIP_RIGHT) {
            image_blit_compressed_color_right(pixels(g_data_ptr));
        } else {
            image_blit_compressed_color(pixels(g_data_ptr));
        }
        return 1;
    }
    if (clip_x == IMAGE_CLIP_NONE) {
        return 0;
    }
    switch (type) {
        case IMAGE_DRAW_SQUARE_32:
            image_blit_square32((const uint32_t *) (uintptr_t) g_data_ptr);
            return 1;
        case IMAGE_DRAW_SQUARE_24:
            image_blit_square24((const uint32_t *) (uintptr_t) g_data_ptr);
            return 1;
        case IMAGE_DRAW_SQUARE_16:
            image_blit_square16((const uint32_t *) (uintptr_t) g_data_ptr);
            return 1;
        case IMAGE_DRAW_TWO_PIXELS:
            image_blit_two_pixels(pixels(g_data_ptr));
            return 1;
        default:
            return 0;
    }
}

static int32_t draw_normal(const image_header *header, int32_t mirrored, int32_t mask_raw)
{
    uint8_t type = g_img_type;
    int16_t width = g_img_width;
    if (type == IMAGE_DRAW_ISOMETRIC) {
        if (width == FOOTPRINT_ONE_TILE_WIDTH) {
            g_img_height = FOOTPRINT_ONE_TILE_HEIGHT;
        } else if (width == FOOTPRINT_SMALL_WIDTH) {
            g_img_height = FOOTPRINT_SMALL_HEIGHT;
        } else if (width == FOOTPRINT_TINY_WIDTH) {
            g_img_height = FOOTPRINT_TINY_HEIGHT;
        } else if (footprint_tiles(width) >= 2) {
            return draw_footprint(footprint_tiles(width));
        }
    }
    graphics_clip_horizontal();
    graphics_clip_vertical();
    if (g_clip_y == IMAGE_CLIP_INVISIBLE) {
        return 0;
    }
    if (type == IMAGE_DRAW_PLAIN) {
        if (header->is_compressed == 0) {
            g_data_ptr = g_data_ptr + (((g_img_width * g_clip_top) + g_clip_left) * 2);
            draw_raw(pixels(g_data_ptr), mirrored);
            if (mask_raw) {
                draw_raw_masked(g_data_ptr);
            }
            return g_visible_w;
        }
        draw_compressed(pixels(g_data_ptr), mirrored);
        draw_compressed_masked(g_data_ptr);
        return g_visible_w;
    }
    if (type == IMAGE_DRAW_ISOMETRIC) {
        return draw_isometric_clipped(width);
    }
    if (draw_special_type(type)) {
        return g_visible_w;
    }
    uint8_t clip_x = g_clip_x;
    g_data_ptr = g_data_ptr + (((g_img_width * g_clip_top) + g_clip_left) * 2);
    if (mirrored) {
        if (clipped_left_or_right(clip_x)) {
            image_blit_raw_mirrored_clipped(pixels(g_data_ptr));
        } else {
            image_blit_raw_mirrored(pixels(g_data_ptr));
        }
        return g_visible_w;
    }
    if (clipped_left_or_right(clip_x)) {
        image_blit_raw_clipped(pixels(g_data_ptr));
        return g_visible_w;
    }
    if (g_clip_y != IMAGE_CLIP_NONE) {
        image_blit_raw(pixels(g_data_ptr));
        return g_visible_w;
    }
    image_blit_copy(pixels(g_data_ptr));
    return g_visible_w;
}

int32_t image_draw_isometric_top(int32_t image_id, int32_t x, int32_t y)
{
    if (image_id < 1 || image_id >= IMAGE_HEADERS) {
        return 0;
    }
    const image_header *header = image_entry(&image_collection, image_id);
    if (header->has_isometric_top == 0) {
        return 0;
    }
    int32_t data = header->data;
    if (data < 1) {
        return 0;
    }
    g_data_ptr = data + header->isometric_top_offset;
    int16_t width = header->width;
    g_img_width = width;
    apply_isometric_top_offsets(x, y, width, header->height);
    graphics_clip_horizontal();
    graphics_clip_vertical();
    if (g_clip_y != IMAGE_CLIP_INVISIBLE) {
        draw_compressed(pixels(g_data_ptr), 0);
        draw_compressed_masked(g_data_ptr);
    }
    return 1;
}

static int32_t is_empire_map_picture(int32_t image_id)
{
    return (window_current_id == WINDOW_EDITOR_EMPIRE || window_current_id == WINDOW_EMPIRE) && image_id == image_group(IMAGE_GROUP_FULLSCREEN);
}

static int32_t draw_empire_map_view_of_screen_layout(void)
{
    int32_t area_x = g_draw_x;
    int32_t area_y = g_draw_y;
    int32_t skip_x = image_view_x < 0 ? -image_view_x : 0;
    int32_t skip_y = image_view_y < 0 ? -image_view_y : 0;
    int32_t width = smaller(image_view_width - skip_x, (int32_t) g_img_width - (image_view_x + skip_x));
    int32_t height = smaller(image_view_height - skip_y, (int32_t) g_img_height - (image_view_y + skip_y));
    if (width <= 0 || height <= 0) {
        graphics_fill_rect(area_x, area_y, image_view_width, image_view_height, 0);
        return 0;
    }
    g_draw_x = area_x + skip_x;
    g_draw_y = area_y + skip_y;
    g_visible_h = height;
    g_visible_w = width;
    g_dst_skip = (display_width - width) * 2;
    g_src_skip = ((int32_t) g_img_width - width) * 2;
    int32_t view = g_data_ptr + ((((int32_t) g_img_width * (image_view_y + skip_y)) + image_view_x + skip_x) * 2);
    image_blit_copy(pixels(view));
    fill_around(area_x + skip_x, area_y + skip_y, width, height, area_x, area_y, image_view_width, image_view_height);
    return width;
}

static int32_t draw_empire_map_view(void)
{
    if (!screen_layout_is_standard_size()) {
        return draw_empire_map_view_of_screen_layout();
    }
    g_visible_h = image_view_height;
    g_dst_skip = (display_width - image_view_width) * 2;
    g_src_skip = ((int32_t) g_img_width - image_view_width) * 2;
    g_visible_w = image_view_width;
    int32_t view = g_data_ptr + ((((int32_t) g_img_width * image_view_y) + image_view_x) * 2);
    image_blit_copy(pixels(view));
    return g_visible_w;
}

static int32_t draw_header(image_collection_object *collection, const image_header *header, int32_t image_id, int32_t mirrored, int32_t mask_raw)
{
    if (header->is_external != 0) {
        if (is_fullscreen_picture_of_screen_layout(header)) {
            return draw_external_centred(collection, header, image_id);
        }
        if ((int32_t) g_img_width <= display_width) {
            return draw_external_file(collection, header, image_id, mirrored);
        }
        return draw_external_fullscreen(collection, image_id);
    }
    if (is_empire_map_picture(image_id)) {
        return draw_empire_map_view();
    }
    return draw_normal(header, mirrored, mask_raw);
}

static void remember_draw(int32_t image_id, int32_t x, int32_t y)
{
    image_draw_last_x = x;
    image_draw_last_id = image_id;
    image_draw_last_y = y;
}

int32_t image_draw(int32_t image_id, int32_t x, int32_t y)
{
    image_collection_object *collection = &image_collection;
    remember_draw(image_id, x, y);
    if (image_id < 1 || image_id >= IMAGE_HEADERS) {
        return 0;
    }
    const image_header *header = image_entry(collection, image_id);
    int32_t mirrored = 0;
    if (header->is_mirrored != 0) {
        header = image_entry(collection, header->mirror_source_offset + image_id);
        mirrored = 1;
    }
    int32_t data = header->data;
    if (data < 1) {
        return 0;
    }
    g_data_ptr = data;
    g_img_width = header->width;
    g_img_height = header->height;
    g_img_type = header->draw_type;
    g_draw_x = x;
    g_draw_y = y;
    g_visible_w = (int32_t) g_img_width;
    return draw_header(collection, header, image_id, mirrored, 1);
}

int32_t image_draw_mirrored(int32_t image_id, int32_t x, int32_t y)
{
    image_collection_object *collection = &image_collection;
    remember_draw(image_id, x, y);
    const image_header *header = image_entry(collection, image_id);
    if (image_id < 1 || image_id >= IMAGE_HEADERS) {
        return 0;
    }
    int32_t data = header->data;
    g_data_ptr = data;
    if (data < 1) {
        return 0;
    }
    g_img_width = header->width;
    g_img_height = header->height;
    g_img_type = header->draw_type;
    g_draw_y = y;
    g_draw_x = x;
    g_visible_w = (int32_t) g_img_width;
    return draw_header(collection, header, image_id, 1, 0);
}
