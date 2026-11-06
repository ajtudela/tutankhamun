// The event record methods of scenario/event_helpers.c, the phrase slots an event passes to its city messages and the
// size of the reason phrase table.

#ifndef SCENARIO_EVENT_HELPERS_H
#define SCENARIO_EVENT_HELPERS_H

#include "scenario/scenario_event_types.h"

#include <stdbool.h>
#include <stdint.h>

enum event_phrase_slot {
    EVENT_PHRASE_FIRST = 0,
    EVENT_PHRASE_SECOND = 1,
    EVENT_PHRASE_NONE = 2,
    EVENT_PHRASE_CITY = 3,
    EVENT_PHRASE_AMOUNT = 4,
    EVENT_PHRASE_ITEM = 5,
    EVENT_PHRASE_MONTHS_TO_COMPLY = 6,
    EVENT_PHRASE_MONTHS_LEFT = 7,
    EVENT_PHRASE_TEXT = 8,
    EVENT_PHRASE_REASON_CITY = 9,
    EVENT_PHRASE_DELIVERED_AMOUNT = 10,
    EVENT_PHRASE_BUTTONS = 11,
    EVENT_PHRASE_REASON_AMOUNT = 12,
    EVENT_PHRASE_REASON_ITEM = 13,
    EVENT_PHRASE_VARIANTS = 14,
    EVENT_PHRASES = 16,
};

enum event_months {
    EVENT_TRAVEL_MONTHS_DEFAULT = 12,
    EVENT_MONTHS_MARGIN_BASE = 6,
};

enum scenario_reason_table {
    SCENARIO_REASON_GROUPS = 54,
    SCENARIO_REASON_COLUMNS = 36,
};

bool scenario_event_is_due(scenario_event *e, bool any);
bool scenario_event_is_in_future(scenario_event *e);
bool scenario_event_is_running(scenario_event *e);
int32_t scenario_event_months_left(scenario_event *e);
bool scenario_request_is_waiting(scenario_event *e);
bool scenario_event_parent_by_favour(scenario_event *e);
void scenario_event_fill_phrases(scenario_event *e, int16_t *phrases);
void scenario_event_init_phrases(scenario_event *e, int16_t *phrases);
int32_t scenario_event_message_text(scenario_event *e);
int32_t scenario_event_message_text_completed(scenario_event *e);
int32_t scenario_event_reason_group_completed(scenario_event *e);
int32_t scenario_event_reason_column(scenario_event *e);
int32_t scenario_event_reason_group(scenario_event *e, int32_t action);
int32_t scenario_event_travel_months(scenario_event *e);
int32_t scenario_event_months_margin(scenario_event *e);
int32_t scenario_reason_phrase_variant(int32_t text, int32_t variant);
int32_t scenario_event_reason_phrase(scenario_event *e, int32_t action);

#endif
