// Texts of the overlay tooltips: the goods amount at a bazaar, granary, storage yard or house, a house's
// disease and malaria risk levels, and the numbered list of a building's problems.

#include "widget/tooltip_overlay.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "city/resource_type.h"
#include "core/text.h"
#include "graphics/graphics_data.h"
#include "text/lang.h"
#include "widget/city_building_tiles.h"
#include "widget/widget_data.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum overlay_tooltip_risk {
    DISEASE_RISK_VERY_HIGH_ABOVE = 849,
    DISEASE_RISK_HIGH_ABOVE = 599,
    DISEASE_RISK_MEDIUM_ABOVE = 449,
    DISEASE_RISK_LOW_ABOVE = 9,
    MALARIA_RISK_TENTHS = 10,
    MALARIA_RISK_VERY_HIGH_ABOVE = 84,
    MALARIA_RISK_HIGH_ABOVE = 59,
    MALARIA_RISK_MEDIUM_ABOVE = 44,
    MALARIA_RISK_LOW_ABOVE = 0,
};

static const char *tooltip_text(int32_t text)
{
    return lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, text);
}

static int32_t goods_text(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
            return TOOLTIP_GOODS_GRAIN;
        case RESOURCE_MEAT:
            return TOOLTIP_GOODS_MEAT;
        case RESOURCE_LETTUCE:
            return TOOLTIP_GOODS_LETTUCE;
        case RESOURCE_FIGS:
            return TOOLTIP_GOODS_FIGS;
        case RESOURCE_POTTERY:
            return TOOLTIP_GOODS_POTTERY;
        case RESOURCE_LINEN:
            return TOOLTIP_GOODS_LINEN;
        case RESOURCE_LUXURY_GOODS:
            return TOOLTIP_GOODS_LUXURY_GOODS;
        default:
            return TOOLTIP_GOODS_OTHER;
    }
}

const char *tooltip_overlay_goods(int32_t building_id, int32_t resource, int32_t slot, int32_t *length)
{
    int32_t text = goods_text(resource);
    const building *b = building_get(building_id);
    int32_t amount;
    switch (b->type) {
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
            amount = b->data.market.inventory[slot];
            break;
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            amount = b->data.granary.resource_stored[resource];
            break;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            amount = building_storageyard_get_amount(building_id, resource);
            break;
        case BUILDING_STORAGE_ROOM:
            amount = building_storageyard_get_amount(building_main(building_id), resource);
            break;
        default:
            if (b->house_size == 0) {
                return tooltip_text(TOOLTIP_NOTHING_STORED);
            }
            amount = b->data.house.inventory[slot];
            break;
    }
    *length += string_from_int(amount, 0, 0);
    strcpy(tooltip_composed, string_number_buffer);
    return tooltip_text(text + ((amount == 1) ? TOOLTIP_GOODS_AMOUNT_ONE : TOOLTIP_GOODS_AMOUNT));
}

const char *tooltip_overlay_malaria(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        int32_t risk = b->house_malaria_risk / MALARIA_RISK_TENTHS;
        if (risk > MALARIA_RISK_VERY_HIGH_ABOVE) {
            return tooltip_text(TOOLTIP_MALARIA_RISK_VERY_HIGH);
        }
        if (risk > MALARIA_RISK_HIGH_ABOVE) {
            return tooltip_text(TOOLTIP_MALARIA_RISK_HIGH);
        }
        if (risk > MALARIA_RISK_MEDIUM_ABOVE) {
            return tooltip_text(TOOLTIP_MALARIA_RISK_MEDIUM);
        }
        if (risk > MALARIA_RISK_LOW_ABOVE) {
            return tooltip_text(TOOLTIP_MALARIA_RISK_LOW);
        }
    }
    return tooltip_text(TOOLTIP_MALARIA_RISK_NONE);
}

const char *tooltip_overlay_disease(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        int32_t risk = b->house_plague_risk;
        if (risk > DISEASE_RISK_VERY_HIGH_ABOVE) {
            return tooltip_text(TOOLTIP_DISEASE_RISK_VERY_HIGH);
        }
        if (risk > DISEASE_RISK_HIGH_ABOVE) {
            return tooltip_text(TOOLTIP_DISEASE_RISK_HIGH);
        }
        if (risk > DISEASE_RISK_MEDIUM_ABOVE) {
            return tooltip_text(TOOLTIP_DISEASE_RISK_MEDIUM);
        }
        if (risk > DISEASE_RISK_LOW_ABOVE) {
            return tooltip_text(TOOLTIP_DISEASE_RISK_LOW);
        }
    }
    return tooltip_text(TOOLTIP_DISEASE_RISK_NONE);
}

int32_t tooltip_overlay_problems(int32_t building_id)
{
    const building *b = building_get(building_main(building_id));
    if (b->problem_flags == 0) {
        return 0;
    }
    strcpy(tooltip_composed, lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, (b->house_size != 0) ? TOOLTIP_HOUSE_PROBLEMS : TOOLTIP_BUILDING_PROBLEMS));
    strcat(tooltip_composed, text_line_break);
    int32_t count = 0;
    for (int32_t bit = 0; bit < BUILDING_PROBLEM_BITS; bit++) {
        if ((b->problem_flags & (1 << bit)) == 0) {
            continue;
        }
        char number[8];
        strcat(tooltip_composed, text_line_break);
        count++;
        sprintf(number, format_numbered, count);
        strcat(tooltip_composed, number);
        strcat(tooltip_composed, lang_get_string(LANG_GROUP_OVERLAY_TOOLTIPS, TOOLTIP_FIRST_PROBLEM + bit));
    }
    return 1;
}
