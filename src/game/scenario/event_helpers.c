// Methods of a scenario event record: its date checks (due, still to come, running), the months left to comply and
// the margin, whether a request is waiting, the phrases and texts of its messages (from config/event_type_texts.json
// by type, or by subtype, invader or sender), the reason phrases of its follow-ups and the travel time of its city.

#include "empire/empire.h"
#include "empire/empire_route.h"
#include "game/game_data.h"
#include "scenario/scenario_climate.h"
#include "scenario/scenario_data.h"
#include "scenario/scenario_internal.h"
#include "scenario/scenario_rules.h"
#include "window/window_data.h"

#include <stdbool.h>

static int32_t start_year(void)
{
    return scenario_data.start.year;
}

bool scenario_event_is_due(scenario_event *e, bool any)
{
    if (!any && (e->flags & (EVENT_FLAG_CHAIN_ONLY | EVENT_FLAG_FIRED))) {
        return 0;
    }
    if (game_time_year != e->year.value + start_year()) {
        return 0;
    }
    return game_time_month == e->month;
}

bool scenario_event_is_in_future(scenario_event *e)
{
    int32_t year = e->year.value + start_year();
    if (game_time_year < year) {
        return 1;
    }
    if (game_time_year > year) {
        return 0;
    }
    return game_time_month < e->month;
}

bool scenario_event_is_running(scenario_event *e)
{
    if (e->flags & (EVENT_FLAG_CHAIN_ONLY | EVENT_FLAG_BY_FAVOUR)) {
        return 0;
    }
    if (scenario_event_is_in_future(e)) {
        return 0;
    }
    int32_t months = (game_time_year - e->year.value - start_year()) * 12 - e->month + game_time_month;
    return months <= e->months_initial;
}

int32_t scenario_event_months_left(scenario_event *e)
{
    return e->months_initial + (e->year.value + start_year() - game_time_year) * 12 + e->month - game_time_month;
}

bool scenario_request_is_waiting(scenario_event *e)
{
    if (e->type != EVENT_TYPE_REQUEST || !e->is_active) {
        return 0;
    }
    return (uint16_t) e->state <= 1;
}

bool scenario_event_parent_by_favour(scenario_event *e)
{
    int32_t id = e->parent_id;
    if (id < 0 || id >= scenario_events.count) {
        return 0;
    }
    return (scenario_events.events[id].flags & EVENT_FLAG_BY_FAVOUR) != 0;
}

void scenario_event_fill_phrases(scenario_event *e, int16_t *phrases)
{
    int16_t city = (int16_t) empire_city_get_for_trade_route(e->route.value);
    phrases[EVENT_PHRASE_CITY] = city;
    phrases[EVENT_PHRASE_REASON_CITY] = city;
    phrases[EVENT_PHRASE_REASON_AMOUNT] = e->amount.value;
    phrases[EVENT_PHRASE_REASON_ITEM] = e->item.value;
    uint8_t reason_variant = e->phrase_var;
    if (scenario_event_get(e->parent_id)) {
        if (e->parent_phrase >= 0) {
            phrases[EVENT_PHRASE_TEXT] = e->parent_phrase;
        }
        if (e->parent_city) {
            phrases[EVENT_PHRASE_REASON_CITY] = (int16_t) empire_city_get_for_trade_route(e->parent_city);
        }
        phrases[EVENT_PHRASE_REASON_AMOUNT] = e->parent_amount;
        phrases[EVENT_PHRASE_REASON_ITEM] = e->parent_item;
        reason_variant = e->parent_phrase_var;
    }
    phrases[EVENT_PHRASE_NONE] = -1;
    phrases[EVENT_PHRASE_AMOUNT] = e->amount.value;
    phrases[EVENT_PHRASE_ITEM] = e->item.value;
    phrases[EVENT_PHRASE_MONTHS_TO_COMPLY] = e->months_initial;
    phrases[EVENT_PHRASE_MONTHS_LEFT] = (int16_t) scenario_event_months_left(e);
    phrases[EVENT_PHRASE_DELIVERED_AMOUNT] = 0;
    phrases[EVENT_PHRASE_BUTTONS] = 0;
    phrases[EVENT_PHRASE_VARIANTS] = (int16_t) (e->phrase_var | (reason_variant << 8));
}

static const request_message_rule *request_rule(const scenario_event *e)
{
    return (e->subtype < REQUEST_SUBTYPES) ? &request_message_rules[e->subtype] : NULL;
}

