// The main menu's link buttons in their order (Discord, GitHub, Patreon, wiki) and their 36x36 pictures, RGB565 colours and
// alpha bytes per pixel (main_menu_link_icons.c, generated).

#ifndef WINDOW_MAIN_MENU_LINK_ICONS_H
#define WINDOW_MAIN_MENU_LINK_ICONS_H

#include <stdint.h>

enum main_menu_link {
    MAIN_MENU_LINK_DISCORD = 0,
    MAIN_MENU_LINK_GITHUB = 1,
    MAIN_MENU_LINK_PATREON = 2,
    MAIN_MENU_LINK_WIKI = 3,
    MAIN_MENU_LINKS = 4,
};

enum main_menu_link_icon_size {
    MAIN_MENU_LINK_ICON_SIZE = 36,
    MAIN_MENU_LINK_ICON_PIXELS = MAIN_MENU_LINK_ICON_SIZE * MAIN_MENU_LINK_ICON_SIZE,
};

extern const uint16_t main_menu_link_icon_colors[MAIN_MENU_LINKS][MAIN_MENU_LINK_ICON_PIXELS];
extern const uint8_t main_menu_link_icon_alpha[MAIN_MENU_LINKS][MAIN_MENU_LINK_ICON_PIXELS];

#endif
