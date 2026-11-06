// The ferry queue records kept in globals: per landing a ring queue of the figures waiting there and the
// landing's building id; record 0 is unused. Also the ferry path records: in use, then the passengers aboard.

#ifndef BUILDING_FERRY_QUEUE_TYPES_H
#define BUILDING_FERRY_QUEUE_TYPES_H

#include "core/queue.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum ferry_queue_limit {
    FERRY_QUEUE_CAPACITY = 50,
    FERRY_QUEUES = 50,
};

typedef struct ferry_queue {
    ring_queue queue;
    int32_t items_after_first[FERRY_QUEUE_CAPACITY - 1];
    int32_t building_id;
} ferry_queue;

static_assert(sizeof(ferry_queue) == 0xE0, "ferry_queue is a savegame record of 224 bytes");
static_assert(offsetof(ferry_queue, building_id) == 0xDC, "ferry_queue is the savegame's layout");

typedef ferry_queue ferry_queue_records[FERRY_QUEUES];

enum ferry_path_record_limit {
    FERRY_PATH_RECORDS = 50,
    FERRY_PATH_PASSENGER_SLOTS = 10,
};

typedef struct ferry_path {
    uint8_t in_use;
    uint8_t reserved_1[3];
    int32_t passengers[FERRY_PATH_PASSENGER_SLOTS];
} ferry_path;

static_assert(sizeof(ferry_path) == 0x2C, "ferry_path is a savegame record of 44 bytes");
static_assert(offsetof(ferry_path, passengers) == 4, "ferry_path is the savegame's layout");

#endif
