// The storage panels of the building info window (building_info_storage.c), the rows of the orders list, the storage
// yard's scroll bar and space state.

#ifndef WINDOW_BUILDING_INFO_STORAGE_H
#define WINDOW_BUILDING_INFO_STORAGE_H

#include <stdint.h>

enum storage_orders_scrollbar {
    STORAGE_ORDERS_SCROLLBAR_X = 414,
    STORAGE_ORDERS_SCROLLBAR_Y = 106,
    STORAGE_ORDERS_SCROLLBAR_WIDTH = 32,
    STORAGE_ORDERS_SCROLLBAR_THUMB = 16,
};

enum storage_yard_space_state {
    STORAGE_YARD_SPACE_FREE = 0,
    STORAGE_YARD_SPACE_FULL = 1,
    STORAGE_YARD_SPACE_GETTING_FULL = 2,
};

void window_building_draw_bazaar(void);
void window_building_draw_bazaar_foreground(void);
void window_building_draw_bazaar_orders(void);
void window_building_draw_bazaar_orders_foreground(void);
void window_building_draw_granary(void);
void window_building_draw_granary_foreground(void);
void window_building_draw_granary_orders(void);
void window_building_draw_granary_orders_foreground(void);
void window_building_draw_storage_yard(void);
void window_building_draw_storage_yard_foreground(void);
void window_building_draw_storage_yard_orders(void);
void window_building_draw_storage_yard_orders_foreground(void);
void window_building_draw_storage_scroll_thumb(void);
int32_t storage_orders_rows(void);
int32_t storage_orders_scrollbar_height(void);
int32_t building_info_orders_rows(void);
int32_t building_storageyard_get_space_info(int32_t building_id);
void window_building_info_storage_special_button(void);

#endif
