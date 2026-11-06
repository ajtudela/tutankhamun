// Roads and canals placed along a distance map, walking back from the end tile to the start, and the canal
// dragged between two water lifts: the cheapest pair of their canal ends is walked, and two new lifts need some
// distance between them.
//
// The current tile is kept in the routing's last offset and the neighbours are read from it.

#include "building/construction_routed.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "core/calc.h"
#include "game/undo_build.h"
#include "map/building_tiles.h"
#include "map/construction_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing.h"
#include "map/routing_distance.h"
#include "map/routing_internal.h"
#include "map/routing_terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"
#include <stdint.h>
#include <string.h>

int32_t construction_lift_end;

int32_t construction_lift_start;

int32_t routing_last_offset;

static int32_t building_type_at(int32_t grid_offset)
{
    return building_get(map_grids.building_ids[grid_offset])->type;
}

int32_t place_routed_building(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type)
{
    routed_building_items_placed = 0;
    int32_t guard = 1;
    int32_t x = x_end;
    int32_t y = y_end;
    int32_t offset = map_grid_offset(x, y);
    routing_last_offset = offset;
    for (;;) {
        int32_t distance = routing_grids.distance[offset];
        if ((distance <= 0) && (building_type_at(offset) != BUILDING_ROADBLOCK)) {
            return 0;
        }
        switch (type) {
            case ROUTED_BUILDING_ROAD:
                if ((map_grids.terrain[offset] & TERRAIN_CANAL) && !map_can_place_road_under_canal(offset)) {
                    return 0;
                }
                map_tiles_set_road(x, y, 1);
                break;
            case ROUTED_BUILDING_COUNT_ONLY:
                routed_building_items_placed++;
                break;
            case ROUTED_BUILDING_CANAL:
                if ((map_grids.terrain[offset] & TERRAIN_ROAD) && !map_can_place_canal_on_road(offset)) {
                    return 0;
                }
                map_tiles_set_canal(x, y, 1, 1);
                break;
            default:
                break;
        }
        int32_t general_direction = calc_general_direction(x, y, x_start, y_start) & 0xFF;
        if (general_direction == DIRECTION_AT_DESTINATION) {
            return 1;
        }
        const int32_t *indices = &routed_building_direction_indices[general_direction * ROUTED_DIRECTION_CHOICES];
        int32_t i = 0;
        int32_t direction;
        int32_t next;
        for (;;) {
            direction = indices[i];
            next = map_grid_direction_delta[direction] + routing_last_offset;
            int32_t next_distance = routing_grids.distance[next];
            if ((next_distance > 0) && (next_distance < distance) && (!(map_grids.terrain[next] & TERRAIN_CANAL) || map_can_place_road_under_canal(next))) {
                break;
            }
            if (++i >= ROUTED_DIRECTION_CHOICES) {
                return 0;
            }
        }
        x += direction_xy_delta[direction * 2];
        y += direction_xy_delta[(direction * 2) + 1];
        routing_last_offset = next;
        if (++guard >= ROUTED_MAX_TILES) {
            return 0;
        }
        offset = next;
    }
}

static int32_t start_tile_allowed(int32_t offset)
{
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & TERRAIN_BUILDING) {
        int32_t building_id = map_grids.building_ids[offset];
        if (building_id == 0 || building_get(building_id)->type != BUILDING_ROADBLOCK) {
            return 0;
        }
    }
    return (terrain & ROUTED_TERRAIN_NOT_ALLOWED) == 0;
}

static void save_backups(void)
{
    memcpy(map_grids.terrain, map_terrain_backup, GRID_CELLS * sizeof(map_grids.terrain[0]));
    memcpy(map_grids.canals, map_canals_backup, GRID_CELLS * sizeof(map_grids.canals[0]));
    map_images_restore_free_tiles();
    routed_building_items_placed = 0;
}

static int32_t calculate_routed_distances(int32_t x, int32_t y, int32_t mode)
{
    if (editor_active) {
        return map_routing_calculate_distances_for_building(x, y, mode);
    }
    return map_routing_calculate_distances_for_building_checked(x, y, mode);
}

void building_construction_place_road(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    save_backups();
    if (!start_tile_allowed(map_grid_offset(x_start, y_start))) {
        return;
    }
    uint32_t end_terrain = map_grids.terrain[map_grid_offset(x_end, y_end)];
    if ((end_terrain & TERRAIN_BUILDING) || (end_terrain & ROUTED_TERRAIN_NOT_ALLOWED)) {
        return;
    }
    if (!calculate_routed_distances(x_start, y_start, ROUTED_MODE_ROAD)) {
        return;
    }
    if (!place_routed_building(x_start, y_start, x_end, y_end, ROUTED_BUILDING_ROAD) || measure_only) {
        return;
    }
    map_routing_update_land_citizen_rect(x_start, y_start, x_end, y_end, 1);
    map_routing_update_land_noncitizen_rect(x_start, y_start, x_end, y_end);
    map_tiles_update_empty_land_around(x_start, y_start, x_end - x_start);
    window_redraw_request = 1;
}

void building_construction_place_canal(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    save_backups();
    if (!start_tile_allowed(map_grid_offset(x_start, y_start))) {
        return;
    }
    if (!start_tile_allowed(map_grid_offset(x_end, y_end))) {
        return;
    }
    if (!calculate_routed_distances(x_start, y_start, ROUTED_MODE_CANAL)) {
        return;
    }
    if (!place_routed_building(x_start, y_start, x_end, y_end, ROUTED_BUILDING_CANAL) || measure_only) {
        return;
    }
    int32_t x_min = x_start, x_max = x_end;
    if (x_min > x_max) {
        x_min = x_end;
        x_max = x_start;
    }
    int32_t y_min = y_start, y_max = y_end;
    if (y_min > y_max) {
        y_min = y_end;
        y_max = y_start;
    }
    map_tiles_update_region_water(x_min - 1, y_min - 1, x_max + 1, y_max + 1, 1);
    map_routing_update_water_rect(x_min, y_min, x_max, y_max);
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen_rect(x_min, y_min, x_max, y_max);
    map_routing_update_walls();
    window_redraw_request = 1;
}

