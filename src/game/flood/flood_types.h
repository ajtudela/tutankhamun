// The records of the flood object (floods_data of map/map_data.h): the per-tile flood records, their lists by depth,
// the floodplain settings and the flood state; apart from flood_internal.h so that map/map_data.h can use the type.

#ifndef FLOOD_FLOOD_TYPES_H
#define FLOOD_FLOOD_TYPES_H

#include "core/sorted_list.h"
#include "map/grid_size.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define FLOOD_LIST_COUNT 30

typedef struct flood_tile flood_tile;
typedef struct flood_list flood_list;
typedef struct floods floods;

struct flood_tile {
    flood_list *list;
    sorted_link *link;
    int32_t grid_offset;
    int32_t x;
    int32_t y;
    int32_t depth;
    uint8_t is_floodplain;
    uint8_t padding[3];
};

struct flood_list {
    int32_t count;
    int32_t recede_passes;
    int32_t depth;
    floods *owner;
    sorted_list links;
    flood_tile *cursor;
};

typedef struct {
    int32_t season_initial;
    int32_t duration_initial;
    int32_t quality_initial;
    int32_t season;
    int32_t duration;
    int32_t quality;
    int32_t reserved_1;
    int32_t quality_next;
    int32_t quality_last;
} floodplain_settings;

enum {
    FLOOD_STATE_IMMINENT = 1,
    FLOOD_STATE_FLOODING = 2,
    FLOOD_STATE_PEAK = 3,
    FLOOD_STATE_RECEDING = 4,
    FLOOD_STATE_RESTING = 5,
    FLOOD_STATE_IDLE = 6,
};

struct floods {
    flood_tile *tiles[GRID_CELLS];
    floodplain_settings settings;
    flood_list lists[FLOOD_LIST_COUNT];
    int32_t progress;
    int32_t season_month;
    int32_t state;
    int32_t width;
    uint8_t has_floodplain;
    uint8_t padding[3];
};

static_assert(sizeof(floods) == 0x32FC0, "floods is 208832 bytes, as in the original program");
static_assert(sizeof(flood_list) == 0x1C, "flood_list is 28 bytes, as in the original program");
static_assert(sizeof(flood_tile) == 0x1C, "flood_tile is 28 bytes, as in the original program");
static_assert(sizeof(floodplain_settings) == 0x24, "floodplain_settings is a savegame record of 36 bytes");

static inline bool floods_is_running(const floods *self)
{
    int32_t state = self->state;
    return state == FLOOD_STATE_IMMINENT || state == FLOOD_STATE_FLOODING || state == FLOOD_STATE_PEAK;
}

#endif
