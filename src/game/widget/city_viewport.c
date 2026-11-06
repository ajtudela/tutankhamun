// The city view's viewport for the screen width, with the sidebar collapsed or expanded; screen sizes other
// than the three standard ones use the layout of widget/screen_layout.c.

#include "widget/city_viewport.h"
#include "graphics/graphics_data.h"
#include "map/grid_size.h"
#include "widget/screen_layout.h"
#include "widget/widget_data.h"
#include <stdint.h>

int32_t city_view_set_viewport(
    int32_t map_width, int32_t map_height, int32_t x, int32_t y, int32_t width, int32_t height, int32_t tile_width, int32_t tile_height)
{
    city_view.height_tiles = height;
    city_view.map_height_tiles = map_height;
    city_view.origin_y = y;
    city_view.tile_half_width = tile_width / 2;
    city_view.size3_x = (tile_width / 2) + tile_width;
    city_view.size4_x = tile_width * 2;
    city_view.tile_width = tile_width;
    city_view.size3_y = (tile_height / 2) + tile_height;
    city_view.size4_y = tile_height * 2;
    city_view.camera_col_max = map_width - width;
    city_view.viewport_x1 = ((tile_width + 2) * width) - 2 + x;
    city_view.viewport_y1 = ((height - 1) * (tile_height / 2)) + y;
    city_view.tile_half_height = tile_height / 2;
    city_view.map_width_tiles = map_width;
    city_view.tile_height = tile_height;
    city_view.camera_col = map_width / 2;
    city_view.origin_x = x;
    city_view.viewport_x = x;
    city_view.width_tiles = width;
    city_view.viewport_y = y;
    city_view.camera_row = map_height;
    return map_height;
}

static void set_viewport_of_screen_layout(int32_t sidebar_collapsed)
{
    city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, screen_layout_view_columns(sidebar_collapsed), screen_layout_view_rows(),
        VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
}

void city_view_set_viewport_without_sidebar(void)
{
    if (!screen_layout_is_standard_size()) {
        set_viewport_of_screen_layout(1);
        return;
    }
    if (g_screen_width == 640) {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 10, 31, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    } else if (g_screen_width == 800) {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 12, 39, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    } else if (g_screen_width == 1024) {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 16, 50, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    } else {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 8, 31, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    }
}

void city_view_set_viewport_with_sidebar(void)
{
    if (!screen_layout_is_standard_size()) {
        set_viewport_of_screen_layout(0);
        return;
    }
    if (g_screen_width == 800) {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 10, 39, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    } else if (g_screen_width == 1024) {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 14, 50, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    } else {
        city_view_set_viewport(GRID_SIZE, GRID_SIZE, 0, VIEWPORT_TOP, 8, 31, VIEWPORT_TILE_WIDTH, VIEWPORT_TILE_HEIGHT);
    }
}
