// The scenario editor's event list: twelve rows with the scroll bar and arrows, a row opening the event editor,
// new, delete and back; deleting an event moves the later ones up and fixes their links.

#include "editor/editor_event_list.h"
#include "core/calc.h"
#include "core/static_init.h"
#include "editor/editor_data.h"
#include "editor/editor_events.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "text/lang.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int32_t event_editor_previous_window;

enum event_list_layout {
    EVENT_LIST_PANEL_WIDTH = 40,
    EVENT_LIST_PANEL_HEIGHT = 27,
    EVENT_LIST_SCROLL_LEFT = 508,
    EVENT_LIST_SCROLL_RIGHT = 540,
    EVENT_LIST_SCROLL_TOP = 80,
    EVENT_LIST_SCROLL_BOTTOM = 300,
    EVENT_LIST_SCROLL_RANGE = 196,
    EVENT_LIST_SCROLL_DOT_X = 516,
    EVENT_LIST_SCROLL_DOT_IMAGE = 39,
    EVENT_LIST_TEXT_PADDING = 4,
    EVENT_LIST_LINE_TEXT = 88,
};

enum event_list_arrow {
    EVENT_LIST_ARROW_UP = 0,
    EVENT_LIST_ARROW_DOWN = 1,
};

enum new_event_defaults {
    NEW_EVENT_AMOUNT = 8,
    NEW_EVENT_MONTHS = 12,
};

int32_t event_list_scrollbar(void)
{
    int32_t count = scenario_events.count;
    if (count <= EVENT_LIST_ROWS && scenario_events.editor_scroll == 0) {
        return 0;
    }
    if (!mouse_left_down) {
        return 0;
    }
    int32_t max_scroll = count - EVENT_LIST_ROWS;
    int32_t x = mouse_x;
    if (x < ui_origin_x + EVENT_LIST_SCROLL_LEFT || x > ui_origin_x + EVENT_LIST_SCROLL_RIGHT) {
        return 0;
    }
    int32_t y = mouse_y;
    if (y < ui_origin_y + EVENT_LIST_SCROLL_TOP || y > ui_origin_y + EVENT_LIST_SCROLL_BOTTOM) {
        return 0;
    }
    if (max_scroll < 0) {
        max_scroll = 0;
    }
    int32_t dy = y - ui_origin_y - EVENT_LIST_SCROLL_TOP;
    if (dy > EVENT_LIST_SCROLL_RANGE) {
        dy = EVENT_LIST_SCROLL_RANGE;
    }
    int32_t scroll = calc_adjust_with_percentage(max_scroll, calc_percentage(dy, EVENT_LIST_SCROLL_RANGE));
    if (scroll >= 0 && scroll < scenario_events.count) {
        scenario_events.editor_scroll = scroll;
    }
    window_redraw_request = 2;
    return 1;
}

void event_list_scroll_dot(void)
{
    int32_t range = scenario_events.count - EVENT_LIST_ROWS;
    int32_t percent;
    if (range > 0) {
        percent = calc_percentage(scenario_events.editor_scroll, range);
        if (percent > 100) {
            percent = 100;
        }
    } else {
        if (scenario_events.editor_scroll == 0) {
            return;
        }
        percent = 100;
    }
    int32_t dy = calc_adjust_with_percentage(EVENT_LIST_SCROLL_RANGE, percent);
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + EVENT_LIST_SCROLL_DOT_IMAGE, ui_origin_x + EVENT_LIST_SCROLL_DOT_X,
        ui_origin_y + dy + EVENT_LIST_SCROLL_TOP);
}

void event_list_arrow(void)
{
    if (button_parameter == EVENT_LIST_ARROW_UP) {
        if (scenario_events.editor_scroll > 0) {
            scenario_events.editor_scroll--;
        }
    } else if (button_parameter == EVENT_LIST_ARROW_DOWN) {
        if (scenario_events.editor_scroll < scenario_events.count - 1) {
            scenario_events.editor_scroll++;
        }
    }
}

void event_list_row(void)
{
    int32_t index = button_parameter + scenario_events.editor_scroll;
    if (index < 0 || index >= scenario_events.count) {
        scenario_events.editor_selected_id = -1;
        return;
    }
    scenario_events.editor_selected_id = index;
    if (event_editor_selected() != 0) {
        uint8_t window = window_current_id;
        if (window != WINDOW_EDITOR_EVENT) {
            window_current_id = WINDOW_EDITOR_EVENT;
            window_redraw_request = 1;
            event_editor_previous_window = window;
        }
    }
}

