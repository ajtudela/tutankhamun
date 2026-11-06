// Tiles of the build tool (map/construction_tiles.c): preview marks and site checks, walls placed while dragging and
// the roads redrawn around a cleared area.

#ifndef MAP_CONSTRUCTION_TILES_H
#define MAP_CONSTRUCTION_TILES_H

#include <stdbool.h>
#include <stdint.h>

enum construction_tiles_limit {
    BUILDING_MAP_EDGE_MARGIN = 3,
};

void map_tiles_update_roads_around(int32_t x, int32_t y, int32_t size);
void map_tiles_set_wall(int32_t x, int32_t y, int32_t set, uint32_t bits);
void map_building_tiles_mark_construction(int32_t size, int32_t x, int32_t y, uint32_t mask, int32_t fixed, int32_t floodplain_ok);
int32_t map_building_tiles_area_free(int32_t size, int32_t x, int32_t y, uint32_t mask, bool floodplain_ok);
void map_building_tiles_clear_marks(int32_t keep, int32_t x, int32_t y, int32_t size_x, int32_t size_y);
void map_building_tiles_mark_square(void);

#endif
