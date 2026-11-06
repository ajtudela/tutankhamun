// Drawing the scenario editor's event editor: the fields of the selected event as bordered buttons with their
// values, the focused one highlighted. Without a selected event the editor goes back; an event of type 0 is deleted.

#include "editor/editor_event_draw.h"
#include "editor/editor_events.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "text/lang.h"
#include "window/map_selection.h"
#include "window/window_image_group.h"

enum event_editor_layout {
    EVENT_EDITOR_PANEL_WIDTH = 38,
    EVENT_EDITOR_PANEL_HEIGHT = 25,
    EVENT_EDITOR_TITLE_X = 40,
    EVENT_EDITOR_TITLE_Y = 20,
    EVENT_EDITOR_YEAR_LABEL_X = 40,
    EVENT_EDITOR_YEAR_FROM_LABEL_Y = 100,
    EVENT_EDITOR_YEAR_TO_LABEL_Y = 173,
    EVENT_EDITOR_YEAR_LABEL_WIDTH = 100,
    EVENT_EDITOR_RANGE_LABEL_X = 339,
    EVENT_EDITOR_RANGE_TO_LABEL_X = 488,
    EVENT_EDITOR_CITIES_LABEL_X = 160,
    EVENT_EDITOR_CITIES_TO_LABEL_X = 384,
    EVENT_EDITOR_CITIES_Y = 104,
    EVENT_EDITOR_AMOUNT_Y = 148,
    EVENT_EDITOR_INVADER_Y = 186,
    EVENT_EDITOR_CARTLOAD_X = 290,
    EVENT_EDITOR_CARTLOAD_Y = 168,
    EVENT_EDITOR_PHRASE_LABEL_X = 388,
    EVENT_EDITOR_PHRASE_LABEL_Y = 218,
    EVENT_EDITOR_LINK_LABEL_X = 140,
    EVENT_EDITOR_LINK_TYPE_X = 410,
    EVENT_EDITOR_LINK_COMPLETED_Y = 248,
    EVENT_EDITOR_LINK_REFUSED_Y = 280,
    EVENT_EDITOR_LINK_TOO_LATE_Y = 312,
    EVENT_EDITOR_LINK_DEFEAT_Y = 344,
    EVENT_EDITOR_TEXT_X = 4,
    EVENT_EDITOR_TEXT_Y = 7,
    EVENT_EDITOR_DATE_Y = 24,
    EVENT_EDITOR_MONTHS_TEXT_Y = 4,
    EVENT_EDITOR_MONTHS_PER_YEAR = 12,
    EVENT_EDITOR_ATTACK_TARGETS = 5,
    EVENT_EDITOR_PHRASES = 5,
};

enum event_reason_text {
    EVENT_REASON_TEXT_FIRST = 7,
};

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

