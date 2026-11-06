// The functions of labor.c that other files call.

#ifndef WINDOW_ADVISOR_LABOR_H
#define WINDOW_ADVISOR_LABOR_H

#include <stdint.h>

void window_advisor_labor_draw_background(void);
void window_advisor_labor_draw_foreground(void);
void window_labor_priority_draw_background(void);
void window_labor_priority_draw_foreground(void);
void advisor_labor_wages_arrow(void);
void button_labor_priority_open(void);
void button_labor_priority_set(void);

#endif
