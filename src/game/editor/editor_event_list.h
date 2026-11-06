// The functions of editor_event_list.c that other files call.

#ifndef EDITOR_EDITOR_EVENT_LIST_H
#define EDITOR_EDITOR_EVENT_LIST_H

#include <stdint.h>

int32_t event_list_scrollbar(void);

void event_list_new(void);
void window_event_list_draw(void);
void event_list_arrow(void);
void event_list_row(void);
void event_list_delete(void);
void event_editor_back(void);

#endif
