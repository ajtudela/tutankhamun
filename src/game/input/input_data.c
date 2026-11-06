// The built-in data of the input: the key tables. input/input_data.h declares them.

#include "input/input_data.h"

const char network_info_label[] = "Network info (F2)";

char *capture_text[20];

uint8_t capture_allow_space[20];

int32_t capture_width[20];

int32_t capture_text_length[20];

int32_t capture_max_length[20];

int32_t capture_font[20];

int32_t capture_limit_state;

int32_t cursor_raw_y;

int32_t cursor_raw_x;

uint8_t mouse_right_down_prev;

uint8_t mouse_right_down;

uint8_t mouse_right_clicked;

uint8_t mouse_right_went_down;

uint8_t mouse_right_went_up;

uint8_t mouse_left_down;

uint8_t mouse_left_clicked;

uint8_t mouse_left_went_down;

uint8_t mouse_left_went_up;

uint8_t mouse_buttons;

int16_t mouse_window_y;

int16_t mouse_window_x;

int32_t mouse_drag_state;

int32_t mouse_y_ui;

int32_t mouse_x_ui;

int32_t mouse_y;

int32_t mouse_x;

int32_t mouse_moved;

int32_t debug_network_info_on;
