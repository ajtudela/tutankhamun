// Entertainment venues placed over road crossings, and the removal of a venue's tiles.

#ifndef BUILDING_CONSTRUCTION_VENUE_H
#define BUILDING_CONSTRUCTION_VENUE_H

#include <stdbool.h>
#include <stdint.h>

void venue_remove_tiles(int32_t building_id, int32_t type);

int32_t construction_place_booth(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
int32_t construction_place_bandstand(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
int32_t construction_place_pavilion(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
int32_t construction_place_festival_square(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
int32_t construction_venue_can_place(int32_t type);
int32_t construction_venue_take_spot(int32_t x, int32_t y, int32_t *out_x, int32_t *out_y, int32_t venue_type, int32_t part_type);
bool construction_venue_pattern(int32_t x, int32_t y, int32_t size);
void construction_venue_fill_road_grid(int32_t x, int32_t y, int32_t type);
bool construction_venue_is_crossing(int32_t x, int32_t y, int32_t x0, int32_t y0, int32_t size);
bool construction_venue_free_spots(int32_t size, int32_t w, int32_t h, int32_t *count, int32_t *spots);

#endif
