// Scrolling the empire map, in the game and in the scenario editor: a 20-pixel step in one of eight directions, then
// the view clamped to the map picture; on a screen larger than the map the map is centred.

#include "window/empire_scroll.h"
#include "graphics/graphics_data.h"
#include "input/input_data.h"
#include "map/grid.h"
#include "widget/screen_layout.h"
#include "window/window_data.h"
#include <stdint.h>
#include <stdlib.h>

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

int32_t empire_scroll_area_contains(int32_t x, int32_t y)
{
    return x >= empire_border_x && x < display_width - empire_border_x && y >= empire_border_y && y < screen_height - empire_border_bottom;
}

// a finger's or a mouse's drag: the map follows it
void empire_scroll_by(int32_t dx, int32_t dy)
{
    if (dx == 0 && dy == 0) {
        return;
    }
    empire_scroll_x -= dx;
    empire_scroll_y -= dy;
    empire_check_scroll_boundaries();
}

static struct {
    int32_t active;
    int32_t moved;
    int32_t x;
    int32_t y;
} drag;

int32_t empire_scroll_drag(void)
{
    if (mouse_right_went_down && empire_scroll_area_contains(mouse_x, mouse_y)) {
        drag.active = 1;
        drag.moved = 0;
        drag.x = mouse_x;
        drag.y = mouse_y;
    }
    if (!drag.active) {
        return 0;
    }
    if (!drag.moved && (abs(mouse_x - drag.x) > EMPIRE_DRAG_START || abs(mouse_y - drag.y) > EMPIRE_DRAG_START)) {
        drag.moved = 1;
    }
    if (drag.moved) {
        empire_scroll_by(mouse_x - drag.x, mouse_y - drag.y);
        drag.x = mouse_x;
        drag.y = mouse_y;
    }
    if (!mouse_right_down) {
        drag.active = 0;
    }
    return drag.moved;
}
