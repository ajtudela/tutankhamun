// Water supply: water lifts filling the canals from the river, the irrigation range of watered canals and lifts that
// farms use, well ranges, the water access of houses and farms, and whether the houses around a building all have
// water.
//
// A canal fill that reaches a water tile spins over it until its guard runs out, to no effect.

#include "map/water_supply.h"

#include "building/building.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "core/calc.h"
#include "figure/figure_data.h"
#include "flood/flood_internal.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/orientation.h"
#include "map/road_network.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/water.h"

#include <stdint.h>
#include <string.h>

static const int32_t building_list_small_max = (int32_t) (sizeof(building_list_small) / sizeof(building_list_small[0]));

static int32_t canal_has_plaza(int32_t grid_offset)
{
    return (map_grids.bitfields[grid_offset] & MAP_BITFIELD_PLAZA_OR_FAULT) != 0;
}

static int32_t canal_image_follows_water(int32_t grid_offset)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    return building_id == 0 || building_get(building_id)->type == BUILDING_ROADBLOCK;
}

int32_t map_water_supply_lift_has_water(int32_t id)
{
    const building *lift = building_get(id);
    int32_t orientation = lift->data.oriented.orientation;
    int32_t grid_offset = lift->grid_offset;
    if (map_grids.terrain[water_lift_sides.side_a[orientation] + grid_offset] & TERRAIN_WATER) {
        return 1;
    }
    if (map_grids.terrain[water_lift_sides.side_b[orientation] + grid_offset] & TERRAIN_WATER) {
        return 1;
    }
    return 0;
}

