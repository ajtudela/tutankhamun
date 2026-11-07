// The functions of touch_construction.c that other files call, and its switch, which the platform sets on a touch
// screen.

#ifndef WIDGET_TOUCH_CONSTRUCTION_H
#define WIDGET_TOUCH_CONSTRUCTION_H

#include <stdint.h>

extern int32_t touch_construction_enabled;

int32_t touch_construction_cancel_input(void);
int32_t touch_construction_input(void);
void touch_construction_draw(void);

#endif
