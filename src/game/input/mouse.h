// The functions of mouse.c, the cursor slot of each shape and the bits of mouse_buttons the window procedure sets.

#ifndef INPUT_MOUSE_H
#define INPUT_MOUSE_H

#include <stdint.h>

enum cursor_shape {
    CURSOR_SHAPE_ARROW = 0,
    CURSOR_SHAPE_LEGION_MOVE = 1,
    CURSOR_SHAPE_CLEAR_LAND = 2,
    CURSOR_SHAPE_SHIP_MOVE = 3,
    CURSOR_SHAPE_SHIP_LAND = 4,
    CURSOR_SHAPE_SHIP_EMBARK = 5,
    CURSOR_SHAPE_TRANSPORT_DOCKED = 6,
    CURSOR_SHAPE_WARSHIP_DOCKED = 7,
    CURSOR_SHAPE_INFANTRY_ATTACK = 8,
    CURSOR_SHAPE_ATTACK = 9,
    CURSOR_SHAPE_LEGION_ORDER = 10,
    CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_1 = 11,
    CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_2 = 12,
    CURSOR_SHAPE_LAYOUT_SINGLE_LINE_1 = 13,
    CURSOR_SHAPE_LAYOUT_SINGLE_LINE_2 = 14,
    CURSOR_SHAPE_LAYOUT_CHARGE = 15,
    CURSOR_SHAPE_TRANSPORT_EVADING = 16,
    CURSOR_SHAPE_WARSHIP_HUNTING = 17,
    CURSOR_SHAPE_SHIP_BLOCKED = 18,
};

enum cursor_limits {
    CURSOR_SHAPES = 19,
};

enum cursor_slot {
    CURSOR_SLOT_TRANSPORT_TO_LANDING = 13,
    CURSOR_SLOT_TRANSPORT_TO_LEGION = 14,
};

typedef void *cursor_handle_table[CURSOR_SHAPES];

extern const uint8_t cursor_slot_of_shape[CURSOR_SHAPES];

enum mouse_button_bit {
    MOUSE_BUTTON_LEFT = 1,
    MOUSE_BUTTON_RIGHT = 2,
};

void cursor_set(int32_t shape);
void mouse_update(void);
void mouse_clear_clicks(void);
void debug_draw_network_info(void);

#endif
