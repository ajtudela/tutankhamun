// The queue records (core/queue.c). A ring queue keeps its items after the header and leaves one slot free, so it
// holds capacity - 1 values. A heap queue (dyn_queue) has its items on the heap; the saved game keeps it as a fixed
// image of 200 slots (dyn_queue_save).

#ifndef CORE_QUEUE_H
#define CORE_QUEUE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    int32_t in_use;
    int32_t head;
    int32_t tail;
    int32_t capacity;
    int32_t reserved_1;
    int32_t items[1];
} ring_queue;

typedef struct {
    uint8_t flag;
    uint8_t reserved_1[3];
    int32_t head;
    int32_t tail;
    int32_t count;
    int32_t capacity;
    int32_t *items;
} dyn_queue;

enum dyn_queue_save_size {
    DYN_QUEUE_SAVE_SLOTS = 200,
};

typedef struct {
    int32_t count;
    int32_t capacity;
    int32_t head;
    int32_t tail;
    uint8_t flag;
    uint8_t reserved_1[3];
    int32_t items[DYN_QUEUE_SAVE_SLOTS];
} dyn_queue_save;

static_assert(offsetof(ring_queue, items) == 0x14, "ring_queue is the savegame's layout");
static_assert(sizeof(dyn_queue_save) == 0x334, "dyn_queue_save is a savegame record of 820 bytes");

int32_t ring_queue_init(ring_queue *q);
void ring_queue_clear(ring_queue *q);
int32_t ring_queue_contains(ring_queue *q, int32_t value);
int32_t ring_queue_is_full(ring_queue *q);
int32_t ring_queue_push(ring_queue *q, const int32_t *value);
int32_t ring_queue_next(ring_queue *q, int32_t *index);
int32_t ring_queue_pop(ring_queue *q);
int32_t ring_queue_size(ring_queue *q);

void dyn_queue_free(dyn_queue *q);
bool dyn_queue_reserve(dyn_queue *q, int32_t capacity);
void dyn_queue_load(dyn_queue *q, dyn_queue_save saved);
bool dyn_queue_wrap_index(dyn_queue *q, int32_t *index);
int32_t dyn_queue_pop(dyn_queue *q);
void dyn_queue_copy(dyn_queue *q, const dyn_queue *src);
void dyn_queue_copy_construct(dyn_queue *q, const dyn_queue *src);
void dyn_queue_grow(dyn_queue *q, int32_t capacity);
int32_t dyn_queue_push(dyn_queue *q, int32_t value);
int32_t dyn_queue_remove(dyn_queue *q, int32_t value);
void dyn_queue_save_to(dyn_queue *q, dyn_queue_save *out);

#endif
