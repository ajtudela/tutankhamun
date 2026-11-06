// Tile images of the map (map/tiles.c, map/tiles_road.c): the image updates of a tile, a region or the whole map after
// the terrain changed, and the blood river sweep.

#ifndef MAP_TILES_H
#define MAP_TILES_H

#include "map/tiles_road.h"

#include <stdbool.h>
#include <stdint.h>

enum empty_land_redraw {
    EMPTY_LAND_REDRAW_MARGIN = 5,
};

enum rock_monument_image {
    ROCK_MONUMENT_IMAGE_OUTER_CORNER = 0,
    ROCK_MONUMENT_IMAGE_INNER_CORNER = 12,
    ROCK_MONUMENT_IMAGE_SIDE = 24,
    ROCK_MONUMENT_IMAGE_FULL = 48,
    ROCK_MONUMENT_IMAGE_TWO_CORNERS = 72,
};

enum rock_monument_variants {
    ROCK_MONUMENT_CORNER_VARIANTS = 3,
    ROCK_MONUMENT_SIDE_VARIANTS = 6,
    ROCK_MONUMENT_FULL_VARIANTS = 24,
};

int32_t map_sweep_has_passed(int32_t x);
int32_t map_sweep_is_at(int32_t x);
void map_tiles_set_vegetation_image(int32_t offset);
void map_tiles_set_water_image(int32_t x, int32_t y, int32_t set);
void map_tiles_update_region_water(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t update_deepwater);
void map_tiles_set_marshland_image(int32_t x, int32_t y, int32_t set);
void map_tiles_update_region_marshland(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_tree_image(int32_t x, int32_t y, int32_t set);
void map_tiles_update_region_trees(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_meadow_image(int32_t x, int32_t y, int32_t set);
void map_tiles_update_region_meadow(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_floodplain_image(int32_t x, int32_t y, int32_t set);
void map_tiles_update_region_floodplain(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_rubble_image(int32_t x, int32_t y);
void map_tiles_update_region_rubble(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void map_tiles_set_fault_image(int32_t x, int32_t y, int32_t set);
void map_tiles_update_region_faults(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
int32_t map_tiles_rock_monument_image(int32_t offset);
void map_tiles_update_all_rocks(void);
void map_tiles_update_all_gardens(void);
void map_tiles_update_all(void);
int32_t map_terrain_exists_tile_in_radius_with_type(int32_t x, int32_t y, int32_t size, int32_t radius, uint32_t mask);
void map_tiles_update_all_dunes(void);
void map_tiles_update_all_plazas(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t clear);
void map_tiles_update_region_empty_land(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t keep_drawn, int32_t exact);
void map_tiles_update_empty_land_around(int32_t x, int32_t y, int32_t size);
bool map_plaza_square_fits(int32_t offset);

#endif
