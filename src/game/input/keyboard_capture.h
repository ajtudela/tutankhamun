// The functions of keyboard_capture.c that other files call.

#ifndef INPUT_KEYBOARD_CAPTURE_H
#define INPUT_KEYBOARD_CAPTURE_H

#include <stdint.h>

int32_t keyboard_start_capture(int32_t slot, char *text, int32_t max_length, int32_t width, int32_t allow_space, int32_t font);

#endif
