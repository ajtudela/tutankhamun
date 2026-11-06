// Vegetation regrowth, once a day: harvested trees and marshland (reeds) grow back, faster with the oracle of an
// Osiris temple complex, and wood cutters and reed gatherers are told whether a fully grown tile is within their reach
// (through the citizen router, in lists of fully grown tiles).
//
// Growth is a byte addition whose clamp never triggers, so it wraps past 0xFF; the marshland part of the reach list
// starts one entry before the last stored tile.

#include "map/vegetation.h"

#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "core/calc.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/routing_land.h"
#include "map/terrain.h"
#include "map/tiles.h"

#include <stdint.h>

static void grow(int32_t grid_offset, int32_t base, int32_t percentage)
{
    random_generate_next();
    int32_t amount = calc_adjust_with_percentage((random_generator_state.random1_15bit % VEGETATION_RANDOM_GROWTH_RANGE) + base, percentage);
    map_grids.vegetation_growth[grid_offset] = (uint8_t) (map_grids.vegetation_growth[grid_offset] + amount);
    map_tiles_set_vegetation_image(grid_offset);
}

static void check_gatherers(int32_t tree_count, int32_t marsh_index)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->num_workers == 0) {
            continue;
        }
        int32_t type = b->type;
        if (type == BUILDING_WOOD_CUTTER && !vegetation_growth_state.trees_reachable) {
            if (map_routing_citizen_find_first_of_prefer_roads(b->road_access_x, b->road_access_y, route_candidates.tiles, tree_count)) {
                vegetation_growth_state.trees_reachable = 1;
            }
        } else if (type == BUILDING_REED_GATHERER && !vegetation_growth_state.reeds_reachable) {
            if (map_routing_citizen_find_first_of_prefer_roads(
                    b->road_access_x, b->road_access_y, &route_candidates.tiles[marsh_index], VEGETATION_CANDIDATES_MAX - marsh_index)) {
                vegetation_growth_state.reeds_reachable = 1;
            }
        }
    }
}

void map_vegetation_growth_update(void)
{
    int32_t tree_count = 0;
    int32_t marsh_index = VEGETATION_CANDIDATES_MAX;
    vegetation_growth_state.trees_reachable = 0;
    vegetation_growth_state.trees_present = 0;
    vegetation_growth_state.reeds_reachable = 0;
    vegetation_growth_state.marsh_present = 0;
    int32_t percentage = VEGETATION_GROWTH_NORMAL_PERCENT;
    const city_player *player = city_player_current();
    if (player->has_temple_complex != 0 && building_temple_god(player->temple_complex_building_id) == GOD_OSIRIS &&
        building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ORACLE)) {
        percentage = VEGETATION_GROWTH_OSIRIS_PERCENT;
    }
    if (map_info.grid_height <= 0) {
        return;
    }
    for (int32_t y = 0; y < map_info.grid_height; y++) {
        if (map_row_x_start[y] >= map_row_x_end[y]) {
            continue;
        }
        int32_t grid_offset = map_row_start_offset[y];
        for (int32_t n = map_row_x_end[y] - map_row_x_start[y]; n > 0; n--, grid_offset++) {
            uint32_t terrain = map_grids.terrain[grid_offset];
            if (terrain & TERRAIN_TREE) {
                if (map_grids.vegetation_growth[grid_offset] != VEGETATION_FULLY_GROWN) {
                    grow(grid_offset, vegetation_growth_state.tree_growth_a + vegetation_growth_state.tree_growth_b, percentage);
                } else if (!vegetation_growth_state.trees_reachable) {
                    route_candidates.tiles[tree_count++] = grid_offset;
                }
                vegetation_growth_state.trees_present = 1;
            } else if (terrain & TERRAIN_MARSH) {
                if (map_grids.vegetation_growth[grid_offset] != VEGETATION_FULLY_GROWN) {
                    grow(grid_offset, vegetation_growth_state.marsh_growth_a + vegetation_growth_state.marsh_growth_b, percentage);
                } else if (!vegetation_growth_state.reeds_reachable) {
                    route_candidates.tiles[marsh_index--] = grid_offset;
                }
                vegetation_growth_state.marsh_present = 1;
            }
            if (tree_count == marsh_index) {
                check_gatherers(tree_count, marsh_index);
                tree_count = 0;
                marsh_index = VEGETATION_CANDIDATES_MAX;
            }
        }
    }
    if (tree_count != 0 || marsh_index < VEGETATION_CANDIDATES_MAX) {
        check_gatherers(tree_count, marsh_index);
    }
}

static void set_result_tile(int32_t found)
{
    int32_t tile = route_candidates.tiles[found - 1] - map_info.grid_start;
    map_result_x = tile % GRID_SIZE;
    map_result_y = tile / GRID_SIZE;
}

int32_t map_vegetation_find_reachable(int32_t x, int32_t y, uint32_t mask, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t gatherer_type;
    if (mask == TERRAIN_MARSH) {
        if (vegetation_growth_state.reeds_reachable == 0) {
            return 0;
        }
        gatherer_type = FIGURE_REED_GATHERER;
    } else {
        if (vegetation_growth_state.trees_reachable == 0) {
            return 0;
        }
        gatherer_type = FIGURE_LUMBERJACK;
    }
    int32_t count = 0;
    if (y0 < y1) {
        for (int32_t row = y0; row < y1; row++) {
            int32_t offset;
            int32_t col;
            if (x0 < map_row_x_start[row]) {
                offset = map_row_start_offset[row];
                col = map_row_x_start[row];
            } else {
                offset = map_info.grid_start + (row * GRID_SIZE) + x0;
                col = x0;
            }
            int32_t end = map_row_x_end[row];
            if (x1 <= end) {
                end = x1;
            }
            while (col < end) {
                uint32_t terrain = map_grids.terrain[offset];
                if ((terrain & TERRAIN_DIKE) == 0) {
                    if ((mask & terrain) != 0) {
                        int32_t skip = 0;
                        if (mask == TERRAIN_MARSH && map_terrain_count_neighbours_with(TERRAIN_MARSH, offset) != DIRECTION_COUNT) {
                            skip = 1;
                        } else if (figure_get(map_grids.figure_ids[offset])->type == gatherer_type) {
                            skip = 1;
                        }
                        if (!skip && map_grids.vegetation_growth[offset] == VEGETATION_FULLY_GROWN) {
                            route_candidates.tiles[count++] = offset;
                        }
                        if (count == VEGETATION_CANDIDATES_MAX) {
                            int32_t found = map_routing_citizen_find_first_of_prefer_roads(x, y, route_candidates.tiles, VEGETATION_CANDIDATES_MAX);
                            if (found != 0) {
                                set_result_tile(found);
                                return 1;
                            }
                            count = 0;
                        }
                    }
                }
                col++;
                offset++;
            }
        }
        if (count != 0) {
            int32_t found = map_routing_citizen_find_first_of_prefer_roads(x, y, route_candidates.tiles, count);
            if (found != 0) {
                set_result_tile(found);
                return 1;
            }
        }
    }
    if (mask != TERRAIN_MARSH) {
        vegetation_growth_state.trees_reachable = 0;
        return 0;
    }
    vegetation_growth_state.reeds_reachable = 0;
    return 0;
}

int32_t map_vegetation_find_reachable_all(int32_t x, int32_t y, uint32_t mask)
{
    return map_vegetation_find_reachable(x, y, mask, 0, 0, map_info.grid_width, map_info.grid_height);
}
