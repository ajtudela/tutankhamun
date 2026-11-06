// Config table goods_overlays.json: per overlay the resource it shows, the inventory slot that holds it and
// whether its tooltip shows the stock.

#include "building/house_evolve.h"
#include "city/resource_type.h"
#include "core/config.h"
#include "widget/city_overlay_goods.h"

enum { GOODS_OVERLAY_STRIDE = sizeof(goods_overlay) / sizeof(int32_t) };

static const config_name inventory_slot_names[] = {
    { "INVENTORY_GRAIN", INVENTORY_GRAIN },
    { "INVENTORY_MEAT", INVENTORY_MEAT },
    { "INVENTORY_LETTUCE", INVENTORY_LETTUCE },
    { "INVENTORY_CHICKPEAS", INVENTORY_CHICKPEAS },
    { "INVENTORY_POMEGRANATES", INVENTORY_POMEGRANATES },
    { "INVENTORY_FIGS", INVENTORY_FIGS },
    { "INVENTORY_FISH", INVENTORY_FISH },
    { "INVENTORY_GAMEMEAT", INVENTORY_GAMEMEAT },
    { "INVENTORY_BEER", INVENTORY_BEER },
    { "INVENTORY_POTTERY", INVENTORY_POTTERY },
    { "INVENTORY_LINEN", INVENTORY_LINEN },
    { "INVENTORY_JEWELRY", INVENTORY_JEWELRY },
    { 0, 0 },
};

static const config_field goods_overlay_fields[] = {
    { "resource", &goods_overlays[0].resource, GOODS_OVERLAY_STRIDE, 1, RESOURCE_NONE, RESOURCE_MAX - 1, config_resource_names },
    { "inventory_slot", &goods_overlays[0].inventory_slot, GOODS_OVERLAY_STRIDE, 1, INVENTORY_GRAIN, HOUSE_INVENTORY_SLOTS - 1, inventory_slot_names },
    { "shows_tooltip", &goods_overlays[0].shows_tooltip, GOODS_OVERLAY_STRIDE, 1, 0, 1, 0 },
};

static const config_storage goods_overlay_storage[] = {
    { goods_overlays, sizeof(goods_overlays) },
};

const config_table goods_overlay_table = {
    "goods_overlays.json",
    "overlay",
    config_overlay_names,
    OVERLAY_COUNT,
    CONFIG_TABLE_COLUMNS(goods_overlay_fields, goods_overlay_storage),
};
