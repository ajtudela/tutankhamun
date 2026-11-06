// The menu bar and drop-down functions the top menu, the window input and draw and the arrow buttons call.

#ifndef WIDGET_MENU_BAR_H
#define WIDGET_MENU_BAR_H

#include "graphics/menu.h"
#include <stdint.h>

void menu_bar_draw(menu_bar_entry *entries, int32_t count);
void menu_bar_draw_side_button(void);
void top_menu_draw_panel_strip(void);
int32_t ui_layout_column_width(int32_t index);
int32_t ui_layout_menu_width(int32_t group);
void menu_bar_close(void);
void menu_bar_clear(void);
int32_t menu_dropdown_focus(const menu_item *items, int32_t count, int32_t x, int32_t y);
int32_t menu_dropdown_handle_mouse(menu_bar_entry *menu, int32_t count);
void menu_dropdown_draw(void);
int32_t menu_bar_focus(const menu_bar_entry *menu, int32_t count);
int32_t menu_bar_handle_mouse(menu_bar_entry *menu, int32_t count);

#endif
