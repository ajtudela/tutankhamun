// The functions of editor_new_scenario.c and the records of its tables: the new map sizes and the empire regions'
// Pharaohmap.inf records with their positions in Pharaohmaps.bmp (overridable by config/new_map_sizes.json and
// config/empire_map_regions.json).

#ifndef EDITOR_EDITOR_NEW_SCENARIO_H
#define EDITOR_EDITOR_NEW_SCENARIO_H

#include "map/map_grid_types.h"

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
    int32_t x;
    int32_t y;
} empire_map_region_position;

void figure_create_editor_flags(void);
void game_file_editor_create_scenario(int32_t size);
void editor_terrain_from_pharaohmaps(int32_t index);
void editor_select_empire_map(int32_t index);
void editor_terrain_to_pharaohmaps(int32_t index);

#endif
