// The pyramid's construction preview and the view tile under a screen point (building/pyramid_preview.c).

#ifndef BUILDING_PYRAMID_PREVIEW_H
#define BUILDING_PYRAMID_PREVIEW_H

#include <stdbool.h>
#include <stdint.h>

typedef bool (*diamond_visit_fn)(int32_t offset, int32_t x, int32_t y, int32_t param);

void map_diamond_walk(int32_t offset, int32_t radius, diamond_visit_fn visit, int32_t param);
bool pyramid_preview_try_tile(int32_t offset, int32_t x, int32_t y, int32_t param);
void pyramid_preview_find_place(void);
void view_tile_from_screen(int32_t x, int32_t y, int32_t *row, int32_t *column);

#endif
