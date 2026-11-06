// The name list window (name_list.c): its draw, the choice of a name and its opening.

#ifndef WINDOW_NAME_LIST_H
#define WINDOW_NAME_LIST_H

#include <stdint.h>

enum player_names_entry {
    PLAYER_NAMES_MALE_HEADING = 0,
    PLAYER_NAMES_FEMALE_HEADING = 21,
};

enum name_list_focus {
    NAME_LIST_ENTRIES = 42,
    NAME_LIST_OK_BUTTON = 43,
    NAME_LIST_BUTTONS = 43,
};

void window_go_name_list(void);
void name_list_choose(void);
void window_name_list_draw(void);

#endif
