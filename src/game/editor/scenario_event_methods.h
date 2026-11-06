// The event record and event list methods of scenario_event_methods.c and editor_event_list.c, and their reads of
// the event type properties (config/event_types.json).

#ifndef EDITOR_SCENARIO_EVENT_METHODS_H
#define EDITOR_SCENARIO_EVENT_METHODS_H

#include "scenario/scenario_event_types.h"
#include "scenario/scenario_rules.h"
#include <stdbool.h>
#include <stdint.h>

enum event_subtype_kind {
    REQUEST_KINDS = 7,
    FESTIVAL_GODS = 5,
};

static inline uint8_t event_type_table_shows_resource(int32_t type)
{
    return (uint8_t) event_type_properties[type].shows_items;
}

static inline int32_t event_type_table_resource_group(int32_t type)
{
    return event_type_properties[type].item_text_group;
}

static inline uint8_t event_type_table_has_amount(int32_t type)
{
    return (uint8_t) event_type_properties[type].has_amount;
}

bool event_shows_resource(scenario_event *e);
int32_t event_resource_group(scenario_event *e);
bool event_has_months(scenario_event *e);
bool event_has_amount(scenario_event *e);
bool event_is_invasion_kind(scenario_event *e);
bool event_has_subtype(scenario_event *e);
bool event_has_phrase_number(scenario_event *e);
bool event_has_phrase_list(scenario_event *e);
int32_t event_subtype_count(scenario_event *e);
int32_t event_subtype_group(scenario_event *e);
int32_t event_resource_list(scenario_event *e);
void event_set_type(scenario_event *e, int32_t type);
bool troops_request_state(scenario_event *e, int32_t *city, int32_t *percent, uint8_t *by_land, int32_t *zero);
void scenario_value_set(scenario_value *v, int32_t fixed, int32_t min, int32_t max);
const char *event_description(scenario_event *e);
bool events_delete(scenario_events_t *self, int32_t index);

#endif
