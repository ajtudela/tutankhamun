// The functions of city_hud.c that other files call, and the kinds of the debug event line.

#ifndef WIDGET_CITY_HUD_H
#define WIDGET_CITY_HUD_H

#include <stdint.h>

enum debug_event_type {
    DEBUG_EVENT_NONE = 0,
    DEBUG_EVENT_MAJOR_GOOD = 1,
    DEBUG_EVENT_MINOR_GOOD = 2,
    DEBUG_EVENT_MAJOR_BAD = 3,
    DEBUG_EVENT_MINOR_BAD = 4,
};

void window_city_draw_paused_and_time_left(void);
void window_fast_forward_note_draw(void);

#endif
