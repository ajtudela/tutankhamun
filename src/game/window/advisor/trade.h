// The functions of trade.c that other files call.

#ifndef WINDOW_ADVISOR_TRADE_H
#define WINDOW_ADVISOR_TRADE_H

#include <stdint.h>

void window_advisor_trade_draw_background(void);
void city_list_scroll_arrow(void);
void window_advisor_trade_draw_foreground(void);
void window_advisor_trade_draw_scrollbar(void);
void city_toggle_resource_settings_step(void);
void button_trade_resource(void);
void button_trade_prices_mode_toggle(void);
void button_trade_exit(void);

#endif
