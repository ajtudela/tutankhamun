// The message list over the city: up to ten messages at a time with icon, date and title, the scroll bar, the help and
// close buttons, and the sidebar's button that goes to the latest problem (a disaster or invasion of the last two
// years). Tutorial messages are kept when the read ones are deleted.

#include "window/message_list.h"
#include "city/city_data.h"
#include "city/message.h"
#include "core/calc.h"
#include "figure/formation_orders.h"
#include "game/game_data.h"
#include "graphics/arrow_buttons.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/generic_buttons.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "text/event_text.h"
#include "text/lang_message.h"
#include "text/text_data.h"
#include "widget/city_draw_view.h"
#include "widget/city_view.h"
#include "window/message_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t inner_height;

int32_t list_height;

int32_t list_width;

int32_t list_y;

int32_t list_x;

enum message_list_text {
    MESSAGE_LIST_TEXT_GROUP = 63,
    MESSAGE_LIST_TEXT_TITLE = 0,
    MESSAGE_LIST_TEXT_NO_MESSAGES = 1,
    MESSAGE_LIST_TEXT_DATE = 2,
    MESSAGE_LIST_TEXT_SUBJECT = 3,
    MESSAGE_LIST_TEXT_HINT = 4,
    MESSAGE_LIST_TEXT_DELETE_READ = 6,
    MESSAGE_LIST_MONTH_GROUP = 25,
    MESSAGE_LIST_TOOLTIP_HELP = 10,
    MESSAGE_LIST_TOOLTIP_CLOSE = 11,
    MESSAGE_LIST_TOOLTIP_SCROLL = 12,
};

enum message_list_layout {
    MESSAGE_LIST_ROW_HEIGHT = 20,
    MESSAGE_LIST_TITLE_LENGTH = 200,
    MESSAGE_LIST_BLOCK = 16,
    MESSAGE_ICON_UNREAD = 14,
    MESSAGE_ICON_READ = 15,
    MESSAGE_ICON_GENERAL = 0,
    MESSAGE_ICON_DISASTER = 2,
    MESSAGE_ICON_TUTORIAL = 4,
    MESSAGE_SCROLL_THUMB = 39,
    MESSAGE_PHRASE_HAS_PHRASES = 0,
    MESSAGE_PHRASE_EVENT_TEXT = 1,
    MESSAGE_SCROLL_UP = 0,
    MESSAGE_SCROLL_DOWN = 1,
    MESSAGE_GO_TO_GRID_LIMIT = 52000,
    CITY_MESSAGES = 1000,
};

static city_message *message_at(int32_t index)
{
    return &city_messages[index];
}

static int32_t message_kind(int32_t type)
{
    return lang_message_get(type)->message_type;
}

static int32_t list_bottom(void)
{
    return (list_height * MESSAGE_LIST_BLOCK) + list_y;
}

static int32_t delete_read_x(void)
{
    return ((list_width * MESSAGE_LIST_BLOCK) / 2) + list_x - 140;
}

static int32_t scroll_x(void)
{
    return (inner_width * MESSAGE_LIST_BLOCK) + inner_x;
}

static int32_t scroll_down_y(void)
{
    return (inner_height * MESSAGE_LIST_BLOCK) + inner_y - 26;
}

