// Scrolling the empire map (window/empire_scroll.c): the scroll by a direction and the clamp to the map picture.

#ifndef WINDOW_EMPIRE_SCROLL_H
#define WINDOW_EMPIRE_SCROLL_H

#include <stdint.h>

enum empire_scroll_direction {
    EMPIRE_SCROLL_NONE = 8,
};

enum empire_scroll_step {
    EMPIRE_SCROLL_STEP = 20,
};

void empire_check_scroll_boundaries(void);
void empire_scroll_map(int32_t direction);

#endif
