// Arrow buttons: their draw, the press, hold and repeat of a held button, the focus test, the image button
// tooltip test and the city window's rotation button (its left, middle and right thirds pick the parameter).
//
// Callers depend on a few odd results here:
// - the tooltip test steps with the image button stride also when callers pass arrow buttons;
// - an arrow button under the mouse in the editor's map preview sets the editor arrows' tooltip id;
// - the focus test returns `base` on a hit, else the pointer past the last button (`count` with none).

#include "graphics/arrow_buttons.h"
#include "editor/editor_data.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/image_draw.h"
#include "input/input_data.h"
#include "platform/platform_time.h"
#include "widget/menu_bar.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_input.h"
#include <stdint.h>

uint32_t arrow_last_repeat_time;

enum arrow_button_timing {
    ARROW_REPEAT_INTERVAL_MS = 30,
    ARROW_PRESS_COUNTDOWN = 3,
    ARROW_TOGGLE_COUNTDOWN = 1,
    ARROW_DELAYED_COUNTDOWN = 10,
    ARROW_FIRST_PATTERN_REPEAT = 8,
    ARROW_REPEATS = 48,
};

enum button_focus_id {
    ROTATE_BUTTON_FOCUS_ID = 20,
    EDITOR_ARROW_FOCUS_ID = 21,
};

enum rotate_button_part {
    ROTATE_BUTTON_MIDDLE = 0,
    ROTATE_BUTTON_LEFT = 1,
    ROTATE_BUTTON_RIGHT = 2,
};

static void arrow_button_fire(arrow_button *button)
{
    button_parameter = button->parameter1;
    button_parameter2 = button->parameter2;
    button->handler();
}

static void count_down(arrow_button *buttons, int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        arrow_button *button = &buttons[i];
        if (button->held_countdown == 0) {
            button->repeats = 0;
        } else if (--button->held_countdown == 0) {
            button->pressed = 1;
            button->repeats = 0;
            if (button->type == ARROW_BUTTON_DELAYED) {
                arrow_button_fire(button);
            }
        }
        if (button->type == ARROW_BUTTON_NEVER_PRESSED) {
            button->pressed = 0;
        }
    }
}

static int32_t arrow_button_is_under_mouse(const arrow_button *button, int32_t x, int32_t y)
{
    int32_t left = button->x + x;
    int32_t top = button->y + y;
    return (left <= mouse_x) && (left + button->size > mouse_x) && (top <= mouse_y) && (top + button->size > mouse_y);
}

static int32_t handle_repeating(arrow_button *button, int32_t index, int32_t time_passed)
{
    if (mouse_left_went_down != 0 || mouse_left_clicked != 0) {
        button->pressed = 1;
        button->held_countdown = ARROW_PRESS_COUNTDOWN;
        button->repeats = 0;
        arrow_repeat_count = 0;
        arrow_button_fire(button);
        return index + 1;
    }
    if (mouse_left_down == 0) {
        return 0;
    }
    button->held_countdown = ARROW_PRESS_COUNTDOWN;
    if (!time_passed) {
        return 0;
    }
    uint8_t repeats = (uint8_t) (button->repeats + 1);
    button->repeats = repeats;
    if (repeats >= ARROW_REPEATS) {
        button->repeats = ARROW_REPEATS - 1;
    } else if (repeats < ARROW_FIRST_PATTERN_REPEAT || arrow_repeat_pattern[repeats - ARROW_FIRST_PATTERN_REPEAT] == 0) {
        return 0;
    }
    arrow_repeat_count = button->repeats;
    arrow_button_fire(button);
    return index + 1;
}

