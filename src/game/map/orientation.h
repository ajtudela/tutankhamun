// View rotation (map/orientation.c): the building images and corner tiles for the new orientation, the entry and exit
// flags, the bridges, the water buildings and the water lift image.

#ifndef MAP_ORIENTATION_H
#define MAP_ORIENTATION_H

#include "map/terrain.h"

#include <stdint.h>

enum view_turn {
    VIEW_TURNS = 4,
};

enum statue_data {
    STATUE_DATA_DIRECTION = 0x0F,
    STATUE_DATA_VARIANT_SHIFT = 4,
    STATUE_VARIANTS_IN_OWN_GROUP = 2,
};

enum water_lift_image {
    WATER_LIFT_IMAGE_FLOODED = 4,
    WATER_LIFT_IMAGE_FLOODED_WITH_WATER_ACCESS = 8,
};

enum entry_exit_flag_search {
    ENTRY_EXIT_FLAG_SEARCH_RADIUS = 10,
};

enum entry_exit_flag_image {
    ENTRY_FLAG_IMAGE = 0,
    EXIT_FLAG_IMAGE = 4,
};

void map_building_tiles_add_water_size3(int32_t building_id, int32_t x, int32_t y, int32_t image_id);
void map_building_tiles_add_water_size2(int32_t building_id, int32_t x, int32_t y, int32_t image_id);
void map_building_update_water_lift_image(int32_t building_id);
int32_t map_terrain_find_free_tile(int32_t x, int32_t y, int32_t size, int32_t radius, int32_t skip_offset);
void map_orientation_update_buildings(void);
void map_orientation_change(void);
void map_orientation_update_corner_tiles(void);
void map_orientation_rotate_bridges(void);
void map_tiles_update_entry_exit_flags(int32_t remove);
int32_t map_terrain_2x2_is_flooded(int32_t x, int32_t y);
int32_t map_building_is_flooded(int32_t building_id);
int32_t map_water_lift_image(int32_t direction, int32_t blood);

#endif
