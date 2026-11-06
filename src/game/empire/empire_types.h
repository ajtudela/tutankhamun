// The records of the empire as the save and the empire file hold them: the trade cities, the map objects (ornaments,
// cities with their trade lists and amounts, texts, battle icons), routes, traded amounts and limits, the traders on
// their way, the empire map headers and the empire window's event markers. The old 0x4C-byte map object layout and
// the city type numbers of old saves are here too.

#ifndef EMPIRE_EMPIRE_TYPES_H
#define EMPIRE_EMPIRE_TYPES_H

#include "city/resource_type.h"
#include "empire/empire_route.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum {
    EMPIRE_CITY_COUNT = 61,
    EMPIRE_CITY_TRADER_FIGURES = 8,
    EMPIRE_OBJECT_COUNT = 200,
    EMPIRE_OBJECT_SOLD_RESOURCES = 14,
    EMPIRE_OBJECT_BOUGHT_RESOURCES = 8,
    EMPIRE_TRADE_ROUTES = 20,
    EMPIRE_CITY_NAMES = 66,
    EMPIRE_EVENT_MARKERS = 92,
    EMPIRE_MAP_HEADERS = 40,
    EMPIRE_TRADERS = 100,
};

enum empire_city_type {
    EMPIRE_CITY_OURS = 0,
    EMPIRE_CITY_PHARAOH_TRADING = 1,
    EMPIRE_CITY_PHARAOH = 2,
    EMPIRE_CITY_EGYPTIAN_TRADING = 3,
    EMPIRE_CITY_EGYPTIAN = 4,
    EMPIRE_CITY_FOREIGN_TRADING = 5,
    EMPIRE_CITY_FOREIGN = 6,
};

enum empire_object_type {
    EMPIRE_OBJECT_ORNAMENT = 0,
    EMPIRE_OBJECT_CITY = 1,
    EMPIRE_OBJECT_TEXT = 2,
    EMPIRE_OBJECT_BATTLE_ICON = 3,
};

enum empire_city_type_before_object_states {
    EMPIRE_CITY_BEFORE_OBJECT_STATES_EGYPTIAN_0 = 0,
    EMPIRE_CITY_BEFORE_OBJECT_STATES_OURS = 1,
    EMPIRE_CITY_BEFORE_OBJECT_STATES_PHARAOH_TRADING = 2,
    EMPIRE_CITY_BEFORE_OBJECT_STATES_EGYPTIAN_3 = 3,
    EMPIRE_CITY_BEFORE_OBJECT_STATES_FOREIGN_4 = 4,
    EMPIRE_CITY_BEFORE_OBJECT_STATES_EGYPTIAN_5 = 5,
    EMPIRE_CITY_BEFORE_OBJECT_STATES_FOREIGN_6 = 6,
};

enum trade_amount {
    TRADE_AMOUNT_NONE = 0,
    TRADE_AMOUNT_LOW = 1,
    TRADE_AMOUNT_MEDIUM = 2,
    TRADE_AMOUNT_HIGH = 3,
};

enum empire_route_kind {
    EMPIRE_ROUTE_KIND_LAND = 1,
    EMPIRE_ROUTE_KIND_SEA = 2,
};

typedef struct {
    uint8_t in_use;
    uint8_t reserved_1;
    uint8_t type;
    uint8_t name_id;
    uint8_t route_id;
    uint8_t is_open;
    uint8_t buys[RESOURCE_MAX];
    uint8_t sells[RESOURCE_MAX];
    int16_t cost_to_open;
    int16_t unread_default;
    int16_t trader_entry_delay;
    int16_t trader_blocked;
    int16_t map_object;
    uint8_t is_sea_trade;
    uint8_t reserved_2;
    int16_t trader_figures[EMPIRE_CITY_TRADER_FIGURES];
} empire_city;

static_assert(sizeof(empire_city) == 0x6A, "empire_city is a savegame record of 106 bytes");
static_assert(offsetof(empire_city, sells) == 0x2A && offsetof(empire_city, cost_to_open) == 0x4E && offsetof(empire_city, trader_entry_delay) == 0x52 &&
                  offsetof(empire_city, map_object) == 0x56 && offsetof(empire_city, trader_figures) == 0x5A, "empire_city is the savegame's layout");

