// Building helpers for the figure generation and the figures (building/building_figure_helpers.c): main part,
// temple gods and upgrades, figures of a building, the labor seeker, water supply and bazaar images, festival
// celebrants, bazaar buyer destinations and granary and storage yard stock.

#ifndef BUILDING_BUILDING_FIGURE_HELPERS_H
#define BUILDING_BUILDING_FIGURE_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

enum priest_names {
    PRIEST_NAME_COUNT = 16,
    PRIEST_NAME_GROUP_FIRST = 266,
};

enum storage_yard_layout {
    STORAGE_YARD_ROOMS = 8,
    STORAGE_YARD_SIZE = 3,
    STORAGE_ROOM_IMAGES_PER_RESOURCE = 4,
};

int32_t building_count_figures_of(int32_t building_id, int32_t figure_type);
int32_t building_main(int32_t id);
int32_t building_temple_god(int32_t building_id);
int32_t building_market_get_max_food_stock(int32_t id);
int32_t building_market_get_max_goods_stock(int32_t id);
bool building_temple_complex_has_upgrade(int32_t building_id, int32_t upgrade);
void generate_labor_seeker(int32_t building_id);
void water_supply_update_image(int32_t building_id);
void building_festival_spawn_celebrant(int32_t building_id);
void building_festival_spawn_celebrants(int32_t size);
int32_t building_festival_celebrant_role(int32_t type);
int32_t building_festival_celebrant_name_group(int32_t role, int32_t *name_count);
int32_t building_market_get_storage_destination(int32_t building_id, int32_t second, int32_t *items, uint8_t *routed);
int32_t market_food_destination(int32_t building_id, const int32_t *counts, const int32_t *best);
int32_t market_goods_destination(int32_t building_id, const int32_t *counts, const int32_t *best);
bool building_bazaar_update_image(int32_t building_id);
bool market_list_contains(int32_t item, const int32_t *list);
void building_bazaar_update_supply(int32_t building_id, int32_t radius);
void market_food_items(int32_t building_id, const int32_t *counts, int32_t *items);
void market_goods_items(int32_t building_id, const int32_t *counts, int32_t *items);
int32_t building_granary_remove_resource(int32_t player, int32_t building_id, int32_t resource, int32_t amount);
int32_t building_granary_resource_to_send(int32_t building_id, int32_t wanted);
int32_t building_storageyard_for_getting(int32_t id, int32_t resource, int32_t full_only_flag);
int32_t building_storageyard_get_amount(int32_t id, int32_t resource);
int32_t building_storageyard_remove_resource(int32_t player, int32_t id, int32_t resource, int32_t amount);
int32_t building_storageyard_resource_to_send(int32_t id, int32_t wanted);
int32_t calc_distance_with_penalty(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t distance1, int32_t distance2);
int32_t storage_image_level(int32_t amount);
int32_t storage_room_image(int32_t resource, int32_t loads);
int32_t storage_count_wanted(int32_t resource1, int32_t resource2);
void building_storage_wanted_resources(
    int32_t building_id, int32_t *resource1, int32_t *resource2, int32_t *percent1, int32_t *percent2, int32_t *destination1, int32_t *destination2);
void storage_choose_wanted(int32_t percent1, int32_t percent2, int32_t count, uint8_t *second_cart, uint8_t *first, int32_t *ok);
void building_market_update_buyer_storages(int32_t figure_id, int32_t building_id);

#endif
