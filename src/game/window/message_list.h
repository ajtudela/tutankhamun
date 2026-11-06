// The message list (message_list.c).

#ifndef WINDOW_MESSAGE_LIST_H
#define WINDOW_MESSAGE_LIST_H

#include <stdint.h>

void city_message_go_to_problem(void);
void button_message_help(void);
void window_message_list_draw_messages(void);
void window_message_list_show(void);
void window_message_list_handle_input(void);
void message_list_scroll(void);
void message_list_button_help(void);
void message_list_button_message(void);
void message_list_delete_read(void);
void message_list_delete_message(void);
void message_dialog_show_help(void);
void message_dialog_go_to(void);
void window_message_list_draw_background(void);
int32_t message_list_handle_scrollbar(void);

#endif
