// The scenario editor's new scenario: the empty map of a chosen size, the terrain of an empire region read from or
// written into Pharaohmaps.bmp (one byte per tile, a colour per terrain kind), and the editor's flag figures.

#include "editor/editor_new_scenario.h"
#include "core/text.h"
#include "editor/editor_data.h"
#include "editor/editor_new_map_tiles.h"
#include "figure/animals_wild.h"
#include "figure/figure.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "figure/natives.h"
#include "game/scenario_start.h"
#include "graphics/screenshot.h"
#include "io/file.h"
#include "io/file_write.h"
#include "map/grid.h"
#include "map/routing_citizen.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/invasion.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int32_t pharaohmaps_pixels_offset;

uint8_t *pharaohmaps_buffer;

enum pharaohmaps_picture {
    PHARAOHMAPS_FILE_SIZE = 0x16E360,
    PHARAOHMAPS_ROW_BYTES = 1000,
};

enum pharaohmaps_colour {
    PHARAOHMAPS_COLOUR_LAND = 2,
    PHARAOHMAPS_COLOUR_ALTERNATE_LAND = 3,
    PHARAOHMAPS_COLOUR_WATER = 4,
    PHARAOHMAPS_COLOUR_ROAD = 8,
    PHARAOHMAPS_COLOUR_TREES = 11,
    PHARAOHMAPS_COLOUR_ROCK = 16,
};

enum new_scenario_camera {
    NEW_SCENARIO_CAMERA_X = 109,
    NEW_SCENARIO_CAMERA_Y = 218,
};

enum editor_flag_player {
    EDITOR_FLAG_PLAYER = 1,
};

static void set_tile_from_colour(int32_t offset, uint8_t colour)
{
    switch (colour) {
        case PHARAOHMAPS_COLOUR_ROCK:
            map_grids.terrain[offset] = TERRAIN_ROCK;
            break;
        case PHARAOHMAPS_COLOUR_TREES:
            map_grids.terrain[offset] = TERRAIN_TREE;
            break;
        case PHARAOHMAPS_COLOUR_WATER:
            map_grids.terrain[offset] = TERRAIN_WATER;
            break;
        case PHARAOHMAPS_COLOUR_ROAD:
            map_grids.terrain[offset] = TERRAIN_ROAD;
            break;
        case PHARAOHMAPS_COLOUR_ALTERNATE_LAND:
            map_grids.bitfields[offset] |= MAP_BITFIELD_ALTERNATE_TERRAIN;
            break;
        default:
            break;
    }
}

void editor_terrain_from_pharaohmaps(int32_t index)
{
    int32_t height = map_info.grid_height;
    int32_t offset = map_info.grid_start;
    const empire_map_region_position *position = &empire_map_region_positions[index];
    if (height <= 0) {
        return;
    }
    const uint8_t *row = pharaohmaps_buffer + pharaohmaps_pixels_offset + (position->y * PHARAOHMAPS_ROW_BYTES) + position->x;
    int32_t width = map_info.grid_width;
    for (int32_t rows = height; rows != 0; rows--) {
        for (int32_t x = 0; x < width; x++, offset++) {
            set_tile_from_colour(offset, row[x]);
        }
        row += PHARAOHMAPS_ROW_BYTES;
        offset += map_info.grid_border_size;
    }
}

static int32_t pharaohmaps_read(void)
{
    pharaohmaps_buffer = malloc(PHARAOHMAPS_FILE_SIZE);
    if (pharaohmaps_buffer == 0) {
        return 0;
    }
    if (!io_read_file_into_buffer(pharaohmaps_bitmap_file, pharaohmaps_buffer, 0)) {
        free(pharaohmaps_buffer);
        return 0;
    }
    pharaohmaps_pixels_offset = bmp_data_offset(pharaohmaps_buffer);
    bmp_flip(pharaohmaps_buffer);
    return 1;
}

void editor_select_empire_map(int32_t index)
{
    memcpy(&map_info, &empire_map_region_infos[index], sizeof(pharaohmap_info));
    if (!pharaohmaps_read()) {
        return;
    }
    map_grids_clear_all();
    editor_terrain_from_pharaohmaps(index);
    map_tiles_update_all();
    free(pharaohmaps_buffer);
}

