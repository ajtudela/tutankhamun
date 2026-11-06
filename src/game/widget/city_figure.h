// Drawing figures in the city view; the highlight is a figure id, FIGURE_HIGHLIGHT_NONE or
// FIGURE_HIGHLIGHT_OVERLAY (the figures the current overlay shows).

#ifndef WIDGET_CITY_FIGURE_H
#define WIDGET_CITY_FIGURE_H

#include <stdint.h>

enum figure_highlight {
    FIGURE_HIGHLIGHT_NONE = 0,
    FIGURE_HIGHLIGHT_OVERLAY = 9999,
};

void city_draw_figure(int32_t figure_id, int32_t x, int32_t y, int32_t highlight);
void city_draw_figures(int32_t highlight);
void city_draw_figure_in_view(int32_t camera_x, int32_t camera_y, int32_t grid_start, int32_t figure_id);

#endif
