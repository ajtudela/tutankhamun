// The functions of debug_info.c that other files call.

#ifndef WIDGET_DEBUG_INFO_H
#define WIDGET_DEBUG_INFO_H

#include <stdint.h>

void debug_draw_grid_value(int32_t x, int32_t y);
void debug_draw_grid_values(void);
void debug_draw_building_info(void);
void debug_draw_figure_info(void);
void debug_frame_update(void);
void debug_draw_city_info(void);
void debug_draw_evolve_info(void);

#endif
