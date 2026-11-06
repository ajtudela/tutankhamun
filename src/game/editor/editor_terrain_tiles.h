// The brush setters, terrain checks and floodplain fixes of editor_terrain_tiles.c, and the terrain masks of the
// brush: the bits every setter keeps, the extra bits a road or canal keeps, and the ground a floodplain cannot lie on.

#ifndef EDITOR_EDITOR_TERRAIN_TILES_H
#define EDITOR_EDITOR_TERRAIN_TILES_H

#include "map/terrain.h"
#include <stdbool.h>
#include <stdint.h>

#define EDITOR_BRUSH_KEPT_TERRAIN \
    (TERRAIN_GROUNDWATER | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_WELL_RANGE | TERRAIN_WALL | TERRAIN_GATEHOUSE | \
        TERRAIN_FERRY_ROUTE | TERRAIN_DIKE | TERRAIN_BRICK_WALL | TERRAIN_STRONG_WALL | TERRAIN_IRRIGATION_RANGE | TERRAIN_SUBMERGED_ROAD | TERRAIN_MONUMENT | \
        TERRAIN_SHORE)
#define EDITOR_ROAD_OR_CANAL_KEPT_TERRAIN (TERRAIN_WALKABLE_BUILDING | TERRAIN_FLOODPLAIN | TERRAIN_CANAL | TERRAIN_ROAD)
#define EDITOR_FLOODPLAIN_BLOCKING_TERRAIN \
    (TERRAIN_TREE | TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ELEVATION | TERRAIN_MEADOW | TERRAIN_RUBBLE | TERRAIN_MARSH | TERRAIN_DUNE | TERRAIN_ROCKS)

int32_t editor_tile_clear(int32_t x, int32_t y);
int32_t editor_tile_set_tree(int32_t x, int32_t y);
int32_t editor_tile_set_rock_kind(int32_t x, int32_t y, uint32_t kind);
int32_t editor_tile_set_dune(int32_t x, int32_t y);
int32_t editor_tile_set_meadow(int32_t x, int32_t y);
int32_t editor_tile_set_marsh(int32_t x, int32_t y);
int32_t editor_tile_set_floodplain(int32_t x, int32_t y);
int32_t editor_tile_set_water(int32_t x, int32_t y);
int32_t editor_tile_set_road(int32_t x, int32_t y);
int32_t editor_tile_set_canal(int32_t x, int32_t y);
int32_t editor_tile_deep_water_edge(int32_t x, int32_t y);
void editor_update_tiles_around(int32_t x, int32_t y, int32_t radius);
bool editor_terrain_corner_open(int32_t offset, uint32_t mask, bool check_diagonals);
bool editor_floodplain_spread(int32_t x, int32_t y);
bool editor_floodplain_shrink(int32_t x, int32_t y);
int32_t editor_find_best_neighbour(int32_t offset, uint32_t mask);
bool editor_floodplain_fill(int32_t x, int32_t y);
void editor_fix_region(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_fix_all(void);

#endif
