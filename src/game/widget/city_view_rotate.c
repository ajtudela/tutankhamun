// Rotating the city view by the keys and the rotation button, and small viewport helpers.

#include "widget/city_view_rotate.h"
#include "building/building_data.h"
#include "city/message.h"
#include "figure/figure_data.h"
#include "game/game_internal.h"
#include "map/grid_size.h"
#include "map/map_data.h"
#include "map/orientation.h"
#include "widget/city_view.h"
#include "widget/city_viewport.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_input.h"
#include <stdint.h>

static void rotate(int32_t step, int32_t dx, int32_t dy)
{
    int32_t center = city_view.lookup[(city_view.width_tiles / 2) + (((city_view.height_tiles / 2) + map_info.camera_y) * VIEW_COLS) + map_info.camera_x];
    int32_t orientation = city_view_orientation + step;
    city_view_orientation = orientation;
    if (orientation > VIEW_ORIENTATION_WEST) {
        city_view_orientation = VIEW_ORIENTATION_NORTH;
    } else if (orientation < VIEW_ORIENTATION_NORTH) {
        city_view_orientation = VIEW_ORIENTATION_WEST;
    }
    city_view_calculate_lookup(city_view_orientation);
    if (center < VIEW_LOOKUP_EMPTY) {
        city_view_grid_offset_to_xy_view(center);
        map_info.camera_x = map_result_x - (city_view.width_tiles / 2);
        map_info.camera_y = map_result_y - (city_view.height_tiles / 2);
        if (city_view_orientation == VIEW_ORIENTATION_SOUTH || city_view_orientation == VIEW_ORIENTATION_NORTH) {
            map_info.camera_x += dx;
            map_info.camera_y += dy;
        }
    }
}

void city_view_rotate_left(void)
{
    rotate(VIEW_ORIENTATION_STEP, 1, 0);
}

void city_view_rotate_right(void)
{
    rotate(-VIEW_ORIENTATION_STEP, 0, 2);
}

int32_t city_view_get_viewport_width_pixels(void)
{
    return (city_view.tile_width + 2) * city_view.width_tiles;
}

void city_view_set_editor_viewport(void)
{
    city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 5, 15, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
}

void rotation_button_click(void)
{
    if (button_parameter == ROTATION_BUTTON_RESET) {
        if (map_info.orientation == VIEW_ORIENTATION_NORTH) {
            return;
        }
        if (map_info.orientation == VIEW_ORIENTATION_EAST) {
            rotation_direction = ROTATION_BUTTON_RIGHT;
            city_view_rotate_right();
            map_info.orientation = city_view_orientation;
        } else {
            rotation_direction = ROTATION_BUTTON_RESET;
            if (map_info.orientation == VIEW_ORIENTATION_WEST) {
                city_view_rotate_left();
                map_info.orientation = city_view_orientation;
            } else {
                city_view_rotate_left();
                city_view_rotate_left();
                map_info.orientation = city_view_orientation;
                map_orientation_change();
            }
        }
    } else {
        if (button_parameter == ROTATION_BUTTON_LEFT) {
            city_view_rotate_left();
        } else if (button_parameter == ROTATION_BUTTON_RIGHT) {
            city_view_rotate_right();
        }
        rotation_direction = button_parameter;
        map_info.orientation = city_view_orientation;
    }
    map_orientation_change();
    city_view_check_camera_boundaries();
    city_warning_show(1);
    window_redraw_request = WINDOW_REDRAW_ALL;
}
