// The building info window (building_info.c): its modes, the terrain kinds of a tile without a building, the height
// classes of the window, and the window's heights and placements per screen size
// (building_info_window_heights.json, building_info_window_placements.json).

#ifndef WINDOW_BUILDING_INFO_H
#define WINDOW_BUILDING_INFO_H

#include "building/building.h"
#include "window/window_data.h"

#include <stdint.h>

enum building_info_mode {
    BUILDING_INFO_MODE_NONE = 0,
    BUILDING_INFO_MODE_TERRAIN = 1,
    BUILDING_INFO_MODE_BUILDING = 2,
    BUILDING_INFO_MODE_LEGION = 4,
    BUILDING_INFO_MODE_WARSHIP = 5,
    BUILDING_INFO_MODE_TRANSPORT_SHIP = 6,
};

enum building_info_terrain_kind {
    TERRAIN_INFO_BUILDING = -2,
    TERRAIN_INFO_UNCHANGED = -1,
    TERRAIN_INFO_NONE = 0,
    TERRAIN_INFO_TREE = 1,
    TERRAIN_INFO_ROCK = 2,
    TERRAIN_INFO_WATER = 3,
    TERRAIN_INFO_SHRUB = 4,
    TERRAIN_INFO_FAULT = 5,
    TERRAIN_INFO_ROAD = 6,
    TERRAIN_INFO_CANAL = 7,
    TERRAIN_INFO_RUBBLE = 8,
    TERRAIN_INFO_MUD_WALL = 9,
    TERRAIN_INFO_EMPTY = 10,
    TERRAIN_INFO_BRIDGE = 11,
    TERRAIN_INFO_GARDEN = 12,
    TERRAIN_INFO_PLAZA = 13,
    TERRAIN_INFO_ENTRY_FLAG = 14,
    TERRAIN_INFO_EXIT_FLAG = 15,
    TERRAIN_INFO_ORE_ROCK = 16,
    TERRAIN_INFO_NORMAL_ROCK = 17,
    TERRAIN_INFO_FLOODPLAIN = 19,
    TERRAIN_INFO_SUBMERGED_FLOODPLAIN = 20,
    TERRAIN_INFO_MARSH = 21,
    TERRAIN_INFO_DUNE = 22,
    TERRAIN_INFO_BRICK_WALL = 23,
    TERRAIN_INFO_STRONG_WALL = 24,
    TERRAIN_INFO_MEADOW = 25,
    TERRAIN_INFO_CLIFF = 26,
};

enum building_info_height_class {
    BUILDING_INFO_HEIGHT_LARGE = 0,
    BUILDING_INFO_HEIGHT_STANDARD = 1,
    BUILDING_INFO_HEIGHT_TALL = 2,
    BUILDING_INFO_HEIGHT_BANDSTAND = 3,
    BUILDING_INFO_HEIGHT_SHORT = 4,
    BUILDING_INFO_HEIGHT_FARM = 5,
    BUILDING_INFO_HEIGHT_CLASSES = 6,
};

enum building_info_screen_size {
    BUILDING_INFO_SCREEN_640 = 0,
    BUILDING_INFO_SCREEN_800 = 1,
    BUILDING_INFO_SCREEN_1024 = 2,
    BUILDING_INFO_SCREEN_SIZES = 3,
};

typedef struct {
    int32_t display_height;
    int32_t x;
    int32_t x_collapsed;
    int32_t low_mouse_y;
    int32_t bottom_y;
} building_info_window_placement;

extern int32_t building_info_window_heights[BUILDING_INFO_HEIGHT_CLASSES];
extern building_info_window_placement building_info_window_placements[BUILDING_INFO_SCREEN_SIZES];

typedef void (*building_info_draw_fn)(void);

int32_t building_info_height_class(void);
int32_t building_info_tile_terrain_kind(int32_t grid_offset);
building_info_draw_fn building_info_draw_function(int32_t type);
int32_t window_building_info_init(void);
void window_building_info_draw_background(void);
void window_building_info_draw_foreground(void);
void window_building_info_handle_input(void);
void window_building_draw_figure_in_city(int32_t grid_offset, int32_t figure_id);
void window_building_prepare_figure_list(void);
void window_building_draw_no_info(void);
void window_building_draw_terrain(void);
void window_building_draw_figure_list(void);
int32_t window_building_figure_big_image(int32_t figure_id);
void window_building_draw_resource_icon(int32_t resource_index, int32_t x, int32_t y);
void window_building_draw_carried_goods(int32_t figure_id, int32_t x, int32_t y);
void window_building_draw_figure_info(int32_t figure_id);
void building_info_toggle_details(void);
void building_info_toggle_dc(void);
void window_building_info_help(void);
void window_building_info_select_figure(void);

static inline building *building_info_building(void)
{
    return building_get(building_info_building_id);
}

#endif
