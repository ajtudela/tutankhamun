// The enable flags of the build menu (building_menu.c) and the size of the menu table (38 groups of 30 rows).

#ifndef WINDOW_BUILDING_MENU_H
#define WINDOW_BUILDING_MENU_H

#include <stdint.h>

enum build_menu_size {
    BUILD_MENU_GROUPS = 38,
    BUILD_MENU_ITEMS = 30,
};

int32_t building_menu_count_items(void);
void building_menu_update(void);
void sidebar_build_buttons_update(void);
void building_menu_enable_all(void);
int32_t building_menu_tutorial_ok(void);

#endif
