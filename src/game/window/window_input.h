// The input dispatch by window id and the cancel of the construction tool (window_input.c); the scenario editor's
// empire map has its own (window_input_empire.c).

#ifndef WINDOW_WINDOW_INPUT_H
#define WINDOW_WINDOW_INPUT_H

#include <stdint.h>

void window_handle_input_dispatch(void);
void window_input_empire_editor(void);

int32_t construction_cancel(void);

#endif
