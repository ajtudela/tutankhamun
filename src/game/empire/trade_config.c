// The gameplay config table of the default trade prices (core/config.h): the buy and sell price of each resource.

#include "city/resource_type.h"
#include "core/config.h"
#include "empire/empire_data.h"

static const config_field trade_default_price_fields[] = {
    { "buy", trade_default_prices, 2, 1, 0, 100000, 0 },
    { "sell", trade_default_prices + 1, 2, 1, 0, 100000, 0 },
};

static const config_storage trade_default_price_storage[] = {
    { trade_default_prices, (sizeof(int32_t) * 2) * RESOURCE_MAX },
};

const config_table trade_default_price_table = {
    "trade_default_prices.json",
    "resource",
    config_resource_names,
    RESOURCE_MAX,
    CONFIG_TABLE_COLUMNS(trade_default_price_fields, trade_default_price_storage),
};
