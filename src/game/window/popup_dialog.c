// The popup dialog: a title and its explanation from the popup texts, with OK and cancel or the hint to right click,
// running the caller's callback and returning to the window below; and the cheat input window, whose Enter applies
// the cheat (the first time with the "cheats enabled" warning).

#include "window/popup_dialog.h"
#include "city/message.h"
#include "game/cheats.h"
#include "game/game_data.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/generic_buttons.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "text/lang.h"
#include "window/text_input.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum popup_dialog_layout {
    POPUP_DIALOG_OFFSET = 80,
    POPUP_DIALOG_WIDTH_BLOCKS = 30,
    POPUP_DIALOG_HEIGHT_BLOCKS = 10,
    POPUP_DIALOG_WIDTH = POPUP_DIALOG_WIDTH_BLOCKS * 16,
    POPUP_DIALOG_TEXT_Y = 60,
    POPUP_DIALOG_TEXT_X = 30,
    POPUP_DIALOG_TEXT_WIDTH = 420,
};

enum cheat_input_layout {
    CHEAT_BUTTON_X = 50,
    CHEAT_BUTTON_Y = 246,
    CHEAT_TEXT_X = 58,
    CHEAT_TEXT_Y = 216,
};

enum cheat_input_text {
    CHEAT_TEXT_DISABLE_CHEATS = 1,
};

static void popup_dialog_run_callback(void)
{
    popup.callback();
}

static void popup_dialog_close(void)
{
    window_redraw_request = 1;
    window_current_id = (uint8_t) popup.return_window;
}

void window_popup_dialog_draw(void)
{
    outer_panel_draw(popup.x, popup.y, POPUP_DIALOG_WIDTH_BLOCKS, POPUP_DIALOG_HEIGHT_BLOCKS);
    lang_text_draw_centered(LANG_GROUP_POPUP, popup.text, popup.x, popup.y + 20, POPUP_DIALOG_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    int32_t explanation = popup.text + 1;
    if (lang_text_get_width(LANG_GROUP_POPUP, explanation, image_group(IMAGE_GROUP_FONT_BLACK)) < POPUP_DIALOG_TEXT_WIDTH) {
        lang_text_draw_centered(LANG_GROUP_POPUP, explanation, popup.x, popup.y + POPUP_DIALOG_TEXT_Y, POPUP_DIALOG_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        return;
    }
    lang_text_draw_multiline(
        LANG_GROUP_POPUP, explanation, popup.x + POPUP_DIALOG_TEXT_X, popup.y + POPUP_DIALOG_TEXT_Y, POPUP_DIALOG_TEXT_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

void window_popup_dialog_draw_foreground(void)
{
    if (popup.button_count == 0) {
        lang_text_draw_centered(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, popup.x, popup.y + 128, POPUP_DIALOG_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        return;
    }
    image_buttons_draw(popup.x, popup.y, popup_buttons, popup.button_count);
}

void window_popup_dialog_show(int32_t text, popup_callback callback, int32_t buttons)
{
    popup.return_window = window_current_id;
    popup.x = ui_origin_x + POPUP_DIALOG_OFFSET;
    popup.y = ui_origin_y + POPUP_DIALOG_OFFSET;
    window_current_id = WINDOW_POPUP_DIALOG;
    popup_answer = 0;
    popup.callback = callback;
    popup.text = text;
    popup.button_count = buttons;
    window_redraw_request = 1;
}

void window_popup_dialog_handle(void)
{
    window_ui_redraw = 1;
    if (image_buttons_handle_mouse(popup.x, popup.y, popup_buttons, popup.button_count) != 0) {
        return;
    }
    if ((popup.button_count != 0) || (mouse_right_went_up == 0)) {
        return;
    }
    popup_dialog_run_callback();
    popup_dialog_close();
}

void popup_dialog_button_ok(void)
{
    popup_answer = 1;
    popup_dialog_run_callback();
    popup_dialog_close();
}

void popup_dialog_button_cancel(void)
{
    popup_answer = 0;
    popup_dialog_run_callback();
    popup_dialog_close();
}

void window_cheat_input_draw(void)
{
    outer_panel_draw(ui_origin_x + 10, ui_origin_y + 160, 24, 8);
    int32_t focused = cheat_focus == 1;
    button_border_draw(ui_origin_x + CHEAT_BUTTON_X, ui_origin_y + CHEAT_BUTTON_Y, 150, 30, focused);
    lang_text_draw(LANG_GROUP_CHEATS, CHEAT_TEXT_DISABLE_CHEATS, ui_origin_x + 55, ui_origin_y + 256, image_group(IMAGE_GROUP_FONT_BLACK), focused);
    inner_panel_draw(ui_origin_x + 42, ui_origin_y + 208, 20, 2);
    text_cursor_reset();
    text_draw((const uint8_t *) cheat_input_text, ui_origin_x + CHEAT_TEXT_X, ui_origin_y + CHEAT_TEXT_Y, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    graphics_draw_text_cursor(ui_origin_x + CHEAT_TEXT_X, ui_origin_y + CHEAT_TEXT_Y + 1, COLOR_WHITE);
}

void window_cheat_input_handle(void)
{
    keyboard_capture_set(KEYBOARD_FIELD_CHEAT);
    cheat_focus = generic_buttons_get_focus(ui_origin_x + CHEAT_BUTTON_X, ui_origin_y + CHEAT_BUTTON_Y, &cheat_buttons.close_button, 1);
    if (generic_buttons_handle_mouse(ui_origin_x + CHEAT_BUTTON_X, ui_origin_y + CHEAT_BUTTON_Y, &cheat_buttons.close_button, 1) != 0) {
        return;
    }
    if (keyboard_accepted == 0) {
        return;
    }
    if (cheat_input_enabled == 0) {
        city_warning_show(WARNING_CHEATS_ENABLED);
    }
    cheat_input_enabled = 1;
    game_cheat_apply_input();
}

void window_cheat_input_show(void)
{
    keyboard_accepted = 0;
    cheat_input_previous_window = window_current_id;
    window_current_id = WINDOW_CHEAT_INPUT;
}

void window_go_cheat_input(void)
{
    keyboard_accepted = 0;
    window_current_id = WINDOW_NAME_INPUT;
    window_redraw_request = 1;
}
