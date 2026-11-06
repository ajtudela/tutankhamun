// Access tiles of the royal tombs and the sun temple (building/monument_access.c) and their offset tables.

#ifndef BUILDING_MONUMENT_ACCESS_H
#define BUILDING_MONUMENT_ACCESS_H

#include "building/monument_object_types.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int8_t dx;
    int8_t dy;
} monument_access_offset;

typedef struct {
    monument_access_offset *offsets;
    int32_t count;
} monument_access_table;

enum monument_access_kind {
    MONUMENT_ACCESS_SMALL_ROYAL_TOMB,
    MONUMENT_ACCESS_MEDIUM_ROYAL_TOMB,
    MONUMENT_ACCESS_LARGE_ROYAL_TOMB,
    MONUMENT_ACCESS_GRAND_ROYAL_TOMB,
    MONUMENT_ACCESS_SUN_TEMPLE,
    MONUMENT_ACCESS_KIND_COUNT,
};

extern const monument_access_table monument_access_tables[MONUMENT_ACCESS_KIND_COUNT];

bool small_royal_tomb_get_access_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool medium_royal_tomb_get_entry_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool large_royal_tomb_get_entry_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool grand_royal_tomb_get_entry_tile(tomb_object *self, int32_t building_id, int32_t *x, int32_t *y);
bool sun_temple_get_access_tile(int32_t building_id, int32_t *x, int32_t *y);

#endif
