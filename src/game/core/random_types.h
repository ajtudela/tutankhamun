// The random generator's state: the two 15-, 7- and 3-bit values and the three seeds.

#ifndef CORE_RANDOM_TYPES_H
#define CORE_RANDOM_TYPES_H

#include <stdint.h>

typedef struct {
    int32_t random2_15bit;
    int32_t random2_7bit;
    int32_t random2_3bit;
    int32_t random1_15bit;
    int32_t random1_7bit;
    int32_t random1_3bit;
    uint32_t iv3;
    uint32_t iv2;
    uint32_t iv1;
} random_data;

#endif
