// The sidebar's draw and slide, and the number of steps of the slide.

#ifndef WIDGET_SIDEBAR_H
#define WIDGET_SIDEBAR_H

#include <stdint.h>

enum {
    SIDEBAR_SLIDE_STEPS = 47,
};

void widget_sidebar_draw_number_of_messages(int32_t force);
void widget_sidebar_draw_background(void);
void widget_sidebar_draw_minimap(int32_t force);
void widget_sidebar_slide(void);

#endif
