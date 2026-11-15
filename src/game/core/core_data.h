// The tables of core/core_data.c.

#ifndef CORE_CORE_DATA_H
#define CORE_CORE_DATA_H

#include "core/random_types.h"

#include <stdint.h>

typedef void (*global_function)(void);


extern uint32_t crt_rand_seed;


extern int32_t random_pool_index;

extern int32_t random_pool[100];

extern random_data random_generator_state;

extern int32_t game_tick_counter;

#endif
