// The tables of empire/empire_data.c.

#ifndef EMPIRE_EMPIRE_DATA_H
#define EMPIRE_EMPIRE_DATA_H

#include <stdint.h>

#include "empire/empire_types.h"

extern empire_city_culture empire_culture_table[EMPIRE_CITY_NAMES];

extern int32_t trade_prices[72];

extern int32_t trade_default_prices[72];

extern const int32_t empire_food_resources[6];

extern char fmt_two_strings[8];

extern const char str_trade_recommends_loaded[36];

extern const char str_trade_recommends_out_of_sync[36];

extern char fmt_trade_recommends_bad_lines[68];

extern const char str_no_trade_recommends_file[36];

extern const char str_trade_recommends_file[24];

extern int32_t trade_export_keep[36];

extern int32_t trade_recommends[24];

extern trade_route_amounts trade_route_traded;

extern trade_route_amounts trade_route_limits;

extern empire_routes_and_traders empire_map_routes;

extern empire_city_records empire_cities;

extern empire_object_records empire_map_objects;

#endif
