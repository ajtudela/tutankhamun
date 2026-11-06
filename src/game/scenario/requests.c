// The kingdom's requests seen from the advisors: whether a waiting request can be dispatched and why not, its
// dispatch (deben, goods or legions, arriving after some months), the amount token of a request message, the arrival
// and losses of the legions sent for a troops request, the count of earthquake events and whether an invasion runs.
//
// The dispatched state is written with the event index's high byte, and the dialog's "sent" flag is the Seth minor
// blessing months.

#include "scenario/requests.h"

#include "building/building_storage.h"
#include "city/legion_requests.h"
#include "core/calc.h"
#include "empire/empire_helpers.h"
#include "empire/empire_route.h"
#include "scenario/scenario_internal.h"
#include "scenario/scenario_rules.h"
#include "window/window_data.h"
#include <stdio.h>

char message_amount_buffer[8];

enum request_dispatch_rules {
    REQUEST_LOAD_UNITS = 100,
    REQUEST_TROOPS_MONTHS_DEFAULT = 12,
};

static int32_t request_troops_status(const scenario_event *e, const city_player *player)
{
    int32_t route = e->route.value;
    if (route >= 0 && route < EMPIRE_ROUTES && empire_map_routes.routes[route].kind == EMPIRE_ROUTE_KIND_SEA) {
        if (!player->total_warships && !player->legions_fighting) {
            return SCENARIO_REQUEST_NO_WATERBORNE_COMPANIES;
        }
        if (!player->empire_service_warships && !player->empire_service_legions_fighting) {
            return SCENARIO_REQUEST_NO_WATERBORNE_IN_SERVICE;
        }
    } else {
        if (!player->legions_not_fighting) {
            return SCENARIO_REQUEST_NO_COMPANIES;
        }
        if (!player->empire_service_legions_not_fighting) {
            return SCENARIO_REQUEST_NO_COMPANIES_IN_SERVICE;
        }
    }
    return SCENARIO_REQUEST_TROOPS;
}

int32_t scenario_request_status(int32_t index, int32_t *event_id)
{
    int32_t n = 0;
    for (scenario_event *e = scenario_request_next_active(&scenario_events, NULL); e; e = scenario_request_next_active(&scenario_events, e)) {
        if (!scenario_request_is_waiting(e)) {
            continue;
        }
        if (n != index) {
            n++;
            continue;
        }
        *event_id = e->id;
        const city_player *player = city_player_current();
        int16_t item = e->item.value;
        if (item == RESOURCE_DEBEN) {
            if (player->treasury <= e->amount.value) {
                return SCENARIO_REQUEST_SHORT;
            }
        } else if (item == RESOURCE_TROOPS) {
            return request_troops_status(e, player);
        } else if (player->storage_stored[item] < (e->amount.value * REQUEST_LOAD_UNITS)) {
            return SCENARIO_REQUEST_SHORT;
        }
        return e->id + 1;
    }
    return SCENARIO_REQUEST_NONE;
}

int32_t scenario_request_status_button(int32_t index)
{
    int32_t event_id = index;
    return scenario_request_status(index, &event_id);
}

int32_t scenario_invasion_running(void)
{
    for (scenario_event *e = scenario_invasion_next(&scenario_events, NULL, 0); e; e = scenario_invasion_next(&scenario_events, e, 0)) {
        if (scenario_event_is_running(e)) {
            return 1;
        }
    }
    return 0;
}

int32_t scenario_count_earthquake_events(scenario_events_t *self)
{
    int32_t count = 0;
    for (int32_t i = 0; i < self->count; i++) {
        if (self->events[i].type == EVENT_TYPE_EARTHQUAKE) {
            count++;
        }
    }
    return count;
}

static int16_t request_delivery_months(void)
{
    random_generate_next();
    return (int16_t) ((random_generator_state.random1_15bit & 3) + 1);
}

void scenario_request_dispatch(int32_t event_id)
{
    if (event_id < 0 || event_id >= scenario_events.count) {
        return;
    }
    scenario_event *e = &scenario_events.events[event_id];
    int32_t amount = e->amount.value;
    e->state = (int16_t) (((event_id & ~0xFF) | (e->state != REQUEST_STATE_NORMAL)) + REQUEST_STATE_DISPATCHED);
    int32_t item = e->item.value;
    if (item == RESOURCE_DEBEN) {
        e->months_left = request_delivery_months();
        city_player *player = city_player_current();
        player->treasury -= amount;
        player->requests_and_festivals_this_year += amount;
        return;
    }
    if (item == RESOURCE_TROOPS) {
        e->state = REQUEST_STATE_RECEIVED;
        e->months_left = (int16_t) scenario_event_travel_months(e);
        formation_legions_dispatch_to_request(e->id);
        if (e->months_left <= 0) {
            e->months_left = REQUEST_TROOPS_MONTHS_DEFAULT;
        }
        return;
    }
    e->months_left = request_delivery_months();
    building_storageyards_remove_resource(city_current_player, item, amount * REQUEST_LOAD_UNITS);
}

static int32_t request_amount_is_in_loads(int32_t resource)
{
    switch (resource) {
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
        case RESOURCE_CHARIOTS:
        case RESOURCE_WEAPONS:
        case RESOURCE_DEBEN:
            return 0;
    }
    return 1;
}

const char *message_token_request_amount_text(void)
{
    const city_message *message = &city_messages[city_message_shown_index];
    int32_t amount = message->phrases[EVENT_PHRASE_DELIVERED_AMOUNT];
    int32_t resource = message->phrases[EVENT_PHRASE_ITEM];
    if (request_amount_is_in_loads(resource)) {
        amount *= REQUEST_LOAD_UNITS;
    }
    sprintf(message_amount_buffer, "%d", amount);
    return message_amount_buffer;
}

int32_t scenario_event_months_left_value(scenario_event *e)
{
    return e->months_left;
}

static int32_t troops_losses_by_surplus(int32_t percent)
{
    for (int32_t i = 0; i < TROOPS_LOSS_BRACKETS - 1; i++) {
        if (percent < troops_loss_brackets[i].below_percent) {
            return troops_loss_brackets[i].losses;
        }
    }
    return troops_loss_brackets[TROOPS_LOSS_BRACKETS - 1].losses;
}

int32_t scenario_troops_request_dispatched(int32_t event_id)
{
    const scenario_event *e = (event_id < 0 || event_id >= scenario_events.count) ? NULL : &scenario_events.events[event_id];
    int32_t strength = formation_request_strength(event_id, NULL, NULL);
    int32_t needed = e ? e->amount.value : 1;
    city_player *player = city_player_current();
    if (strength < needed && player->seth_minor_blessing_months == 0) {
        formation_legions_request_losses(REQUEST_LOSSES_ALL, event_id);
        return 0;
    }
    if (player->seth_minor_blessing_months != 0) {
        player->seth_minor_blessing_months = 0;
        formation_legions_request_losses(0, event_id);
        return 1;
    }
    int32_t percent = calc_percentage(strength - needed, strength);
    formation_legions_request_losses(troops_losses_by_surplus(percent), event_id);
    return 1;
}
