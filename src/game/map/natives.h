// The natives on the map (map/natives.c): their buildings and land, the crops and the huts' walk target.

#ifndef MAP_NATIVES_H
#define MAP_NATIVES_H

#include <stdint.h>

enum native_crops_frames {
    NATIVE_CROPS_FRAMES = 5,
};

void map_native_crops_update(int32_t building_id);
int32_t map_native_target_tile(int32_t x, int32_t y, int32_t size);
void map_native_land_mark(int32_t x, int32_t y, int32_t size, int32_t radius);
void map_native_land_unmark(int32_t x, int32_t y, int32_t size, int32_t radius);
void map_natives_init(void);

#endif
