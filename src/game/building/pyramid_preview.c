// The pyramid's construction preview: the nearest place around the selected tile where a pyramid of the size
// fits, searched on growing diamonds, and the view tile under a screen point of the pyramid view.

#include "building/pyramid_preview.h"
#include "building/building_data.h"
#include "building/monument_data.h"
#include "building/monument_place.h"
#include "map/grid.h"
#include "widget/widget_data.h"
#include <stdbool.h>
#include <stdint.h>

void map_diamond_walk(int32_t offset, int32_t radius, diamond_visit_fn visit, int32_t param)
{
    int32_t x = offset % GRID_SIZE;
    int32_t y = offset / GRID_SIZE;
    int32_t top = y;
    int32_t bottom = y;
    int32_t dx = x - y;
    for (int32_t ring = 0; ring < radius; ring++, top--, bottom++) {
        int32_t x0 = dx + top;
        int32_t x1 = dx + bottom;
        int32_t clip_x0 = 0, clip_x1 = 0, clip_y0 = 0, clip_y1 = 0;
        if (x0 < map_info.diamond_x_min) {
            x0 = map_info.diamond_x_min;
            clip_x0 = 1;
        }
        if (x1 > map_info.diamond_x_max) {
            x1 = map_info.diamond_x_max;
            clip_x1 = 1;
        }
        int32_t y0 = top;
        int32_t y1 = bottom;
        if (y0 < map_info.diamond_y_min) {
            y0 = map_info.diamond_y_min;
            clip_y0 = 1;
        }
        if (y1 > map_info.diamond_y_max) {
            y1 = map_info.diamond_y_max;
            clip_y1 = 1;
        }
        if (!clip_y0 || !clip_y1) {
            for (int32_t xx = x0; xx <= x1; xx++) {
                if (!clip_y0 && !visit(xx + y0 * GRID_SIZE, xx, y0, param)) {
                    return;
                }
                if (!clip_y1 && y1 > y0 && !visit(xx + y1 * GRID_SIZE, xx, y1, param)) {
                    return;
                }
            }
        }
        if (!clip_x0 || !clip_x1) {
            for (int32_t yy = y0 + 1; yy <= y1 - 1; yy++) {
                if (!clip_x0 && !visit(x0 + yy * GRID_SIZE, x0, yy, param)) {
                    return;
                }
                if (!clip_x1 && x1 > x0 && !visit(x1 + yy * GRID_SIZE, x1, yy, param)) {
                    return;
                }
            }
        }
    }
}

bool pyramid_preview_try_tile(int32_t offset, int32_t x, int32_t y, int32_t param)
{
    if (pyramid_can_place(offset, pyramid_layout.size_being_placed, &param)) {
        map_info.hover_offset = offset;
        map_info.hover_x = (offset - map_info.grid_start) % GRID_SIZE;
        map_info.hover_y = (offset - map_info.grid_start) / GRID_SIZE;
        return 0;
    }
    return 1;
}

void pyramid_preview_find_place(void)
{
    map_diamond_walk(map_info.selected_grid, pyramid_search_radius[pyramid_layout.size_being_placed], pyramid_preview_try_tile, 0);
}

void view_tile_from_screen(int32_t x, int32_t y, int32_t *row, int32_t *column)
{
    int32_t cx, cy, a, b, d, e;
    switch (map_info.orientation) {
        case 0:
            cy = 230;
            cx = city_view.map_start_b * 2 + 1;
            a = -1;
            b = 1;
            d = 1;
            e = 1;
            break;
        case 2:
            cx = 230;
            cy = city_view.map_start_a * 2 + 1;
            a = 1;
            b = -1;
            d = 1;
            e = 1;
            break;
        case 4:
            cy = 230;
            cx = 455 - city_view.map_start_b * 2;
            a = 1;
            b = -1;
            d = -1;
            e = -1;
            break;
        default:
            cx = 230;
            cy = 455 - city_view.map_start_b * 2;
            a = -1;
            b = 1;
            d = -1;
            e = -1;
            break;
    }
    int32_t c = ((y - cy) * d - (x - cx) * a) / (e * d - a * b);
    *column = c;
    *row = (x - c * b - cx) / d;
}
