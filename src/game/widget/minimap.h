// The minimap and its place in the sidebar (73 x 111 tiles).
//
// A click is looked up 8 pixels right of the sidebar's edge while the minimap is drawn 13 pixels right (the editor's
// 30 pixels down instead of 36).

#ifndef WIDGET_MINIMAP_H
#define WIDGET_MINIMAP_H

#include <stdint.h>

enum minimap_sidebar_layout {
    MINIMAP_WIDTH = 73,
    MINIMAP_HEIGHT = 111,
    MINIMAP_SIDEBAR_DRAW_X = 13,
    MINIMAP_SIDEBAR_CLICK_X = 8,
    MINIMAP_SIDEBAR_Y = 59,
    MINIMAP_EDITOR_X = 8,
    MINIMAP_EDITOR_DRAW_Y = 36,
    MINIMAP_EDITOR_CLICK_Y = 30,
};

void widget_minimap_set_bounds(int32_t width, int32_t height);
void widget_minimap_draw(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height);
int32_t widget_minimap_get_mouse_grid_offset(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height);
int32_t minimap_sidebar_click(void);

#endif
