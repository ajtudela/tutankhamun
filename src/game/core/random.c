// The pool of random values read by the census (city/population.c): random_generate_pool refills its
// RANDOM_POOL_SIZE slots from the generator of core/random.h.

#include "core/random.h"

void random_generate_pool(void)
{
    random_pool_index = 0;
    for (int32_t i = 0; i < RANDOM_POOL_SIZE; i++) {
        random_generate_next();
    }
}
