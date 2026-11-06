// The entries of the city's top menus (top_menu_entries.c), the texts their toggles set and the info fields.

#ifndef WINDOW_TOP_MENU_ENTRIES_H
#define WINDOW_TOP_MENU_ENTRIES_H

#include <stdint.h>

enum options_menu_text {
    OPTIONS_MENU_PYRAMID_SPEEDUP_OFF = 4,
    OPTIONS_MENU_PYRAMID_SPEEDUP_ON = 5,
    OPTIONS_MENU_CITIES_EGYPTIAN = 7,
    OPTIONS_MENU_CITIES_CLASSICAL = 8,
    OPTIONS_MENU_AUTOSAVE_OFF = 9,
    OPTIONS_MENU_AUTOSAVE_ON = 10,
};

enum help_menu_text {
    HELP_MENU_MOUSE_HELP_OFF = 2,
    HELP_MENU_MOUSE_HELP_SOME = 3,
    HELP_MENU_MOUSE_HELP_FULL = 4,
    HELP_MENU_WARNINGS_OFF = 5,
    HELP_MENU_WARNINGS_ON = 6,
};

enum tooltips_setting {
    TOOLTIPS_NONE = 0,
    TOOLTIPS_SOME = 1,
    TOOLTIPS_FULL = 2,
};

enum top_menu_info {
    TOP_MENU_INFO_NONE = 0,
    TOP_MENU_INFO_FUNDS = 1,
    TOP_MENU_INFO_POPULATION = 2,
    TOP_MENU_INFO_DATE = 3,
};

void help_menu_toggle_tooltips(void);
void help_menu_toggle_warnings(void);
void options_toggle_city_names(void);
void help_menu_show_help(void);
void help_menu_show_about(void);
void menu_file_new_game(void);
void construction_right_click_help(void);

#endif
