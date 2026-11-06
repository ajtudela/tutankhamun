// The mouse state of a frame: mouse_update reads the cursor, scales it from the client rectangle to the game's
// screen and turns the button bits of the window procedure into the edges the windows act on (a press and release
// between two frames is not seen); a drag starts once the mouse leaves a 50-pixel box. Also the cursor of each shape
// and the "Network info (F2)" debug panel.

#include "input/mouse.h"

#include "core/calc.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/panel_blocks.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "platform/platform_input.h"
#include "sound/sound_play.h"
#include "widget/widget_data.h"
#include "window/window_image_group.h"

#include <stdint.h>

uint8_t mouse_left_down_prev;

int32_t mouse_y_prev;

int32_t mouse_x_prev;

int32_t mouse_drag_start_y;

int32_t mouse_drag_start_x;

enum mouse_drag {
    MOUSE_DRAG_PRESSED = 1,
    MOUSE_DRAG_MOVED = 2,
    MOUSE_DRAG_DISTANCE = 50,
};

void mouse_clear_clicks(void)
{
    mouse_left_clicked = 0;
    mouse_right_clicked = 0;
}

static int32_t mouse_left_drag_box(void)
{
    int32_t dx = mouse_x - mouse_drag_start_x;
    int32_t dy = mouse_y - mouse_drag_start_y;
    return dx > MOUSE_DRAG_DISTANCE || dx < -MOUSE_DRAG_DISTANCE || dy > MOUSE_DRAG_DISTANCE || dy < -MOUSE_DRAG_DISTANCE;
}

void mouse_update(void)
{
    int32_t x;
    int32_t y;
    platform_cursor_position(&x, &y);
    cursor_raw_x = x;
    cursor_raw_y = y;
    if (scroll_in_progress != 0) {
        mouse_window_x = (int16_t) x;
        mouse_window_y = (int16_t) y;
    }
    if (mouse_drag_state == MOUSE_DRAG_PRESSED && mouse_left_drag_box()) {
        mouse_drag_state = MOUSE_DRAG_MOVED;
    }
    mouse_x_prev = mouse_x;
    mouse_y_prev = mouse_y;
    mouse_left_down_prev = mouse_left_down;
    mouse_right_down_prev = mouse_right_down;
    int32_t percent_x = calc_percentage(display_width, window_screen_rect.right - window_screen_rect.left);
    int32_t percent_y = calc_percentage(screen_height, window_screen_rect.bottom - window_screen_rect.top);
    mouse_x = calc_adjust_with_percentage(mouse_window_x, percent_x);
    mouse_y = calc_adjust_with_percentage(mouse_window_y, percent_y);
    mouse_left_down = (mouse_buttons & MOUSE_BUTTON_LEFT) != 0;
    mouse_right_down = (mouse_buttons & MOUSE_BUTTON_RIGHT) != 0;
    mouse_moved = mouse_x != mouse_x_prev || mouse_y != mouse_y_prev;
    mouse_left_went_down = 0;
    mouse_left_went_up = 0;
    mouse_right_went_down = 0;
    mouse_right_went_up = 0;
    if (mouse_left_down != mouse_left_down_prev) {
        mouse_moved = 1;
        if (mouse_left_down) {
            mouse_left_went_down = 1;
            sound_effect_play(0);
        } else {
            mouse_left_went_up = 1;
        }
    }
    if (mouse_right_down != mouse_right_down_prev) {
        mouse_moved = 1;
        if (mouse_right_down) {
            mouse_right_went_down = 1;
        } else {
            mouse_right_went_up = 1;
        }
    }
}

const uint8_t cursor_slot_of_shape[CURSOR_SHAPES] = {
    [CURSOR_SHAPE_ARROW] = 18,
    [CURSOR_SHAPE_LEGION_MOVE] = 10,
    [CURSOR_SHAPE_CLEAR_LAND] = 16,
    [CURSOR_SHAPE_SHIP_MOVE] = 15,
    [CURSOR_SHAPE_SHIP_LAND] = CURSOR_SLOT_TRANSPORT_TO_LANDING,
    [CURSOR_SHAPE_SHIP_EMBARK] = CURSOR_SLOT_TRANSPORT_TO_LEGION,
    [CURSOR_SHAPE_TRANSPORT_DOCKED] = 1,
    [CURSOR_SHAPE_WARSHIP_DOCKED] = 11,
    [CURSOR_SHAPE_INFANTRY_ATTACK] = 17,
    [CURSOR_SHAPE_ATTACK] = 9,
    [CURSOR_SHAPE_LEGION_ORDER] = 8,
    [CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_1] = 6,
    [CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_2] = 7,
    [CURSOR_SHAPE_LAYOUT_SINGLE_LINE_1] = 4,
    [CURSOR_SHAPE_LAYOUT_SINGLE_LINE_2] = 5,
    [CURSOR_SHAPE_LAYOUT_CHARGE] = 3,
    [CURSOR_SHAPE_TRANSPORT_EVADING] = 2,
    [CURSOR_SHAPE_WARSHIP_HUNTING] = 12,
    [CURSOR_SHAPE_SHIP_BLOCKED] = 0,
};

void cursor_set(int32_t shape)
{
    void *cursor = cursor_handles[cursor_slot_of_shape[(uint32_t) shape >= CURSOR_SHAPES ? CURSOR_SHAPE_ARROW : shape]];
    cursor_current = cursor;
    platform_cursor_set(cursor);
}


void debug_draw_network_info(void)
{
    if (debug_network_info_on == 0) {
        return;
    }
    int32_t y = ui_origin_y * 2 + 220;
    panel_draw_blocks(6, 0, y, 12, 15, 1);
    text_draw_centered((const uint8_t *) network_info_label, 0, y + 6, 192, image_group(IMAGE_GROUP_FONT_BLACK), 0xFFFF);
}
