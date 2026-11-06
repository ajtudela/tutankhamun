// The reason groups and follow-up message texts of scenario/event_reasons.c.

#ifndef SCENARIO_EVENT_REASONS_H
#define SCENARIO_EVENT_REASONS_H

#include "scenario/scenario_event_types.h"

#include <stdint.h>

int32_t scenario_event_reason_group_refused(scenario_event *e);
int32_t scenario_event_reason_group_too_late(scenario_event *e);
int32_t scenario_event_reason_group_defeat(scenario_event *e);
int32_t scenario_event_message_text_refused(scenario_event *e);
int32_t scenario_event_message_text_too_late(scenario_event *e);
int32_t scenario_event_message_text_defeat(scenario_event *e);

#endif
