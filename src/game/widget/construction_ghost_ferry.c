// The ferry landing's ghost: the first landing at the shore with its water route drawn as a widening fan, then
// the second landing where the route arrives. A site that does not fit draws its flat tiles red.

#include "widget/construction_ghost_ferry.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "graphics/image_collection.h"
#include "map/bridge.h"
#include "map/ferry.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain_image_group.h"
#include "map/water.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/construction_ghost_small.h"
#include "widget/widget_data.h"
#include <stdbool.h>
#include <stdint.h>

enum ferry_ghost {
    FERRY_NO_PARTNER = -1,
    FERRY_LANDING_TILES = 4,
    FERRY_ROUTE_HALF_WIDTH_MAX = 5,
    FERRY_ROUTE_SIZE = 2,
    FERRY_PATH_SLOTS = 1000,
    FERRY_PATH_SLOTS_CLEARED = 250,
    FERRY_PATH_EMPTY = -1,
    FERRY_STATE_FIRST = 0,
    WATER_FITS = 0,
};

static int32_t opposite_direction(int32_t direction)
{
    return (direction + DIRECTION_COUNT / 2) % DIRECTION_COUNT;
}

static int32_t is_free_landing(int32_t grid_offset)
{
    return map_is_ferry_landing(grid_offset) && building_get(map_grids.building_ids[grid_offset])->linked_building_id == FERRY_NO_PARTNER;
}

static int32_t water_direction(int32_t x, int32_t y)
{
    return map_water_orientation_match(x, y) * VIEW_ORIENTATION_STEP;
}

static void draw_landing(int32_t direction, int32_t px, int32_t py)
{
    const building_property *property = building_property_get(BUILDING_FERRY_LANDING);
    city_view.color_mask = GHOST_COLOR_FREE;
    int32_t image = image_group(property->image_group) + property->image_offset + city_view_relative_direction(direction);
    ghost_image_draw(image, px, py);
    ghost_image_draw_top(image, px, py);
    city_view.color_mask = 0;
}

static void route_tile_delta(int32_t direction, int32_t along, int32_t side, int32_t *dx, int32_t *dy)
{
    if (direction == DIRECTION_TOP) {
        *dx = side;
        *dy = -along;
    } else if (direction == DIRECTION_RIGHT) {
        *dx = along;
        *dy = -side;
    } else if (direction == DIRECTION_BOTTOM) {
        *dx = -side;
        *dy = along;
    } else {
        *dx = -along;
        *dy = side;
    }
}

bool city_building_ghost_draw_ferry_route(int32_t direction, int32_t x, int32_t y, int32_t px0, int32_t py0, int32_t length)
{
    bool found = false;
    int32_t path_length = 0;
    int32_t half_width = 0;
    int32_t origin = map_info.grid_start + x + (y * GRID_SIZE);
    int32_t facing = opposite_direction(direction);
    for (int32_t along = 0; along <= length * 2; along++) {
        for (int32_t side = -half_width; side <= half_width; side++) {
            int32_t dx, dy;
            route_tile_delta(direction, along, side, &dx, &dy);
            int32_t tx = dx + x;
            int32_t ty = dy + y;
            int32_t px = px0;
            int32_t py = py0;
            city_view_tile_to_pixel(tx, ty, &px, &py);
            int32_t offset = map_info.grid_start + (ty * GRID_SIZE) + tx;
            int32_t water = map_water_determine_orientation_size2(tx, ty, 1, 0);
            if (is_free_landing(offset) && offset != origin && map_grids.building_ids[offset] != map_grids.building_ids[origin]) {
                int32_t lx, ly;
                city_view_tile_to_pixel(tx, ty, &lx, &ly);
                draw_landing(facing, lx, ly);
                found = true;
            }
            int32_t shore = water_direction(tx, ty);
            if (shore == facing && water == WATER_FITS && map_ferry_route_exists(tx, ty, x, y, FERRY_ROUTE_SIZE, shore, facing, 0, &path_length)) {
                for (int32_t i = 0; i < FERRY_LANDING_TILES; i++) {
                    ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) + py, GHOST_COLOR_FREE);
                }
                found = true;
            }
        }
        if (half_width < FERRY_ROUTE_HALF_WIDTH_MAX) {
            half_width++;
        }
    }
    return found;
}

