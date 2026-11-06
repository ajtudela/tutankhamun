// Generic (text) buttons: the button under the mouse and the input of a list of them (left and right click
// handlers with the button's parameters).
//
// Nothing sets generic_button_repeat_ready, so a held button fires only on a press or a click, not while the button
// is held.

#include "graphics/generic_buttons.h"
#include "graphics/button.h"
#include "input/input_data.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t generic_button_repeat_ready;

int32_t generic_button_repeat_ticks;

void button_none(void)
{
}

static const generic_button *generic_button_at(const void *buttons, int32_t index)
{
    return (const generic_button *) buttons + index;
}

static int32_t generic_button_is_under_mouse(const generic_button *button, int32_t x, int32_t y)
{
    return (button->x + x <= mouse_x) && (mouse_x < button->x_end + x) && (button->y + y <= mouse_y) && (mouse_y < button->y_end + y);
}

int32_t generic_buttons_get_focus(int32_t x, int32_t y, const void *buttons, int32_t count)
{
    const generic_button *button = buttons;
    for (int32_t i = 0; button && i < count; i++, button = generic_button_at(buttons, i)) {
        if (generic_button_is_under_mouse(button, x, y)) {
            mouse_x_ui = mouse_x;
            mouse_y_ui = mouse_y;
            return i + 1;
        }
    }
    return 0;
}

static int32_t held_button_ignores_mouse(void)
{
    if (!mouse_left_down) {
        return !mouse_left_went_down && !mouse_left_clicked && !mouse_right_down && !mouse_right_went_down && !mouse_right_clicked;
    }
    return !mouse_left_went_down && !mouse_left_clicked && !mouse_right_went_down && !mouse_right_clicked && (generic_button_repeat_ready == 0);
}

static void generic_button_press(generic_button *button)
{
    button_parameter = button->parameter1;
    button->pressed = 1;
    button_parameter2 = button->parameter2;
}

int32_t generic_buttons_handle_mouse(int32_t x, int32_t y, const void *buttons, int32_t count)
{
    generic_button *button = (generic_button *) buttons;
    for (int32_t i = 0; button && i < count; i++, button = (generic_button *) generic_button_at(buttons, i)) {
        if (!generic_button_is_under_mouse(button, x, y)) {
            continue;
        }
        int32_t invoke_left = 0;
        int32_t invoke_right = 0;
        if (button->type == GENERIC_BUTTON_PRESS) {
            if (!mouse_left_went_down && !mouse_right_went_down && !mouse_left_clicked) {
                continue;
            }
            generic_button_press(button);
            invoke_right = !mouse_left_went_down && mouse_right_went_down && !mouse_left_clicked;
            invoke_left = !invoke_right;
        } else if (button->type == GENERIC_BUTTON_RELEASE) {
            if (!mouse_left_went_up && !mouse_right_went_up) {
                continue;
            }
            generic_button_press(button);
            invoke_left = mouse_left_went_up;
            invoke_right = !invoke_left && mouse_right_went_up;
        } else if (button->type == GENERIC_BUTTON_LEFT_PRESS) {
            if (!mouse_left_went_down && !mouse_left_clicked) {
                continue;
            }
            generic_button_press(button);
            invoke_left = 1;
        } else if (button->type == GENERIC_BUTTON_HELD) {
            if (held_button_ignores_mouse()) {
                continue;
            }
            generic_button_repeat_ticks = 0;
            generic_button_press(button);
            invoke_left = mouse_left_down;
            invoke_right = !invoke_left && mouse_right_down;
        } else {
            continue;
        }
        if (invoke_left && button->left_click_handler) {
            button->left_click_handler();
        }
        if (invoke_right && button->right_click_handler) {
            button->right_click_handler();
        }
        return i + 1;
    }
    return 0;
}
