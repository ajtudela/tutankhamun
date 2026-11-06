// The caravans, donkeys, dockers and trade ships of figure/trade.c, the storage yard searches and the buying and
// selling, with their limits.

#ifndef FIGURE_TRADE_H
#define FIGURE_TRADE_H

#include "city/resource_type.h"
#include "empire/empire_types.h"
#include <stdint.h>

enum trade_limit {
    LOAD_AMOUNT = 100,
    CARAVAN_MAX_LOADS = 8,
    SHIP_MAX_LOADS = 12,
    DOCKER_LOADS = 4,
    STORAGE_ROOM_CAPACITY = 400,
    STORAGE_YARD_CAPACITY = 3200,
    STORAGE_ROAD_RADIUS = 3,
    EXPORT_BONUS_PERCENT = 150,
    DOCK_DOCKERS = 3,
};

enum trade_storage_score {
    STORAGE_SCORE_BASE = 32,
    CARAVAN_SCORE_ROOM_BUYS = 4,
    CARAVAN_SCORE_ROOM_SELLS = 8,
    CARAVAN_SCORE_EMPTY_ROOM = 16,
    DOCKER_SCORE_ROOM = 4,
    DOCKER_SCORE_EMPTY_ROOM = 8,
};

enum trade_figure_limit {
    TRADE_FIGURE_FRAMES = 12,
    CARAVAN_ENTRY_WAIT_TICKS = 20,
    CARAVAN_LOST_TICKS = 100,
    CARAVAN_TRADE_TICKS = 10,
    CARAVAN_TRADE_KINDS = 2,
    FERRY_QUEUE_CHECK_TICKS = 100,
    DOCK_SHIP_WAIT_DAYS = 120,
    DOCKER_SHIP_TICKS = 80,
    DOCKER_EXPORT_QUEUE_TICKS = 20,
    DOCKER_STORAGE_TICKS = 10,
    TRADE_SHIP_WAIT_TICKS = 20,
    TRADE_SHIP_ANCHORED_TICKS = 40,
    TRADE_SHIP_DONE_CALLS = 10,
};

void trader_create(int32_t figure_id);
void trader_record_bought_resource(int32_t figure_id, int32_t resource);
void trader_record_sold_resource(int32_t figure_id, int32_t resource);
int32_t caravan_find_storage(int32_t figure_id, int32_t x, int32_t y, int32_t city_id, int32_t distance_penalty);
int32_t figure_trade_caravan_can_buy(int32_t figure_id, int32_t player, int32_t building_id, int32_t city_id);
int32_t caravan_of_donkey(int32_t figure_id);
int32_t trader_get_buy_resource(int32_t player, int32_t building_id, int32_t city_id);
int32_t trader_get_sell_resource(int32_t player, int32_t building_id, int32_t city_id);
int32_t figure_trade_caravan_can_sell(int32_t figure_id, int32_t player, int32_t building_id, int32_t city_id);
int32_t storageyard_import_loads(int32_t building_id, int32_t resource, int32_t amount);
int32_t figure_docker_get_closest_warehouse_for_import(int32_t x, int32_t y, int32_t city_id, int32_t distance_penalty, int32_t road_network);
int32_t figure_docker_get_closest_warehouse_for_export(int32_t x, int32_t y, int32_t city_id, int32_t distance_penalty, int32_t road_network);
int32_t figure_docker_try_import_resource(int32_t building_id, int32_t resource, int32_t city_id, int32_t figure_id);
int32_t figure_docker_try_export_resource(int32_t building_id, int32_t resource, int32_t city_id, int32_t figure_id);
int32_t docker_deliver_import_resource(int32_t dock_id);
int32_t docker_fetch_export_resource(int32_t dock_id);
int32_t trade_ship_done_trading(int32_t figure_id);
int32_t trade_ship_lost_dock(int32_t figure_id);
void figure_trade_caravan_action(void);
void figure_trade_caravan_donkey_action(void);
void figure_docker_action(void);
void figure_trade_ship_action(void);

#endif
