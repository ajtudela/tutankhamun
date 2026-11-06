// The built-in data of the core: the static initializers' constants and the C runtime's random seed.
// core/core_data.h declares them.

#include "core/core_data.h"


uint32_t crt_rand_seed = 1;

int32_t random_pool_index;

int32_t random_pool[100];

int32_t game_tick_counter;