static int32_t event_sender(const scenario_event *e)
{
    return (e->sender_faction != 0) ? EVENT_SENDER_PHARAOH : EVENT_SENDER_CITY;
}

static const event_type_text *type_texts(const scenario_event *e)
{
    return (e->type < EVENT_TYPE_COUNT) ? &event_type_texts[e->type] : NULL;
}

static int32_t invader_text(const scenario_event *e, int32_t enemy, int32_t egypt, int32_t pharaoh, int32_t bedouins)
{
    switch ((uint16_t) e->item.value) {
        case EVENT_INVADER_ENEMY:
            return enemy;
        case EVENT_INVADER_EGYPT:
            return egypt;
        case EVENT_INVADER_PHARAOH:
            return pharaoh;
        case EVENT_INVADER_BEDOUINS:
            return bedouins;
    }
    return -1;
}

static int32_t subtype_text(const scenario_event *e, int32_t subtypes, int32_t first, int32_t step)
{
    return (e->subtype < subtypes) ? (first + (step * e->subtype)) : -1;
}

static int32_t sender_text(const scenario_event *e, int32_t from_city)
{
    return from_city - event_sender(e);
}

void scenario_event_init_phrases(scenario_event *e, int16_t *phrases)
{
    const event_type_text *texts = type_texts(e);
    int16_t first = (int16_t) (texts ? texts->first_phrase : -1);
    int16_t second = (int16_t) (texts ? texts->second_phrase : -1);
    if (e->type == EVENT_TYPE_LAND_TRADE_PROBLEM && scenario_data.climate_type != CLIMATE_DESERT) {
        first = 244;
        second = 243;
    }
    phrases[EVENT_PHRASE_SECOND] = second;
    phrases[EVENT_PHRASE_FIRST] = first;
    phrases[EVENT_PHRASE_TEXT] = (int16_t) scenario_event_message_text(e);
    scenario_event_fill_phrases(e, phrases);
}

int32_t scenario_event_message_text(scenario_event *e)
{
    const request_message_rule *rule = request_rule(e);
    const event_type_text *texts = type_texts(e);
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return rule ? rule->announcement[event_sender(e)] : -1;
        case EVENT_TYPE_INVASION:
            return invader_text(e, 190, 180, 170, 263);
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            return subtype_text(e, EVENT_CITY_STATUS_SUBTYPES, 129, 4);
        case EVENT_TYPE_MESSAGE:
            return subtype_text(e, EVENT_MESSAGE_SUBTYPES, 149, 4);
        case EVENT_TYPE_GIFT_FROM_PHARAOH:
            return sender_text(e, 293);
    }
    return texts ? texts->announcement : -1;
}

int32_t scenario_event_message_text_completed(scenario_event *e)
{
    const request_message_rule *rule = request_rule(e);
    const event_type_text *texts = type_texts(e);
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return rule ? rule->completed[event_sender(e)] : -1;
        case EVENT_TYPE_INVASION:
            return invader_text(e, 189, 179, 169, 262);
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            return subtype_text(e, EVENT_CITY_STATUS_SUBTYPES, 0x80, 4);
        case EVENT_TYPE_MESSAGE:
            return subtype_text(e, EVENT_MESSAGE_SUBTYPES, 148, 4);
        case EVENT_TYPE_GIFT_FROM_PHARAOH:
            return sender_text(e, 287);
    }
    return texts ? texts->completed : -1;
}

int32_t scenario_event_reason_group_completed(scenario_event *e)
{
    const request_message_rule *rule = request_rule(e);
    const event_type_text *texts = type_texts(e);
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return rule ? rule->completed_group : -1;
        case EVENT_TYPE_INVASION:
            return invader_text(e, 32, 33, 34, 50);
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            return subtype_text(e, EVENT_CITY_STATUS_SUBTYPES, 23, 1);
        case EVENT_TYPE_MESSAGE:
            return subtype_text(e, EVENT_MESSAGE_SUBTYPES, 28, 1);
    }
    return texts ? texts->reason_group : -1;
}

int32_t scenario_event_reason_column(scenario_event *e)
{
    const request_message_rule *rule = request_rule(e);
    const event_type_text *texts = type_texts(e);
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return rule ? rule->reason_column : -1;
        case EVENT_TYPE_INVASION:
            return invader_text(e, 16, 17, 18, 34);
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            return subtype_text(e, EVENT_CITY_STATUS_SUBTYPES, 7, 1);
        case EVENT_TYPE_MESSAGE:
            return subtype_text(e, EVENT_MESSAGE_SUBTYPES, 12, 1);
    }
    return texts ? texts->reason_column : -1;
}