static void draw_text_button(int32_t has_border, int32_t button, int32_t group, int32_t text)
{
    button_border_draw_rect(event_editor_rect(button), event_button_focused(button), 0, 0);
    lang_text_draw_centered_rect(group, text, event_editor_rect(button), image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
}

static void draw_number_button(int32_t value, int32_t button)
{
    button_border_draw_rect(event_editor_rect(button), event_button_focused(button), 0, 0);
    text_draw_number_at_rect(value, NUMBER_PREFIX_NONE, postfix_none, event_editor_rect(button), image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
}

static void draw_range(const scenario_value *v, int32_t label, int32_t label_x, int32_t to_label_x, int32_t y, int32_t from_button)
{
    int16_t lo;
    int16_t hi;
    value_range(v, &lo, &hi);
    lang_text_draw(LANG_GROUP_EDITOR, label, ui_origin_x + label_x, ui_origin_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_number_button(lo, from_button);
    lang_text_draw(LANG_GROUP_EDITOR, EDITOR_TEXT_TO, ui_origin_x + to_label_x, ui_origin_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_number_button(hi, from_button + 1);
}

static void draw_link(scenario_event *e, int32_t label, int32_t y, int16_t link, int32_t link_slot, uint8_t reason)
{
    int32_t button = EVENT_BUTTON_LINK_FIRST + link_slot;
    int32_t reason_button = EVENT_BUTTON_REASON_FIRST + link_slot;
    lang_text_draw(LANG_GROUP_EDITOR, label, ui_origin_x + EVENT_EDITOR_LINK_LABEL_X, ui_origin_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw_rect(event_editor_rect(button), event_button_focused(button), 0, 0);
    text_draw_number_rect(link, NUMBER_PREFIX_NONE, postfix_none, event_editor_rect(button), image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
    int32_t group;
    int32_t index;
    if (!scenario_event_is_chain_target(e, link)) {
        group = LANG_GROUP_EDITOR;
        index = EDITOR_TEXT_INVALID_TRIGGER_EVENT;
    } else {
        group = LANG_GROUP_EVENT_TYPES;
        index = scenario_event_get(link)->type;
    }
    lang_text_draw(group, index, ui_origin_x + EVENT_EDITOR_LINK_TYPE_X, ui_origin_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_text_button(0, reason_button, LANG_GROUP_EVENT_REASONS, reason + EVENT_REASON_TEXT_FIRST);
}

static int32_t trigger_text(const scenario_event *e)
{
    if (e->flags & EVENT_FLAG_CHAIN_ONLY) {
        return EDITOR_TEXT_TRIGGERED_ONLY;
    }
    if (e->flags & EVENT_FLAG_RECURRING) {
        return EDITOR_TEXT_RECURRING_EVENT;
    }
    return (e->flags & EVENT_FLAG_BY_FAVOUR) ? EDITOR_TEXT_TRIGGERED_BY_FAVOR : EDITOR_TEXT_ONE_TIME_EVENT;
}

static void draw_year(const scenario_event *e, int32_t label_y, int32_t year, int32_t button)
{
    const int16_t *rect = event_editor_rect(button);
    lang_text_draw_centered(LANG_GROUP_EDITOR, button == EVENT_BUTTON_YEAR_FROM ? EDITOR_TEXT_BETWEEN : EDITOR_TEXT_AND,
        ui_origin_x + EVENT_EDITOR_YEAR_LABEL_X, ui_origin_y + label_y, EVENT_EDITOR_YEAR_LABEL_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    button_border_draw_rect(rect, event_button_focused(button), 0, 0);
    text_draw_number_at_rect(year, NUMBER_PREFIX_PLUS, postfix_none, rect, image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
    if (e->flags & EVENT_FLAG_CHAIN_ONLY) {
        lang_text_draw_centered_rect(LANG_GROUP_EDITOR, EDITOR_TEXT_MONTHS, rect, image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_DATE_Y);
    } else {
        lang_text_draw_year_at_rect(TEXT_YEAR_BC_CLOSER, scenario_data.start.year + year, rect, image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_DATE_Y);
    }
}

static void draw_date(const scenario_event *e)
{
    if (!(e->flags & EVENT_FLAG_CHAIN_ONLY)) {
        button_border_draw_rect(event_editor_rect(EVENT_BUTTON_MONTH), event_button_focused(EVENT_BUTTON_MONTH), 0, 0);
        if (e->month < EVENT_EDITOR_MONTHS_PER_YEAR) {
            lang_text_draw_centered_rect(
                LANG_GROUP_MONTHS_SHORT, e->month, event_editor_rect(EVENT_BUTTON_MONTH), image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
        }
    }
    int16_t lo;
    int16_t hi;
    value_range(&e->year, &lo, &hi);
    draw_year(e, EVENT_EDITOR_YEAR_FROM_LABEL_Y, lo, EVENT_BUTTON_YEAR_FROM);
    draw_year(e, EVENT_EDITOR_YEAR_TO_LABEL_Y, hi, EVENT_BUTTON_YEAR_TO);
}

static void draw_items(scenario_event *e)
{
    int16_t items[EVENT_ITEM_SLOTS];
    items[0] = e->item.fixed;
    items[1] = e->item.min;
    items[2] = e->item.max;
    for (int32_t i = 0; i < EVENT_ITEM_SLOTS; i++) {
        int32_t button = EVENT_BUTTON_ITEM_FIRST + i;
        button_border_draw_rect(event_editor_rect(button), event_button_focused(button), 0, 0);
    }
    if (event_resource_group(e) >= 0) {
        for (int32_t i = 0; i < EVENT_ITEM_SLOTS; i++) {
            int32_t item = items[i];
            if (scenario_event_resource_in_range(e, item)) {
                lang_text_draw_centered_rect(
                    event_resource_group(e), item, event_editor_rect(EVENT_BUTTON_ITEM_FIRST + i), image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
            }
        }
    }
    if (e->type == EVENT_TYPE_REQUEST) {
        lang_text_draw(
            LANG_GROUP_EDITOR, EDITOR_TEXT_CARTLOAD_IS_100_UNITS, ui_origin_x + EVENT_EDITOR_CARTLOAD_X, ui_origin_y + EVENT_EDITOR_CARTLOAD_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
}

static void draw_months(const scenario_event *e)
{
    const int16_t *rect = event_editor_rect(EVENT_BUTTON_MONTHS);
    button_border_draw_rect(rect, event_button_focused(EVENT_BUTTON_MONTHS), 0, 0);
    g_text_width = 0;
    text_draw_number_rect(e->months_initial, NUMBER_PREFIX_NONE, postfix_none, rect, image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X, EVENT_EDITOR_TEXT_Y);
    int32_t text = (e->months_initial == 1) ? EDITOR_TEXT_MONTH : EDITOR_TEXT_MONTHS;
    lang_text_draw_at_rect(LANG_GROUP_EDITOR, text, rect, image_group(IMAGE_GROUP_FONT_BLACK), 0, g_text_width + EVENT_EDITOR_TEXT_X, EVENT_EDITOR_MONTHS_TEXT_Y);
}

static void draw_phrase(scenario_event *e)
{
    if (event_has_phrase_number(e)) {
        lang_text_draw(
            LANG_GROUP_EDITOR, EDITOR_TEXT_WARSHIPS, ui_origin_x + EVENT_EDITOR_PHRASE_LABEL_X, ui_origin_y + EVENT_EDITOR_PHRASE_LABEL_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        draw_number_button(e->phrase_var, EVENT_BUTTON_PHRASE);
    }
    if (event_has_phrase_list(e)) {
        lang_text_draw(LANG_GROUP_EDITOR, EDITOR_TEXT_GOD, ui_origin_x + EVENT_EDITOR_PHRASE_LABEL_X, ui_origin_y + EVENT_EDITOR_PHRASE_LABEL_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        draw_text_button(0, EVENT_BUTTON_PHRASE, LANG_GROUP_GOD_NAMES, e->phrase_var);
    }
}

static void draw_links(scenario_event *e)
{
    if (scenario_event_has_chain(e)) {
        int32_t label = scenario_event_type_chains(e) ? EDITOR_TEXT_COMPLY_EVENT : EDITOR_TEXT_NEXT_EVENT;
        draw_link(e, label, EVENT_EDITOR_LINK_COMPLETED_Y, e->on_completed, EVENT_LINK_COMPLETED, e->reason[EVENT_LINK_COMPLETED]);
    }
    if (scenario_event_type_chains(e)) {
        draw_link(e, EDITOR_TEXT_REFUSE_EVENT, EVENT_EDITOR_LINK_REFUSED_Y, e->on_refusal, EVENT_LINK_REFUSED, e->reason[EVENT_LINK_REFUSED]);
    }
    if (scenario_event_type_chains(e)) {
        int32_t label = (e->type == EVENT_TYPE_GIFT_FROM_PHARAOH) ? EDITOR_TEXT_FORFEIT_EVENT : EDITOR_TEXT_TOO_LATE_EVENT;
        draw_link(e, label, EVENT_EDITOR_LINK_TOO_LATE_Y, e->on_too_late, EVENT_LINK_TOO_LATE, e->reason[EVENT_LINK_TOO_LATE]);
    }
    if (scenario_event_has_followup(e)) {
        draw_link(e, EDITOR_TEXT_LOSE_BATTLE_EVENT, EVENT_EDITOR_LINK_DEFEAT_Y, e->on_defeat, EVENT_LINK_DEFEAT, e->reason[EVENT_LINK_DEFEAT]);
    }
}

void window_event_editor_draw(void)
{
    scenario_event *e = event_editor_selected();
    if (e == 0) {
        event_editor_back();
        return;
    }
    if (e->type == EVENT_TYPE_NONE) {
        events_delete(&scenario_events, scenario_events.editor_selected_id);
        event_editor_back();
    }
    if (event_has_phrase_list(e) && e->phrase_var >= EVENT_EDITOR_PHRASES) {
        e->phrase_var = 0;
    }
    outer_panel_draw(ui_origin_x, ui_origin_y, EVENT_EDITOR_PANEL_WIDTH, EVENT_EDITOR_PANEL_HEIGHT);
    g_text_width = 0;
    lang_text_draw(LANG_GROUP_EDITOR, EDITOR_TEXT_EVENT, ui_origin_x + EVENT_EDITOR_TITLE_X, ui_origin_y + EVENT_EDITOR_TITLE_Y, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    text_draw_number(
        e->id, NUMBER_PREFIX_NONE, postfix_none, g_text_width + ui_origin_x + EVENT_EDITOR_TITLE_X, ui_origin_y + EVENT_EDITOR_TITLE_Y, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    draw_text_button(1, EVENT_BUTTON_TYPE, LANG_GROUP_EVENT_TYPES, e->type);
    if (scenario_event_type_chains(e)) {
        draw_text_button(1, EVENT_BUTTON_SENDER, LANG_GROUP_EDITOR, e->sender_faction ? EDITOR_TEXT_PHARAOH : EDITOR_TEXT_CITY);
    }
    draw_text_button(1, EVENT_BUTTON_TRIGGER, LANG_GROUP_EDITOR, trigger_text(e));
    if (event_has_subtype(e)) {
        draw_text_button(1, EVENT_BUTTON_SUBTYPE, event_subtype_group(e), e->subtype);
    }
    if (!(e->flags & EVENT_FLAG_BY_FAVOUR)) {
        draw_date(e);
    }
    if (scenario_event_type_uses_city(e)) {
        int32_t label = scenario_event_type_is_invasion(e) ? EDITOR_TEXT_FROM_MARKERS : EDITOR_TEXT_CITIES;
        draw_range(&e->route, label, EVENT_EDITOR_CITIES_LABEL_X, EVENT_EDITOR_CITIES_TO_LABEL_X, EVENT_EDITOR_CITIES_Y, EVENT_BUTTON_ROUTE_FROM);
    }
    if (event_has_amount(e)) {
        int32_t label = (e->type == EVENT_TYPE_EARTHQUAKE) ? EDITOR_TEXT_SCALE : EDITOR_TEXT_AMOUNT;
        draw_range(&e->amount, label, EVENT_EDITOR_RANGE_LABEL_X, EVENT_EDITOR_RANGE_TO_LABEL_X, EVENT_EDITOR_AMOUNT_Y, EVENT_BUTTON_AMOUNT_FROM);
    }
    if (event_is_invasion_kind(e)) {
        draw_range(
            &e->invasion, EDITOR_TEXT_ROUTE, EVENT_EDITOR_RANGE_LABEL_X, EVENT_EDITOR_RANGE_TO_LABEL_X, EVENT_EDITOR_INVADER_Y, EVENT_BUTTON_INVADER_FROM);
    }
    if (event_shows_resource(e)) {
        draw_items(e);
    }
    if (event_has_months(e)) {
        draw_months(e);
    }
    if (event_is_invasion_kind(e)) {
        button_border_draw_rect(event_editor_rect(EVENT_BUTTON_ATTACK_TARGET), event_button_focused(EVENT_BUTTON_ATTACK_TARGET), 0, 0);
        lang_text_draw_at_rect(LANG_GROUP_ATTACK_TARGETS, e->attack_target, event_editor_rect(EVENT_BUTTON_ATTACK_TARGET), image_group(IMAGE_GROUP_FONT_BLACK), 0, EVENT_EDITOR_TEXT_X,
            EVENT_EDITOR_TEXT_Y);
    }
    draw_phrase(e);
    draw_links(e);
}
