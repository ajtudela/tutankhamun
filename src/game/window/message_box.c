// The game's message box: a title, a text and one to three buttons chosen by the PLATFORM_BOX_* type, run by a modal
// message loop of its own; the answer is the PLATFORM_ANSWER_* of the pressed button.

#include "window/message_box.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_present.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "input/mouse.h"
#include "platform/platform_dialog.h"
#include "platform/platform_message.h"
#include "text/lang.h"
#include "window/window_draw.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include "window/window_input.h"
#include <stdint.h>

enum message_box_layout {
    MESSAGE_BOX_MARGIN = 20,
    MESSAGE_BOX_TOP_ABOVE_CENTER = 50,
    MESSAGE_BOX_HEIGHT_BLOCKS = 7,
    MESSAGE_BOX_BUTTONS_BELOW_CENTER = 20,
    MESSAGE_BOX_BUTTON_BOTTOM = 95,
    MESSAGE_BOX_MAX_BUTTONS = 3,
    MESSAGE_BOX_TYPE_MASK = 0xF,
    MESSAGE_BOX_NO_ANSWER = -1,
};

typedef struct {
    int32_t count;
    int32_t labels[MESSAGE_BOX_MAX_BUTTONS];
    int32_t answers[MESSAGE_BOX_MAX_BUTTONS];
} message_box_button_set;

static const message_box_button_set message_box_button_sets[] = {
    [PLATFORM_BOX_OK] = { 1, { LANG_OK }, { PLATFORM_ANSWER_OK } },
    [PLATFORM_BOX_OK_CANCEL] = { 2, { LANG_OK, LANG_CANCEL }, { PLATFORM_ANSWER_OK, PLATFORM_ANSWER_CANCEL } },
    [PLATFORM_BOX_ABORT_RETRY_IGNORE] = { 3, { LANG_ABORT, LANG_RETRY, LANG_IGNORE }, { PLATFORM_ANSWER_ABORT, PLATFORM_ANSWER_RETRY, PLATFORM_ANSWER_IGNORE } },
    [PLATFORM_BOX_YES_NO_CANCEL] = { 3, { LANG_YES, LANG_NO, LANG_CANCEL }, { PLATFORM_ANSWER_YES, PLATFORM_ANSWER_NO, PLATFORM_ANSWER_CANCEL } },
    [PLATFORM_BOX_YES_NO] = { 2, { LANG_YES, LANG_NO }, { PLATFORM_ANSWER_YES, PLATFORM_ANSWER_NO } },
    [PLATFORM_BOX_RETRY_CANCEL] = { 2, { LANG_RETRY, LANG_CANCEL }, { PLATFORM_ANSWER_RETRY, PLATFORM_ANSWER_CANCEL } },
};


void window_message_box_button(void)
{
    message_box_answered = 1;
    message_box_button = button_parameter;
}

static const message_box_button_set *message_box_buttons_of(uint32_t type)
{
    uint32_t kind = type & MESSAGE_BOX_TYPE_MASK;
    if (kind > PLATFORM_BOX_RETRY_CANCEL) {
        kind = PLATFORM_BOX_OK;
    }
    return &message_box_button_sets[kind];
}

int32_t window_message_box_draw(const char *title, const char *text, uint32_t type)
{
    const message_box_button_set *set = message_box_buttons_of(type);
    int32_t half_width = g_screen_width / 2;
    int32_t half_height = g_screen_height / 2;
    int32_t text_width = text_get_width((const uint8_t *) text, image_group(IMAGE_GROUP_FONT_BLACK));
    int32_t title_width = text_get_width((const uint8_t *) title, image_group(IMAGE_GROUP_FONT_LARGE));
    int32_t width = (title_width <= text_width) ? text_width : title_width;
    int32_t panel_width = width + MESSAGE_BOX_MARGIN;
    int32_t x = half_width - (panel_width / 2);
    int32_t blocks = panel_width / 16;
    int32_t top = half_height - MESSAGE_BOX_TOP_ABOVE_CENTER;
    outer_panel_draw(x, top, blocks + 1, MESSAGE_BOX_HEIGHT_BLOCKS);
    inner_panel_draw_ui(x + 8, top + 5, blocks, 2, ui_origin_x, ui_origin_y);
    message_box_button_count = set->count;
    text_draw_centered((const uint8_t *) title, x + 16, top + 10, panel_width, image_group(IMAGE_GROUP_FONT_LARGE_B), 0);
    text_draw_centered((const uint8_t *) text, x + 5, top + 48, panel_width - 10, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    int32_t y = half_height + MESSAGE_BOX_BUTTONS_BELOW_CENTER;
    int32_t button_width = panel_width / 3;
    int32_t button_height = top - y + MESSAGE_BOX_BUTTON_BOTTOM;
    int32_t button_x = half_width - ((button_width / 2) * set->count);
    for (int32_t i = 0; i < set->count; i++) {
        int32_t focus_color = (focus_button_id == (i + 1)) ? COLOR_RED : 0;
        lang_text_draw_centered_ui(LANG_GROUP_YES_NO, set->labels[i], button_x, y + 8, button_width, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
        button_border_draw_ui(button_x, y, button_width, button_height, focus_color, ui_origin_x, ui_origin_y);
        generic_button *button = &message_box_buttons[i];
        button->x = (int16_t) button_x;
        button->y = (int16_t) y;
        button->x_end = (int16_t) (button_x + button_width);
        button->y_end = (int16_t) (button_height + y);
        button_x += button_width;
    }
    if (message_box_answered == 0) {
        return MESSAGE_BOX_NO_ANSWER;
    }
    return set->answers[message_box_button];
}

int32_t window_message_box_run_frame(const char *title, const char *text, uint32_t type)
{
    mouse_update();
    window_draw_background_dispatch(0);
    int32_t result = window_message_box_draw(title, text, type);
    window_handle_input_dispatch();
    graphics_refresh_screen(0);
    mouse_clear_clicks();
    cursor_set(CURSOR_SHAPE_ARROW);
    return result;
}

int32_t window_message_box_show(const char *title, const char *text, uint32_t type)
{
    int32_t window = window_current_id;
    int32_t done = 0;
    int32_t result = 0;
    platform_message msg;
    window_current_id = WINDOW_MESSAGE_BOX;
    message_box_answered = 0;
    do {
        if (!platform_message_peek(&msg, 0, 0, false)) {
            if ((app_paused == 0) && (app_active != 0)) {
                result = window_message_box_run_frame(title, text, type);
                done = result != MESSAGE_BOX_NO_ANSWER;
            } else {
                platform_message_wait();
            }
        } else {
            if (!platform_message_get(&msg)) {
                break;
            }
            platform_message_dispatch(&msg);
        }
        if (game_quit_request == 1) {
            platform_message_post(platform_main_window, PLATFORM_MSG_CLOSE, 0, 0);
            game_quit_request = 0;
        }
    } while (!done);
    window_current_id = (uint8_t) window;
    window_redraw_request = 1;
    return result;
}
