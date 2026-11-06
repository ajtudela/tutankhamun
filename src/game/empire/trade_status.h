// The functions of trade_status.c that other files call.

#ifndef EMPIRE_TRADE_STATUS_H
#define EMPIRE_TRADE_STATUS_H

#include <stdint.h>

int32_t empire_object_city_sells_resource(int32_t object_id, int32_t resource);
int32_t empire_object_city_buys_resource(int32_t object_id, int32_t resource);
int32_t empire_can_export_resource(int32_t resource);
int32_t empire_city_is_trade_route_open(int32_t route);
int32_t city_trade_status_update(int32_t resource);

#endif
