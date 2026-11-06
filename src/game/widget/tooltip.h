// The button, senate and overlay tooltips: handling, drawing, reset, and the tooltip kinds.

#ifndef WIDGET_TOOLTIP_H
#define WIDGET_TOOLTIP_H

#include <stdint.h>

enum tooltip_kind {
    TOOLTIP_KIND_BUTTON = 0,
    TOOLTIP_KIND_OVERLAY = 1,
    TOOLTIP_KIND_SENATE = 2,
    TOOLTIP_KIND_LUXURY_GOODS = 3,
    TOOLTIP_KIND_FIGURE_GOODS = 4,
};

int32_t tooltip_handle(void);
int32_t tooltip_handle_overlay(void);
int32_t tooltip_reset(void);
int32_t tooltip_invalidate(void);
void tooltip_draw(void);
void tooltip_draw_text_box(void);
void tooltip_draw_senate(void);

#endif