int32_t arrow_buttons_handle_mouse(int32_t x, int32_t y, void *buttons, int32_t count)
{
    uint32_t now = platform_time_ms();
    int32_t time_passed = 0;
    if ((int32_t) (now - arrow_last_repeat_time) >= ARROW_REPEAT_INTERVAL_MS) {
        time_passed = 1;
        arrow_last_repeat_time = now;
    }
    arrow_button *button_table = buttons;
    count_down(button_table, count);
    for (int32_t i = 0; i < count; i++) {
        arrow_button *button = &button_table[i];
        if (!arrow_button_is_under_mouse(button, x, y)) {
            continue;
        }
        if (editor_map_preview > 0 && window_current_id == WINDOW_CITY && editor_active == 1) {
            image_button_focus_id = EDITOR_ARROW_FOCUS_ID;
        }
        if (button->type == ARROW_BUTTON_REPEATING) {
            return handle_repeating(button, i, time_passed);
        }
        if (button->type == ARROW_BUTTON_TOGGLE) {
            button->held_countdown = ARROW_TOGGLE_COUNTDOWN;
            if (button->repeats == 0) {
                button->pressed ^= 1;
                arrow_button_fire(button);
            }
            return i + 1;
        }
        if (button->type == ARROW_BUTTON_DELAYED) {
            if (mouse_left_went_down == 0 && mouse_left_clicked == 0) {
                return 0;
            }
            button->held_countdown = ARROW_DELAYED_COUNTDOWN;
            button->pressed = 1;
            return i + 1;
        }
    }
    return 0;
}

void image_buttons_set_tooltip(int32_t x, int32_t y, const void *buttons, int32_t count, int32_t base)
{
    const image_button *button = buttons;
    for (int32_t i = 0; i < count; i++, button++) {
        int32_t left = button->x + x;
        int32_t top = button->y + y;
        if ((left <= mouse_x) && (left + button->width > mouse_x) && (top <= mouse_y) && (top + button->height > mouse_y)) {
            mouse_x_ui = mouse_x;
            mouse_y_ui = mouse_y;
            image_button_focus_id = i + base;
            return;
        }
    }
}

static int32_t rotate_button_part_under_mouse(const image_button *button)
{
    if (mouse_x < (button->width / 3) + button->x) {
        return ROTATE_BUTTON_LEFT;
    }
    if (mouse_x >= ((button->width * 2) / 3) + button->x) {
        return ROTATE_BUTTON_RIGHT;
    }
    return ROTATE_BUTTON_MIDDLE;
}

bool rotate_button_handle_mouse(void)
{
    int32_t saved_hover = image_button_focus_id;
    rotate_button_hovered_part = 0;
    image_button *button = &rotate_button_x[0];
    if (image_buttons_handle_mouse(0, 0, button, 1) == 0) {
        image_button_focus_id = saved_hover;
        return 0;
    }
    int32_t part = rotate_button_part_under_mouse(button);
    image_button_focus_id = ROTATE_BUTTON_FOCUS_ID;
    button_parameter = part;
    rotate_button_x[0].parameter1 = part;
    window_ui_redraw = 1;
    menu_bar_draw_side_button();
    rotate_button_hovered_part = button_parameter + 1;
    return 1;
}

static int32_t arrow_button_image(const arrow_button *button)
{
    if (button->type == ARROW_BUTTON_REPEATING || button->type == ARROW_BUTTON_DELAYED) {
        return (button->held_countdown != 0) ? (button->image_id - 1) : button->image_id;
    }
    return (button->pressed != 0) ? (button->image_id + 1) : button->image_id;
}

void arrow_buttons_draw(int32_t x, int32_t y, const void *buttons, int32_t count)
{
    const arrow_button *button = buttons;
    for (int32_t i = 0; i < count; i++, button++) {
        g_draw_x = button->x + x;
        g_draw_y = button->y + y;
        int32_t image = arrow_button_image(button);
        g_last_image_id = image;
        image_draw(image, g_draw_x, g_draw_y);
        if (button->held_countdown != 0) {
            g_force_refresh = 1;
        }
    }
}

int32_t arrow_buttons_get_focus(int32_t x, int32_t y, const void *buttons, int32_t count, int32_t base)
{
    if (count <= 0) {
        return count;
    }
    const arrow_button *button = buttons;
    for (int32_t i = 0; i < count; i++, button++) {
        if (arrow_button_is_under_mouse(button, x, y)) {
            mouse_x_ui = mouse_x;
            mouse_y_ui = mouse_y;
            arrow_button_focus_id = i + base;
            return base;
        }
    }
    return (int32_t) (uintptr_t) button;
}
