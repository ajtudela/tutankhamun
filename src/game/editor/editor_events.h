// Shared by the scenario editor's event list and event editor: the buttons of both windows, the editor's state in
// scenario_events (selected event, scroll, the value a select list or numeric input writes and the field it is for),
// the window it was opened from and the store of a value's fixed / minimum / maximum.

#ifndef EDITOR_EDITOR_EVENTS_H
#define EDITOR_EDITOR_EVENTS_H

#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/editor_select_list.h"
#include "editor/editor_text.h"
#include "editor/scenario_event_methods.h"
#include "graphics/button_border.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_internal.h"
#include "window/window_data.h"
#include <stdint.h>

enum event_editor_button {
    EVENT_BUTTON_TYPE = 0,
    EVENT_BUTTON_MONTH = 1,
    EVENT_BUTTON_YEAR_FROM = 2,
    EVENT_BUTTON_YEAR_TO = 3,
    EVENT_BUTTON_ITEM_FIRST = 4,
    EVENT_BUTTON_AMOUNT_FROM = 7,
    EVENT_BUTTON_AMOUNT_TO = 8,
    EVENT_BUTTON_ROUTE_FROM = 10,
    EVENT_BUTTON_ROUTE_TO = 11,
    EVENT_BUTTON_LINK_FIRST = 13,
    EVENT_BUTTON_TRIGGER = 17,
    EVENT_BUTTON_MONTHS = 18,
    EVENT_BUTTON_ATTACK_TARGET = 19,
    EVENT_BUTTON_SENDER = 20,
    EVENT_BUTTON_INVADER_FROM = 21,
    EVENT_BUTTON_INVADER_TO = 22,
    EVENT_BUTTON_SUBTYPE = 24,
    EVENT_BUTTON_PHRASE = 25,
    EVENT_BUTTON_REASON_FIRST = 26,
};

enum event_list_button {
    EVENT_LIST_ROWS = 12,
    EVENT_LIST_BUTTON_ADD = 12,
    EVENT_LIST_BUTTON_DELETE = 13,
    EVENT_LIST_ARROWS = 2,
};

enum event_link {
    EVENT_LINK_COMPLETED = 0,
    EVENT_LINK_REFUSED = 1,
    EVENT_LINK_TOO_LATE = 2,
    EVENT_LINK_DEFEAT = 3,
};

enum event_range_end {
    EVENT_RANGE_FROM = 0,
    EVENT_RANGE_TO = 1,
};

enum event_item_slot {
    EVENT_ITEM_SLOTS = 3,
};

static inline const generic_button *event_editor_button(int32_t index)
{
    return &editor_event_type_buttons[index];
}

static inline const int16_t *event_editor_rect(int32_t index)
{
    return &event_editor_button(index)->x;
}

static inline const int16_t *event_list_rect(int32_t index)
{
    return &event_list_buttons[0].rows[index].x;
}

static inline int32_t event_button_focused(int32_t index)
{
    return focus_button_id == index + 1;
}

static inline scenario_event *event_editor_selected(void)
{
    int32_t index = scenario_events.editor_selected_id;
    if (index < 0 || index >= scenario_events.count) {
        return 0;
    }
    return &scenario_events.events[index];
}

static inline void scenario_value_store(scenario_value *v, int16_t fixed, int16_t min, int16_t max)
{
    if (fixed < 0) {
        if (min >= 0) {
            fixed = min;
            min = -1;
        } else if (max >= 0) {
            fixed = max;
            max = -1;
        }
    }
    if (min < 0 && max >= 0) {
        min = max;
        max = -1;
    }
    v->fixed = fixed;
    v->min = min;
    v->max = max;
}

const char *event_type_name(int32_t type);
void event_list_scroll_dot(void);
int32_t event_list_scrollbar(void);
void event_editor_back(void);
void event_item_chosen(void);

#endif
