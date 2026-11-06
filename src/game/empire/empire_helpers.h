// The empire queries of empire/empire_helpers.c: who sells or buys a resource, availability, food kinds, trade cities
// of map objects, sea routes, trade amounts and the trade cities made at the start of a scenario.

#ifndef EMPIRE_EMPIRE_HELPERS_H
#define EMPIRE_EMPIRE_HELPERS_H

#include <stdint.h>

int32_t empire_city_get_for_object(int32_t object_id);
int32_t empire_our_city_sells_resource(int32_t resource);
int32_t empire_any_city_sells_resource(int32_t resource);
int32_t empire_any_city_buys_resource(int32_t resource);
int32_t empire_trade_city_sells_resource(int32_t resource);
int32_t empire_our_city_can_get_resource(int32_t resource);
int32_t empire_resource_available(int32_t resource);
int32_t empire_can_export_resource_potentially(int32_t resource);
int32_t empire_count_food_kinds(void);
int32_t empire_route_is_sea(int32_t route);
int32_t empire_object_trade_amount(int32_t object_id, int32_t resource);
void empire_object_set_trade_amount(int32_t object_id, int32_t resource, int32_t amount);
void empire_objects_fix_trade_amounts(void);
void empire_object_init_cities(void);

#endif
