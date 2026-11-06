// City resources (city/resources.c): food consumption, import payment, the available resources, the stock caps and the
// bazaar goods.

#ifndef CITY_RESOURCES_H
#define CITY_RESOURCES_H

#include "city/resource_type.h"
#include "empire/empire_data.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t buy_price;
    int32_t sell_price;
} trade_price;

static_assert(sizeof(trade_price) * RESOURCE_MAX == sizeof(trade_prices), "trade_prices holds one trade_price per resource");

static inline const trade_price *trade_price_get(int32_t resource)
{
    return &((const trade_price *) trade_prices)[resource];
}

void city_resource_consume_food(void);
void city_finance_pay_for_import(int32_t player_id, int32_t resource);
bool city_resource_available_from_events(int32_t resource);
void city_resource_determine_available(void);
int32_t city_resource_cap_amount(int32_t resource, int32_t amount);
int32_t resource_is_bazaar_good(int32_t resource);

#endif
