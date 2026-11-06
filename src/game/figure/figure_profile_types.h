// The records of the figure dispatcher's timing profile (figure/action.c): per figure type the calls, milliseconds
// and performance counter seconds (total and most), and the counter values at the start and end of an action.

#ifndef FIGURE_FIGURE_PROFILE_TYPES_H
#define FIGURE_FIGURE_PROFILE_TYPES_H

#include <stdint.h>

enum { FIGURE_PROFILE_TYPES = 110 };

typedef struct {
    int32_t calls;
    int32_t ticks_total;
    int32_t ticks_max;
    int32_t pad;
    double seconds_total;
    double seconds_max;
} figure_profile_entry;

typedef figure_profile_entry figure_profile_table[FIGURE_PROFILE_TYPES];

typedef struct {
    uint32_t low_part;
    int32_t high_part;
} figure_profile_clock;

typedef struct {
    figure_profile_clock clock;
    uint8_t reserved_1[864];
} figure_profile_end_clock;

#endif