int32_t scenario_event_reason_group(scenario_event *e, int32_t action)
{
    switch (action) {
        case 0:
            return scenario_event_reason_group_completed(e);
        case 1:
            return scenario_event_reason_group_refused(e);
        case 2:
            return scenario_event_reason_group_too_late(e);
        case 3:
            return scenario_event_reason_group_defeat(e);
        default:
            return -1;
    }
}

int32_t scenario_event_travel_months(scenario_event *e)
{
    uint8_t ours_first;
    if (e->type == EVENT_TYPE_INVASION) {
        int32_t index = e->invasion.value;
        if (index >= 0 && index < EMPIRE_ROUTES) {
            empire_route *route = &empire_map_routes.routes[index];
            if (route->num_points <= 1) {
                route->in_use = 0;
            }
            if (route->in_use) {
                return empire_route_travel_months(route);
            }
        }
    } else if (scenario_event_uses_trade_city(e)) {
        int32_t route = e->route.value;
        empire_route_find_for_city(empire_city_get_for_trade_route(route), &ours_first);
        return empire_route_travel_months(&empire_map_routes.routes[route]);
    }
    return 0;
}

int32_t scenario_event_months_margin(scenario_event *e)
{
    int32_t months = scenario_event_months_left(e);
    int32_t travel = scenario_event_travel_months(e);
    if (travel <= 0) {
        travel = EVENT_TRAVEL_MONTHS_DEFAULT;
    }
    int32_t result = months - e->months_margin - travel + EVENT_MONTHS_MARGIN_BASE;
    if (result < 1) {
        result = 1;
    }
    return result;
}

static int32_t reason_phrase_variant_case(int32_t text)
{
    for (int32_t c = 0; c < REASON_PHRASE_VARIANT_TEXTS; c++) {
        if (reason_phrase_variant_texts[c] == text) {
            return c;
        }
    }
    return -1;
}

int32_t scenario_reason_phrase_variant(int32_t text, int32_t variant)
{
    int32_t c = reason_phrase_variant_case(text);
    if (c < 0) {
        return -1;
    }
    switch (variant % 3) {
        case 0:
            return text;
        case 1:
            return REASON_PHRASE_VARIANT_FIRST + (2 * c);
        case 2:
            return REASON_PHRASE_VARIANT_FIRST + (2 * c) + 1;
        default:
            return -1;
    }
}

int32_t scenario_event_reason_phrase(scenario_event *e, int32_t action)
{
    int32_t id;
    int32_t variant;
    switch (action) {
        case 0:
            id = e->on_completed;
            variant = e->reason[0];
            break;
        case 1:
            id = e->on_refusal;
            variant = e->reason[1];
            break;
        case 2:
            id = e->on_too_late;
            variant = e->reason[2];
            break;
        case 3:
            id = e->on_defeat;
            variant = e->reason[3];
            break;
        default:
            return -1;
    }
    int32_t use_action_text = 0;
    if (variant >= 6) {
        if (id < 0 || id >= scenario_events.count) {
            return -1;
        }
        int32_t group = scenario_event_reason_group(e, action);
        int32_t column = scenario_event_reason_column(&scenario_events.events[id]);
        if (group < 0 || group >= SCENARIO_REASON_GROUPS || column < 0 || column >= SCENARIO_REASON_COLUMNS) {
            variant = 0;
            use_action_text = 1;
        } else {
            variant = reason_phrases[(group * SCENARIO_REASON_COLUMNS) + column];
        }
    }
    if (!use_action_text) {
        if (variant >= 3 && variant <= 5 && id >= 0 && id < scenario_events.count) {
            scenario_event *next = &scenario_events.events[id];
            return scenario_reason_phrase_variant(scenario_event_message_text(next), variant);
        }
        if (variant >= 6) {
            return -1;
        }
    }
    switch (action) {
        case 0:
            return scenario_reason_phrase_variant(scenario_event_message_text_completed(e), variant);
        case 1:
            return scenario_reason_phrase_variant(scenario_event_message_text_refused(e), variant);
        case 2:
            return scenario_reason_phrase_variant(scenario_event_message_text_too_late(e), variant);
        default:
            return scenario_reason_phrase_variant(scenario_event_message_text_defeat(e), variant);
    }
}
