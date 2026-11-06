// The city view drawn: the view's clip, then the draw with or without an overlay.

#include "widget/city_draw.h"
#include "figure/action.h"
#include "graphics/graphics_data.h"
#include "widget/city_building_ghost.h"
#include "widget/city_figure.h"
#include "widget/city_with_overlay.h"
#include "widget/city_with_overlay_footprint.h"
#include "widget/city_with_overlay_top.h"
#include "widget/city_without_overlay.h"
#include "widget/debug_info.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t g_sidebar_flash;

enum water_animation {
    WATER_ANIMATION_INTERVAL_MS = 60,
};

static void update_water_animation(void)
{
    city_view.advance_water = 0;
    int32_t delta = time_current_millis - city_view.last_water_ms;
    if (delta > WATER_ANIMATION_INTERVAL_MS || delta < 0) {
        city_view.last_water_ms = time_current_millis;
        city_view.advance_water = 1;
    }
}

void widget_city_draw(int32_t camera_x, int32_t camera_y, int32_t selected_tile)
{
    city_view.camera_col = camera_x;
    city_view.camera_row = camera_y;
    city_view.selected_tile = selected_tile;
    g_clip_x0 = city_view.viewport_x;
    g_clip_x1 = city_view.viewport_x1;
    g_clip_y0 = city_view.viewport_y;
    g_clip_y1 = city_view.viewport_y1;
    update_water_animation();
    if (city_overlay == OVERLAY_NONE) {
        if (overlay_show_all != 0) {
            city_with_overlay_draw_footprint();
        } else {
            city_without_overlay_draw_footprint();
        }
        city_without_overlay_draw_top(0);
        city_building_ghost_draw();
        city_draw_figures(FIGURE_HIGHLIGHT_NONE);
    } else if (city_overlay != OVERLAY_HIDE_CLIFFS && overlay_show_all == 0) {
        city_with_overlay_draw_footprint();
        city_with_overlay_draw_top();
        city_building_ghost_draw();
        city_draw_figures(FIGURE_HIGHLIGHT_OVERLAY);
    } else {
        city_with_overlay_draw_footprint();
        city_without_overlay_draw_top(0);
        city_building_ghost_draw();
        city_draw_figures(FIGURE_HIGHLIGHT_NONE);
    }
    debug_draw_grid_values();
    if (g_sidebar_flash != 0) {
        g_sidebar_flash--;
    }
    g_clip_x0 = 0;
    g_clip_x1 = g_screen_width;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}