void map_water_supply_mark_well_access(int32_t well_id, int32_t radius)
{
    const building *well = building_get(well_id);
    int32_t x_min = well->x - radius;
    int32_t x_max = well->x + radius;
    int32_t y_min = well->y - radius;
    int32_t y_max = well->y + radius;
    if (x_min < 0) {
        x_min = 0;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    map_result_y = y_min;
    int32_t grid_offset = map_info.grid_start + (y_min * GRID_SIZE) + x_min;
    for (int32_t y = y_min; y <= y_max; y++) {
        map_result_y = y + 1;
        map_result_x = x_min;
        for (int32_t x = x_min; x <= x_max; x++, grid_offset++) {
            map_result_x = x + 1;
            int32_t building_id = map_grids.building_ids[grid_offset];
            if (building_id) {
                building_get(building_id)->has_well_access = 1;
            }
        }
        grid_offset += GRID_SIZE - 1 - (x_max - x_min);
    }
}

void map_water_supply_set_all_canals_to_no_water(void)
{
    int32_t image_without_water = image_group(IMAGE_GROUP_CANAL) + CANAL_NO_WATER_IMAGE_OFFSET;
    int32_t grid_offset = map_info.grid_start;
    for (int32_t y = 0; y < map_info.grid_height; y++, grid_offset += map_info.grid_border_size) {
        for (int32_t x = 0; x < map_info.grid_width; x++, grid_offset++) {
            uint32_t terrain = map_grids.terrain[grid_offset];
            if ((terrain & TERRAIN_WATER) || !(terrain & TERRAIN_CANAL)) {
                continue;
            }
            if (!canal_image_follows_water(grid_offset)) {
                continue;
            }
            map_grids.canals[grid_offset] = 0;
            if (!canal_has_plaza(grid_offset) && map_grids.building_ids[grid_offset] == 0 && map_grids.images[grid_offset] < image_without_water) {
                map_grids.images[grid_offset] += CANAL_NO_WATER_IMAGE_OFFSET;
            }
        }
    }
}

static void add_irrigation_range(int32_t grid_offset)
{
    int32_t offset = grid_offset - (IRRIGATION_RANGE_RADIUS * GRID_SIZE) - IRRIGATION_RANGE_RADIUS;
    int32_t side = (2 * IRRIGATION_RANGE_RADIUS) + 1;
    for (int32_t y = 0; y < side; y++, offset += GRID_SIZE) {
        for (int32_t x = 0; x < side; x++) {
            map_grids.terrain[offset + x] |= TERRAIN_IRRIGATION_RANGE;
        }
    }
    map_tile_cursor_offset = offset;
}

void map_water_supply_fill_canals_from_offset(int32_t grid_offset)
{
    if (!(map_grids.terrain[grid_offset] & TERRAIN_CANAL)) {
        return;
    }
    int32_t image_without_water = image_group(IMAGE_GROUP_CANAL) + CANAL_NO_WATER_IMAGE_OFFSET;
    memset(road_network_queue, 0, sizeof(road_network_queue));
    road_network_queue_head = 0;
    road_network_queue_tail = 0;
    int32_t guard = 1;
    do {
        if (map_grids.terrain[grid_offset] & TERRAIN_WATER) {
            continue;
        }
        if (canal_image_follows_water(grid_offset) && map_grids.images[grid_offset] >= image_without_water && !canal_has_plaza(grid_offset)) {
            map_grids.images[grid_offset] -= CANAL_NO_WATER_IMAGE_OFFSET;
        }
        map_grids.canals[grid_offset] = 1;
        int32_t next_offset = -1;
        for (int32_t i = 0; i < 4; i++) {
            int32_t new_offset = grid_offset + road_network_cardinal_delta[i];
            int32_t building_id = map_grids.building_ids[new_offset];
            building *neighbour = building_get(building_id);
            if (building_id && neighbour->type == BUILDING_WATER_LIFT) {
                if (!neighbour->has_water_access) {
                    neighbour->has_water_access = WATER_LIFT_GETS_WATER;
                    map_building_update_water_lift_image(building_id);
                }
            } else if ((map_grids.terrain[new_offset] & TERRAIN_CANAL) && !map_grids.canals[new_offset]) {
                add_irrigation_range(new_offset);
                if (next_offset == -1) {
                    next_offset = new_offset;
                } else {
                    road_network_queue[road_network_queue_tail++] = new_offset;
                    if (road_network_queue_tail >= MAP_BFS_QUEUE_SIZE) {
                        road_network_queue_tail = 0;
                    }
                }
            }
        }
        if (next_offset == -1) {
            if (road_network_queue_head == road_network_queue_tail) {
                return;
            }
            next_offset = road_network_queue[road_network_queue_head++];
            if (road_network_queue_head >= MAP_BFS_QUEUE_SIZE) {
                road_network_queue_head = 0;
            }
        }
        if (next_offset <= -1) {
            return;
        }
        grid_offset = next_offset;
    } while (guard++ < GRID_CELLS);
}

void map_water_supply_update_houses(void)
{
    building_list_small_size = 0;
    if (building_highest_id < 1) {
        return;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type == BUILDING_WELL) {
            building_list_small[building_list_small_size++] = (int16_t) id;
            if (building_list_small_size >= building_list_small_max) {
                building_list_small_size = building_list_small_max - 1;
            }
        } else if (!b->house_size) {
            if (building_is_farm(id)) {
                b->has_water_access = map_terrain_any_tile_in_square_with(b->x, b->y, b->size, TERRAIN_IRRIGATION_RANGE) ? 1 : 0;
            }
        } else {
            b->has_water_access = 0;
            b->has_well_access = 0;
            if (b->data.house.water_supply) {
                b->has_water_access = 1;
            }
        }
    }
    if (building_list_small_size > 0) {
        for (building_list_small_index = 0; building_list_small_index < building_list_small_size;) {
            int32_t well_id = building_list_small[building_list_small_index++];
            map_water_supply_mark_well_access(well_id, WELL_ACCESS_RADIUS);
        }
    }
}

