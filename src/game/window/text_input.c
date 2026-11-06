// The text fields: the editing keys (Enter, Delete, Backspace, Insert), a typed character inserted or overwritten,
// the cursor clamped, the limit state, and the capture of a field (one of the capture tables' 19 slots).
//
// ", . ? !" are accepted only by a field that does not take spaces (its flag reads as "punctuation off").

#include "window/text_input.h"
#include "graphics/graphics_data.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "window/window_data.h"
#include <stdint.h>
#include <string.h>

enum text_input_limit {
    TEXT_INPUT_LIMIT_NONE = 0,
    TEXT_INPUT_LIMIT_END_OF_TEXT = 1,
    TEXT_INPUT_LIMIT_FULL = 2,
};

void keyboard_return(void)
{
    keyboard_accepted = 1;
}

void text_input_remove(char *from, char *end)
{
    if (from < end) {
        size_t count = (size_t) (end - from);
        for (size_t i = 0; i < count; i++) {
            from[i] = from[i + 1];
        }
        from[count] = 0;
    } else {
        *from = 0;
    }
}

void keyboard_delete(void)
{
    int32_t slot = keyboard_field_index;
    if (slot <= 0) {
        return;
    }
    char *text = capture_text[slot];
    text_input_remove(text + capture_cursor_position[slot], text + capture_text_length[slot]);
    text_input_update_cursor();
}

void keyboard_backspace(void)
{
    int32_t slot = keyboard_field_index;
    if (capture_cursor_position[slot] <= 0) {
        return;
    }
    capture_cursor_position[slot]--;
    keyboard_delete();
}

void keyboard_insert(void)
{
    capture_insert_mode ^= 1;
}

void keyboard_left(void)
{
    int32_t slot = keyboard_field_index;
    if (capture_cursor_position[slot] > 0) {
        capture_cursor_position[slot]--;
    }
}

void keyboard_right(void)
{
    int32_t slot = keyboard_field_index;
    if (capture_cursor_position[slot] < capture_text_length[slot]) {
        capture_cursor_position[slot]++;
    }
}

void keyboard_home(void)
{
    capture_cursor_position[keyboard_field_index] = 0;
}

void keyboard_end(void)
{
    capture_cursor_position[keyboard_field_index] = capture_text_length[keyboard_field_index];
}

void keyboard_capture_set(int32_t slot)
{
    int32_t max = capture_max_length[slot];
    keyboard_field_index = slot;
    capture_text_length[slot] = 0;
    if (max < 0) {
        return;
    }
    const char *text = capture_text[slot];
    for (int32_t i = 0; (i <= max) && (text[i] != 0); i++) {
        capture_text_length[slot]++;
    }
}

void text_cursor_reset(void)
{
    g_cursor_seen = 0;
    g_char_index = 0;
    g_cursor_width = 0;
    g_text_width = 0;
    g_draw_y_accum = 0;
}

void text_input_shift(uint8_t *from, uint8_t *to)
{
    to[1] = 0;
    while (from < to) {
        *to = to[-1];
        to--;
    }
}

void text_input_update_cursor(void)
{
    int32_t slot = keyboard_field_index;
    capture_limit_state = TEXT_INPUT_LIMIT_NONE;
    if (slot <= 0) {
        return;
    }
    int32_t width = text_get_width((const uint8_t *) capture_text[slot], capture_font[slot]);
    int32_t *cursor = &capture_cursor_position[slot];
    int32_t length = capture_text_length[slot];
    int32_t max = capture_max_length[slot];
    if ((length + 1) <= *cursor) {
        *cursor = length + 1;
        capture_limit_state = TEXT_INPUT_LIMIT_END_OF_TEXT;
    }
    if (max <= *cursor) {
        *cursor = max;
        capture_limit_state = TEXT_INPUT_LIMIT_FULL;
    }
    if (capture_width[slot] <= width) {
        capture_limit_state = TEXT_INPUT_LIMIT_FULL;
    }
    if (*cursor < 0) {
        *cursor = 0;
    }
}

void text_input_insert(uint8_t c)
{
    int32_t slot = keyboard_field_index;
    if (slot <= 0) {
        return;
    }
    text_input_update_cursor();
    if (capture_limit_state == TEXT_INPUT_LIMIT_FULL) {
        return;
    }
    uint8_t *buffer = (uint8_t *) capture_text[slot];
    int32_t *cursor = &capture_cursor_position[slot];
    if (capture_insert_mode == 0) {
        buffer[*cursor] = c;
        (*cursor)++;
        buffer[*cursor] = 0;
    } else if (capture_text_length[slot] < capture_max_length[slot]) {
        text_input_shift(buffer + *cursor, buffer + capture_text_length[slot] + 1);
        buffer[*cursor] = c;
        (*cursor)++;
    }
}

static int32_t is_text_character(uint8_t c)
{
    return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 0x80) && (c <= 0x9A)) ||
        ((c >= 0xA0) && (c <= 0xA7)) || (c == 0xE1);
}

void keyboard_character(uint8_t c)
{
    int32_t slot = keyboard_field_index;
    if ((c == ' ') || (c == '-')) {
        text_input_insert(c);
    } else if ((c == ',') || (c == '.') || (c == '?') || (c == '!')) {
        if (capture_allow_space[slot] == 0) {
            text_input_insert(c);
        }
    } else if (is_text_character(c)) {
        text_input_insert(c);
    }
    text_input_update_cursor();
}
