// The tables of input/input_data.c.

#ifndef INPUT_INPUT_DATA_H
#define INPUT_INPUT_DATA_H

#include <stdint.h>

extern const char network_info_label[];

extern char *capture_text[20];

extern uint8_t capture_allow_space[20];

extern int32_t capture_width[20];

extern int32_t capture_text_length[20];

extern int32_t capture_max_length[20];

extern int32_t capture_font[20];

extern int32_t capture_limit_state;

extern int32_t cursor_raw_y;

extern int32_t cursor_raw_x;

extern uint8_t mouse_right_down_prev;

extern uint8_t mouse_right_down;

extern uint8_t mouse_right_clicked;

extern uint8_t mouse_right_went_down;

extern uint8_t mouse_right_went_up;

extern uint8_t mouse_left_down;

extern uint8_t mouse_left_clicked;

extern uint8_t mouse_left_went_down;

extern uint8_t mouse_left_went_up;

extern uint8_t mouse_buttons;

extern int16_t mouse_window_y;

extern int16_t mouse_window_x;

extern int32_t mouse_drag_state;

extern int32_t mouse_y_ui;

extern int32_t mouse_x_ui;

extern int32_t mouse_y;

extern int32_t mouse_x;

extern int32_t mouse_moved;

extern int32_t debug_network_info_on;

#endif
