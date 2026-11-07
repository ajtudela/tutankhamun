// The built-in data of the empire: trade prices, cultures, food and the trade recommendations.
// empire/empire_data.h declares them.

#include "empire/empire_data.h"

#include <assert.h>

empire_city_culture empire_culture_table[EMPIRE_CITY_NAMES] = { { 11, 38 }, { 11, 38 }, { 14, 40 }, { 9, 38 }, { 11, 38 }, { 11, 38 }, { 9, 38 }, { 15, 42 },
    { 11, 38 }, { 14, 44 }, { 11, 38 }, { 14, 46 }, { 1, 48 }, { 14, 50 }, { 16, 46 }, { 9, 38 }, { 11, 38 }, { 11, 38 }, { 9, 44 }, { 11, 38 }, { 11, 38 },
    { 16, 46 }, { 11, 38 }, { 9, 44 }, { 14, 42 }, { 11, 38 }, { 0, 48 }, { 5, 40 }, { 11, 38 }, { 11, 38 }, { 11, 38 }, { 4, 40 }, { 11, 38 }, { 11, 38 },
    { 11, 38 }, { 11, 38 }, { 2, 46 }, { 12, 42 }, { 11, 38 }, { 11, 38 }, { 11, 38 }, { 11, 38 }, { 3, 38 }, { 14, 44 }, { 11, 46 }, { 9, 42 }, { 11, 52 },
    { 11, 38 }, { 11, 52 }, { 9, 38 }, { 10, 46 }, { 11, 38 }, { 11, 52 }, { 11, 40 }, { 15, 46 }, { 11, 38 }, { 9, 38 }, { 4, 48 }, { 4, 40 }, { 11, 38 },
    { 11, 38 }, { 14, 44 }, { 11, 38 }, { 11, 38 }, { 4, 40 }, { 4, 40 } };

int32_t trade_prices[72] = { 0, 0, 28, 22, 44, 36, 38, 30, 38, 30, 38, 30, 38, 30, 44, 36, 44, 36, 44, 36, 250, 180, 40, 30, 200, 150, 180, 140, 44, 36, 215,
    160, 42, 34, 180, 140, 60, 40, 200, 150, 50, 35, 60, 40, 44, 36, 215, 160, 200, 140, 200, 140, 200, 140, 200, 140, 250, 180, 60, 40, 200, 140, 110, 85, 40,
    30, 155, 116, 250, 190, 72, 54 };

int32_t trade_default_prices[72] = { 0, 0, 28, 21, 47, 35, 33, 25, 33, 25, 33, 25, 33, 25, 42, 33, 44, 34, 21, 16, 325, 275, 38, 29, 150, 120, 140, 105, 48, 37,
    185, 140, 54, 42, 210, 160, 120, 92, 310, 150, 225, 170, 0, 0, 31, 23, 200, 165, 38, 29, 46, 35, 60, 45, 0, 0, 375, 315, 240, 185, 40, 32, 110, 85, 40, 30,
    155, 116, 250, 190, 72, 54 };

const int32_t empire_food_resources[6] = { 1, 13, 15, 17, 23, 19 };

char fmt_two_strings[8] = "%s %s";

const char str_trade_recommends_loaded[36] = " OK: trade_recommends data loaded";

const char str_trade_recommends_out_of_sync[36] = "trade_recommends files out of sync";

char fmt_trade_recommends_bad_lines[68] = "ERR: trade_recommends has incorrect no of lines (%d), expected %d";

const char str_no_trade_recommends_file[36] = "ERR:No trade_recommends.txt file";

const char str_trade_recommends_file[24] = "trade_recommends.txt";

int32_t trade_export_keep[36];

int32_t trade_recommends[24];

static_assert(sizeof(trade_route_amounts) == 2880, "trade_route_traded is the savegame's traded amounts of 2880 bytes");
trade_route_amounts trade_route_traded;

static_assert(sizeof(trade_route_amounts) == 2880, "trade_route_limits is the savegame's trade limits of 2880 bytes");
trade_route_amounts trade_route_limits;

static_assert(sizeof(empire_routes_and_traders) == 25008, "empire_map_routes is the savegame's routes and traders of 25008 bytes");
empire_routes_and_traders empire_map_routes;

static empire_route_point *empire_route_paths[EMPIRE_ROUTES];

empire_route_point *empire_route_path(const empire_route *route)
{
    if (route->path_slot <= 0 || route->path_slot > EMPIRE_ROUTES) {
        return NULL;
    }
    return empire_route_paths[route->path_slot - 1];
}

void empire_route_path_set(empire_route *route, empire_route_point *path)
{
    if (!path) {
        route->path_slot = 0;
        return;
    }
    int32_t index = (int32_t) (route - empire_map_routes.routes);
    assert(index >= 0 && index < EMPIRE_ROUTES);
    empire_route_paths[index] = path;
    route->path_slot = index + 1;
}

static_assert(sizeof(empire_city_records) == 6466, "empire_cities is the savegame's empire cities of 6466 bytes");
empire_city_records empire_cities;

static_assert(sizeof(empire_object_records) == 19600, "empire_map_objects is the savegame and scenario empire objects of 19600 bytes");
empire_object_records empire_map_objects;
