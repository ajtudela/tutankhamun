// Text input slots of the keyboard (the game's text boxes: names, file names, chat; 20 slots): starting the capture
// of one sets its text buffer and limits, its length and cursor to 0, and returns the slot.

#include "input/keyboard_capture.h"

#include "graphics/graphics_data.h"
#include "input/input_data.h"

#include <stdint.h>

int32_t keyboard_start_capture(int32_t slot, char *text, int32_t max_length, int32_t width, int32_t allow_space, int32_t font)
{
    capture_max_length[slot] = max_length;
    capture_width[slot] = width;
    capture_font[slot] = font;
    capture_allow_space[slot] = (uint8_t) allow_space;
    capture_text_length[slot] = 0;
    capture_cursor_position[slot] = 0;
    capture_text[slot] = text;
    capture_limit_state = 0;
    text_cursor_y = 0;
    return slot;
}
