// The city problem weights of the walkers' phrases, grouped with their neighbours (phrase_weight_block).
//
// The problem button clears the weight at phrase -1..8, a byte of the neighbouring fields at both ends.

#ifndef FIGURE_FIGURE_PHRASE_TYPES_H
#define FIGURE_FIGURE_PHRASE_TYPES_H

#include <stdint.h>

enum phrase_weight_size {
    PHRASE_WEIGHTS = 8,
};

typedef struct {
    int32_t was_in_int;
    uint8_t phrase_weights[PHRASE_WEIGHTS];
    int32_t extra_script_monument;
} phrase_weight_record;

#endif
