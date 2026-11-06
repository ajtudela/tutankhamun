// The functions of resource_settings.c that other files call.

#ifndef WINDOW_RESOURCE_SETTINGS_H
#define WINDOW_RESOURCE_SETTINGS_H

#include <stdint.h>

int32_t window_advisor_trade_handle_scrollbar(void);

void window_resource_settings_draw_foreground(void);
void window_resource_settings_cycle_trade(void);
void window_resource_settings_toggle_industry(void);
void window_resource_settings_toggle_import(void);
void window_resource_settings_toggle_export(void);
void window_resource_settings_change_trade_amount(void);
void window_resource_settings_toggle_stockpile(void);
void window_resource_settings_help(void);
void window_resource_settings_back(void);

#endif
