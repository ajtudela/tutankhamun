// The saved vegetation state (vegetation_growth_state): the daily regrowth values of trees and marshland and the
// flags of the reach pass.

#ifndef MAP_VEGETATION_TYPES_H
#define MAP_VEGETATION_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

typedef struct vegetation_growth_record {
    int32_t marsh_growth_b;
    int32_t tree_growth_a;
    int32_t marsh_growth_a;
    int32_t tree_growth_b;
    uint8_t trees_reachable;
    uint8_t trees_present;
    uint8_t reeds_reachable;
    uint8_t marsh_present;
} vegetation_growth_record;

static_assert(sizeof(vegetation_growth_record) == 0x14, "vegetation_growth_record is a savegame record of 20 bytes");
static_assert(offsetof(vegetation_growth_record, trees_reachable) == 0x10 && offsetof(vegetation_growth_record, marsh_present) == 0x13,
              "vegetation_growth_record is the savegame's layout");

#endif
