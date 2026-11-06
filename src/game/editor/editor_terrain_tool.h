// The terrain tools of editor_terrain_tool.c and the editor's tool kinds (enum editor_tool), named after their texts.

#ifndef EDITOR_EDITOR_TERRAIN_TOOL_H
#define EDITOR_EDITOR_TERRAIN_TOOL_H

#include "building/building_type.h"
#include <stdint.h>
enum editor_tool {
    EDITOR_TOOL_GRASS = 0,
    EDITOR_TOOL_TREES = 1,
    EDITOR_TOOL_WATER = 2,
    EDITOR_TOOL_EARTHQUAKE = 3,
    EDITOR_TOOL_SCRUB = 4,
    EDITOR_TOOL_ROCKS = 5,
    EDITOR_TOOL_MEADOW = 6,
    EDITOR_TOOL_PLATEAU = 7,
    EDITOR_TOOL_BRUSH_SIZE = 8,
    EDITOR_TOOL_ACCESS_RAMP = 9,
    EDITOR_TOOL_ROAD = 10,
    EDITOR_TOOL_RAISE_LAND = 11,
    EDITOR_TOOL_LOWER_LAND = 12,
    EDITOR_TOOL_INVASION_POINT = 13,
    EDITOR_TOOL_MIGRATION_POINT = 14,
    EDITOR_TOOL_ENTRY_POINT = 15,
    EDITOR_TOOL_EXIT_POINT = 16,
    EDITOR_TOOL_RIVER_POINT = 17,
    EDITOR_TOOL_RIVER_IN = 18,
    EDITOR_TOOL_RIVER_OUT = 19,
    EDITOR_TOOL_NATIVES = 20,
    EDITOR_TOOL_NATIVE_HUT = 21,
    EDITOR_TOOL_NATIVE_CENTER = 22,
    EDITOR_TOOL_NATIVE_FIELD = 23,
    EDITOR_TOOL_FISHING_WATERS = 24,
    EDITOR_TOOL_KILLER_POINT = 25,
    EDITOR_TOOL_FLOODPLAIN = 26,
    EDITOR_TOOL_IRRIGATION = 27,
    EDITOR_TOOL_PREY = 28,
    EDITOR_TOOL_MARSHLAND = 29,
    EDITOR_TOOL_SAND_DUNES = 30,
    EDITOR_TOOL_DISEMBARK_POINT = 31,
    EDITOR_TOOL_KILLER_TYPE = 32,
    EDITOR_TOOL_CLIFF = 33,
};

void editor_tool_update_use(int32_t x, int32_t y);
void editor_elevation_smooth_all(void);
void editor_region_clear(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_elevation(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_floodplain(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_meadow(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_road(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_shrub(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_canal(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_dune(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_marsh(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_trees(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_water(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_region_rock_kind(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void editor_elevation_smooth_pass(void);
int32_t editor_tile_raise(int32_t x, int32_t y);
int32_t editor_tile_lower(int32_t x, int32_t y);

#endif
