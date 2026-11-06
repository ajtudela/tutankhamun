// Soil fertility of the floodplain (map/soil.c): the fertility update of a rectangle and the ring shares.

#ifndef MAP_SOIL_H
#define MAP_SOIL_H

#include <stdbool.h>
#include <stdint.h>

enum soil_fertility {
    SOIL_FERTILITY_MAX = 99,
};

enum floodplain_fertility_images {
    FLOODPLAIN_FERTILITY_IMAGES = 12,
};

int32_t map_terrain_ring_share(int32_t x, int32_t y, int32_t radius, uint32_t mask);
int32_t map_floodplain_ring_share(int32_t x, int32_t y, int32_t radius);
void map_soil_fertility_update_rect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool update_meadow);

#endif
