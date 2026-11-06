// The gameplay config table gift_costs.json: gift_cost_rules (city/population.h), per gift size (modest, generous,
// lavish) the divisor of the savings and the base cost.

#include "city/population.h"
#include "core/config.h"

static const config_name gift_cost_names[] = {
    { "modest", GIFT_MODEST },
    { "generous", GIFT_GENEROUS },
    { "lavish", GIFT_LAVISH },
    { 0, 0 },
};

static const config_field gift_cost_fields[] = {
    { "savings_divisor", &gift_cost_rules[0].savings_divisor, 2, 1, 1, 100000, 0 },
    { "base_cost", &gift_cost_rules[0].base_cost, 2, 1, 0, 1000000, 0 },
};

static const config_storage gift_cost_storage[] = {
    { gift_cost_rules, sizeof(gift_cost_rules) },
};

const config_table gift_cost_table = {
    "gift_costs.json",
    "gift",
    gift_cost_names,
    GIFT_SIZES,
    CONFIG_TABLE_COLUMNS(gift_cost_fields, gift_cost_storage),
};
