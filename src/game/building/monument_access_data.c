// The built-in access offsets of the monuments: per part of a royal tomb and per part kind of the sun temple,
// the tile offset where its workers enter. config/monument_access_tiles.json can override them.

#include "building/monument_access.h"

static monument_access_offset small_royal_tomb_offsets[] = {
    { 2, 1 },
    { 3, 2 },
    { 3, 1 },
    { 2, -1 },
    { 2, -1 },
    { 1, -1 },
    { 1, -1 },
    { -1, 0 },
    { -1, 0 },
};

static monument_access_offset medium_royal_tomb_offsets[] = {
    { 2, 1 },
    { 3, 2 },
    { 3, 0 },
    { 1, 0 },
    { 3, 2 },
    { 1, 3 },
    { 0, 1 },
    { 3, 5 },
    { -1, 0 },
    { -1, 1 },
};

static monument_access_offset large_royal_tomb_offsets[] = {
    { 1, -1 },
    { 2, -1 },
    { 1, -1 },
    { 0, -1 },
    { 1, -1 },
    { 1, 0 },
    { 3, 1 },
    { 2, -1 },
    { 1, -1 },
    { 2, -1 },
    { 3, -1 },
    { 1, 0 },
    { 2, 0 },
    { -1, 0 },
    { -1, 0 },
    { 1, 0 },
    { 2, 1 },
    { -1, 0 },
    { -1, 1 },
    { 0, -1 },
    { 2, -1 },
};

static monument_access_offset grand_royal_tomb_offsets[] = {
    { 2, 1 },
    { 3, 2 },
    { 2, 1 },
    { 1, 0 },
    { 2, 1 },
    { 1, 0 },
    { 3, 1 },
    { 0, 1 },
    { 1, 3 },
    { 0, 2 },
    { 3, 2 },
    { 3, 2 },
    { 5, 4 },
    { 0, 1 },
    { 3, 6 },
    { 1, 0 },
    { 2, 1 },
    { -1, 0 },
    { -1, 0 },
    { -1, 0 },
    { -1, 1 },
    { 1, 0 },
    { 2, 0 },
    { 0, 1 },
    { 1, 3 },
    { 0, 1 },
    { 1, 3 },
    { -1, 0 },
    { -1, 1 },
    { 0, 1 },
    { 1, 3 },
};

static monument_access_offset sun_temple_offsets[] = {
    { 2, 5 },
    { 1, 2 },
    { 1, 1 },
    { -1, 1 },
    { -1, -1 },
    { 1, -1 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 1, 1 },
};

#define ACCESS_TABLE(offsets) { offsets, (int32_t) (sizeof(offsets) / sizeof(offsets[0])) }

const monument_access_table monument_access_tables[MONUMENT_ACCESS_KIND_COUNT] = {
    [MONUMENT_ACCESS_SMALL_ROYAL_TOMB] = ACCESS_TABLE(small_royal_tomb_offsets),
    [MONUMENT_ACCESS_MEDIUM_ROYAL_TOMB] = ACCESS_TABLE(medium_royal_tomb_offsets),
    [MONUMENT_ACCESS_LARGE_ROYAL_TOMB] = ACCESS_TABLE(large_royal_tomb_offsets),
    [MONUMENT_ACCESS_GRAND_ROYAL_TOMB] = ACCESS_TABLE(grand_royal_tomb_offsets),
    [MONUMENT_ACCESS_SUN_TEMPLE] = ACCESS_TABLE(sun_temple_offsets),
};
