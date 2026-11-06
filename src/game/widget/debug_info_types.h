// Records of the debug display: the four test values of the city info panel and the debug frame state (frame times and
// the debug sprite pool buffer).

#ifndef WIDGET_DEBUG_INFO_TYPES_H
#define WIDGET_DEBUG_INFO_TYPES_H

#include <stdint.h>

typedef struct {
    int32_t value4a;
    int32_t reserved_1[3];
    int32_t value3a;
    int32_t reserved_2[3];
    int32_t value2a;
    int32_t reserved_3[3];
    int32_t value1a;
} debug_test_value_set;

typedef struct {
    int32_t time_current;
    int32_t time_last;
    uint8_t reserved_1[328];
    int32_t sprite_pool_buffer_size;
    void *sprite_pool_buffer;
    uint8_t reserved_2[4272];
} debug_frame_state;

#endif
