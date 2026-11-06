// Queues: a fixed ring buffer of ints and a heap-allocated queue (dyn_queue) that is saved as a fixed image of 200
// slots: init, push, pop, grow, copy, remove, and the save and load of the image.
//
// Copying, growing and saving are exact about the slots:
// - copying into a queue that already had more slots than the source reads past the end of the source's items;
// - growing does not unwrap head and tail, so a full queue whose tail wrapped gets the new slots inside its live range;
// - saving without items clears all 200 slots, otherwise only `capacity` of them are written.

#include "core/queue.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum queue_sizes {
    RING_QUEUE_CAPACITY = 50,
    DYN_QUEUE_GROW_STEP = 10,
};

void ring_queue_clear(ring_queue *q)
{
    q->tail = 0;
    q->head = 0;
}

int32_t ring_queue_contains(ring_queue *q, int32_t value)
{
    int32_t i = q->head;
    int32_t tail = q->tail;
    while (i != tail) {
        if (q->items[i] == value) {
            return 1;
        }
        i++;
        if (i >= q->capacity) {
            i = 0;
        }
    }
    return 0;
}

int32_t ring_queue_pop(ring_queue *q)
{
    return ring_queue_next(q, &q->head);
}

int32_t ring_queue_size(ring_queue *q)
{
    int32_t size = q->tail - q->head;
    if (size < 0) {
        return q->capacity + size;
    }
    return size;
}

int32_t ring_queue_is_full(ring_queue *q)
{
    return ring_queue_size(q) == q->capacity - 1;
}

void dyn_queue_free(dyn_queue *q)
{
    if (q->items) {
        free(q->items);
        q->items = 0;
    }
    q->capacity = 0;
    q->tail = 0;
    q->count = 0;
    q->head = 1;
}

bool dyn_queue_reserve(dyn_queue *q, int32_t capacity)
{
    if (capacity <= q->capacity) {
        return 1;
    }
    dyn_queue_free(q);
    q->capacity = capacity;
    q->items = malloc((size_t) capacity * sizeof(int32_t));
    memset(q->items, 0, (size_t) capacity * sizeof(int32_t));
    q->head = 1;
    q->tail = 0;
    q->count = 0;
    return 1;
}

void dyn_queue_load(dyn_queue *q, dyn_queue_save saved)
{
    dyn_queue_reserve(q, saved.capacity);
    q->count = saved.count;
    q->flag = saved.flag;
    q->tail = saved.tail;
    q->head = saved.head;
}

bool dyn_queue_wrap_index(dyn_queue *q, int32_t *index)
{
    if (q->capacity == 0) {
        return 0;
    }
    while (*index < 0) {
        *index += q->capacity;
    }
    while (*index >= q->capacity) {
        *index -= q->capacity;
    }
    return 1;
}

int32_t dyn_queue_pop(dyn_queue *q)
{
    if (q->count == 0) {
        return 0;
    }
    int32_t index = q->head;
    q->head = index + 1;
    dyn_queue_wrap_index(q, &q->head);
    q->count--;
    return q->items[index];
}

int32_t ring_queue_init(ring_queue *q)
{
    q->head = 0;
    q->tail = 0;
    q->capacity = RING_QUEUE_CAPACITY;
    for (int32_t i = 0; i < q->capacity; i++) {
        q->items[i] = 0;
    }
    return 1;
}

void dyn_queue_copy(dyn_queue *q, const dyn_queue *src)
{
    dyn_queue_reserve(q, src->capacity);
    q->count = src->count;
    q->head = src->head;
    q->tail = src->tail;
    q->flag = src->flag;
    memcpy(q->items, src->items, (size_t) q->capacity * sizeof(int32_t));
}

void dyn_queue_copy_construct(dyn_queue *q, const dyn_queue *src)
{
    q->tail = 0;
    q->count = 0;
    q->items = 0;
    q->capacity = 0;
    q->flag = 0;
    q->head = 1;
    dyn_queue_copy(q, src);
}

void dyn_queue_grow(dyn_queue *q, int32_t capacity)
{
    if (capacity > q->capacity) {
        dyn_queue old_queue;
        dyn_queue_copy_construct(&old_queue, q);
        dyn_queue_reserve(q, capacity);
        dyn_queue_copy(q, &old_queue);
        dyn_queue_free(&old_queue);
    }
}

int32_t dyn_queue_push(dyn_queue *q, int32_t value)
{
    if (q->count == q->capacity) {
        dyn_queue_grow(q, q->capacity + DYN_QUEUE_GROW_STEP);
    }
    if (q->items == 0) {
        return 0;
    }
    q->tail++;
    dyn_queue_wrap_index(q, &q->tail);
    q->items[q->tail] = value;
    return ++q->count;
}

int32_t dyn_queue_remove(dyn_queue *q, int32_t value)
{
    int32_t index = q->head;
    int32_t i = 0;
    if (q->count > 0) {
        while (q->items[index] != value) {
            index++;
            dyn_queue_wrap_index(q, &index);
            i++;
            if (i >= q->count) {
                break;
            }
        }
    }
    if (i == q->count) {
        return 0;
    }
    while (i < q->count - 1) {
        int32_t next = index + 1;
        dyn_queue_wrap_index(q, &next);
        q->items[index] = q->items[next];
        index++;
        dyn_queue_wrap_index(q, &index);
        i++;
    }
    q->tail--;
    q->count--;
    dyn_queue_wrap_index(q, &q->tail);
    return 1;
}

int32_t ring_queue_push(ring_queue *q, const int32_t *value)
{
    int32_t tail = q->tail;
    int32_t next = tail + 1;
    if (next >= q->capacity) {
        next = 0;
    }
    if (next == q->head) {
        return 0;
    }
    q->items[tail] = *value;
    q->tail = next;
    return 1;
}

int32_t ring_queue_next(ring_queue *q, int32_t *index)
{
    int32_t i = *index;
    int32_t value = 0;
    if (i != q->tail) {
        value = q->items[i];
        i++;
        *index = i;
        if (i >= q->capacity) {
            *index = 0;
        }
    }
    return value;
}

void dyn_queue_save_to(dyn_queue *q, dyn_queue_save *out)
{
    if (q->capacity > DYN_QUEUE_SAVE_SLOTS) {
        q->capacity = DYN_QUEUE_SAVE_SLOTS;
    }
    dyn_queue_wrap_index(q, &q->head);
    dyn_queue_wrap_index(q, &q->tail);
    dyn_queue_wrap_index(q, &q->count);
    out->count = q->count;
    out->capacity = q->capacity;
    out->head = q->head;
    out->tail = q->tail;
    out->flag = q->flag;
    if (q->items) {
        memcpy(out->items, q->items, (size_t) q->capacity * sizeof(int32_t));
    } else {
        memset(out->items, 0, sizeof(out->items));
    }
}