int32_t construction_is_water_lift(int32_t offset)
{
    int32_t id = map_grids.building_ids[offset];
    if (id == 0 || building_get(id)->type != BUILDING_WATER_LIFT) {
        return 0;
    }
    for (int32_t group = 0; group < 2; group++) {
        for (int32_t i = 0; i < 2; i++) {
            if (id != map_grids.building_ids[offset + building_square_tile_offset(group, i)]) {
                return 0;
            }
        }
    }
    return 1;
}

static int32_t lift_end(int32_t x, int32_t y)
{
    if (construction_is_water_lift(map_grid_offset(x - 1, y - 1)) != 0) {
        return WATER_LIFT_END_EXISTING;
    }
    int32_t site_fits = map_building_tiles_shrub_fits(x - 1, y - 1, WATER_LIFT_SIZE, WATER_LIFT_SIZE, WATER_LIFT_SITE_TERRAIN, 0, 1);
    return (site_fits != 0) ? WATER_LIFT_END_NEW : WATER_LIFT_END_BLOCKED;
}

int32_t building_construction_place_canal_for_connector(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    int32_t canal_cost = building_model_table[BUILDING_IRRIGATION_DITCH].cost;
    construction_cost = 0;
    construction_lift_start = WATER_LIFT_END_NONE;
    construction_lift_end = WATER_LIFT_END_NONE;
    memcpy(map_grids.terrain, map_terrain_backup, GRID_CELLS * sizeof(map_grids.terrain[0]));
    memcpy(map_grids.canals, map_canals_backup, GRID_CELLS * sizeof(map_grids.canals[0]));
    map_images_restore_free_tiles();
    int32_t length = calc_maximum_distance(x_start, y_start, x_end, y_end);
    if ((measure_only != 0) && (construction_in_progress == 0)) {
        length = 0;
    } else if (length > 0) {
        construction_lift_start = lift_end(x_start, y_start);
    }
    construction_lift_end = lift_end(x_end, y_end);
    if (construction_lift_start == WATER_LIFT_END_BLOCKED || construction_lift_end == WATER_LIFT_END_BLOCKED) {
        return 0;
    }
    int32_t two_new_lifts = (construction_lift_start == WATER_LIFT_END_NEW) && (construction_lift_end == WATER_LIFT_END_NEW);
    if (two_new_lifts && (length < WATER_LIFT_NEW_PAIR_MIN_LENGTH)) {
        return 0;
    }
    if (length == 0) {
        construction_cost = building_model_table[BUILDING_WATER_LIFT].cost;
        return 1;
    }
    if (map_routing_calculate_distances_for_building(x_start, y_start, ROUTED_MODE_CANAL) == 0) {
        return 0;
    }
    if (construction_lift_start != WATER_LIFT_END_NONE) {
        map_building_tiles_clear_marks(0, x_start - 1, y_start - 1, WATER_LIFT_SIZE, WATER_LIFT_SIZE);
        map_building_tiles_mark_construction(WATER_LIFT_SIZE, x_start - 1, y_start - 1, UINT32_MAX, 1, 1);
    }
    if (construction_lift_end != WATER_LIFT_END_NONE) {
        map_building_tiles_clear_marks(0, x_end - 1, y_end - 1, WATER_LIFT_SIZE, WATER_LIFT_SIZE);
        map_building_tiles_mark_construction(WATER_LIFT_SIZE, x_end - 1, y_end - 1, UINT32_MAX, 1, 1);
    }
    int32_t best = ROUTED_COST_NONE, best_start = 0, best_end = 0;
    for (int32_t s = 0; s < WATER_LIFT_CANAL_ENDS; s++) {
        int32_t sx = canal_lift_ends[s * 2];
        int32_t sy = canal_lift_ends[(s * 2) + 1];
        for (int32_t e = 0; e < WATER_LIFT_CANAL_ENDS; e++) {
            int32_t end_x = canal_lift_ends[e * 2] + x_end;
            int32_t end_y = canal_lift_ends[(e * 2) + 1] + y_end;
            int32_t placed = place_routed_building(sx + x_start, sy + y_start, end_x, end_y, ROUTED_BUILDING_COUNT_ONLY);
            if ((placed != 0) && (routed_building_items_placed != 0) && (routed_building_items_placed < best)) {
                best = routed_building_items_placed;
                best_end = e;
                best_start = s;
            }
        }
    }
    if (best == ROUTED_COST_NONE) {
        return 0;
    }
    int32_t start_x = canal_lift_ends[best_start * 2] + x_start;
    int32_t start_y = canal_lift_ends[(best_start * 2) + 1] + y_start;
    int32_t end_x = canal_lift_ends[best_end * 2] + x_end;
    int32_t end_y = canal_lift_ends[(best_end * 2) + 1] + y_end;
    place_routed_building(start_x, start_y, end_x, end_y, ROUTED_BUILDING_PATH_ONLY);
    if (construction_lift_start == WATER_LIFT_END_NEW) {
        construction_cost += building_model_table[BUILDING_WATER_LIFT].cost;
    }
    if (construction_lift_end == WATER_LIFT_END_NEW) {
        construction_cost += building_model_table[BUILDING_WATER_LIFT].cost;
    }
    if (best != 0) {
        construction_cost += best * canal_cost;
    }
    return 1;
}
