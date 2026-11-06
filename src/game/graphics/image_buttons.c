// Image buttons: their draw (normal, focused, pressed, disabled), the input of a list of them, the pressed
// state that decays after a few frames and the radio buttons cleared.
//
// Two results callers rely on:
// - whether the input ignores the mouse depends on the type of the record after the last button, which belongs to the
//   next table in memory;
// - the draw returns its loop counter (0 after the buttons, `count` when there are none).

#include "graphics/image_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "input/input_data.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include <stdint.h>

enum image_button_animation {
    IMAGE_BUTTON_ANIMATION_STATE_BITS = 2,
    IMAGE_BUTTON_ANIMATION_STATE_MASK = 3,
    IMAGE_BUTTON_ANIMATION_PRESSED_STATE = 2,
    IMAGE_BUTTON_ANIMATION_PHASES = 0x10,
    IMAGE_BUTTON_ANIMATION_PHASE_STEP = 0x3F,
};

static image_button *image_button_at(image_button *buttons, int32_t index)
{
    return buttons + index;
}

static int16_t type_after_last_button(const image_button *buttons, int32_t count)
{
    int32_t records = (count > 0) ? count : 0;
    return buttons[records].type;
}

void image_buttons_decay(image_button *buttons, int32_t count)
{
    image_button *button = buttons;
    for (int32_t i = 0; button && i < count; i++, button++) {
        int32_t decays = (button->type == IMAGE_BUTTON_ANIMATED) || (button->type == IMAGE_BUTTON_REPEATING);
        if (button->pressed && decays && (--button->pressed == 0)) {
            window_ui_redraw = 1;
        }
    }
}

void image_buttons_clear_radio(image_button *buttons, int32_t count)
{
    image_button *button = buttons;
    for (int32_t i = 0; button && i < count; i++, button++) {
        if (button->pressed && (button->type == IMAGE_BUTTON_RADIO)) {
            button->pressed = 0;
            window_ui_redraw = 1;
        }
    }
}

static int32_t image_button_is_under_mouse(const image_button *button, int32_t x, int32_t y)
{
    return (mouse_x >= button->x + x) && (mouse_x < button->x + button->width + x) && (mouse_y >= button->y + y) &&
        (mouse_y < button->y + button->height + y);
}

static int32_t update_focus(void *buttons, int32_t x, int32_t y, int32_t count)
{
    int32_t redraw = 0;
    for (int32_t i = 0; buttons && i < count; i++) {
        image_button *button = image_button_at(buttons, i);
        if ((button->focused != 0) && (--button->focused == 0)) {
            redraw = 1;
        }
        if (button->enabled != 0 && image_button_is_under_mouse(button, x, y)) {
            mouse_x_ui = mouse_x;
            mouse_y_ui = mouse_y;
            if (button->focused == 0) {
                redraw = 1;
            }
            button->focused = IMAGE_BUTTON_FOCUS_FRAMES;
        }
    }
    return redraw;
}

static int32_t mouse_ignored_by_type(int16_t tail_type)
{
    if ((tail_type == IMAGE_BUTTON_RADIO || tail_type == IMAGE_BUTTON_ANIMATED) && !mouse_left_went_down && !mouse_right_went_up) {
        return 1;
    }
    return (tail_type == IMAGE_BUTTON_REPEATING) && !mouse_left_went_down && !mouse_left_down;
}

static int32_t click_button(void *buttons, image_button *button, int32_t index, int32_t count)
{
    if (button->type == IMAGE_BUTTON_RADIO) {
        image_buttons_clear_radio(buttons, count);
    }
    button_parameter = button->parameter1;
    button_parameter2 = button->parameter2;
    if (mouse_left_went_down) {
        button->pressed = IMAGE_BUTTON_PRESSED_FRAMES;
        button->animation = 0;
        if (button->left_click_handler) {
            button->left_click_handler();
        }
        return index + 1;
    }
    if (mouse_right_went_up) {
        if (button->right_click_handler) {
            button->right_click_handler();
        }
        return index + 1;
    }
    if (button->type == IMAGE_BUTTON_REPEATING && mouse_left_down) {
        button->pressed = IMAGE_BUTTON_PRESSED_FRAMES;
        if (button->left_click_handler) {
            button->left_click_handler();
        }
    }
    return index + 1;
}

int32_t image_buttons_handle_mouse(int32_t x, int32_t y, void *buttons, int32_t count)
{
    image_buttons_decay(buttons, count);
    int32_t redraw = update_focus(buttons, x, y, count);
    int16_t tail_type = type_after_last_button(buttons, count);
    if (mouse_ignored_by_type(tail_type)) {
        return redraw;
    }
    for (int32_t i = 0; buttons && i < count; i++) {
        image_button *button = image_button_at(buttons, i);
        if (button->enabled != 0 && button->focused != 0) {
            return click_button(buttons, button, i, count);
        }
    }
    return redraw;
}

static int32_t animated_image_step(image_button *button)
{
    uint8_t phase = button->animation >> IMAGE_BUTTON_ANIMATION_STATE_BITS;
    uint8_t state = button->animation & IMAGE_BUTTON_ANIMATION_STATE_MASK;
    if (phase == 0) {
        state = (state != IMAGE_BUTTON_ANIMATION_PRESSED_STATE) + 1;
        phase = IMAGE_BUTTON_ANIMATION_PHASES;
    }
    button->animation = (uint8_t) (((phase + IMAGE_BUTTON_ANIMATION_PHASE_STEP) << IMAGE_BUTTON_ANIMATION_STATE_BITS) | state);
    return state - 1;
}

static int32_t image_button_state_offset(image_button *button)
{
    if (!button->enabled) {
        return IMAGE_BUTTON_IMAGE_DISABLED;
    }
    if (button->animation != 0) {
        return animated_image_step(button);
    }
    if (button->pressed == 0) {
        return button->focused ? IMAGE_BUTTON_IMAGE_FOCUSED : 0;
    }
    if (button->pressed == IMAGE_BUTTON_PRESSED_FRAMES) {
        g_force_refresh = 1;
    }
    return IMAGE_BUTTON_IMAGE_PRESSED;
}

int32_t image_buttons_draw(int32_t x, int32_t y, void *buttons, int32_t count)
{
    image_button *button = buttons;
    for (int32_t i = 0; button && i < count; i++, button++) {
        g_draw_x = button->x + x;
        g_draw_y = button->y + y;
        int32_t image = image_group(button->image_group) + button->image_offset;
        g_last_image_id = image;
        if (window_ui_redraw || window_redraw_request || button->animation) {
            image += image_button_state_offset(button);
            g_last_image_id = image;
            image_draw(image, g_draw_x, g_draw_y);
        }
    }
    return (count > 0) ? 0 : count;
}
