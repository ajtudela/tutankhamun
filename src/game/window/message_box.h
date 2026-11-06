// The game's message box (message_box.c): showing it, one frame of it, its draw and buttons.

#ifndef WINDOW_MESSAGE_BOX_H
#define WINDOW_MESSAGE_BOX_H

#include <stdint.h>

int32_t window_message_box_show(const char *title, const char *text, uint32_t type);
int32_t window_message_box_run_frame(const char *title, const char *text, uint32_t type);
int32_t window_message_box_draw(const char *title, const char *text, uint32_t type);
void window_message_box_button(void);

extern int32_t (*window_message_box_test_answer)(const char *title, const char *text, uint32_t type);

#endif
