// The buttons of the scenario editor's event editor and the callbacks of the select list and numeric input they
// open, which write the chosen value into the edited event: type, subtype, month, year / amount / invader / route
// ranges, linked events, trigger, sender, months, phrase, attack target, reasons and items.
//
// Without a selected event the link test runs on a null event and faults; that path is left as it is.

#include "editor/editor_event_buttons.h"
#include "editor/editor_data.h"
#include "editor/editor_events.h"
#include "editor/editor_monuments.h"
#include "game/game_data.h"
#include "text/lang.h"
#include "window/input_dialogs.h"
#include "window/map_selection.h"
#include "window/window_data.h"
#include <stddef.h>

enum event_dialog_layout {
    EVENT_DIALOG_X = 200,
    EVENT_DIALOG_Y = 40,
    EVENT_TYPE_LIST_STEP = 75,
    EVENT_TYPE_LIST_Y_OFFSET = 50,
    EVENT_TYPE_LIST_ROWS = 15,
    EVENT_SUBTYPE_LIST_X = 417,
    EVENT_SUBTYPE_LIST_Y = 138,
    EVENT_REASON_LIST_X = 150,
    EVENT_REASON_LIST_Y = 200,
    EVENT_ITEM_LIST_STEP = 50,
    EVENT_ITEM_LIST_TWO_COLUMNS = 16,
    EVENT_LIST_ROWS_AUTO = -1,
};

enum event_editor_values {
    EVENT_LINK_NONE_TYPED = 999,
    EVENT_REASONS = 7,
    EVENT_ATTACK_TARGETS = 5,
    EVENT_MONTHS_PER_YEAR = 12,
};

static int32_t type_is_selectable(int32_t type, uint8_t mummy_allowed)
{
    if (type == EVENT_TYPE_NONE || type == EVENT_TYPE_EARTHQUAKE || type == EVENT_TYPE_REVOLT || type == EVENT_TYPE_CHANGE_PHARAOH) {
        return 0;
    }
    return (type != EVENT_TYPE_MUMMY) || mummy_allowed;
}

static void show_short_list(int32_t x, int32_t y, int32_t count, int32_t group, bool keep_items, input_dialog_callback on_choose, int32_t rows)
{
    editor_select_list_show(x, y, count, group, 0, 0, &scenario_events.editor_value, 0, 0, INPUT_TARGET_INT16, keep_items, on_choose, 0, rows, 0);
}

static void show_value_input(input_dialog_callback on_ok)
{
    numeric_input_show_int16(EVENT_DIALOG_X, EVENT_DIALOG_Y, &scenario_events.editor_value, on_ok);
}

void event_type_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    int16_t type = scenario_events.editor_value;
    if (type >= 0 && type < EVENT_TYPE_COUNT && e->type != (uint8_t) type) {
        event_set_type(e, type);
    }
}

void event_phrase_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    uint8_t phrase = scenario_events.editor_phrase;
    if (phrase < FESTIVAL_GODS) {
        e->phrase_var = phrase;
    }
}

void event_type_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    scenario_events.editor_value = e->type;
    uint8_t mummy_allowed = cheat_mummys_revenge_used;
    int32_t count = 0;
    int32_t *item = select_list.items;
    for (int32_t type = 0; type < EVENT_TYPE_COUNT; type++) {
        if (type_is_selectable(type, mummy_allowed)) {
            *item++ = type;
            count++;
        }
    }
    int32_t mode = settings_data.display_mode;
    show_short_list((mode * EVENT_TYPE_LIST_STEP) + EVENT_TYPE_LIST_STEP, (mode * EVENT_TYPE_LIST_STEP) - EVENT_TYPE_LIST_Y_OFFSET, count,
        LANG_GROUP_EVENT_TYPES, 1, event_type_chosen, EVENT_TYPE_LIST_ROWS);
    window_redraw_request = 1;
}

static int32_t first_resource_in_range(scenario_event *e)
{
    for (int32_t r = scenario_event_type_resource_min(e); r < scenario_event_type_resource_max(e); r++) {
        if (scenario_event_resource_in_range(e, r)) {
            return r;
        }
    }
    return 0;
}

void event_subtype_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0 || scenario_events.editor_value < 0) {
        return;
    }
    int16_t subtype = scenario_events.editor_value;
    if (subtype >= event_subtype_count(e)) {
        return;
    }
    if (e->subtype == (uint8_t) subtype) {
        return;
    }
    e->subtype = (uint8_t) subtype;
    if (!event_shows_resource(e)) {
        return;
    }
    int32_t first = first_resource_in_range(e);
    int16_t items[EVENT_ITEM_SLOTS];
    items[0] = e->item.fixed;
    items[1] = e->item.min;
    items[2] = e->item.max;
    for (int32_t i = 0; i < EVENT_ITEM_SLOTS; i++) {
        if (!scenario_event_resource_in_range(e, items[i])) {
            items[i] = (int16_t) ((i == 0) ? first : -1);
        }
    }
    scenario_value_store(&e->item, items[0], items[1], items[2]);
}

