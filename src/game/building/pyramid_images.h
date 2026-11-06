// The images of the pyramid parts and the area of the complex temple (building/pyramid_images.c).

#ifndef BUILDING_PYRAMID_IMAGES_H
#define BUILDING_PYRAMID_IMAGES_H

#include <stdint.h>

enum pyramid_complex_area_mode {
    PYRAMID_COMPLEX_AREA_WALK = 0,
    PYRAMID_COMPLEX_AREA_DRAW_GHOST = 1,
    PYRAMID_COMPLEX_AREA_MARK_GROUND = 2,
    PYRAMID_COMPLEX_AREA_FIND_FREE_TILE = 3,
    PYRAMID_COMPLEX_AREA_MARK_FREE_GROUND = 4,
    PYRAMID_COMPLEX_AREA_CLEAR = 5,
    PYRAMID_COMPLEX_AREA_FIND_BUILDING = 6,
};

void pyramid_complex_area_op(int32_t x, int32_t y, int32_t mode, int32_t param);
void city_view_tile_to_pixel(int32_t x, int32_t y, int32_t *px, int32_t *py);
void pyramid_part_update_image(int32_t building_id, int32_t view, int32_t size_variant);
void pyramid_part_pixel_offset(int32_t building_id, int32_t height, int32_t *px, int32_t *py);
void pyramid_part_draw(int32_t image, int32_t x, int32_t y, int32_t building_id);
void pyramid_figure_get_pixel_offset(int32_t figure_id, int32_t *out_x, int32_t *out_y);

#endif
