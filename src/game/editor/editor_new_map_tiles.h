// The functions of editor_new_map_tiles.c, the numbers of the editor's flag figures (which special point a flag
// stands on) and the empire map object fields the empire editor's tick reads.

#ifndef EDITOR_EDITOR_NEW_MAP_TILES_H
#define EDITOR_EDITOR_NEW_MAP_TILES_H

#include <stdint.h>

enum editor_flag_number {
    EDITOR_FLAG_NONE = 0,
    EDITOR_FLAG_EARTHQUAKE = 1,
    EDITOR_FLAG_ENTRY = 2,
    EDITOR_FLAG_EXIT = 3,
    EDITOR_FLAG_RIVER_ENTRY = 12,
    EDITOR_FLAG_RIVER_EXIT = 13,
    EDITOR_FLAG_FISHING_FIRST = 14,
    EDITOR_FLAG_FISHING_LAST = 21,
    EDITOR_FLAG_PREDATOR_FIRST = 22,
    EDITOR_FLAG_PREDATOR_LAST = 25,
    EDITOR_FLAG_PREY_FIRST = 26,
    EDITOR_FLAG_PREY_LAST = 29,
    EDITOR_FLAG_LANDING_FIRST = 30,
    EDITOR_FLAG_LANDING_LAST = 32,
    EDITOR_FLAG_INVASION_FIRST = 50,
    EDITOR_FLAG_INVASION_LAST = 65,
};

enum empire_object_field {
    EMPIRE_OBJECT_FIELD_KIND = 0,
    EMPIRE_OBJECT_FIELD_IN_USE = 1,
    EMPIRE_OBJECT_FIELD_CITY_TYPE = 24,
    EMPIRE_OBJECT_FIELD_COUNTDOWN = 51,
    EMPIRE_OBJECT_KIND_CITY = 1,
};

void editor_set_climate(int32_t climate);
void map_tiles_update_region_dunes(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_dune_image(int32_t x, int32_t y, int32_t set);
void figure_editor_flag_action(void);
void empire_editor_update_city_kinds(void);
void map_images_set_inside(void);
void map_tiles_update_all_canals(void);

#endif
