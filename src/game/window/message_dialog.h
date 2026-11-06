// The message dialog (message_dialog.c): its draw and input, and the messages other windows open by id.

#ifndef WINDOW_MESSAGE_DIALOG_H
#define WINDOW_MESSAGE_DIALOG_H

#include <stdbool.h>
#include <stdint.h>

enum dialog_history_size {
    DIALOG_HISTORY_ENTRIES = 200,
};

enum message_dialog_id {
    MESSAGE_DIALOG_ABOUT = 0,
    MESSAGE_DIALOG_HELP = 10,
    MESSAGE_DIALOG_TOP_FUNDS = 15,
    MESSAGE_DIALOG_TOP_POPULATION = 16,
    MESSAGE_DIALOG_TOP_DATE = 17,
    MESSAGE_DIALOG_FESTIVAL_HELP = 29,
    MESSAGE_DIALOG_EMPIRE_HELP = 32,
    MESSAGE_DIALOG_MESSAGES_HELP = 34,
    MESSAGE_DIALOG_TRADE_RESOURCE_HELP = 46,
};

void window_message_dialog_show(int32_t type, int32_t arg, bool has_phrases);
void window_message_dialog_close(void);
void message_dialog_draw_scrollbar_dot(void);
void message_dialog_draw_phrases(void);
void message_dialog_draw_background_normal(void);
void message_dialog_draw_background_video(void);
void window_message_dialog_draw_foreground(void);
void window_message_dialog_draw_background(void);
void window_message_dialog_handle_input(void);
int32_t message_dialog_handle_scrollbar(void);

#endif