static void set_single_value(scenario_value *v, int16_t value)
{
    v->fixed = value;
    v->min = -1;
    v->max = -1;
}

void event_list_new(void)
{
    scenario_event *e = scenario_event_add(&scenario_events, 1);
    if (e == 0) {
        return;
    }
    int16_t id = e->id;
    scenario_event_set_defaults(e);
    e->id = id;
    e->type = EVENT_TYPE_REQUEST;
    e->subtype = 0;
    e->flags = 0;
    set_single_value(&e->item, 1);
    set_single_value(&e->year, 1);
    set_single_value(&e->route, 1);
    set_single_value(&e->amount, NEW_EVENT_AMOUNT);
    e->months_initial = NEW_EVENT_MONTHS;
    e->on_completed = -1;
    e->on_refusal = -1;
    e->on_too_late = -1;
    e->on_defeat = -1;
}

void event_list_delete(void)
{
    events_delete(&scenario_events, scenario_events.editor_selected_id);
}

void event_editor_back(void)
{
    window_redraw_request = 1;
    window_current_id = (uint8_t) event_editor_previous_window;
}

static void draw_list_button(int32_t index, int32_t text)
{
    const int16_t *rect = event_list_rect(index);
    button_border_draw_rect(rect, event_button_focused(index), 0, 0);
    lang_text_draw_centered_rect(LANG_GROUP_EDITOR, text, rect, image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_LIST_TEXT_PADDING, EVENT_LIST_TEXT_PADDING);
}

static void draw_row(int32_t row)
{
    char line[EVENT_LIST_LINE_TEXT];
    const int16_t *rect = event_list_rect(row - 1);
    int32_t index = scenario_events.editor_scroll + row - 1;
    if (index < 0 || index >= scenario_events.count) {
        return;
    }
    scenario_event *e = &scenario_events.events[index];
    int32_t font = (scenario_events.editor_selected_id == e->id) ? image_group(IMAGE_GROUP_FONT_WHITE) : image_group(IMAGE_GROUP_FONT_GREEN);
    const char *name = event_type_name(e->type);
    const char *text = event_description(e);
    sprintf(line, "%2d %s %s", (int32_t) e->id, name, text);
    button_border_draw_rect(rect, focus_button_id == row, 0, 0);
    text_draw((const uint8_t *) line, rect[0] + ui_origin_x + EVENT_LIST_TEXT_PADDING, rect[1] + ui_origin_y + EVENT_LIST_TEXT_PADDING, font, 0);
}

void window_event_list_draw(void)
{
    outer_panel_draw(ui_origin_x, ui_origin_y, EVENT_LIST_PANEL_WIDTH, EVENT_LIST_PANEL_HEIGHT);
    window_ui_redraw = 1;
    draw_list_button(EVENT_LIST_BUTTON_ADD, EDITOR_TEXT_ADD_EVENT);
    draw_list_button(EVENT_LIST_BUTTON_DELETE, EDITOR_TEXT_DELETE_EVENT);
    for (int32_t row = 1; row <= EVENT_LIST_ROWS; row++) {
        draw_row(row);
    }
    image_buttons_draw(ui_origin_x, ui_origin_y, event_list_buttons[0].arrows, EVENT_LIST_ARROWS);
    event_list_scroll_dot();
}

const char *event_type_name(int32_t type)
{
    if (type < 0 || type >= EVENT_TYPE_COUNT) {
        return empty_text;
    }
    return lang_get_string(LANG_GROUP_EVENT_TYPES, type);
}

static void shift_link(int16_t *link, int32_t index)
{
    if (*link > index) {
        (*link)--;
    }
}

bool events_delete(scenario_events_t *self, int32_t index)
{
    if (index < 0 || index >= self->count) {
        return false;
    }
    for (int32_t i = index; i < self->count - 1; i++) {
        memcpy(&self->events[i], &self->events[i + 1], sizeof(scenario_event));
        self->events[i].id = (int16_t) i;
    }
    self->count--;
    for (int32_t i = 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        shift_link(&e->on_completed, index);
        shift_link(&e->on_refusal, index);
        shift_link(&e->on_too_late, index);
        shift_link(&e->on_defeat, index);
    }
    if (self->editor_scroll >= self->count) {
        self->editor_scroll--;
        if (self->editor_scroll < 0) {
            self->editor_scroll = 0;
        }
    }
    if (self->editor_selected_id >= self->count) {
        self->editor_selected_id = -1;
    }
    return true;
}
