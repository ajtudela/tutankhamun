// String helpers used by the simulation (city messages, names) and the front-end: comparing, finding a character
// or a needle, copying, upper case, length, and reading and writing decimal numbers.
//
// Edge cases of the number and search helpers:
// - string_find returns 0 both when the needle is absent and when it is found at 0;
// - string_to_int weights the digits by an 8-entry table, so a 9th digit reads the next global;
// - string_from_int writes no digits for values of 10^9 and more, and one ('0' - 8) for INT_MIN.

#include "core/text.h"

#include "graphics/graphics_data.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum string_limits {
    STRING_MAX_LENGTH = 10000,
    STRING_NUMBER_SEARCH_LENGTH = 999,
    STRING_MAX_DIGITS = 9,
    STRING_UPPER_CASE_OFFSET = 'a' - 'A',
};

const int32_t string_powers_of_ten[8] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

int32_t string_compare_n(const char *a, const char *b, int32_t length)
{
    for (int32_t i = 0; i < length; i++) {
        if (b[i] != a[i]) {
            return i + 1;
        }
    }
    return 0;
}

int32_t string_find(const char *text, const char *needle, int32_t length)
{
    int32_t needle_length = (int32_t) strlen(needle);
    for (int32_t i = 0; i < length; i++) {
        if ((text[i] == needle[0]) && (string_compare_n(text + i, needle, needle_length) == 0)) {
            return i;
        }
    }
    return 0;
}

int32_t string_index_of(const char *text, char c, int32_t length)
{
    for (int32_t i = 0; i < length; i++) {
        if (text[i] == c) {
            return i + 1;
        }
    }
    return 0;
}

void string_to_upper(char *text)
{
    for (; *text; text++) {
        if ((*text >= 'a') && (*text <= 'z')) {
            *text = (char) (*text - STRING_UPPER_CASE_OFFSET);
        }
    }
}

int32_t string_length(const char *text)
{
    int32_t length = 0;
    int32_t left = STRING_MAX_LENGTH;
    while (*text++) {
        length++;
        if (--left <= 0) {
            break;
        }
    }
    return length;
}

int32_t string_to_int(const char *text)
{
    int32_t negative = 0;
    const char *p = text;
    if (*text == '-') {
        negative = 1;
        p = text + 1;
    }
    int32_t digits = 0;
    while ((*p >= '0') && (*p <= '9')) {
        digits++;
        p++;
    }
    p = (*text == '-') ? text + 1 : text;
    int32_t value = 0;
    for (; digits; digits--, p++) {
        value += ((uint8_t) *p - '0') * string_powers_of_ten[digits - 1];
    }
    return negative ? -value : value;
}

int32_t string_index_of_number(const char *text)
{
    int32_t i;
    for (i = 0; i < STRING_NUMBER_SEARCH_LENGTH; i++) {
        char c = text[i];
        if (((c >= '0') && (c <= '9')) || (c == '-')) {
            return i;
        }
    }
    return i;
}

static int32_t decimal_digits(int32_t value)
{
    int32_t limit = 10;
    for (int32_t digits = 1; digits < STRING_MAX_DIGITS; digits++) {
        if (value < limit) {
            return digits;
        }
        limit *= 10;
    }
    return (value >= 1000000000) ? 0 : STRING_MAX_DIGITS;
}

int32_t string_from_int(int32_t value, int32_t offset, int32_t force_plus)
{
    memset(string_number_buffer, 0, sizeof(string_number_buffer));
    if (value < 0) {
        value = -value;
        string_number_buffer[offset++] = '-';
    } else if (force_plus) {
        string_number_buffer[offset++] = '+';
    }
    int32_t digits = decimal_digits(value);
    int32_t end = offset + digits;
    for (int32_t i = digits - 1; i >= 0; i--) {
        string_number_buffer[offset + i] = (char) ('0' + (value % 10));
        value /= 10;
    }
    return end;
}