void map_water_supply_update_reservoir_fountain(void)
{
    memand_grid_u32(map_grids.terrain, GRID_CELLS, ~(uint32_t) (TERRAIN_IRRIGATION_RANGE | TERRAIN_WELL_RANGE));
    map_water_supply_set_all_canals_to_no_water();
    memset(building_list_large, 0, sizeof(building_list_large));
    building_list_large_index = 0;
    building_list_large_size = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WATER_LIFT) {
            continue;
        }
        building_list_large[building_list_large_size++] = (int16_t) id;
        int32_t old_water = b->has_water_access;
        if (map_sweep_countdown == 0 && map_water_supply_lift_has_water(id)) {
            b->has_water_access = WATER_LIFT_GETS_WATER;
        } else {
            b->has_water_access = WATER_LIFT_NO_WATER;
        }
        if (b->has_water_access != old_water) {
            map_building_update_water_lift_image(id);
        }
    }
    floods_fill_canals(&floods_data);

    int32_t changed;
    do {
        changed = 0;
        int32_t i = 0;
        if (building_list_large_size < 1) {
            break;
        }
        do {
            int32_t id = building_list_large[i];
            building *lift = building_get(id);
            building_list_large_index = ++i;
            if (lift->has_water_access == WATER_LIFT_GETS_WATER && lift->num_workers > 0) {
                lift->has_water_access = WATER_LIFT_HAS_WATER;
                changed = 1;
                map_water_determine_orientation_size2(lift->x, lift->y, 0, 1);
                map_water_orientation_relative = lift->data.oriented.orientation;
                int32_t back = map_water_orientation_relative ^ 2;
                map_water_supply_fill_canals_from_offset(water_lift_sides.side_a[back] + lift->grid_offset);
                map_water_supply_fill_canals_from_offset(water_lift_sides.side_b[back] + lift->grid_offset);
                i = building_list_large_index;
            }
        } while (i < building_list_large_size);
    } while (changed);

    building_list_large_index = 0;
    for (int32_t i = 0; i < building_list_large_size;) {
        int32_t id = building_list_large[i];
        const building *lift = building_get(id);
        building_list_large_index = ++i;
        if (lift->num_workers > 0 && lift->has_water_access) {
            map_terrain_or_area(lift->x, lift->y, WATER_LIFT_SIZE, IRRIGATION_RANGE_RADIUS, TERRAIN_IRRIGATION_RANGE);
            i = building_list_large_index;
        }
    }

    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int32_t type = b->type;
        if (type != BUILDING_WELL && type != BUILDING_WATER_SUPPLY) {
            continue;
        }
        int32_t image_offset = (map_grids.desirability[b->grid_offset] < WATER_BUILDING_FANCY_DESIRABILITY) ? 0 : WATER_BUILDING_FANCY_IMAGE_OFFSET;
        if (type == BUILDING_WELL) {
            map_building_tiles_add(id, b->x, b->y, 1, 1, image_group(IMAGE_GROUP_WELL) + image_offset, TERRAIN_BUILDING);
            map_terrain_or_area(b->x, b->y, 1, WELL_RANGE_RADIUS, TERRAIN_WELL_RANGE);
        } else {
            map_building_tiles_add(id, b->x, b->y, 2, 2, image_group(IMAGE_GROUP_WATER_SUPPLY) + image_offset, TERRAIN_BUILDING);
        }
        b->has_water_access = 1;
    }
}

int32_t map_houses_water_in_radius(int32_t building_id, int32_t radius)
{
    building *origin = building_get(building_id);
    int32_t x0 = origin->x - radius;
    int32_t y0 = origin->y - radius;
    int32_t x1 = origin->x + radius;
    int32_t y1 = origin->y + radius;
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 >= map_info.grid_width) {
        x1 = map_info.grid_width - 1;
    }
    if (y1 >= map_info.grid_height) {
        y1 = map_info.grid_height - 1;
    }
    int32_t offset = map_info.grid_start + (y0 * GRID_SIZE) + x0;
    int32_t row_adjust = x0 - x1 + (GRID_SIZE - 1);
    map_result_y = y0;
    if (y0 > y1) {
        return HOUSES_WATER_NO_HOUSES;
    }
    int32_t count = 0;
    int32_t y = y0;
    while (y <= y1) {
        int32_t x = x0;
        map_result_x = x;
        if (x0 <= x1) {
            for (;;) {
                int32_t id = map_grids.building_ids[offset];
                const building *house = building_get(id);
                if (id > 0 && house->house_size != 0) {
                    count++;
                    if (house->data.house.water_supply == 0) {
                        return HOUSES_WATER_SOME_WITHOUT;
                    }
                }
                x++;
                offset++;
                map_result_x = x;
                if (x > x1) {
                    break;
                }
            }
        }
        offset += row_adjust;
        y++;
        map_result_y = y;
    }
    return (count != 0) ? HOUSES_WATER_ALL : HOUSES_WATER_NO_HOUSES;
}
