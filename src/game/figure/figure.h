// The figure record and the movement and route functions on it. A figure's state is none, alive or dead; is_boat is 0
// on land, 1 for a boat and 2 for flotsam; cross-country coordinates are tiles * 15. Bytes that different figure kinds
// use differently are unions of one member per role; the terrain usage chooses the route search of figure_route_add.

#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H

#include "figure/figure_data.h"
#include "figure/figure_types.h"
#include "figure/movement.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int32_t figure_current_id_get(void);
void figure_current_id_set(int32_t value);
int32_t figure_highest_id_get(void);
void figure_highest_id_set(int32_t value);
int32_t figure_image_direction_get(void);
void figure_image_direction_set(int32_t value);
int32_t figure_created_sequence_get(void);
void figure_created_sequence_set(int32_t value);
int32_t *figure_created_sequence_ptr(void);
int32_t figure_search_best_distance_get(void);
void figure_search_best_distance_set(int32_t value);
uint8_t figure_corpse_offset(int32_t index);
int8_t figure_cart_offset(int32_t index);
// the bytes the original lays after the formations: four figure counters, then the figure table; 0 past those
uint8_t *figure_bytes_past_formations(int32_t offset);

// the whole table (the records, then the route paths), for the readers by byte offset and the saved game's chunks
figure_table *figure_table_get(void);
figure *figure_get(int32_t id);
// the figure at figure_current_id, the one the figure loop is at
figure *figure_current(void);
// MAX_PATHS paths of MAX_PATH_LENGTH directions
uint8_t *figure_route_paths(void);

int32_t figure_route_get_first_available(int32_t figure_id, int32_t *path_id);
void figure_route_remove(int32_t figure_id);
void figure_route_clear_all(void);
void figure_route_clean(void);
int32_t figure_route_count_free(void);
void figure_route_add(int32_t figure_id);
int32_t figure_route_add_to_closest_of(int32_t figure_id, const int32_t *grid_offsets, int32_t count);
int32_t figure_create(int32_t player, int32_t type, int32_t x, int32_t y, int32_t direction);
void map_figure_delete(int32_t figure_id);
bool figure_is_predator_animal(int32_t type);
bool figure_is_boat(int32_t type);
void map_figure_add(int32_t figure_id);
void figure_delete(int32_t figure_id);

#endif
