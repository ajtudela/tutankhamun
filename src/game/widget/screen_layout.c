// The city screen's layout for screen sizes other than the standard ones: the city view takes whole tile
// columns and rows, the sidebar follows at the right with filler strips up to the screen's edge, and a black bar
// covers the bottom below the last full row. Only a custom size of the play build uses it.
//
// The approach follows the widescreen support of Julius and Akhenaten.

#include "widget/screen_layout.h"
#include "graphics/graphics_data.h"
#include "widget/city_viewport.h"
#include "window/display_resolution_list.h"
#include <stdint.h>

enum screen_layout_view {
    TILE_GAP = 2,
    TILE_STEP = VIEWPORT_TILE_WIDTH + TILE_GAP,
    COLLAPSED_EXTRA_COLUMNS = 2,
    ROW_STEP = VIEWPORT_TILE_HEIGHT / 2,
    BOTTOM_BAR_EXTRA = 6,
    STANDARD_SIZES = 3,
};

static const int32_t standard_sizes[STANDARD_SIZES][2] = {
    { 640, 480 },
    { 800, 600 },
    { 1024, 768 },
};

int32_t screen_layout_is_standard_size(void)
{
    if (!display_resolution_custom_active()) {
        return 1;
    }
    for (int32_t i = 0; i < STANDARD_SIZES; i++) {
        if (g_screen_width == standard_sizes[i][0] && g_screen_height == standard_sizes[i][1]) {
            return 1;
        }
    }
    return 0;
}

int32_t screen_layout_view_columns(int32_t sidebar_collapsed)
{
    int32_t columns = (g_screen_width - (SCREEN_LAYOUT_SIDEBAR_EXPANDED_WIDTH - TILE_GAP)) / TILE_STEP;
    return sidebar_collapsed ? columns + COLLAPSED_EXTRA_COLUMNS : columns;
}

int32_t screen_layout_view_rows(void)
{
    return ((g_screen_height - VIEWPORT_TOP) / ROW_STEP) + 1;
}

int32_t screen_layout_view_width(int32_t sidebar_collapsed)
{
    return (screen_layout_view_columns(sidebar_collapsed) * TILE_STEP) - TILE_GAP;
}

int32_t screen_layout_sidebar_right(void)
{
    return screen_layout_view_width(0) + SCREEN_LAYOUT_SIDEBAR_EXPANDED_WIDTH;
}

int32_t screen_layout_bottom_bar_height(void)
{
    int32_t last_row_y = ((screen_layout_view_rows() - 1) * ROW_STEP) + VIEWPORT_TOP;
    return (g_screen_height - last_row_y) + BOTTOM_BAR_EXTRA;
}
