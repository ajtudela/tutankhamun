// The queue of ships waiting at one shipwright's dock (the ship figures and the dock's building), and the table of the
// saved queue records followed by the live queues; the dock index reaches back from the queues into the records' tail.

#ifndef FIGURE_DOCK_QUEUE_TYPES_H
#define FIGURE_DOCK_QUEUE_TYPES_H

#include "core/queue.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum dock_queue_count {
    DOCK_QUEUES = 10,
};

typedef struct dock_queue {
    dyn_queue queue;
    int32_t building_id;
} dock_queue;

static_assert(sizeof(dock_queue) == 0x1C, "dock_queue is a savegame record of 28 bytes");
static_assert(offsetof(dock_queue, building_id) == 0x18, "dock_queue is the savegame's layout");

typedef struct dock_queue_table {
    dyn_queue_save records[DOCK_QUEUES];
    dock_queue queues[DOCK_QUEUES];
} dock_queue_table;

static_assert(sizeof(dock_queue_table) == 8480, "dock_queue_table is a savegame record of 8480 bytes");
static_assert(offsetof(dock_queue_table, queues) == 8200, "dock_queue_table is the savegame's layout");

#endif
