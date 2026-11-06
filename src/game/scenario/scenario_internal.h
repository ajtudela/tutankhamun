// Shared by the scenario event modules: the event flags, the request states and subtypes, the city status change and
// message subtypes, the gift states and buttons, and the headers of the event manager, record methods and processors.

#ifndef SCENARIO_SCENARIO_INTERNAL_H
#define SCENARIO_SCENARIO_INTERNAL_H

#include "city/city_internal.h"
#include "city/resource_type.h"
#include "core/random.h"
#include "scenario/disaster_events.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "scenario/event_reasons.h"
#include "scenario/scenario_event_types.h"
#include "scenario/scenario_events.h"

#include <stddef.h>
#include <stdint.h>

enum scenario_event_flag {
    EVENT_FLAG_CHAIN_ONLY = 1,
    EVENT_FLAG_RECURRING = 2,
    EVENT_FLAG_FIRED = 4,
    EVENT_FLAG_CLONE = 8,
    EVENT_FLAG_BY_FAVOUR = 0x10,
};

enum scenario_request_state {
    REQUEST_STATE_NORMAL = 0,
    REQUEST_STATE_OVERDUE = 1,
    REQUEST_STATE_DISPATCHED = 2,
    REQUEST_STATE_DISPATCHED_LATE = 3,
    REQUEST_STATE_IGNORED = 4,
    REQUEST_STATE_RECEIVED = 5,
    REQUEST_STATE_TROOPS_RETURN = 6,
    REQUEST_STATE_TROOPS_RETURN_TOO_LATE = 7,
    REQUEST_STATE_FINISHED = 8,
};

enum request_subtype {
    REQUEST_SUBTYPE_GENERIC = 0,
    REQUEST_SUBTYPE_TROOPS = 1,
    REQUEST_SUBTYPE_DISTANT_BATTLE = 2,
    REQUEST_SUBTYPE_FESTIVAL = 3,
    REQUEST_SUBTYPE_CONSTRUCTION = 4,
    REQUEST_SUBTYPE_FAMINE = 5,
    REQUEST_SUBTYPE_CITY_IN_TROUBLE = 6,
};

enum city_status_subtype {
    CITY_STATUS_FELL_TO_ENEMY = 0,
    CITY_STATUS_CONQUERED = 1,
    CITY_STATUS_NEW_TRADE_ROUTE = 2,
    CITY_STATUS_LOST_TRADE_ROUTE = 3,
    CITY_STATUS_UNDER_SIEGE = 4,
};

enum event_message_subtype {
    EVENT_MESSAGE_CITY_SAVED = 0,
    EVENT_MESSAGE_BATTLE_WON = 1,
    EVENT_MESSAGE_BATTLE_LOST = 2,
    EVENT_MESSAGE_ACKNOWLEDGEMENT = 3,
};

enum gift_state {
    GIFT_STATE_NEW = 0,
    GIFT_STATE_OFFERED = 1,
    GIFT_STATE_OFFER_AGAIN = 2,
    GIFT_STATE_OFFERED_AGAIN = 3,
    GIFT_STATE_OFFER_LAST = 4,
    GIFT_STATE_OFFERED_LAST = 5,
    GIFT_STATE_ACCEPTED = 6,
    GIFT_STATE_REFUSED = 7,
    GIFT_STATE_TOO_LATE = 8,
    GIFT_STATE_DONE = 9,
};

enum gift_button_flag {
    GIFT_BUTTON_REFUSE = 1,
    GIFT_BUTTON_LATER = 2,
    GIFT_BUTTON_ACCEPT = 4,
};

#endif
