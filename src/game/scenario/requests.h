// The request functions of scenario/requests.c: status and dispatch of a waiting request, the message amount token,
// the legions' arrival, whether an invasion runs, the earthquake count and the months left of an event.

#ifndef SCENARIO_REQUESTS_H
#define SCENARIO_REQUESTS_H

#include "scenario/scenario_event_types.h"

#include <stdint.h>

enum scenario_request_status {
    SCENARIO_REQUEST_NO_WATERBORNE_COMPANIES = -6,
    SCENARIO_REQUEST_NO_WATERBORNE_IN_SERVICE = -5,
    SCENARIO_REQUEST_NO_COMPANIES = -4,
    SCENARIO_REQUEST_NO_COMPANIES_IN_SERVICE = -3,
    SCENARIO_REQUEST_TROOPS = -2,
    SCENARIO_REQUEST_SHORT = -1,
    SCENARIO_REQUEST_NONE = 0,
};

int32_t scenario_request_status(int32_t index, int32_t *event_id);
int32_t scenario_request_status_button(int32_t index);
int32_t scenario_invasion_running(void);
int32_t scenario_count_earthquake_events(scenario_events_t *self);
void scenario_request_dispatch(int32_t event_id);
const char *message_token_request_amount_text(void);
int32_t scenario_event_months_left_value(scenario_event *e);
int32_t scenario_troops_request_dispatched(int32_t event_id);

#endif
