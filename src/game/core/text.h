// String helpers (core/text.c): finding text and numbers in a string, writing a number into string_number_buffer.

#ifndef CORE_TEXT_H
#define CORE_TEXT_H

#include <stdint.h>

int32_t string_compare_n(const char *a, const char *b, int32_t length);
int32_t string_find(const char *text, const char *needle, int32_t length);
int32_t string_index_of(const char *text, char c, int32_t length);
int32_t string_index_of_number(const char *text);
int32_t string_to_int(const char *text);
int32_t string_from_int(int32_t value, int32_t offset, int32_t force_plus);
int32_t string_length(const char *text);
void string_to_upper(char *text);

#endif
