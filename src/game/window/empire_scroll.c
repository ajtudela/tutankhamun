// Scrolling the empire map, in the game and in the scenario editor: a 20-pixel step in one of eight directions, then
// the view clamped to the map picture; on a screen larger than the map the map is centred.

#include "window/empire_scroll.h"
#include "graphics/graphics_data.h"
#include "map/grid.h"
#include "widget/screen_layout.h"
#include "window/window_data.h"
#include <stdint.h>

void empire_check_scroll_boundaries(void)
{
    if (empire_scroll_x < 0) {
        empire_scroll_x = 0;
    }
    int32_t max_x = empire_map_width + ((empire_border_x * 2) - display_width);
    if (empire_scroll_x >= max_x) {
        empire_scroll_x = max_x - 1;
    }
    if (max_x <= 0 && !screen_layout_is_standard_size()) {
        empire_scroll_x = max_x / 2;
    }
    if (empire_scroll_y < 0) {
        empire_scroll_y = 0;
    }
    int32_t max_y = empire_map_height + (empire_border_bottom - screen_height + empire_border_y);
    if (empire_scroll_y >= max_y) {
        empire_scroll_y = max_y - 1;
    }
    if (max_y <= 0 && !screen_layout_is_standard_size()) {
        empire_scroll_y = max_y / 2;
    }
}

void empire_scroll_map(int32_t direction)
{
    switch (direction) {
        case EMPIRE_SCROLL_NONE:
            return;
        case DIRECTION_TOP:
            empire_scroll_y -= EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_TOP_RIGHT:
            empire_scroll_y -= EMPIRE_SCROLL_STEP;
            empire_scroll_x += EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_RIGHT:
            empire_scroll_x += EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_BOTTOM_RIGHT:
            empire_scroll_y += EMPIRE_SCROLL_STEP;
            empire_scroll_x += EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_BOTTOM:
            empire_scroll_y += EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_BOTTOM_LEFT:
            empire_scroll_y += EMPIRE_SCROLL_STEP;
            empire_scroll_x -= EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_LEFT:
            empire_scroll_x -= EMPIRE_SCROLL_STEP;
            break;
        case DIRECTION_TOP_LEFT:
            empire_scroll_y -= EMPIRE_SCROLL_STEP;
            empire_scroll_x -= EMPIRE_SCROLL_STEP;
            break;
        default:
            break;
    }
    empire_check_scroll_boundaries();
}
