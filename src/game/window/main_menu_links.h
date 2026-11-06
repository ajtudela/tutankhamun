// The link buttons of the front menu (main_menu_links.c): their addresses and places (main_menu_links.json), their
// draw and their click.

#ifndef WINDOW_MAIN_MENU_LINKS_H
#define WINDOW_MAIN_MENU_LINKS_H

#include "window/main_menu_link_icons.h"
#include <stdint.h>

enum main_menu_link_limits {
    MAIN_MENU_LINK_URL_SIZE = 128,
    MAIN_MENU_LINK_MAX_OFFSET = 4096,
};

typedef struct main_menu_link_place {
    char url[MAIN_MENU_LINK_URL_SIZE];
    int32_t right;
    int32_t bottom;
} main_menu_link_place;

extern main_menu_link_place main_menu_links[MAIN_MENU_LINKS];

void main_menu_links_keep_background(void);
void main_menu_links_draw(void);
int32_t main_menu_links_handle_mouse(void);

#endif
