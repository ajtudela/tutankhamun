// The functions of empire/empire.c: trade checks, produce checks, the stock kept before export, the yearly reset,
// new traders, map object images, routes and travel times, the message tokens and the getters of the trade cities.

#ifndef EMPIRE_EMPIRE_H
#define EMPIRE_EMPIRE_H

#include "city/resource_type.h"
#include "empire/empire_data.h"
#include "empire/empire_route.h"
#include "empire/empire_types.h"

#include <stddef.h>
#include <stdint.h>

enum trade_status {
    TRADE_STATUS_NONE = 0,
    TRADE_STATUS_IMPORT = 1,
    TRADE_STATUS_EXPORT = 2,
    TRADE_STATUS_IMPORT_AS_NEEDED = 3,
    TRADE_STATUS_EXPORT_SURPLUS = 4,
};

enum empire_trade_rule {
    EMPIRE_TRADE_RULE_IMPORT_DEBT_LIMIT,
    EMPIRE_TRADE_RULE_LIMIT_SMALL,
    EMPIRE_TRADE_RULE_LIMIT_MEDIUM,
    EMPIRE_TRADE_RULE_LIMIT_LARGE,
    EMPIRE_TRADE_RULE_SEA_ENTRY_DELAY,
    EMPIRE_TRADE_RULE_LAND_ENTRY_DELAY,
    EMPIRE_TRADE_RULE_NEW_CITY_ENTRY_DELAY,
    EMPIRE_TRADE_RULE_NO_DOCKS_MESSAGE_TICKS,
    EMPIRE_TRADE_RULE_SHIP_LOADS,
    EMPIRE_TRADE_RULE_TRADER_WAIT_TICKS,
    EMPIRE_TRADE_RULE_KEEP_WORKSHOP_AMOUNT,
    EMPIRE_TRADE_RULE_KEEP_MONUMENT_AMOUNT,
    EMPIRE_TRADE_RULE_KEEP_OTHER_AMOUNT,
    EMPIRE_TRADE_RULE_FOOD_POPULATION_SMALL,
    EMPIRE_TRADE_RULE_FOOD_POPULATION_MEDIUM,
    EMPIRE_TRADE_RULE_FOOD_POPULATION_LARGE,
    EMPIRE_TRADE_RULE_COUNT,
};

extern int32_t empire_trade_rules[EMPIRE_TRADE_RULE_COUNT];

static inline const empire_city *empire_city_get(int32_t id)
{
    return &empire_cities[id];
}

static inline empire_city *empire_city_edit(int32_t id)
{
    return &empire_cities[id];
}

int32_t empire_can_produce_resource(int32_t resource);
int32_t empire_route_find_for_city(int32_t city_id, uint8_t *ours_first);
int32_t empire_can_import_resource(int32_t resource);
int32_t empire_can_import_resource_potentially(int32_t resource);
int32_t empire_can_produce_resource_potentially(int32_t resource);
int32_t empire_city_has_trade_limit(int32_t city_id, int32_t resource);
int32_t empire_can_export_resource_to_city(int32_t player, int32_t city_id, int32_t resource);
int32_t empire_can_import_resource_from_city(int32_t player, int32_t city_id, int32_t resource);
int32_t building_count_workshops_using_resource(int32_t resource);
int32_t empire_object_city_image(int32_t object_id);
int32_t empire_count_luxury_goods_kinds(int32_t *kinds);
void empire_update_export_keep(void);
void empire_city_generate_trader(void);
void empire_update_yearly_trade_stats(void);
void empire_city_reset_yearly_trade_amounts(void);
void empire_object_set_images(void);
void empire_object_set_city_images(void);
int32_t empire_city_get_for_trade_route(uint32_t route_id);
int32_t empire_route_travel_months(const empire_route *route);
const char *message_token_player_rank(void);
const char *message_token_player_name(void);
const char *message_token_city_name(void);
const char *message_token_amount(void);
const char *message_token_resource_name(void);

#endif
