// Setting up the map grids for a map: everything cleared, the tiles outside the playable area marked, the playable
// diamond and its row tables, the random byte of each tile, the alternate grass images, and the outside-offset test.

#include "map/grid.h"

#include "building/building_data.h"
#include "core/calc.h"
#include "core/random.h"
#include "flood/flood_internal.h"
#include "graphics/image_collection.h"
#include "map/map_data.h"
#include "map/terrain_image_group.h"
#include "map/tiles_internal.h"
#include "map/vegetation.h"

#include <stdint.h>
#include <string.h>

static void clear_grid(void *grid, int32_t bytes_per_tile)
{
    memset(grid, 0, GRID_CELLS * bytes_per_tile);
}

void map_grid_init(void)
{
    int32_t x_min = (GRID_SIZE - map_info.grid_width) / 2;
    int32_t y_min = (GRID_SIZE - map_info.grid_height) / 2;
    map_info.diamond_x_max = x_min - 1 + map_info.grid_width;
    map_info.diamond_y_max = y_min - 1 + map_info.grid_height;
    map_info.diamond_x_center = (map_info.diamond_x_max + x_min) / 2;
    map_info.diamond_y_center = (map_info.diamond_y_max + y_min) / 2;
    map_info.diamond_x_min = x_min;
    map_info.diamond_y_min = y_min;
    if (y_min <= map_info.diamond_y_max) {
        int32_t y = y_min;
        int32_t row_offset = y_min * GRID_SIZE;
        int32_t top_half_left_x = (map_info.diamond_y_center - y_min) + x_min;
        do {
            int32_t left_x = top_half_left_x;
            if (y > map_info.diamond_y_center) {
                left_x = (x_min - map_info.diamond_y_center) - 1 + y;
            }
            int32_t row = y - y_min;
            map_row_x_start[row] = left_x - x_min;
            map_row_x_end[row] = ((map_info.diamond_x_center * 2) - left_x) + 2 - x_min;
            map_row_start_offset[row] = left_x + row_offset;
            y++;
            row_offset += GRID_SIZE;
            top_half_left_x--;
        } while (y <= map_info.diamond_y_max);
    }
}

void map_random_init(void)
{
    int32_t offset = 0;
    for (int32_t y = 0; y < GRID_SIZE; y++) {
        for (int32_t x = 0; x < GRID_SIZE; x++, offset++) {
            random_generate_next();
            map_grids.random[offset] = (uint8_t) random_generator_state.iv1;
        }
    }
}

void map_grids_clear_all(void)
{
    clear_grid(map_grids.images, sizeof(map_grids.images[0]));
    clear_grid(map_grids.edge, sizeof(map_grids.edge[0]));
    clear_grid(map_grids.building_ids, sizeof(map_grids.building_ids[0]));
    clear_grid(map_grids.terrain, sizeof(map_grids.terrain[0]));
    clear_grid(map_grids.soil_fertility, sizeof(map_grids.soil_fertility[0]));
    clear_grid(map_grids.canals, sizeof(map_grids.canals[0]));
    clear_grid(map_grids.figure_ids, sizeof(map_grids.figure_ids[0]));
    clear_grid(map_grids.bitfields, sizeof(map_grids.bitfields[0]));
    clear_grid(map_grids.sprite, sizeof(map_grids.sprite[0]));
    clear_grid(map_grids.random, sizeof(map_grids.random[0]));
    clear_grid(map_grids.desirability, sizeof(map_grids.desirability[0]));
    clear_grid(map_grids.elevation, sizeof(map_grids.elevation[0]));
    clear_grid(map_grids.building_damage, sizeof(map_grids.building_damage[0]));
    clear_grid(map_grids.rubble_building_type, sizeof(map_grids.rubble_building_type[0]));
    clear_grid(map_grids.soldier_strength, sizeof(map_grids.soldier_strength[0]));
    clear_grid(map_grids.road_network, sizeof(map_grids.road_network[0]));
    clear_grid(map_grids.road_segment, sizeof(map_grids.road_segment[0]));
    clear_grid(map_grids.moisture, sizeof(map_grids.moisture[0]));
    memset(map_grids.vegetation_growth, VEGETATION_FULLY_GROWN, GRID_CELLS);
    floods_reset(&floods_data);
    map_image_context_reset(context_water, 48);
    map_image_context_reset(context_water_alt, 48);
    map_image_context_reset(context_floodsystem, 24);
    map_image_context_reset(context_wall, 48);
    map_image_context_reset(context_wall_gate, 15);
    map_image_context_reset(context_aqueduct, 14);
    map_image_context_reset(context_fault, 17);
    map_image_context_reset(context_meadow, 17);
    map_image_context_reset(context_dirt_road, 17);
    map_image_context_reset(context_paved_road, 48);
    map_image_context_reset(context_canal, 16);
    map_terrain_init_outside_map();
    map_random_init();
    map_property_init_alternate_terrain();
}

void map_terrain_init_outside_map(void)
{
    for (int32_t y = 0; y < GRID_SIZE; y++) {
        uint32_t *row = map_grids.terrain + y * GRID_SIZE;
        for (int32_t x = 0; x < GRID_SIZE; x++) {
            if (!map_grid_is_inside(x, y)) {
                row[x] = TERRAIN_TREE | TERRAIN_WATER | TERRAIN_DIKE;
            }
        }
    }
}

int32_t map_grid_offset_is_outside(int32_t offset)
{
    int32_t y = offset / GRID_SIZE;
    int32_t x = offset % GRID_SIZE;
    return !map_grid_is_inside(x, y);
}

void map_property_init_alternate_terrain(void)
{
    int32_t height = map_info.grid_height;
    int32_t first_grass_image = image_group(IMAGE_GROUP_LEVELED_GROUND);
    for (int32_t y = 0; y < height; y++) {
        if (map_row_x_start[y] >= map_row_x_end[y]) {
            continue;
        }
        int32_t offset = map_row_start_offset[y];
        for (int32_t n = map_row_x_end[y] - map_row_x_start[y]; n > 0; n--, offset++) {
            uint8_t random = map_grids.random[offset];
            map_grids.images[offset] = (random % ALTERNATE_GRASS_IMAGES) + first_grass_image;
            if (random & 1) {
                map_grids.bitfields[offset] |= MAP_BITFIELD_ALTERNATE_TERRAIN;
            }
        }
    }
}
