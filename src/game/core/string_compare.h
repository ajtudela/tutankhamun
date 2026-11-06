// The string compare function type the file lists sort with.

#ifndef CORE_STRING_COMPARE_H
#define CORE_STRING_COMPARE_H

#include <stdint.h>

typedef int32_t (*string_compare_fn)(const char *a, const char *b);

#endif
