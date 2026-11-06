// The text fields (text_input.c): the editing keys, the capture of a field and the cursor.

#ifndef WINDOW_TEXT_INPUT_H
#define WINDOW_TEXT_INPUT_H

#include <stdint.h>

enum keyboard_field {
    KEYBOARD_FIELD_PLAYER_NAME = 1,
    KEYBOARD_FIELD_FILE_NAME = 2,
    KEYBOARD_FIELD_MAP_NAME = 4,
    KEYBOARD_FIELD_SCENARIO_NAME = 5,
    KEYBOARD_FIELD_SCENARIO_BRIEFING = 6,
    KEYBOARD_FIELD_CHEAT = 7,
};

void text_cursor_reset(void);
void keyboard_capture_set(int32_t slot);
void keyboard_home(void);
void keyboard_end(void);
void text_input_update_cursor(void);
void keyboard_delete(void);
void keyboard_right(void);
void keyboard_left(void);
void keyboard_backspace(void);
void keyboard_return(void);
void keyboard_insert(void);
void keyboard_character(uint8_t c);
void text_input_insert(uint8_t c);
void text_input_remove(char *from, char *end);
void text_input_shift(uint8_t *from, uint8_t *to);

#endif
