// The MSVC6 C runtime helpers the game relies on (core/crt_compat.c): rand / srand, the truncation of a float to a
// 64-bit integer and the runtime's unstable qsort.

#ifndef CORE_CRT_COMPAT_H
#define CORE_CRT_COMPAT_H

#include <stdint.h>

enum crt_rand_range {
    CRT_RAND_RANGE = 32768,
};


void crt_srand(uint32_t seed);
int32_t crt_rand(void);

int32_t crt_ftol_double(double value);

typedef int (*crt_qsort_compare)(const void *a, const void *b);
void crt_qsort(void *base, uint32_t count, uint32_t size, crt_qsort_compare compare);

#endif
