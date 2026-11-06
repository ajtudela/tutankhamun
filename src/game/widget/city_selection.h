// The city window's selection of a legion, a transport ship or a warship under the mouse tile, and the figure
// lookups on a tile it uses.

#ifndef WIDGET_CITY_SELECTION_H
#define WIDGET_CITY_SELECTION_H

#include <stdint.h>

int32_t formation_legion_at_grid_offset(int32_t grid_offset);
int32_t figure_transport_ship_at_grid_offset(int32_t grid_offset);
int32_t figure_warship_at_grid_offset(int32_t grid_offset);
int32_t city_select_legion(void);
int32_t city_select_transport(void);
int32_t city_select_warship(void);

#endif
