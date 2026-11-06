// Methods of the event records used by the editor's event windows: what a type shows, the resources a type allows,
// a new type, how far requested troops travelled, a value's fixed / minimum / maximum, and the one-line description
// of an event in the event list.
//
// The route finder writes whether our city comes first only when it finds a route; otherwise the low byte of the city
// pointer is what gets tested.

#include "editor/scenario_event_methods.h"
#include "editor/editor_data.h"
#include "editor/editor_events.h"
#include "empire/empire.h"
#include "io/game_file_internal.h"
#include "scenario/scenario_internal.h"
#include "text/lang.h"
#include "window/map_selection.h"
#include "window/window_data.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

char event_description_text[300];

enum phrase_number {
    INVASION_PHRASE_NUMBER_FIRST_ROUTE = 9,
};

enum troops_travel {
    TROOPS_STATE_RETURNING_LAST = 7,
    EMPIRE_ROUTE_BY_LAND = 1,
};

enum event_description_size {
    EVENT_TRIGGER_TEXT = 52,
    EVENT_PART_TEXT = 20,
    EVENT_AMOUNT_TEXT = 40,
    EVENT_ITEMS_TEXT = 200,
    EVENT_DESCRIPTION_LENGTH = 40,
};

bool event_shows_resource(scenario_event *e)
{
    return event_type_table_shows_resource(e->type);
}

int32_t event_resource_group(scenario_event *e)
{
    return event_type_table_resource_group(e->type);
}

bool event_has_months(scenario_event *e)
{
    return (e->type == EVENT_TYPE_REQUEST) || (e->type == EVENT_TYPE_INVASION);
}

bool event_has_amount(scenario_event *e)
{
    return event_type_table_has_amount(e->type);
}

bool event_is_invasion_kind(scenario_event *e)
{
    return e->type == EVENT_TYPE_INVASION;
}

bool event_has_subtype(scenario_event *e)
{
    uint8_t type = e->type;
    return (type == EVENT_TYPE_REQUEST) || (type == EVENT_TYPE_CITY_STATUS_CHANGE) || (type == EVENT_TYPE_MESSAGE);
}

bool event_has_phrase_number(scenario_event *e)
{
    if (e->type != EVENT_TYPE_INVASION) {
        return false;
    }
    int16_t v;
    if (e->route.fixed < 0) {
        v = e->route.max;
    } else if (e->route.min < 0) {
        v = e->route.fixed;
    } else {
        return false;
    }
    return v >= INVASION_PHRASE_NUMBER_FIRST_ROUTE;
}

bool event_has_phrase_list(scenario_event *e)
{
    return (e->type == EVENT_TYPE_REQUEST) && (e->subtype == REQUEST_SUBTYPE_FESTIVAL);
}

int32_t event_subtype_count(scenario_event *e)
{
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return REQUEST_KINDS;
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            return EVENT_CITY_STATUS_SUBTYPES;
        case EVENT_TYPE_MESSAGE:
            return EVENT_MESSAGE_SUBTYPES;
        default:
            return 0;
    }
}

int32_t event_subtype_group(scenario_event *e)
{
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return LANG_GROUP_REQUEST_KINDS;
        case EVENT_TYPE_CITY_STATUS_CHANGE:
            return LANG_GROUP_CITY_STATUS_EVENTS;
        case EVENT_TYPE_MESSAGE:
            return LANG_GROUP_MESSAGE_EVENTS;
        default:
            return -1;
    }
}

int32_t event_resource_list(scenario_event *e)
{
    int32_t count = 0;
    int32_t *item = select_list.items;
    for (int32_t r = scenario_event_type_resource_min(e); r < scenario_event_type_resource_max(e); r++) {
        if (scenario_event_resource_in_range(e, r)) {
            *item++ = r;
            count++;
        }
    }
    return count;
}

void event_set_type(scenario_event *e, int32_t type)
{
    uint8_t t = (uint8_t) type;
    if (e->type == t) {
        return;
    }
    e->type = t;
    e->item.fixed = 1;
    e->item.min = -1;
    e->item.max = -1;
    if ((e->flags & EVENT_FLAG_BY_FAVOUR) && t != EVENT_TYPE_INVASION) {
        e->flags = 0;
    }
}

static int32_t troops_travel_percent(const scenario_event *e, int32_t travel, int32_t *percent)
{
    if (e->state == REQUEST_STATE_RECEIVED) {
        *percent = 100 - ((e->months_left * 100) / travel);
        return 1;
    }
    if (e->state > REQUEST_STATE_RECEIVED && e->state <= TROOPS_STATE_RETURNING_LAST) {
        *percent = (e->months_left * 100) / travel;
        return 1;
    }
    return 0;
}