void window_message_list_draw_background(void)
{
    list_y = ui_origin_y + 32;
    list_x = ui_origin_x;
    inner_x = ui_origin_x + 16;
    inner_y = ui_origin_y + 102;
    list_width = 30;
    list_height = 22;
    inner_width = 26;
    inner_height = 13;
    widget_city_draw_current_view();
    outer_panel_draw(list_x, list_y, list_width, list_height);
    lang_text_draw_centered(MESSAGE_LIST_TEXT_GROUP, MESSAGE_LIST_TEXT_TITLE, list_x, list_y + 16, list_width * MESSAGE_LIST_BLOCK, image_group(IMAGE_GROUP_FONT_LARGE), COLOR_WHITE);
    inner_panel_draw(inner_x, inner_y, inner_width, inner_height);
    if (city_message_total <= 0) {
        lang_text_draw_multiline(MESSAGE_LIST_TEXT_GROUP, MESSAGE_LIST_TEXT_NO_MESSAGES, inner_x + 16, inner_y + 80, (inner_width - 3) * MESSAGE_LIST_BLOCK,
            image_group(IMAGE_GROUP_FONT_GREEN), COLOR_WHITE);
        return;
    }
    lang_text_draw(MESSAGE_LIST_TEXT_GROUP, MESSAGE_LIST_TEXT_DATE, inner_x + 42, inner_y - 18, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
    lang_text_draw(MESSAGE_LIST_TEXT_GROUP, MESSAGE_LIST_TEXT_SUBJECT, inner_x + 200, inner_y - 18, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
    lang_text_draw_multiline(MESSAGE_LIST_TEXT_GROUP, MESSAGE_LIST_TEXT_HINT, inner_x + 40, (inner_height * MESSAGE_LIST_BLOCK) + inner_y + 3,
        (inner_width * MESSAGE_LIST_BLOCK) - 50, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
    lang_text_draw_centered(MESSAGE_LIST_TEXT_GROUP, MESSAGE_LIST_TEXT_DELETE_READ, delete_read_x(), list_bottom() - 33, 280, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
}

static int32_t message_icon(int32_t type)
{
    int32_t kind = message_kind(type);
    if (kind == LANG_MESSAGE_TYPE_TUTORIAL) {
        return MESSAGE_ICON_TUTORIAL;
    }
    return kind == LANG_MESSAGE_TYPE_DISASTER ? MESSAGE_ICON_DISASTER : MESSAGE_ICON_GENERAL;
}

static void draw_message_row(int32_t row, int32_t index, int32_t y)
{
    char text[MESSAGE_LIST_TITLE_LENGTH];
    const city_message *message = message_at(city_message_list_scroll + row);
    city_message_shown_type = message->type;
    int32_t icon = message_icon(city_message_shown_type);
    if (message->is_read == 0 && icon != MESSAGE_ICON_TUTORIAL) {
        image_draw(image_group(IMAGE_GROUP_MESSAGE_ICONS) + icon + MESSAGE_ICON_UNREAD, inner_x + 8, y + inner_y + 6);
    } else {
        image_draw(image_group(IMAGE_GROUP_MESSAGE_ICONS) + icon + MESSAGE_ICON_READ, inner_x + 12, y + inner_y + 6);
    }
    int32_t font = focus_button_id == row + 1 ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_WHITE);
    g_text_width = 0;
    lang_text_draw(MESSAGE_LIST_MONTH_GROUP, message->month, inner_x + 42, y + inner_y + 8, font, COLOR_BLACK);
    lang_text_draw_year(0, message->year, inner_x + 90, y + inner_y + 8, font, COLOR_BLACK);
    const char *title = (const char *) (lang_message_data + lang_message_get(city_message_shown_type)->title_text);
    int32_t event_text = message->phrases[MESSAGE_PHRASE_EVENT_TEXT];
    if (event_text >= 0) {
        int32_t saved = city_message_shown_index;
        city_message_shown_index = index;
        event_text_format(event_text, text, MESSAGE_LIST_TITLE_LENGTH);
        city_message_shown_index = saved;
        title = text;
    }
    text_draw((const uint8_t *) title, inner_x + 200, y + inner_y + 8, font, COLOR_BLACK);
}

static void draw_scrollbar(void)
{
    image_buttons_draw(scroll_x(), inner_y, message_list_scroll_up.buttons, 1);
    image_buttons_draw(scroll_x(), scroll_down_y(), message_list_scroll_down.buttons, 1);
    int32_t percent;
    if (city_message_list_scroll <= 0) {
        percent = 0;
    } else if (city_message_list_scroll >= city_message_scroll_position) {
        percent = 100;
    } else {
        percent = calc_percentage(city_message_list_scroll, city_message_scroll_position);
    }
    int32_t offset = calc_adjust_with_percentage((inner_height * MESSAGE_LIST_BLOCK) - 77, percent);
    if (thumb_dragged != 0) {
        offset = thumb_y;
    }
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + MESSAGE_SCROLL_THUMB, scroll_x() + 9, inner_y + offset + 26);
}

void window_message_list_draw_messages(void)
{
    window_ui_redraw = 1;
    image_buttons_draw(list_x + 16, list_bottom() - 42, message_list_help.buttons, 1);
    image_buttons_draw((list_width * MESSAGE_LIST_BLOCK) + list_x - 38, list_bottom() - 36, message_list_close.buttons, 1);
    if (city_message_total <= 0) {
        return;
    }
    int32_t shown = city_message_total > MESSAGE_LIST_VISIBLE_ROWS ? MESSAGE_LIST_VISIBLE_ROWS : city_message_total;
    for (int32_t row = 0; row < shown; row++) {
        draw_message_row(row, city_message_list_scroll + row, row * MESSAGE_LIST_ROW_HEIGHT);
    }
    button_border_draw(delete_read_x(), list_bottom() - 40, 280, 28, focus_button_id_secondary != 0 ? COLOR_RED : 0);
    if (city_message_scroll_position > 0) {
        draw_scrollbar();
    }
}

static int32_t image_button_handled(int32_t x, int32_t y, void *buttons, int32_t tooltip)
{
    image_buttons_set_tooltip(x, y, buttons, 1, tooltip);
    return image_buttons_handle_mouse(x, y, buttons, 1) != 0;
}

int32_t message_list_handle_scrollbar(void)
{
    if (city_message_scroll_position <= 0 || mouse_left_down == 0) {
        return 0;
    }
    int32_t x = scroll_x();
    int32_t height = (inner_height * MESSAGE_LIST_BLOCK) - 52;
    if (mouse_x < (x + 1) || mouse_x > (x + 41)) {
        return 0;
    }
    if (mouse_y < (inner_y + 26) || mouse_y > (inner_y + 26 + height)) {
        return 0;
    }
    int32_t at = mouse_y - inner_y - 11;
    if (at > height) {
        at = height;
    }
    city_message_list_scroll = calc_adjust_with_percentage(city_message_scroll_position, calc_percentage(at, height));
    thumb_y = at - 25;
    thumb_dragged = 1;
    if (thumb_y < 0) {
        thumb_y = 0;
    }
    window_redraw_request = 1;
    return 1;
}

void window_message_list_handle_input(void)
{
    if (image_button_handled(list_x + 16, list_bottom() - 42, message_list_help.buttons, MESSAGE_LIST_TOOLTIP_HELP)) {
        return;
    }
    if (image_button_handled((list_width * MESSAGE_LIST_BLOCK) + list_x - 38, list_bottom() - 36, message_list_close.buttons, MESSAGE_LIST_TOOLTIP_CLOSE)) {
        return;
    }
    if (image_button_handled(scroll_x(), inner_y, message_list_scroll_up.buttons, MESSAGE_LIST_TOOLTIP_SCROLL)) {
        return;
    }
    if (image_button_handled(scroll_x(), scroll_down_y(), message_list_scroll_down.buttons, MESSAGE_LIST_TOOLTIP_SCROLL)) {
        return;
    }
    focus_button_id = generic_buttons_get_focus(inner_x, inner_y + 4, buttons_rows.rows, MESSAGE_LIST_VISIBLE_ROWS);
    if (generic_buttons_handle_mouse(inner_x, inner_y + 4, buttons_rows.rows, MESSAGE_LIST_VISIBLE_ROWS) != 0) {
        return;
    }
    focus_button_id_secondary = generic_buttons_get_focus(delete_read_x(), list_bottom() - 40, buttons_delete_read.buttons, 1);
    if (generic_buttons_handle_mouse(delete_read_x(), list_bottom() - 40, buttons_delete_read.buttons, 1) != 0) {
        return;
    }
    if (message_list_handle_scrollbar() != 0) {
        return;
    }
    if (mouse_right_went_up != 0) {
        window_current_id = WINDOW_CITY;
        window_redraw_request = 1;
    }
}

void message_list_scroll(void)
{
    if (button_parameter == MESSAGE_SCROLL_UP) {
        if (city_message_list_scroll > 0) {
            city_message_list_scroll--;
        }
    } else if (button_parameter == MESSAGE_SCROLL_DOWN && city_message_list_scroll < city_message_scroll_position) {
        city_message_list_scroll++;
    }
    thumb_dragged = 0;
    window_redraw_request = 1;
}

void message_list_button_help(void)
{
    window_message_dialog_show(MESSAGE_DIALOG_MESSAGES_HELP, 0, 0);
    window_redraw_request = 1;
}

void message_list_button_message(void)
{
    int32_t id = city_message_list_scroll + button_parameter;
    city_message_current_id = id;
    city_message_shown_index = id;
    if (id >= city_message_total) {
        return;
    }
    city_message *message = message_at(id);
    message->is_read = 1;
    dialog_month = message->month;
    dialog_year = message->year;
    dialog_param1 = message->param1;
    dialog_param2 = message->param2;
    city_message_shown_type = message->type;
    city_message_set_category(city_message_shown_type);
    bool has_phrases = message_at(city_message_current_id)->phrases[MESSAGE_PHRASE_HAS_PHRASES] >= 0;
    window_message_dialog_show(city_message_shown_type, 0, has_phrases);
}

static void message_list_clamp_scroll(void)
{
    if (city_message_list_scroll < 0) {
        city_message_list_scroll = 0;
    }
    window_redraw_request = 1;
}

void message_list_delete_read(void)
{
    for (int32_t i = 0; i < CITY_MESSAGES; i++) {
        city_message *message = message_at(i);
        if (message->type != 0 && message->is_read != 0 && message_kind(message->type) != LANG_MESSAGE_TYPE_TUTORIAL) {
            message->type = 0;
        }
    }
    city_message_sort_and_compact();
    if (city_message_list_scroll >= city_message_scroll_position) {
        city_message_list_scroll = city_message_scroll_position - 1;
    }
    message_list_clamp_scroll();
}

void message_list_delete_message(void)
{
    int32_t id = city_message_list_scroll + button_parameter;
    city_message_current_id = id;
    if (id >= city_message_total) {
        return;
    }
    message_at(id)->type = 0;
    city_message_sort_and_compact();
    if (city_message_list_scroll >= city_message_scroll_position) {
        city_message_list_scroll--;
    }
    message_list_clamp_scroll();
}

void message_dialog_show_help(void)
{
    window_message_dialog_close();
    window_message_dialog_show(MESSAGE_DIALOG_HELP, 0, 0);
}

void message_dialog_go_to(void)
{
    const city_message *message = message_at(city_message_shown_index);
    int32_t grid = message->param2;
    if (message_kind(dialog_text_id) == LANG_MESSAGE_TYPE_INVASION) {
        int32_t found = formation_grid_offset_for_invasion(message->param1);
        if (found > 0) {
            grid = found;
        }
    }
    if (grid > 0 && grid < MESSAGE_GO_TO_GRID_LIMIT) {
        city_view_go_to_grid_offset(grid);
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

static int32_t message_is_problem(const city_message *message, int32_t *kind)
{
    if (message->type == 0 || message->year < game_time_year - 1) {
        return 0;
    }
    *kind = message_kind(message->type);
    if (*kind == LANG_MESSAGE_TYPE_DISASTER) {
        return 1;
    }
    if (*kind != LANG_MESSAGE_TYPE_INVASION) {
        return 0;
    }
    return formation_grid_offset_for_invasion(message->param1) > 0 || message->param2 > 0;
}

void city_message_go_to_problem(void)
{
    city_message_sort_and_compact();
    city_message_problem_count = 0;
    int32_t kind;
    for (int32_t i = 0; i < CITY_MESSAGES; i++) {
        if (message_is_problem(message_at(i), &kind)) {
            city_message_problem_count++;
        }
    }
    if (city_message_problem_count <= 0) {
        city_message_problem_area_b = 0;
        return;
    }
    if (city_message_problem_area_b >= city_message_problem_count) {
        city_message_problem_area_b = 0;
    }
    int32_t n = 0;
    for (int32_t i = 0; i < CITY_MESSAGES; i++) {
        const city_message *message = message_at(i);
        if (!message_is_problem(message, &kind)) {
            continue;
        }
        n++;
        if (city_message_problem_area_b >= n) {
            continue;
        }
        int32_t grid = message->param2;
        city_message_problem_area_b++;
        if (kind == LANG_MESSAGE_TYPE_INVASION) {
            int32_t found = formation_grid_offset_for_invasion(message->param1);
            if (found > 0) {
                grid = found;
            } else if (grid <= 0) {
                continue;
            }
        }
        city_view_go_to_grid_offset(grid);
        window_current_id = WINDOW_CITY;
        window_redraw_request = 1;
        return;
    }
}

void window_message_list_show(void)
{
    city_message_sort_and_compact();
    window_current_id = WINDOW_MESSAGE_LIST;
    window_redraw_request = 1;
}

void button_message_help(void)
{
    window_message_dialog_show(button_parameter2, 0, 0);
    window_redraw_request = 1;
}
