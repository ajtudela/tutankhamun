// Scrolling the empire map (window/empire_scroll.c): the scroll by a direction and the clamp to the map picture.

#ifndef WINDOW_EMPIRE_SCROLL_H
#define WINDOW_EMPIRE_SCROLL_H

#include <stdint.h>

enum empire_scroll_direction {
    EMPIRE_SCROLL_NONE = 8,
};

enum empire_scroll_step {
    EMPIRE_SCROLL_STEP = 20,
    EMPIRE_DRAG_START = 4,
};

void empire_check_scroll_boundaries(void);
void empire_scroll_map(int32_t direction);
int32_t empire_scroll_area_contains(int32_t x, int32_t y);
void empire_scroll_by(int32_t dx, int32_t dy);
// the map follows the mouse while the right button is held on it, as in Julius; 1 from the first move until the
// release, so a drag's release does not close the window the way a right click does
int32_t empire_scroll_drag(void);

#endif
