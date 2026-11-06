// The sprite helpers of the top pass: image_draw_sprite_offset draws an image at its sprite origin plus an
// offset (image 0's offsets for ids past the image table).

#ifndef WIDGET_CITY_BUILDING_SPRITES_H
#define WIDGET_CITY_BUILDING_SPRITES_H

#include <stdint.h>

int32_t image_draw_sprite_offset(int32_t image_id, int32_t x, int32_t y, int32_t dx, int32_t dy);
void draw_workshop_raw_material_storage(int32_t building_id, int32_t x, int32_t y);

#endif
