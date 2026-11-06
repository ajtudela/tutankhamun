// Shared by the routing modules: the distance grid and the ring-buffer queue, the queue and path limits, and the
// breadth-first search over the four orthogonal neighbours (top, right, bottom, left) the searches are built on.

#ifndef MAP_ROUTING_INTERNAL_H
#define MAP_ROUTING_INTERNAL_H

#include "core/calc.h"
#include "map/grid.h"

#include <stdint.h>

#define MAX_QUEUE GRID_CELLS

enum routing_neighbour {
    ROUTING_NEIGHBOUR_TOP = 0,
    ROUTING_NEIGHBOUR_RIGHT = 1,
    ROUTING_NEIGHBOUR_BOTTOM = 2,
    ROUTING_NEIGHBOUR_LEFT = 3,
};

enum {
    ROUTING_NEIGHBOUR_COUNT = 4,
};

enum {
    ROUTING_NO_DESTINATION = -1,
};

extern const int32_t routing_neighbour_offsets[ROUTING_NEIGHBOUR_COUNT];
extern const int32_t routing_direction_dx[DIRECTION_COUNT];
extern const int32_t routing_direction_dy[DIRECTION_COUNT];

static inline int32_t routing_opposite_direction(int32_t direction)
{
    return (direction + 4 < DIRECTION_COUNT) ? (direction + 4) : (direction - 4);
}

void routing_clear_distances(void);
void routing_queue_reset(int32_t source_offset);
void routing_queue_push(int32_t offset);
void routing_queue_set_source(int32_t src_x, int32_t src_y);
void routing_queue_start(int32_t src_x, int32_t src_y);
void routing_enqueue(int32_t offset, int32_t distance);
void routing_queue_advance(void);
void routing_store_path(int32_t path_id, int32_t length);
int32_t routing_target_index(int32_t offset, const int32_t *offsets, int32_t count);

void routing_search(int32_t dst_offset, int32_t (*can_enter)(int32_t next, int32_t current));
void routing_route_queue(int32_t src_x, int32_t src_y, int32_t dst_offset, int32_t (*can_enter)(int32_t next, int32_t current));
int32_t routing_route_queue_first_of(int32_t src_x, int32_t src_y, const int32_t *offsets, int32_t count, int32_t (*can_enter)(int32_t next, int32_t current));

#endif
