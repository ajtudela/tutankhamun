// The parts of the MSVC6 C runtime the simulation depends on, written to give the same results: rand / srand,
// the truncation of a floating-point value to an integer as the runtime's __ftol does it, and qsort.
//
// The qsort is the runtime's unstable quicksort (mid pivot, selection sort for 8 elements or fewer), so equal elements end
// in the same order as in the game. The truncation goes through 64 bits: a value inside the 64-bit range gives the low
// 32 bits of its integer part, NaN, infinity and anything outside the range give 0.

#include "core/crt_compat.h"

#include "core/core_data.h"

#include <string.h>

enum crt_rand_values {
    CRT_RAND_MULTIPLIER = 0x343FD,
    CRT_RAND_INCREMENT = 0x269EC3,
    CRT_RAND_SHIFT = 16,
    CRT_RAND_MASK = 0x7FFF,
};

enum crt_qsort_limits {
    QSORT_STACK = 30,
    QSORT_CUTOFF = 8,
};

static const double ftol_range = 9223372036854775808.0;

int32_t crt_ftol_double(double value)
{
    if (!((value > -ftol_range) && (value < ftol_range))) {
        return 0;
    }
    return (int32_t) (int64_t) value;
}

void crt_srand(uint32_t seed)
{
    crt_rand_seed = seed;
}

int32_t crt_rand(void)
{
    uint32_t seed = (crt_rand_seed * CRT_RAND_MULTIPLIER) + CRT_RAND_INCREMENT;
    crt_rand_seed = seed;
    return (int32_t) ((seed >> CRT_RAND_SHIFT) & CRT_RAND_MASK);
}

static void qsort_swap(uint8_t *a, uint8_t *b, uint32_t size)
{
    if (a == b) {
        return;
    }
    while (size--) {
        uint8_t t = *a;
        *a++ = *b;
        *b++ = t;
    }
}

static void qsort_shortsort(uint8_t *lo, uint8_t *hi, uint32_t size, crt_qsort_compare compare)
{
    while (hi > lo) {
        uint8_t *maxp = lo;
        for (uint8_t *p = lo + size; p <= hi; p += size) {
            if (compare(p, maxp) > 0) {
                maxp = p;
            }
        }
        qsort_swap(maxp, hi, size);
        hi -= size;
    }
}

void crt_qsort(void *base, uint32_t count, uint32_t size, crt_qsort_compare compare)
{
    uint8_t *stack_lo[QSORT_STACK];
    uint8_t *stack_hi[QSORT_STACK];
    int32_t depth = 0;

    if ((count < 2) || (size == 0)) {
        return;
    }

    uint8_t *lo = (uint8_t *) base;
    uint8_t *hi = lo + ((count - 1) * size);

    for (;;) {
        uint32_t num = (uint32_t) ((hi - lo) / (int32_t) size) + 1;
        if (num <= QSORT_CUTOFF) {
            qsort_shortsort(lo, hi, size, compare);
        } else {
            uint8_t *mid = lo + ((num >> 1) * size);
            qsort_swap(mid, lo, size);

            uint8_t *loguy = lo;
            uint8_t *higuy = hi + size;

            for (;;) {
                do {
                    loguy += size;
                } while ((loguy <= hi) && (compare(loguy, lo) <= 0));

                do {
                    higuy -= size;
                } while ((higuy > lo) && (compare(higuy, lo) >= 0));

                if (higuy < loguy) {
                    break;
                }
                qsort_swap(loguy, higuy, size);
            }
            qsort_swap(lo, higuy, size);

            if (((int32_t) (higuy - lo) - 1) < (int32_t) (hi - loguy)) {
                if (loguy < hi) {
                    stack_lo[depth] = loguy;
                    stack_hi[depth] = hi;
                    depth++;
                }
                if (lo + size < higuy) {
                    hi = higuy - size;
                    continue;
                }
            } else {
                if (lo + size < higuy) {
                    stack_lo[depth] = lo;
                    stack_hi[depth] = higuy - size;
                    depth++;
                }
                if (loguy < hi) {
                    lo = loguy;
                    continue;
                }
            }
        }

        if (--depth < 0) {
            return;
        }
        lo = stack_lo[depth];
        hi = stack_hi[depth];
    }
}
