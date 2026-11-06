// The city viewport: the view geometry from the map size, the viewport's place and size in tiles, and the
// layout every screen width shares.

#ifndef WIDGET_CITY_VIEWPORT_H
#define WIDGET_CITY_VIEWPORT_H

#include <stdint.h>

enum city_viewport_layout {
    VIEWPORT_TOP = 30,
    VIEWPORT_TILE_WIDTH = 58,
    VIEWPORT_TILE_HEIGHT = 30,
};

int32_t city_view_set_viewport(
    int32_t map_width, int32_t map_height, int32_t x, int32_t y, int32_t width, int32_t height, int32_t tile_width, int32_t tile_height);
void city_view_set_viewport_without_sidebar(void);
void city_view_set_viewport_with_sidebar(void);

#endif