static uint8_t tile_colour(int32_t offset)
{
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain == TERRAIN_TREE) {
        return PHARAOHMAPS_COLOUR_TREES;
    }
    if (terrain == TERRAIN_ROCK) {
        return PHARAOHMAPS_COLOUR_ROCK;
    }
    if (terrain == TERRAIN_WATER) {
        return PHARAOHMAPS_COLOUR_WATER;
    }
    if (terrain == TERRAIN_ROAD) {
        return PHARAOHMAPS_COLOUR_ROAD;
    }
    return (map_grids.bitfields[offset] & MAP_BITFIELD_ALTERNATE_TERRAIN) ? PHARAOHMAPS_COLOUR_ALTERNATE_LAND : PHARAOHMAPS_COLOUR_LAND;
}

void editor_terrain_to_pharaohmaps(int32_t index)
{
    if (!pharaohmaps_read()) {
        return;
    }
    const empire_map_region_position *position = &empire_map_region_positions[index];
    int32_t offset = map_info.grid_start;
    int32_t row = position->y * PHARAOHMAPS_ROW_BYTES;
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        for (int32_t x = 0; x < map_info.grid_width; x++, offset++) {
            pharaohmaps_buffer[pharaohmaps_pixels_offset + row + x + position->x] = tile_colour(offset);
        }
        row += PHARAOHMAPS_ROW_BYTES;
        offset += map_info.grid_border_size;
    }
    bmp_flip(pharaohmaps_buffer);
    int32_t size = file_get_size(pharaohmaps_bitmap_file);
    io_write_file(pharaohmaps_bitmap_file, pharaohmaps_buffer, size);
    free(pharaohmaps_buffer);
}

static void clear_special_points(void)
{
    scenario_data.earthquake_x = -1;
    scenario_data.earthquake_y = -1;
    scenario_data.entry_point_x = -1;
    scenario_data.entry_point_y = -1;
    scenario_data.exit_point_x = -1;
    scenario_data.exit_point_y = -1;
    scenario_data.river_entry_x = -1;
    scenario_data.river_entry_y = -1;
    scenario_data.river_exit_x = -1;
    scenario_data.river_exit_y = -1;
    for (int32_t i = 0; i < SCENARIO_INVASION_POINTS; i++) {
        scenario_data.invasion_point_x[i] = -1;
        scenario_data.invasion_point_y[i] = -1;
    }
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        scenario_data.predator_herd_x[i] = -1;
        scenario_data.predator_herd_y[i] = -1;
        scenario_data.prey_herd_x[i] = -1;
        scenario_data.prey_herd_y[i] = -1;
    }
    for (int32_t i = 0; i < SCENARIO_FISHING_POINTS; i++) {
        scenario_data.fishing_point_x[i] = -1;
        scenario_data.fishing_point_y[i] = -1;
    }
    for (int32_t i = 0; i < SCENARIO_INVASION_LANDINGS; i++) {
        scenario_data.invasion_landing_x[i] = -1;
        scenario_data.invasion_landing_y[i] = -1;
    }
}

void game_file_editor_create_scenario(int32_t size)
{
    const new_map_size *map_size = &new_map_sizes[size];
    scenario_data.map_width = map_size->width;
    scenario_data.map_height = map_size->height;
    scenario_data.grid_start = map_size->grid_start;
    scenario_data.grid_border_size = map_size->grid_border_size;
    clear_special_points();
    map_info.grid_width = scenario_data.map_width;
    map_info.grid_height = scenario_data.map_height;
    map_info.grid_start = scenario_data.grid_start;
    map_info.grid_border_size = scenario_data.grid_border_size;
    map_grid_init();
    map_grids_clear_all();
    map_images_set_inside();
    city_view_calculate_lookup(city_view_orientation);
    map_tiles_update_region_empty_land(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0, 0);
    city_view_calculate_map_bounds();
    city_view_check_camera_boundaries();
    map_routing_clear_land_citizen();
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
    map_info.camera_x = NEW_SCENARIO_CAMERA_X;
    map_info.camera_y = NEW_SCENARIO_CAMERA_Y;
    map_info.orientation = 0;
}

static figure *create_flag(void)
{
    return figure_get(figure_create(EDITOR_FLAG_PLAYER, FIGURE_MAP_FLAG, -1, -1, 0));
}

static void create_flags(int32_t first, int32_t last)
{
    for (int32_t number = first; number <= last; number++) {
        create_flag()->editor_flag_number = (uint8_t) number;
    }
}

void figure_create_editor_flags(void)
{
    figure_init_scenario();
    scenario_invasion_amounts_clear();
    create_flags(EDITOR_FLAG_EARTHQUAKE, EDITOR_FLAG_EXIT);
    create_flag();
    create_flags(EDITOR_FLAG_RIVER_ENTRY, EDITOR_FLAG_LANDING_LAST);
    create_flags(EDITOR_FLAG_INVASION_FIRST, EDITOR_FLAG_INVASION_LAST);
}
