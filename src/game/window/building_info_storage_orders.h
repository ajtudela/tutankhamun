// The input of the storage buildings' orders panel (building_info_storage_orders.c).

#ifndef WINDOW_BUILDING_INFO_STORAGE_ORDERS_H
#define WINDOW_BUILDING_INFO_STORAGE_ORDERS_H

#include <stdint.h>

void building_info_show_orders(void);
void storage_orders_toggle_resource(void);
void storage_orders_limit_arrow(void);
void storage_orders_accept_none(void);
void building_orders_scroll_arrow(void);
int32_t storage_orders_scrollbar(void);

#endif
