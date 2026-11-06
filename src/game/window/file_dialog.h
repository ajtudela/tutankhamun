// The file dialogs (file_dialog.c): their draw and scroll bar, the dialog kinds and the dialogs' texts.

#ifndef WINDOW_FILE_DIALOG_H
#define WINDOW_FILE_DIALOG_H

#include <stdint.h>

enum file_dialog_rows {
    FILE_DIALOG_ROWS = 12,
};

enum file_dialog_type {
    FILE_DIALOG_SAVE = 0,
    FILE_DIALOG_LOAD = 1,
    FILE_DIALOG_DELETE = 2,
};

enum file_dialog_text {
    FILE_DIALOG_TEXT_SAVING_YOUR_CITY = 0,
    FILE_DIALOG_TEXT_LOADING_SAVED_GAME = 1,
    FILE_DIALOG_TEXT_FILE_DOES_NOT_EXIST = 2,
    FILE_DIALOG_TEXT_SAVING_A_MISSION = 3,
    FILE_DIALOG_TEXT_LOADING_A_MISSION = 4,
    FILE_DIALOG_TEXT_PROCEED = 5,
    FILE_DIALOG_TEXT_DELETE_A_FILE = 6,
};

void window_file_dialog_draw_scrollbar(void);
void window_file_dialog_draw(void);
void window_map_file_dialog_draw(void);

#endif
