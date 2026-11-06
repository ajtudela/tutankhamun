// The fleet advisor (advisors_fleet.c): its draws and buttons.

#ifndef WINDOW_ADVISORS_FLEET_H
#define WINDOW_ADVISORS_FLEET_H

#include <stdint.h>

void window_advisor_fleet_draw_background(void);
void window_advisor_fleet_draw_foreground(void);
int32_t advisor_ship_at_dock(int32_t ship);
void button_fleet_go_to_ship(void);
void button_fleet_return_to_dock(void);
void button_fleet_empire_service(void);

#endif