bool troops_request_state(scenario_event *e, int32_t *city, int32_t *percent, uint8_t *by_land, int32_t *zero)
{
    *city = -1;
    *percent = 0;
    *by_land = 0;
    *zero = 0;
    if (e->type != EVENT_TYPE_REQUEST || !e->is_active || e->item.value != RESOURCE_TROOPS) {
        return false;
    }
    union {
        int32_t *pointer;
        uint8_t byte;
    } ours_first;
    ours_first.pointer = city;
    empire_route_find_for_city(e->route.value, &ours_first.byte);
    *city = e->route.value;
    int32_t travel = scenario_event_travel_months(e);
    if (travel < 1) {
        travel = 1;
    }
    if (!troops_travel_percent(e, travel, percent)) {
        return false;
    }
    if (*percent < 0) {
        *percent = 0;
    } else if (*percent > 100) {
        *percent = 100;
    }
    if (ours_first.byte) {
        *percent = 100 - *percent;
    }
    *by_land = empire_map_routes.routes[*city].kind == EMPIRE_ROUTE_BY_LAND;
    *zero = 0;
    return true;
}

void scenario_value_set(scenario_value *v, int32_t fixed, int32_t min, int32_t max)
{
    scenario_value_store(v, (int16_t) fixed, (int16_t) min, (int16_t) max);
}

static void value_range(const scenario_value *v, int16_t *lo, int16_t *hi)
{
    if (v->fixed < 0) {
        *lo = v->min;
        *hi = v->max;
    } else if (v->min < 0) {
        *lo = v->fixed;
        *hi = v->fixed;
    } else {
        *lo = -1;
        *hi = -1;
    }
}

static int32_t event_chains_to(scenario_event *other, int16_t id)
{
    return (scenario_event_has_chain(other) && other->on_completed == id) || (scenario_event_type_chains(other) && other->on_refusal == id) ||
        (scenario_event_type_chains(other) && other->on_too_late == id) || (scenario_event_has_followup(other) && other->on_defeat == id);
}

static void format_trigger(char *trigger, const scenario_event *e, const char *month)
{
    if (e->flags & EVENT_FLAG_CHAIN_ONLY) {
        int32_t found = -1;
        for (int32_t i = 0; i < scenario_events.count; i++) {
            if (event_chains_to(&scenario_events.events[i], e->id)) {
                found = i;
                break;
            }
        }
        if (found >= 0) {
            sprintf(trigger, "*%d ", found);
        } else {
            sprintf(trigger, "*** ");
        }
    } else if (e->flags & EVENT_FLAG_BY_FAVOUR) {
        sprintf(trigger, "** ");
    } else {
        sprintf(trigger, "%s ", month);
    }
}

static void append_years(char *trigger, const scenario_event *e)
{
    char part[EVENT_PART_TEXT];
    int16_t lo;
    int16_t hi;
    value_range(&e->year, &lo, &hi);
    sprintf(part, "+%d", (int32_t) lo);
    strcat(trigger, part);
    if (lo < hi) {
        sprintf(part, "-%d", (int32_t) hi);
        strcat(trigger, part);
    }
}

static void format_amount(char *amount, scenario_event *e)
{
    if (!event_has_amount(e)) {
        amount[0] = 0;
        return;
    }
    int16_t lo;
    int16_t hi;
    value_range(&e->amount, &lo, &hi);
    if (lo < hi) {
        sprintf(amount, " %d-%d", (int32_t) lo, (int32_t) hi);
    } else {
        sprintf(amount, " %d", (int32_t) lo);
    }
}

static void join_items(char *items, const int16_t *values, const char *const *names)
{
    items[0] = ' ';
    items[1] = 0;
    for (int32_t i = 0; i < EVENT_ITEM_SLOTS; i++) {
        if (values[i] > 0) {
            if (items[1] != 0) {
                strcat(items, "/");
            }
            strcat(items, names[i]);
        }
    }
}

const char *event_description(scenario_event *e)
{
    char trigger[EVENT_TRIGGER_TEXT];
    char amount[EVENT_AMOUNT_TEXT];
    char items[EVENT_ITEMS_TEXT];
    int16_t values[EVENT_ITEM_SLOTS] = { -1, -1, -1 };
    const char *names[EVENT_ITEM_SLOTS] = { 0, 0, 0 };
    const char *month = lang_get_string(LANG_GROUP_MONTHS_SHORT, e->month);
    int32_t group = event_resource_group(e);
    if (group >= 0) {
        values[0] = e->item.fixed;
        values[1] = e->item.min;
        values[2] = e->item.max;
        for (int32_t i = 0; i < EVENT_ITEM_SLOTS; i++) {
            int32_t item = values[i];
            if (item >= scenario_event_type_resource_min(e)) {
                names[i] = lang_get_string(group, item);
            } else {
                names[i] = empty_text;
            }
        }
    }
    format_trigger(trigger, e, month);
    append_years(trigger, e);
    format_amount(amount, e);
    join_items(items, values, names);
    char *out = event_description_text;
    if (event_shows_resource(e)) {
        sprintf(out, "%s%s%s", trigger, amount, items);
    } else {
        sprintf(out, "%s%s", trigger, amount);
    }
    out[EVENT_DESCRIPTION_LENGTH] = 0;
    return out;
}
