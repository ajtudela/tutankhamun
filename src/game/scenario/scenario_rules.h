// The tables of the scenario events, loaded from config/*.json:
// - request_messages: per request subtype its message texts and reason table rows;
// - earthquake_steps: the front and move of each earthquake step;
// - troops_request_losses: the losses of legions sent for a troops request by their strength's surplus;
// - reason_phrase_variant_texts: the message texts with two alternative phrases;
// - event_types: per event type its city or invasion point, items and amount;
// - event_type_texts: per event type its announcement and completed texts, reason table cell and phrases;
// - invasion_warnings: the months before the attack each warning is given at, its phrase and message type.

#ifndef SCENARIO_SCENARIO_RULES_H
#define SCENARIO_SCENARIO_RULES_H

#include "scenario/scenario_event_types.h"

#include <stdint.h>

enum scenario_rule_sizes {
    REQUEST_SUBTYPES = 7,
    EVENT_SENDERS = 2,
    EARTHQUAKE_STEPS = 16,
    TROOPS_LOSS_BRACKETS = 7,
    REASON_PHRASE_VARIANT_TEXTS = 126,
    REASON_PHRASE_VARIANT_FIRST = 298,
    INVASION_WARNINGS = 5,
};

enum event_sender {
    EVENT_SENDER_CITY = 0,
    EVENT_SENDER_PHARAOH = 1,
};

typedef struct request_message_rule {
    int32_t announcement[EVENT_SENDERS];
    int32_t completed[EVENT_SENDERS];
    int32_t refused[EVENT_SENDERS];
    int32_t too_late[EVENT_SENDERS];
    int32_t defeat[EVENT_SENDERS];
    int32_t completed_group;
    int32_t refused_group;
    int32_t too_late_group;
    int32_t defeat_group;
    int32_t reason_column;
} request_message_rule;

typedef struct earthquake_step {
    int32_t front;
    int32_t dx;
    int32_t dy;
} earthquake_step;

typedef struct troops_loss_bracket {
    int32_t below_percent;
    int32_t losses;
} troops_loss_bracket;

typedef struct event_type_property {
    int32_t uses_city;
    int32_t is_invasion;
    int32_t shows_items;
    int32_t uses_resource;
    int32_t item_min;
    int32_t item_max;
    int32_t item_text_group;
    int32_t has_amount;
} event_type_property;

typedef struct event_type_text {
    int32_t announcement;
    int32_t completed;
    int32_t reason_group;
    int32_t reason_column;
    int32_t first_phrase;
    int32_t second_phrase;
} event_type_text;

typedef struct invasion_warning {
    int32_t months_left;
    int32_t phrase[EVENT_INVADERS];
    int32_t message[EVENT_INVADERS];
} invasion_warning;

extern request_message_rule request_message_rules[REQUEST_SUBTYPES];
extern earthquake_step earthquake_steps[EARTHQUAKE_STEPS];
extern troops_loss_bracket troops_loss_brackets[TROOPS_LOSS_BRACKETS];
extern int32_t reason_phrase_variant_texts[REASON_PHRASE_VARIANT_TEXTS];
extern event_type_property event_type_properties[EVENT_TYPE_COUNT];
extern event_type_text event_type_texts[EVENT_TYPE_COUNT];
extern invasion_warning invasion_warnings[INVASION_WARNINGS];

#endif
