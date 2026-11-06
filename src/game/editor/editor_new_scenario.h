// The functions of editor_new_scenario.c and the records of its tables: the new map sizes and the empire regions'
// Pharaohmap.inf records with their positions in Pharaohmaps.bmp (overridable by config/new_map_sizes.json and
// config/empire_map_regions.json).

#ifndef EDITOR_EDITOR_NEW_SCENARIO_H
#define EDITOR_EDITOR_NEW_SCENARIO_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum new_scenario_tables {
    NEW_MAP_SIZES = 6,
    EMPIRE_MAP_REGIONS = 34,
};

typedef struct {
    int32_t width;
    int32_t height;
    int32_t grid_start;
    int32_t grid_border_size;
} new_map_size;

typedef struct {
    int32_t map_id;
    int32_t camera_x;
    int32_t camera_y;
    int32_t selected_grid;
    int32_t cursor_tile_x;
    int32_t cursor_tile_y;
    int32_t grid_width;
    int32_t grid_height;
    int32_t grid_start;
    int32_t grid_border_size;
    int32_t orientation;
    int32_t last_hover;
    int32_t diamond_x_min;
    int32_t diamond_y_min;
    int32_t diamond_x_max;
    int32_t diamond_y_max;
    int32_t diamond_x_center;
    int32_t diamond_y_center;
    int32_t hover_offset;
    int32_t hover_x;
    int32_t hover_y;
} pharaohmap_info;

static_assert(sizeof(pharaohmap_info) == 0x54, "pharaohmap_info is a savegame and scenario record of 84 bytes");
static_assert(offsetof(pharaohmap_info, grid_width) == 0x18, "pharaohmap_info is the savegame and scenario layout");

typedef struct {
    int32_t x;
    int32_t y;
} empire_map_region_position;

void figure_create_editor_flags(void);
void game_file_editor_create_scenario(int32_t size);
void editor_terrain_from_pharaohmaps(int32_t index);
void editor_select_empire_map(int32_t index);
void editor_terrain_to_pharaohmaps(int32_t index);

#endif
