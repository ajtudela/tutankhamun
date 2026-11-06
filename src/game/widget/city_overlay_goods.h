// The goods overlays: per overlay the resource whose stock it shows, the house or bazaar inventory slot that
// holds it, and whether its tooltip shows the stock.

#ifndef WIDGET_CITY_OVERLAY_GOODS_H
#define WIDGET_CITY_OVERLAY_GOODS_H

#include "widget/city_with_overlay.h"
#include <stdint.h>

typedef struct goods_overlay {
    int32_t resource;
    int32_t inventory_slot;
    int32_t shows_tooltip;
} goods_overlay;

extern goods_overlay goods_overlays[OVERLAY_COUNT];

int32_t city_overlay_goods(int32_t overlay, goods_overlay *goods);

#endif
