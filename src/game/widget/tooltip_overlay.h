// The overlay tooltip texts and their indexes in the overlay tooltips text group.

#ifndef WIDGET_TOOLTIP_OVERLAY_H
#define WIDGET_TOOLTIP_OVERLAY_H

#include <stdint.h>

enum overlay_tooltip_text {
    TOOLTIP_GOODS_GRAIN = 0,
    TOOLTIP_GOODS_FIGS = 2,
    TOOLTIP_GOODS_LETTUCE = 4,
    TOOLTIP_GOODS_MEAT = 6,
    TOOLTIP_GOODS_POTTERY = 8,
    TOOLTIP_GOODS_LUXURY_GOODS = 10,
    TOOLTIP_GOODS_LINEN = 12,
    TOOLTIP_GOODS_OTHER = 14,
    TOOLTIP_GOODS_AMOUNT = 110,
    TOOLTIP_GOODS_AMOUNT_ONE = 111,
    TOOLTIP_NOTHING_STORED = 126,
    TOOLTIP_DISEASE_RISK_NONE = 127,
    TOOLTIP_DISEASE_RISK_LOW = 128,
    TOOLTIP_DISEASE_RISK_MEDIUM = 129,
    TOOLTIP_DISEASE_RISK_HIGH = 130,
    TOOLTIP_DISEASE_RISK_VERY_HIGH = 131,
    TOOLTIP_MALARIA_RISK_NONE = 136,
    TOOLTIP_MALARIA_RISK_LOW = 137,
    TOOLTIP_MALARIA_RISK_MEDIUM = 138,
    TOOLTIP_MALARIA_RISK_HIGH = 139,
    TOOLTIP_MALARIA_RISK_VERY_HIGH = 140,
    TOOLTIP_HOUSE_PROBLEMS = 141,
    TOOLTIP_BUILDING_PROBLEMS = 142,
    TOOLTIP_FIRST_PROBLEM = 143,
};

const char *tooltip_overlay_goods(int32_t building_id, int32_t resource, int32_t slot, int32_t *length);
const char *tooltip_overlay_malaria(int32_t building_id);
const char *tooltip_overlay_disease(int32_t building_id);
int32_t tooltip_overlay_problems(int32_t building_id);

#endif
