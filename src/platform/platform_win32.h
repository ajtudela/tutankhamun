// The platform functions a front end can answer itself: the clocks, the keyboard and cursor state, the message pump
// and the message box. A null entry keeps the Windows function.

#ifndef PLATFORM_WIN32_H
#define PLATFORM_WIN32_H

#include "platform/platform_message.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t (*time_ms)(void);
    uint32_t (*tick_count)(void);
    int16_t (*key_state)(int32_t key);
    bool (*cursor_position)(int32_t *x, int32_t *y);
    void *(*cursor_set)(void *cursor);
    bool (*message_peek)(platform_message *msg, uint32_t first, uint32_t last, bool remove);
    bool (*message_get)(platform_message *msg);
    void (*message_wait)(void);
    bool (*message_post)(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam);
    int32_t (*message_box)(void *window, const char *text, const char *caption, uint32_t type);
} platform_bindings;

void platform_bind(const platform_bindings *bindings);

const platform_bindings *platform_bound(void);

#endif
