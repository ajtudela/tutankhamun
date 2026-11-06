// Built-in table of the goods overlays: per overlay the resource it shows; every other overlay has none.
// config/goods_overlays.json can override it.
//
// Five food overlays show another food than their name: chickpeas shows lettuce, pomegranates figs, figs meat, meat
// chickpeas and game pomegranates.

#include "widget/city_overlay_goods.h"
#include "building/house_evolve.h"
#include "city/resource_type.h"

goods_overlay goods_overlays[OVERLAY_COUNT] = {
    [OVERLAY_GRAIN] = { RESOURCE_GRAIN, INVENTORY_GRAIN, 1 },
    [OVERLAY_CHICKPEAS] = { RESOURCE_LETTUCE, INVENTORY_LETTUCE, 1 },
    [OVERLAY_POMEGRANATES] = { RESOURCE_FIGS, INVENTORY_FIGS, 1 },
    [OVERLAY_FIGS] = { RESOURCE_MEAT, INVENTORY_MEAT, 1 },
    [OVERLAY_MEAT] = { RESOURCE_CHICKPEAS, INVENTORY_CHICKPEAS, 1 },
    [OVERLAY_GAME] = { RESOURCE_POMEGRANATES, INVENTORY_POMEGRANATES, 0 },
    [OVERLAY_POTTERY] = { RESOURCE_POTTERY, INVENTORY_POTTERY, 1 },
    [OVERLAY_JEWELRY] = { RESOURCE_LUXURY_GOODS, INVENTORY_JEWELRY, 1 },
    [OVERLAY_LINEN] = { RESOURCE_LINEN, INVENTORY_LINEN, 1 },
    [OVERLAY_BEER] = { RESOURCE_BEER, INVENTORY_BEER, 1 },
};

int32_t city_overlay_goods(int32_t overlay, goods_overlay *goods)
{
    if (overlay < 0 || overlay >= OVERLAY_COUNT || goods_overlays[overlay].resource == RESOURCE_NONE) {
        return 0;
    }
    *goods = goods_overlays[overlay];
    return 1;
}
