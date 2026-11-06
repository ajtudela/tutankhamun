// The platform's free memory and debug output.

#ifndef PLATFORM_SYSTEM_H
#define PLATFORM_SYSTEM_H

#include <stdint.h>

int32_t platform_memory_available(void);
void platform_debug_output(const char *text);

#endif
