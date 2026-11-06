// Ferry landing checks of construction: whether a tile holds a whole ferry landing (a tile outside the map counts as
// one), and whether a water route links two landings, with the route length and the fan of tiles in front of the
// first landing that the second one must lie in.

#include "map/ferry.h"

#include "building/building.h"
#include "building/building_type.h"
#include "figure/boats.h"
#include "figure/figure_data.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/grid_size.h"
#include "map/map_data.h"
#include "map/routing_water.h"
#include "map/water.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static void move_to_landing_top_left(int32_t orientation, int32_t *x, int32_t *y)
{
    if (orientation == DIRECTION_LEFT || orientation == DIRECTION_BOTTOM) {
        (*y)--;
    }
    if (orientation == DIRECTION_BOTTOM || orientation == DIRECTION_RIGHT) {
        (*x)--;
    }
}

int32_t map_ferry_landing_at(int32_t x, int32_t y)
{
    move_to_landing_top_left(map_info.orientation, &x, &y);
    if (x < 0 || y < 0 || x + 1 >= map_info.grid_width || y + 1 >= map_info.grid_height) {
        return FERRY_LANDING_OUTSIDE_MAP;
    }
    int32_t offset = map_info.grid_start + (y * GRID_SIZE) + x;
    int32_t id = map_grids.building_ids[offset];
    if (id == 0 || city_buildings.records[id].type != BUILDING_FERRY_LANDING) {
        return FERRY_LANDING_NOT_HERE;
    }
    for (int32_t group = 0; group < 2; group++) {
        for (int32_t i = 0; i < 2; i++) {
            if (id != map_grids.building_ids[building_square_tile_offset(group, i) + offset]) {
                return FERRY_LANDING_NOT_HERE;
            }
        }
    }
    return FERRY_LANDING_HERE;
}

int32_t map_is_ferry_landing(int32_t grid_offset)
{
    return map_ferry_landing_at((grid_offset - map_info.grid_start) % GRID_SIZE, (grid_offset - map_info.grid_start) / GRID_SIZE);
}

int32_t map_ferry_route_exists(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t size, int32_t direction0, int32_t direction1, int32_t *directions, int32_t *path_length)
{
    bool found = false;
    int32_t ax = x0;
    int32_t ay = y0;
    move_to_landing_top_left(map_info.orientation, &ax, &ay);
    if (map_water_tile_next_to(ax, ay, size) == 0) {
        return 0;
    }
    int32_t water_x0 = map_result_x;
    int32_t water_y0 = map_result_y;
    int32_t bx = x1;
    int32_t by = y1;
    if (construction_ferry_state == 0) {
        move_to_landing_top_left(map_info.orientation, &bx, &by);
    }
    if (map_water_tile_next_to(bx, by, size) == 0) {
        return 0;
    }
    int32_t water_x1 = map_result_x;
    int32_t water_y1 = map_result_y;
    map_routing_calculate_distances_water_ferry(water_x0, water_y0, water_x1, water_y1);
    int32_t length = map_routing_get_path_on_water_4dir(FERRY_ROUTE_PATH_ID, water_x0, water_y0, water_x1, water_y1, 0, directions);
    *path_length = length;
    int32_t width = 0;
    int32_t rows = ferry_route_length * 2;
    for (int32_t row = 0, back = 0; row <= rows; row++, back--) {
        for (int32_t across = -width, mirror = width; across <= width; across++, mirror--) {
            int32_t dx;
            int32_t dy;
            if (direction0 == DIRECTION_TOP) {
                dx = across;
                dy = back;
            } else if (direction0 == DIRECTION_RIGHT) {
                dx = row;
                dy = mirror;
            } else if (direction0 == DIRECTION_BOTTOM) {
                dx = mirror;
                dy = row;
            } else {
                dx = back;
                dy = across;
            }
            if (dx + x0 == x1 && dy + y0 == y1) {
                found = true;
            }
        }
        width++;
        if (width > FERRY_ROUTE_FAN_HALF_WIDTH_MAX) {
            width = FERRY_ROUTE_FAN_HALF_WIDTH_MAX;
        }
    }
    if (direction0 == direction1 && length != 0 && found) {
        return 1;
    }
    return 0;
}