typedef struct {
    uint8_t type;
    uint8_t in_use;
    uint8_t animation_frame;
    uint8_t reserved_1;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    uint16_t image_id;
    int16_t image_offset;
    uint8_t reserved_2[3];
    uint8_t text_align;
    int16_t expanded_x;
    int16_t expanded_y;
    uint8_t city_type;
    uint8_t name_id;
    uint8_t route_id;
    uint8_t is_open;
    int16_t cost_to_open;
    uint8_t sells[EMPIRE_OBJECT_SOLD_RESOURCES];
    uint8_t reserved_3[7];
    uint8_t city_type_countdown;
    uint8_t buys[EMPIRE_OBJECT_BOUGHT_RESOURCES];
    uint8_t invasion_path_id;
    uint8_t invasion_years;
    uint8_t trade_amount[RESOURCE_MAX];
} empire_object;

static_assert(sizeof(empire_object) == 0x62, "empire_object is a savegame and scenario record of 98 bytes");
static_assert(offsetof(empire_object, image_id) == 0x0C && offsetof(empire_object, text_align) == 0x13 && offsetof(empire_object, city_type) == 0x18 &&
                  offsetof(empire_object, cost_to_open) == 0x1C && offsetof(empire_object, sells) == 0x1E &&
                  offsetof(empire_object, city_type_countdown) == 0x33 && offsetof(empire_object, buys) == 0x34 &&
                  offsetof(empire_object, invasion_path_id) == 0x3C && offsetof(empire_object, trade_amount) == 0x3E,
              "empire_object is the savegame and scenario layout");

enum {
    EMPIRE_OBJECT_BEFORE_OBJECT_FLAGS_MASKED_RESOURCES = 32,
};

typedef struct {
    uint8_t object_fields[62];
    uint8_t reserved[2];
    uint32_t resources_traded_high;
    uint32_t resources_traded_medium;
    uint32_t resources_traded_low;
} empire_object_before_object_flags;

static_assert(sizeof(empire_object_before_object_flags) == 0x4C, "empire_object_before_object_flags is an old savegame's record of 76 bytes");

static inline int32_t empire_city_type_is_trading(int32_t type)
{
    return type == EMPIRE_CITY_PHARAOH_TRADING || type == EMPIRE_CITY_EGYPTIAN_TRADING || type == EMPIRE_CITY_FOREIGN_TRADING;
}

typedef struct empire_trader {
    int32_t bought_amount;
    int32_t sold_amount;
    uint8_t bought_resources[RESOURCE_MAX];
    uint8_t sold_resources[RESOURCE_MAX];
    int32_t bought_value;
    int32_t sold_value;
} empire_trader;

static_assert(sizeof(empire_trader) == 0x58, "empire_trader is a savegame record of 88 bytes");
static_assert(offsetof(empire_trader, sold_resources) == 0x2C && offsetof(empire_trader, bought_value) == 0x50, "empire_trader is the savegame's layout");

typedef struct {
    empire_route routes[EMPIRE_ROUTES];
    int32_t unread_after_routes;
    int32_t trader_next_index;
    empire_trader traders[EMPIRE_TRADERS];
} empire_routes_and_traders;

static_assert(sizeof(empire_routes_and_traders) == 25008, "empire_routes_and_traders is a savegame record of 25008 bytes");
static_assert(offsetof(empire_routes_and_traders, trader_next_index) == 0x3F4C && offsetof(empire_routes_and_traders, traders) == 0x3F50,
              "empire_routes_and_traders is the savegame's layout");

typedef struct {
    int32_t image_offset;
    int32_t luxury_kind;
} empire_city_culture;

typedef struct {
    int32_t event_id;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} empire_event_marker;

typedef struct {
    int32_t watched_figure_id;
    empire_event_marker markers[EMPIRE_EVENT_MARKERS];
} empire_event_marker_state;

typedef struct {
    int16_t scroll_x;
    int16_t scroll_y;
    int16_t editor_objects_started;
    uint8_t reserved[26];
} empire_map_header;

static_assert(sizeof(empire_map_header) == 0x20, "empire_map_header is a savegame and scenario record of 32 bytes");

typedef empire_map_header empire_map_headers[EMPIRE_MAP_HEADERS];
typedef empire_city empire_city_records[EMPIRE_CITY_COUNT];
typedef empire_object empire_object_records[EMPIRE_OBJECT_COUNT];
typedef int32_t trade_route_amounts[EMPIRE_TRADE_ROUTES][RESOURCE_MAX];

#endif
