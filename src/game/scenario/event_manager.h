// The functions of scenario/event_manager.c: the scenario event manager and its event list helpers.

#ifndef SCENARIO_EVENT_MANAGER_H
#define SCENARIO_EVENT_MANAGER_H

#include "scenario/scenario_event_types.h"

#include <stdbool.h>
#include <stdint.h>

scenario_event *scenario_event_get(int32_t id);
bool scenario_event_type_uses_city(scenario_event *e);
bool scenario_event_type_is_invasion(scenario_event *e);
int32_t scenario_event_uses_trade_city(scenario_event *e);
bool scenario_event_type_uses_resource(scenario_event *e);
bool scenario_event_is_chain_target(scenario_event *e, int32_t id);
int32_t scenario_value_choice_count(scenario_value *v);
void scenario_value_randomize(scenario_value *v);
void scenario_event_reset(scenario_event *e);
scenario_event *scenario_event_add(scenario_events_t *self, bool add_only);
scenario_event *scenario_request_next_active(scenario_events_t *self, scenario_event *prev);
scenario_event *scenario_invasion_next(scenario_events_t *self, scenario_event *prev, bool make_new);
void scenario_events_rearm(scenario_events_t *self);
void scenario_events_process_all(scenario_events_t *self);
void scenario_earthquake_process(scenario_events_t *self);
void scenario_invasions_process(void);
bool scenario_event_processed_later(scenario_events_t *self, int32_t id);
scenario_event *scenario_invasion_event_at(scenario_events_t *self, int32_t id);
int32_t scenario_event_type_resource_min(scenario_event *e);
int32_t scenario_event_type_resource_max(scenario_event *e);
bool scenario_request_resource_allowed(scenario_event *e, int32_t resource);
bool scenario_gift_resource_allowed(int32_t resource);
bool scenario_event_resource_in_range(scenario_event *e, int32_t resource);

#endif
