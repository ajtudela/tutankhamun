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

static inline figure *figure_get(int32_t id)
{
    return &city_figures.records[id];
}

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
