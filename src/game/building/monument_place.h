// Monument placement checks and part images (building/monument_place.c).

#ifndef BUILDING_MONUMENT_PLACE_H
#define BUILDING_MONUMENT_PLACE_H

#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include <stdbool.h>
#include <stdint.h>

enum pyramid_place_blocked {
    PYRAMID_PLACE_PART_TILES_BLOCKED = 1,
    PYRAMID_PLACE_COMPLEX_TEMPLE_BLOCKED = 2,
    PYRAMID_PLACE_RAMP_BLOCKED = 4,
};

enum monument_part_image_flag {
    MONUMENT_PART_IMAGE_PER_TILE = 1,
    MONUMENT_PART_IMAGE_EDGE_TILES = 2,
    MONUMENT_PART_IMAGE_UNBUILT = 4,
};

int32_t abu_simbel_part_image(monument_object *self, int32_t id, int32_t *flags);
int32_t sun_temple_part_image(int32_t id, int32_t *flags);
bool pyramid_can_place(int32_t offset, int32_t size, int32_t *reason);
bool royal_tomb_can_place(tomb_object *self, int32_t offset, const monument_part_desc *descs, int32_t count);
bool small_royal_tomb_can_place(tomb_object *self, int32_t offset);
bool medium_royal_tomb_can_place(tomb_object *self, int32_t offset);
bool large_royal_tomb_can_place(tomb_object *self, int32_t offset);
bool grand_royal_tomb_can_place(tomb_object *self, int32_t offset);

int32_t small_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags);
int32_t medium_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags);
int32_t large_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags);
int32_t grand_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags);

#endif
