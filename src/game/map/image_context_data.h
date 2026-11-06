// The image context tables of map/map_data.c. Kept out of map/map_data.h because they need the context type of
// map/image_context.h, which cannot be included from there.

#ifndef MAP_IMAGE_CONTEXT_DATA_H
#define MAP_IMAGE_CONTEXT_DATA_H

#include "map/image_context.h"

extern terrain_image_context context_water[48];

extern terrain_image_context context_water_alt[48];

extern terrain_image_context context_floodsystem[216];

extern terrain_image_context context_wall[48];

extern terrain_image_context context_wall_gate[15];

extern terrain_image_context context_aqueduct[14];

extern terrain_image_context context_fault[17];

extern terrain_image_context context_deepwater[26];

extern terrain_image_context context_meadow[17];

extern terrain_image_context context_dirt_road[17];

extern terrain_image_context context_paved_road[48];

extern terrain_image_context context_canal[16];

#endif
