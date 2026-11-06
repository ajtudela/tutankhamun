// The reason groups and message texts of the follow-ups of a request or a gift: refused, too late and (troops
// requests only) defeat; a request's by subtype and sender, a gift's fixed; -1 for other events.

#include "scenario/event_reasons.h"

#include "scenario/scenario_internal.h"
#include "scenario/scenario_rules.h"

enum gift_reasons {
    GIFT_REFUSED_GROUP = 51,
    GIFT_TOO_LATE_GROUP = 52,
    GIFT_REFUSED_TEXT_CITY = 291,
    GIFT_TOO_LATE_TEXT_CITY = 289,
};

static const request_message_rule *request_rule(const scenario_event *e)
{
    if (e->type != EVENT_TYPE_REQUEST || e->subtype >= REQUEST_SUBTYPES) {
        return NULL;
    }
    return &request_message_rules[e->subtype];
}

static int32_t event_sender(const scenario_event *e)
{
    return (e->sender_faction != 0) ? EVENT_SENDER_PHARAOH : EVENT_SENDER_CITY;
}

int32_t scenario_event_reason_group_refused(scenario_event *e)
{
    if (request_rule(e)) {
        return request_rule(e)->refused_group;
    }
    return (e->type == EVENT_TYPE_GIFT_FROM_PHARAOH) ? GIFT_REFUSED_GROUP : -1;
}

int32_t scenario_event_reason_group_too_late(scenario_event *e)
{
    if (request_rule(e)) {
        return request_rule(e)->too_late_group;
    }
    return (e->type == EVENT_TYPE_GIFT_FROM_PHARAOH) ? GIFT_TOO_LATE_GROUP : -1;
}

int32_t scenario_event_reason_group_defeat(scenario_event *e)
{
    if (request_rule(e)) {
        return request_rule(e)->defeat_group;
    }
    return -1;
}

int32_t scenario_event_message_text_refused(scenario_event *e)
{
    if (request_rule(e)) {
        return request_rule(e)->refused[event_sender(e)];
    }
    return (e->type == EVENT_TYPE_GIFT_FROM_PHARAOH) ? (GIFT_REFUSED_TEXT_CITY - event_sender(e)) : -1;
}

int32_t scenario_event_message_text_too_late(scenario_event *e)
{
    if (request_rule(e)) {
        return request_rule(e)->too_late[event_sender(e)];
    }
    return (e->type == EVENT_TYPE_GIFT_FROM_PHARAOH) ? (GIFT_TOO_LATE_TEXT_CITY - event_sender(e)) : -1;
}

int32_t scenario_event_message_text_defeat(scenario_event *e)
{
    if (request_rule(e)) {
        return request_rule(e)->defeat[event_sender(e)];
    }
    return -1;
}