void event_subtype_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0 || !event_has_subtype(e)) {
        return;
    }
    scenario_events.editor_value = e->subtype;
    int32_t group = event_subtype_group(e);
    int32_t count = event_subtype_count(e);
    show_short_list(EVENT_SUBTYPE_LIST_X, EVENT_SUBTYPE_LIST_Y, count, group, 0, event_subtype_chosen, EVENT_LIST_ROWS_AUTO);
    window_redraw_request = 1;
}

void event_month_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e != 0) {
        e->month = (uint8_t) scenario_events.editor_value;
    }
}

void event_month_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0 || (e->flags & EVENT_FLAG_BY_FAVOUR)) {
        return;
    }
    show_short_list(EVENT_DIALOG_X, EVENT_DIALOG_Y, EVENT_MONTHS_PER_YEAR, LANG_GROUP_MONTHS_SHORT, 0, event_month_chosen, EVENT_LIST_ROWS_AUTO);
    window_redraw_request = 1;
}

static void set_single(scenario_value *v, int16_t value)
{
    v->fixed = value;
    v->min = -1;
    v->max = -1;
}

static void set_range(scenario_value *v, int16_t min, int16_t max)
{
    v->fixed = -1;
    v->min = min;
    v->max = max;
}

static void set_range_to(scenario_value *v, int16_t lo, int16_t typed)
{
    if (lo > typed) {
        lo = typed;
    }
    if (lo < 0 || lo == typed) {
        set_single(v, typed);
    } else if (typed < 0) {
        scenario_value_set(v, lo, -1, -1);
    } else if (lo > typed) {
        set_range(v, typed, lo);
    } else {
        set_range(v, lo, typed);
    }
}

static void set_range_from(scenario_value *v, int16_t hi, int16_t typed)
{
    if (hi < typed) {
        hi = typed;
    }
    if (typed < 0 || typed == hi) {
        set_single(v, hi);
    } else if (hi < 0) {
        set_single(v, typed);
    } else if (typed > hi) {
        set_range(v, hi, typed);
    } else {
        set_range(v, typed, hi);
    }
}

static void event_range_chosen(size_t value_offset)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    scenario_value *v = (scenario_value *) ((uint8_t *) e + value_offset);
    int16_t lo;
    int16_t hi;
    if (v->fixed < 0) {
        lo = v->min;
        hi = v->max;
    } else if (v->min < 0) {
        lo = v->fixed;
        hi = v->fixed;
    } else {
        lo = -1;
        hi = -1;
    }
    int32_t field = scenario_events.editor_field;
    int16_t typed = scenario_events.editor_value;
    if (field == EVENT_RANGE_TO) {
        set_range_to(v, lo, typed);
    } else if (field == EVENT_RANGE_FROM) {
        set_range_from(v, hi, typed);
    }
}

void event_year_chosen(void)
{
    event_range_chosen(offsetof(scenario_event, year));
}

void event_amount_chosen(void)
{
    event_range_chosen(offsetof(scenario_event, amount));
}

void event_invader_chosen(void)
{
    event_range_chosen(offsetof(scenario_event, invasion));
}

void event_route_chosen(void)
{
    event_range_chosen(offsetof(scenario_event, route));
}

static void event_range_button(int32_t is_year, input_dialog_callback on_ok)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    if (is_year && (e->flags & EVENT_FLAG_BY_FAVOUR)) {
        return;
    }
    scenario_events.editor_field = (int16_t) button_parameter;
    show_value_input(on_ok);
    window_redraw_request = 1;
}

void event_year_button(void)
{
    event_range_button(1, event_year_chosen);
}

void event_amount_button(void)
{
    event_range_button(0, event_amount_chosen);
}

void event_invader_button(void)
{
    event_range_button(0, event_invader_chosen);
}

void event_route_button(void)
{
    event_range_button(0, event_route_chosen);
}

void event_link_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    switch (scenario_events.editor_field) {
        case EVENT_LINK_COMPLETED:
            e->on_completed = scenario_events.editor_value;
            break;
        case EVENT_LINK_REFUSED:
            e->on_refusal = scenario_events.editor_value;
            break;
        case EVENT_LINK_TOO_LATE:
            e->on_too_late = scenario_events.editor_value;
            break;
        case EVENT_LINK_DEFEAT:
            e->on_defeat = scenario_events.editor_value;
            break;
    }
}

static int32_t event_has_link(scenario_event *e, int32_t link)
{
    switch (link) {
        case EVENT_LINK_COMPLETED:
            return scenario_event_has_chain(e);
        case EVENT_LINK_DEFEAT:
            return scenario_event_has_followup(e);
        default:
            return scenario_event_type_chains(e);
    }
}

void event_link_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    if (button_parameter < EVENT_LINK_COMPLETED || button_parameter > EVENT_LINK_DEFEAT || !event_has_link(e, button_parameter)) {
        return;
    }
    scenario_events.editor_field = (int16_t) button_parameter;
    scenario_events.editor_value = EVENT_LINK_NONE_TYPED;
    show_value_input(event_link_chosen);
    window_redraw_request = 1;
}

