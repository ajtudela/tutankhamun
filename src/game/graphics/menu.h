// Records of the top menu: a drop-down item (offset, text, handler, hidden flag, parameter) and a menu bar
// entry (its text span, text group and the range of its items in the menu item table).

#ifndef GRAPHICS_MENU_H
#define GRAPHICS_MENU_H

#include <stdint.h>

typedef void (*menu_item_handler)(void);

typedef struct {
    int16_t y;
    int16_t text_number;
    menu_item_handler handler;
    int32_t hidden;
    int32_t parameter;
} menu_item;

enum menu_bar_size {
    MENU_ITEMS = 44,
    MENU_BAR_ENTRIES = 4,
};

typedef struct {
    int32_t x_start;
    int32_t x_end;
    int32_t text_group;
    int32_t first_item;
    int32_t item_count;
} menu_bar_entry;

#endif
