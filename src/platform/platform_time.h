// The platform's clocks: the two millisecond clocks, and the high-resolution counter and its frequency (counts per
// second), which answer false when the system has none.

#ifndef PLATFORM_TIME_H
#define PLATFORM_TIME_H

#include <stdbool.h>
#include <stdint.h>

uint32_t platform_time_ms(void);
uint32_t platform_tick_count(void);
bool platform_time_counter(uint64_t *count);
bool platform_time_frequency(uint64_t *frequency);

#endif
