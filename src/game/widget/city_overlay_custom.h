// The functions of city_overlay_custom.c that other files call.

#ifndef WIDGET_CITY_OVERLAY_CUSTOM_H
#define WIDGET_CITY_OVERLAY_CUSTOM_H

#include <stdint.h>

typedef int32_t (*overlay_image_draw_fn)(int32_t image, int32_t x, int32_t y);

int32_t city_overlay_draw_image(int32_t image, int32_t x, int32_t y);
int32_t city_overlay_draw_image_top(int32_t image, int32_t x, int32_t y);
void city_overlay_water_tiles(int32_t x, int32_t y, int32_t water, int8_t top);
void city_overlay_resource(int32_t x, int32_t y, int32_t resource, int32_t slot);
void city_overlay_water_footprint(int32_t x, int32_t y);
void city_overlay_water_top(int32_t x, int32_t y);
void city_overlay_tombs_footprint(int32_t x, int32_t y);
void city_overlay_tombs_top(int32_t x, int32_t y);
void city_overlay_fertility_footprint(int32_t x, int32_t y);
void city_overlay_fertility_top(int32_t x, int32_t y);
void city_overlay_desirability(int32_t x, int32_t y, int8_t top);
void city_overlay_desirability_top(int32_t x, int32_t y);

#endif
