// The reply buttons of a gift's message: accept, refuse and later. Each acts only when the dialog shows it, finds the
// gift event the message names, sets the gift's state and closes the dialog; later offers the gift again in a month.

#include "scenario/gift_buttons.h"

#include "city/resources.h"
#include "scenario/scenario_data.h"
#include "scenario/scenario_internal.h"
#include "window/message_dialog.h"
#include "window/window_data.h"

scenario_event *scenario_gift_for_message(scenario_events_t *self, const int16_t *phrases)
{
    for (int32_t i = 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        if (e->type == EVENT_TYPE_GIFT_FROM_PHARAOH && e->is_active && phrases[EVENT_PHRASE_AMOUNT] == e->amount.value &&
            phrases[EVENT_PHRASE_ITEM] == e->item.value) {
            e->id = (int16_t) i;
            return e;
        }
    }
    return NULL;
}

static scenario_event *shown_gift(void)
{
    city_message *message = &city_messages[city_message_shown_index];
    message->phrases[EVENT_PHRASE_BUTTONS] = 0;
    return scenario_gift_for_message(&scenario_events, message->phrases);
}

void gift_button_accept(void)
{
    if (!(dialog_phrase_flags & GIFT_BUTTON_ACCEPT)) {
        return;
    }
    scenario_event *e = shown_gift();
    if (e == NULL) {
        return;
    }
    e->state = GIFT_STATE_ACCEPTED;
    scenario_event_process_gift_from_pharaoh(e);
    window_message_dialog_close();
    city_player_current()->resource_gift_received[e->item.value] = 1;
    city_resource_determine_available();
}

void gift_button_refuse(void)
{
    if (!(dialog_phrase_flags & GIFT_BUTTON_REFUSE)) {
        return;
    }
    scenario_event *e = shown_gift();
    if (e == NULL) {
        return;
    }
    e->state = GIFT_STATE_REFUSED;
    scenario_event_process_gift_from_pharaoh(e);
    window_message_dialog_close();
}

void gift_button_later(void)
{
    if (!(dialog_phrase_flags & GIFT_BUTTON_LATER)) {
        return;
    }
    scenario_event *e = shown_gift();
    if (e == NULL) {
        return;
    }
    switch (e->state) {
        case GIFT_STATE_OFFERED:
            e->state = GIFT_STATE_OFFER_AGAIN;
            e->months_left = 1;
            break;
        case GIFT_STATE_OFFERED_AGAIN:
            e->state = GIFT_STATE_OFFER_LAST;
            e->months_left = 1;
            break;
        default:
            e->state = GIFT_STATE_TOO_LATE;
            scenario_event_process_gift_from_pharaoh(e);
            break;
    }
    window_message_dialog_close();
}
