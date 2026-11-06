// The city screen's layout for sizes other than 640x480, 800x600 and 1024x768: the city view's columns and rows,
// the sidebar's right edge and the black bar below the view. At the standard sizes the formulas give the
// standard numbers.

#ifndef WIDGET_SCREEN_LAYOUT_H
#define WIDGET_SCREEN_LAYOUT_H

#include <stdint.h>

enum screen_layout_picture {
    SCREEN_LAYOUT_PICTURE_WIDTH = 1024,
    SCREEN_LAYOUT_PICTURE_HEIGHT = 768,
};

enum screen_layout_sidebar {
    SCREEN_LAYOUT_SIDEBAR_EXPANDED_WIDTH = 162,
    SCREEN_LAYOUT_SIDEBAR_COLLAPSED_WIDTH = 42,
    SCREEN_LAYOUT_FILLER_WIDTH = 24,
    SCREEN_LAYOUT_FILLER_HEIGHT = 768,
    SCREEN_LAYOUT_PANEL_TOP_HEIGHT = 450,
    SCREEN_LAYOUT_BOTTOM_PANEL_Y = 480,
    SCREEN_LAYOUT_BOTTOM_PANEL_HEIGHT = 285,
    SCREEN_LAYOUT_SHORT_BOTTOM_PANEL_HEIGHT = 120,
    SCREEN_LAYOUT_TOP_STRIP_WIDTH = 1000,
    SCREEN_LAYOUT_TOP_STRIP_REPEAT = 400,
};

int32_t screen_layout_is_standard_size(void);
int32_t screen_layout_view_columns(int32_t sidebar_collapsed);
int32_t screen_layout_view_rows(void);
int32_t screen_layout_view_width(int32_t sidebar_collapsed);
int32_t screen_layout_sidebar_right(void);
int32_t screen_layout_bottom_bar_height(void);

#endif
