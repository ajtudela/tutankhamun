// Storage records of the granaries, storage yards and bazaars (per resource its state, accept and get limits)
// and the cart destinations and goods transfers of building/building_storage.c.

#ifndef BUILDING_BUILDING_STORAGE_H
#define BUILDING_BUILDING_STORAGE_H

#include "building/building_data.h"
#include "building/building_storage_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum storage_resource_state {
    STORAGE_STATE_ACCEPT = 0,
    STORAGE_STATE_REFUSE = 1,
    STORAGE_STATE_GET = 2,
    STORAGE_STATE_EMPTY = 3,
};

enum storage_room_capacity {
    STORAGE_CART_LOAD = 100,
    STORAGE_ROOM_LOADS = 400,
    STORAGE_AMOUNT_ALL = 3200,
    GRANARY_CAPACITY = 3200,
};

static inline building_storage *storage_get(int32_t storage_id)
{
    return &building_storages[storage_id];
}

int32_t building_granary_for_storing(int32_t source, int32_t figure_id, int32_t resource, int32_t road_network);
int32_t building_storageyard_for_storing(int32_t source, int32_t figure_id, int32_t resource, int32_t road_network);
int32_t building_granary_getting_food(int32_t figure_id, int32_t resource, int32_t road_network);
int32_t building_granary_supplier(int32_t id);
int32_t building_workshop_for_raw_material(int32_t figure_id, int32_t resource, int32_t road_network);
int32_t building_workshop_for_raw_material_any(int32_t figure_id, int32_t resource, int32_t road_network);
int32_t building_recruiter_for_weapons(int32_t source, int32_t figure_id, int32_t resource, int32_t road_network);
int32_t building_recruiter_add_input(int32_t id, int32_t amount, int32_t resource);
int32_t building_storageyards_remove_resource(int32_t player, int32_t resource, int32_t amount);
int32_t building_storage_create(int32_t building_id);
void building_storage_clear_all(void);
void building_storage_reset_building_ids(void);
int32_t building_storage_store_anywhere(int32_t resource, int32_t remaining, int32_t count_only_flag);
bool building_storageyard_is_full(int32_t building_id, int32_t amount, int32_t resource);
int32_t building_storageyard_add_resource(int32_t player, int32_t room_id, int32_t resource, int32_t amount);
int32_t building_granary_add_resource(int32_t id, int32_t resource, int32_t count, int32_t requested_amount);
int32_t building_storage_room_for(int32_t resource);
int32_t building_workshop_add_input(int32_t building_id, int32_t amount, int32_t resource);

#endif
