// The game's random generator, as in Caesar III with the 3-bit values and a third seed added (seeded but never
// used); inline, as the game calls it at every site.

#ifndef CORE_RANDOM_H
#define CORE_RANDOM_H

#include "core/random_types.h"
#include "figure/figure_data.h"

#include <stdint.h>

enum random_generator_values {
    RANDOM_POOL_SIZE = 100,
    RANDOM_SHIFTS_PER_STEP = 31,
    RANDOM_FEEDBACK_TAP = 0x10,
    RANDOM_FEEDBACK_SHIFT = 4,
    RANDOM_TOP_BIT = 0x40000000,
    RANDOM_3BIT_MASK = 0x7,
    RANDOM_7BIT_MASK = 0x7F,
    RANDOM_15BIT_MASK = 0x7FFF,
};

static inline void random_generate_next(void)
{
    random_pool[random_pool_index++] = random_generator_state.random1_7bit;
    if (random_pool_index >= RANDOM_POOL_SIZE) {
        random_pool_index = 0;
    }
    for (int32_t i = 0; i < RANDOM_SHIFTS_PER_STEP; i++) {
        uint32_t feedback1 = (((random_generator_state.iv1 & RANDOM_FEEDBACK_TAP) >> RANDOM_FEEDBACK_SHIFT) ^ random_generator_state.iv1) & 1;
        uint32_t feedback2 = (((random_generator_state.iv2 & RANDOM_FEEDBACK_TAP) >> RANDOM_FEEDBACK_SHIFT) ^ random_generator_state.iv2) & 1;
        random_generator_state.iv1 >>= 1;
        random_generator_state.iv2 >>= 1;
        if (feedback1) {
            random_generator_state.iv1 |= RANDOM_TOP_BIT;
        }
        if (feedback2) {
            random_generator_state.iv2 |= RANDOM_TOP_BIT;
        }
    }
    random_generator_state.random1_3bit = random_generator_state.iv1 & RANDOM_3BIT_MASK;
    random_generator_state.random1_7bit = random_generator_state.iv1 & RANDOM_7BIT_MASK;
    random_generator_state.random1_15bit = random_generator_state.iv1 & RANDOM_15BIT_MASK;
    random_generator_state.random2_3bit = random_generator_state.iv2 & RANDOM_3BIT_MASK;
    random_generator_state.random2_7bit = random_generator_state.iv2 & RANDOM_7BIT_MASK;
    random_generator_state.random2_15bit = random_generator_state.iv2 & RANDOM_15BIT_MASK;
}

void random_generate_pool(void);

#endif
