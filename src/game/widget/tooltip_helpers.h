// The functions of tooltip_helpers.c that other files call.

#ifndef WIDGET_TOOLTIP_HELPERS_H
#define WIDGET_TOOLTIP_HELPERS_H

#include <stdint.h>

int32_t mission_brief_tooltip_text(void);
int32_t empire_tooltip_text(uint8_t *flag);
int32_t trade_prices_tooltip_text(uint8_t *flag);
void tooltip_draw_luxury_goods(void);
void tooltip_draw_figure_goods(void);

#endif
