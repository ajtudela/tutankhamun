// Rotating the city view (orientations 0, 2, 4, 6) and small viewport helpers.

#ifndef WIDGET_CITY_VIEW_ROTATE_H
#define WIDGET_CITY_VIEW_ROTATE_H

#include <stdint.h>

enum rotation_button {
    ROTATION_BUTTON_RESET = 0,
    ROTATION_BUTTON_RIGHT = 1,
    ROTATION_BUTTON_LEFT = 2,
};

enum view_orientation {
    VIEW_ORIENTATION_NORTH = 0,
    VIEW_ORIENTATION_EAST = 2,
    VIEW_ORIENTATION_SOUTH = 4,
    VIEW_ORIENTATION_WEST = 6,
    VIEW_ORIENTATION_STEP = 2,
};

void city_view_rotate_left(void);
void city_view_rotate_right(void);
int32_t city_view_get_viewport_width_pixels(void);
void city_view_set_editor_viewport(void);
void rotation_button_click(void);

#endif
