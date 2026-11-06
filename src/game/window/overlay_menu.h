// The overlay menu and the build menu's foreground (overlay_menu.c).

#ifndef WINDOW_OVERLAY_MENU_H
#define WINDOW_OVERLAY_MENU_H

#include <stdint.h>

void window_build_menu_draw_foreground(void);
void window_overlay_menu_draw_foreground(void);
void window_overlay_menu_show(void);
void overlay_menu_select(void);
void overlay_submenu_select(void);
int32_t overlay_menu_submenu_count(int32_t id);
int32_t overlay_menu_handle_submenu(void);
void overlay_menu_draw_background(void);

#endif
