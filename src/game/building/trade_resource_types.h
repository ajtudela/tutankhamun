// The trade goods list (the goods of the editor's trade prices and of the briefing tooltip) followed by the ghost's
// beauty variant, and the size of the overlay menu's submenu ids.
//
// The editor's price reset walks the list and the two words after it as one run: the beauty variant and the 0 after
// it are reset as goods too, as in the original.

#ifndef BUILDING_TRADE_RESOURCE_TYPES_H
#define BUILDING_TRADE_RESOURCE_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum trade_resource_table_counts {
    TRADE_PRICE_LIST_ENTRIES = 34,
    TRADE_PRICE_LIST_WALKED = 36,
    OVERLAY_SUBMENU_IDS = 11,
};

typedef struct trade_price_list {
    int32_t resources[TRADE_PRICE_LIST_ENTRIES];
    int32_t beauty_variant;
    int32_t after_beauty_variant;
} trade_price_list;

typedef union trade_price_walk {
    trade_price_list list;
    int32_t walked[TRADE_PRICE_LIST_WALKED];
} trade_price_walk;

static_assert(sizeof(trade_price_walk) == 144, "trade_price_walk covers the editor's price reset run of 144 bytes");
static_assert(offsetof(trade_price_list, beauty_variant) == 0x88, "trade_price_list is the run the editor's price reset walks");

#endif
