// The storage record of a granary, storage yard or bazaar: per resource its state and its accept and get limits.

#ifndef BUILDING_BUILDING_STORAGE_TYPES_H
#define BUILDING_BUILDING_STORAGE_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#define STORAGE_RECORDS 200
#define STORAGE_RESOURCES 36

typedef struct {
    int32_t reserved_1;
    int16_t building_id;
    int16_t reserved_2;
    uint8_t in_use;
    uint8_t empty_all;
    uint8_t resource_state[STORAGE_RESOURCES];
    uint8_t reserved_3[6];
    int16_t accept_amount[STORAGE_RESOURCES];
    int16_t get_amount[STORAGE_RESOURCES];
} building_storage;

static_assert(sizeof(building_storage) == 0xC4, "building_storage is a savegame record of 196 bytes");
static_assert(offsetof(building_storage, in_use) == 0x08 && offsetof(building_storage, accept_amount) == 0x34, "building_storage is the savegame's layout");

#endif
