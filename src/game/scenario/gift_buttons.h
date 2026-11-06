// The accept, refuse and later buttons of a gift's message (scenario/gift_buttons.c) and the gift event a message's
// phrases name.

#ifndef SCENARIO_GIFT_BUTTONS_H
#define SCENARIO_GIFT_BUTTONS_H

#include "scenario/scenario_event_types.h"

#include <stdint.h>

scenario_event *scenario_gift_for_message(scenario_events_t *self, const int16_t *phrases);
void gift_button_accept(void);
void gift_button_refuse(void);
void gift_button_later(void);

#endif
