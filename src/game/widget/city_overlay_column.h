// The overlay column and its styles (service, risk, water).

#ifndef WIDGET_CITY_OVERLAY_COLUMN_H
#define WIDGET_CITY_OVERLAY_COLUMN_H

#include <stdint.h>

enum overlay_column_style {
    OVERLAY_COLUMN_SERVICE = 0,
    OVERLAY_COLUMN_RISK = 1,
    OVERLAY_COLUMN_WATER = 2,
};

void city_overlay_draw_column(int32_t height, int32_t x, int32_t y, int32_t column_style);

#endif