static void draw_red_tiles(int32_t px, int32_t py_raised)
{
    for (int32_t i = 0; i < FERRY_LANDING_TILES; i++) {
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) - city_view.tile_half_height + py_raised, GHOST_COLOR_BLOCKED);
    }
}

static void draw_first_landing(int32_t px, int32_t py_raised)
{
    map_ferry_route_measure(map_info.hover_x, map_info.hover_y);
    int32_t direction = ferry_route_direction;
    if (direction < 0) {
        direction += DIRECTION_COUNT;
    }
    int32_t py = py_raised - city_view.tile_half_height;
    city_view.bridge_draw_count = 0;
    int32_t water = map_water_determine_orientation_size2(map_info.hover_x, map_info.hover_y, 1, 0);
    int32_t on_landing = is_free_landing(map_info.hover_offset);
    if (on_landing) {
        city_building_ghost_draw_ferry_route(direction, map_info.hover_x, map_info.hover_y, px, py, ferry_route_length);
        draw_landing(direction, px, py);
    }
    if (water == WATER_FITS && city_building_ghost_draw_ferry_route(direction, map_info.hover_x, map_info.hover_y, px, py, ferry_route_length)) {
        construction_ferry_site_ok = 1;
        city_building_ghost_draw_shore2(0, 0);
        city_view.color_mask = 0;
        return;
    }
    if (on_landing) {
        return;
    }
    construction_ferry_site_ok = 0;
    draw_red_tiles(px, py_raised);
}

static void draw_second_landing(int32_t px, int32_t py_raised)
{
    int32_t path[FERRY_PATH_SLOTS];
    for (int32_t i = 0; i < FERRY_PATH_SLOTS_CLEARED; i++) {
        path[i] = FERRY_PATH_EMPTY;
    }
    int32_t shore = water_direction(map_info.hover_x, map_info.hover_y);
    const building *first = building_get(construction_ferry_first);
    int32_t path_length = 0;
    int32_t size = first->size;
    int32_t first_direction = first->data.oriented.orientation * VIEW_ORIENTATION_STEP;
    int32_t first_x = first->x;
    int32_t first_y = first->y;
    if (map_is_ferry_landing(map_info.hover_offset)) {
        int32_t building_id = map_grids.building_ids[map_info.hover_offset];
        if (building_id != construction_ferry_first && building_get(building_id)->linked_building_id == FERRY_NO_PARTNER) {
            draw_landing(shore, px, py_raised - city_view.tile_half_height);
            return;
        }
    }
    int32_t first_px, first_py;
    city_view_tile_to_pixel(first_x, first_y, &first_px, &first_py);
    city_building_ghost_draw_ferry_route(first_direction, first_x, first_y, first_px, first_py, ferry_route_length);
    if (map_ferry_route_exists(
            map_info.hover_x, map_info.hover_y, first_x, first_y, size, shore, opposite_direction(first_direction), path, &path_length) == 1) {
        construction_ferry_site_ok = 1;
        city_building_ghost_draw_shore2(0, 0);
        return;
    }
    construction_ferry_site_ok = 0;
    draw_red_tiles(px, py_raised);
}

void city_building_ghost_draw_ferry(void)
{
    construction_ferry_site_ok = 0;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    if (construction_ferry_state == FERRY_STATE_FIRST) {
        draw_first_landing(px, py + city_view.tile_half_height);
    } else {
        draw_second_landing(px, py + city_view.tile_half_height);
    }
}
