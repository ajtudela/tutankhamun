// The records of the scenario's events: the event types, invaders and subtypes, a randomised event value (fixed, or
// drawn from a range), the 124-byte event record (dates, item, route, follow-up events, parent, phrases, attack
// target) and the event manager object with the editor's state. The list grows up to SCENARIO_EVENTS_LIMIT records;
// the record after them is the cheats' invasion.

#ifndef SCENARIO_SCENARIO_EVENT_TYPES_H
#define SCENARIO_SCENARIO_EVENT_TYPES_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

enum event_type {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_REQUEST = 1,
    EVENT_TYPE_INVASION = 2,
    EVENT_TYPE_EARTHQUAKE = 3,
    EVENT_TYPE_REVOLT = 4,
    EVENT_TYPE_CHANGE_PHARAOH = 5,
    EVENT_TYPE_SEA_TRADE_PROBLEM = 6,
    EVENT_TYPE_LAND_TRADE_PROBLEM = 7,
    EVENT_TYPE_WAGE_INCREASE = 8,
    EVENT_TYPE_WAGE_DECREASE = 9,
    EVENT_TYPE_CONTAMINATED_WATER = 10,
    EVENT_TYPE_GOLD_MINE_COLLAPSE = 11,
    EVENT_TYPE_CLAY_PIT_FLOOD = 12,
    EVENT_TYPE_DEMAND_INCREASE = 13,
    EVENT_TYPE_DEMAND_DECREASE = 14,
    EVENT_TYPE_PRICE_INCREASE = 15,
    EVENT_TYPE_PRICE_DECREASE = 16,
    EVENT_TYPE_REPUTATION_INCREASE = 17,
    EVENT_TYPE_REPUTATION_DECREASE = 18,
    EVENT_TYPE_CITY_STATUS_CHANGE = 19,
    EVENT_TYPE_MESSAGE = 20,
    EVENT_TYPE_FLOOD_WORSE = 21,
    EVENT_TYPE_FLOOD_BETTER = 22,
    EVENT_TYPE_GIFT_FROM_PHARAOH = 23,
    EVENT_TYPE_LOCUSTS = 24,
    EVENT_TYPE_FROGS = 25,
    EVENT_TYPE_HAILSTORM = 26,
    EVENT_TYPE_BLOOD_RIVER = 27,
    EVENT_TYPE_CRIME_WAVE = 28,
    EVENT_TYPE_MUMMY = 29,
    EVENT_TYPE_COUNT = 30,
};

enum event_invader {
    EVENT_INVADER_ENEMY = 1,
    EVENT_INVADER_EGYPT = 2,
    EVENT_INVADER_PHARAOH = 3,
    EVENT_INVADER_BEDOUINS = 4,
    EVENT_INVADERS = 4,
};

enum event_subtype_limit {
    EVENT_CITY_STATUS_SUBTYPES = 5,
    EVENT_MESSAGE_SUBTYPES = 4,
};

enum scenario_event_limit {
    SCENARIO_MAX_EVENTS = 150,
    SCENARIO_EVENTS_LIMIT = 140,
    SCENARIO_EVENTS_ADD_ONLY_LIMIT = 120,
    SCENARIO_CHEAT_INVASION_EVENT = 140,
    SCENARIO_EARTHQUAKE_FRONTS = 4,
};

typedef struct {
    int16_t value;
    int16_t fixed;
    int16_t min;
    int16_t max;
} scenario_value;

typedef struct {
    int16_t duration;
    int16_t delay;
    int16_t ticks;
    int16_t locations[2 * SCENARIO_EARTHQUAKE_FRONTS];
} scenario_earthquake_data;

typedef struct {
    int16_t first_formation_id;
    int16_t home_x;
    int16_t home_y;
    int16_t layout;
    int16_t destination_x;
    int16_t destination_y;
    int16_t destination_building_id;
    int16_t num_formations;
    int16_t ignore_soldiers;
    uint8_t reserved_1[4];
} scenario_invasion_data;

typedef struct {
    int16_t id;
    uint8_t type;
    uint8_t month;
    scenario_value item;
    scenario_value amount;
    scenario_value year;
    scenario_value route;
    int16_t on_completed;
    int16_t on_refusal;
    uint32_t flags;
    int16_t months_initial;
    int16_t months_left;
    int16_t state;
    uint8_t is_active;
    uint8_t can_comply_shown;
    int16_t parent_id;
    uint8_t phrase_var;
    uint8_t parent_phrase_var;
    int16_t attack_target;
    union {
        scenario_earthquake_data earthquake;
        scenario_invasion_data invasion;
    } data;
    int16_t earthquake_step;
    int16_t on_too_late;
    int16_t on_defeat;
    uint8_t sender_faction;
    uint8_t invasion_warnings_shown;
    scenario_value invasion;
    uint8_t subtype;
    uint8_t months_margin;
    uint8_t coin;
    uint8_t parent_city;
    int16_t parent_phrase;
    int16_t parent_item;
    int16_t parent_amount;
    uint8_t reason[4];
    uint8_t chain_marked;
    uint8_t reserved_1[13];
} scenario_event;

static_assert(sizeof(scenario_event) == 0x7C, "scenario_event is a savegame and scenario record of 124 bytes");
static_assert(offsetof(scenario_event, flags) == 0x28, "scenario_event is the savegame and scenario layout");
static_assert(offsetof(scenario_event, reason) == 0x6A, "scenario_event is the savegame and scenario layout");
static_assert(offsetof(scenario_event, data.earthquake.duration) == 0x3A && offsetof(scenario_event, data.earthquake.ticks) == 0x3E &&
                  offsetof(scenario_event, data.earthquake.locations) == 0x40 && offsetof(scenario_event, data.invasion.layout) == 0x40 &&
                  offsetof(scenario_event, data.invasion.destination_building_id) == 0x46 && offsetof(scenario_event, data.invasion.num_formations) == 0x48 &&
                  offsetof(scenario_event, data.invasion.ignore_soldiers) == 0x4A && offsetof(scenario_event, earthquake_step) == 0x50 &&
                  offsetof(scenario_event, invasion) == 0x58 && offsetof(scenario_event, parent_amount) == 0x68,
              "scenario_event is the savegame and scenario layout");


typedef struct {
    int32_t count;
    scenario_event events[SCENARIO_MAX_EVENTS];
    int32_t editor_selected_id;
    int32_t editor_scroll;
    uint8_t earthquake_active;
    uint8_t reserved_1;
    uint8_t spare_47[14];
    uint8_t invasion_search_flag;
    uint8_t spare_48;
    int16_t editor_value;
    int16_t editor_field;
    uint8_t editor_phrase;
    uint8_t spare_49;
} scenario_events_t;

enum scenario_events_save_size {
    SCENARIO_EVENTS_SAVED_BYTES = 18628,
};

static_assert(sizeof(scenario_events_t) == 0x48CC, "scenario_events_t is a savegame and scenario record of 18636 bytes");
static_assert(offsetof(scenario_events_t, invasion_search_flag) == 0x48C4 && offsetof(scenario_events_t, editor_value) == 0x48C6 &&
                  offsetof(scenario_events_t, editor_phrase) == 0x48CA, "scenario_events_t is the savegame and scenario layout");
static_assert(offsetof(scenario_events_t, events) == 0x04 && offsetof(scenario_events_t, editor_selected_id) == 0x48AC &&
                  offsetof(scenario_events_t, editor_scroll) == 0x48B0 && offsetof(scenario_events_t, earthquake_active) == 0x48B4,
              "scenario_events_t is the savegame and scenario layout");

#endif