void event_trigger_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    uint32_t flags = e->flags;
    if (flags == 0) {
        e->flags = EVENT_FLAG_RECURRING;
    } else if (flags & EVENT_FLAG_RECURRING) {
        e->flags = EVENT_FLAG_CHAIN_ONLY;
    } else if (flags & EVENT_FLAG_CHAIN_ONLY) {
        e->flags = (e->type == EVENT_TYPE_INVASION) ? EVENT_FLAG_BY_FAVOUR : 0;
    } else {
        e->flags = 0;
    }
    window_redraw_request = 1;
}

void event_sender_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    e->sender_faction = e->sender_faction == 0;
    window_redraw_request = 1;
}

void event_months_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e != 0) {
        e->months_initial = scenario_events.editor_value;
    }
}

void event_months_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0 || !event_has_months(e)) {
        return;
    }
    show_value_input(event_months_chosen);
}

void event_phrase_number_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e != 0) {
        e->phrase_var = (uint8_t) scenario_events.editor_value;
    }
}

void event_phrase_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    if (event_has_phrase_number(e)) {
        show_value_input(event_phrase_number_chosen);
        return;
    }
    if (event_has_phrase_list(e)) {
        editor_select_list_show(EVENT_DIALOG_X, EVENT_DIALOG_Y, FESTIVAL_GODS, LANG_GROUP_GOD_NAMES, 0, &scenario_events.editor_phrase, 0, 0, 0,
            INPUT_TARGET_INT8, 0, event_phrase_chosen, 0, EVENT_LIST_ROWS_AUTO, 0);
    }
}

void event_attack_target_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0 || !event_is_invasion_kind(e)) {
        return;
    }
    e->attack_target++;
    if (e->attack_target >= EVENT_ATTACK_TARGETS) {
        e->attack_target = 0;
    }
    window_redraw_request = 1;
}

static void event_reason_chosen(int32_t link)
{
    scenario_event *e = event_editor_selected();
    if (e != 0) {
        e->reason[link] = (uint8_t) scenario_events.editor_value;
    }
}

void event_reason_completed_chosen(void)
{
    event_reason_chosen(EVENT_LINK_COMPLETED);
}

void event_reason_refused_chosen(void)
{
    event_reason_chosen(EVENT_LINK_REFUSED);
}

void event_reason_too_late_chosen(void)
{
    event_reason_chosen(EVENT_LINK_TOO_LATE);
}

void event_reason_defeat_chosen(void)
{
    event_reason_chosen(EVENT_LINK_DEFEAT);
}

static void event_reason_button(int32_t link, input_dialog_callback on_select)
{
    scenario_event *e = event_editor_selected();
    if (!event_has_link(e, link)) {
        return;
    }
    show_short_list(EVENT_REASON_LIST_X, EVENT_REASON_LIST_Y, EVENT_REASONS, LANG_GROUP_EVENT_REASONS, 0, on_select, EVENT_LIST_ROWS_AUTO);
}

void event_reason_completed_button(void)
{
    event_reason_button(EVENT_LINK_COMPLETED, event_reason_completed_chosen);
}

void event_reason_refused_button(void)
{
    event_reason_button(EVENT_LINK_REFUSED, event_reason_refused_chosen);
}

void event_reason_too_late_button(void)
{
    event_reason_button(EVENT_LINK_TOO_LATE, event_reason_too_late_chosen);
}

void event_reason_defeat_button(void)
{
    event_reason_button(EVENT_LINK_DEFEAT, event_reason_defeat_chosen);
}

void event_item_chosen(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    if (scenario_event_type_resource_min(e) > 0 && scenario_events.editor_value < scenario_event_type_resource_min(e)) {
        scenario_events.editor_value = -1;
    }
    int16_t field = scenario_events.editor_field;
    if (field < 0 || field >= EVENT_ITEM_SLOTS) {
        return;
    }
    int16_t items[EVENT_ITEM_SLOTS];
    items[0] = e->item.fixed;
    items[1] = e->item.min;
    items[2] = e->item.max;
    items[field] = scenario_events.editor_value;
    scenario_value_store(&e->item, items[0], items[1], items[2]);
}

void event_item_button(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        return;
    }
    int32_t count = event_resource_list(e);
    int32_t rows = (count >= EVENT_ITEM_LIST_TWO_COLUMNS) ? ((count + 1) / 2) : count;
    int32_t parameter = button_parameter;
    scenario_events.editor_field = (int16_t) parameter;
    if (count > 1) {
        int32_t group = event_resource_group(e);
        int32_t at = settings_data.display_mode * EVENT_ITEM_LIST_STEP;
        show_short_list(at, at, count, group, 1, event_item_chosen, rows);
        return;
    }
    if (count == 1) {
        scenario_events.editor_value = (int16_t) ((parameter == 0) ? select_list.items[0] : 0);
        event_item_chosen();
    }
}
