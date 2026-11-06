// The File menu, the file dialogs' buttons and the chief advisor's rows (file_dialog_buttons.c).

#ifndef WINDOW_FILE_DIALOG_BUTTONS_H
#define WINDOW_FILE_DIALOG_BUTTONS_H

#include <stdint.h>

void file_menu_load_game(void);
void file_menu_save_game(void);
void file_menu_delete_game(void);
void file_menu_exit(void);
void file_dialog_button_row(void);
void file_dialog_button(void);
void file_dialog_button_scroll(void);
int32_t file_dialog_handle_scrollbar(void);
void chief_advisor_button_row(void);

#endif
