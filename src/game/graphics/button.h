// Button records of the windows: generic (text) buttons, image buttons and arrow buttons, their kinds and
// states. Window tables are arrays of them; a click handler takes no arguments and reads the button's
// parameters from the window parameter globals; button_none is the handler of a click that does nothing.

#ifndef GRAPHICS_BUTTON_H
#define GRAPHICS_BUTTON_H

#include <stdint.h>

enum scroll_arrow_parameter {
    SCROLL_ARROW_UP = 0,
    SCROLL_ARROW_DOWN = 1,
};

enum generic_button_type {
    GENERIC_BUTTON_PRESS = 1,
    GENERIC_BUTTON_HELD = 2,
    GENERIC_BUTTON_RELEASE = 3,
    GENERIC_BUTTON_LEFT_PRESS = 4,
};

enum image_button_type {
    IMAGE_BUTTON_RADIO = 2,
    IMAGE_BUTTON_ANIMATED = 4,
    IMAGE_BUTTON_REPEATING = 6,
};

enum image_button_image {
    IMAGE_BUTTON_IMAGE_FOCUSED = 1,
    IMAGE_BUTTON_IMAGE_PRESSED = 2,
    IMAGE_BUTTON_IMAGE_DISABLED = 3,
};

enum image_button_timing {
    IMAGE_BUTTON_PRESSED_FRAMES = 20,
    IMAGE_BUTTON_FOCUS_FRAMES = 2,
};

enum arrow_button_type {
    ARROW_BUTTON_TOGGLE = 2,
    ARROW_BUTTON_NEVER_PRESSED = 3,
    ARROW_BUTTON_REPEATING = 4,
    ARROW_BUTTON_DELAYED = 5,
};

typedef void (*button_handler)(void);

void button_none(void);

typedef struct {
    int16_t x;
    int16_t y;
    int16_t x_end;
    int16_t y_end;
    button_handler left_click_handler;
    button_handler right_click_handler;
    uint8_t reserved_1;
    uint8_t pressed;
    uint8_t reserved_2;
    uint8_t type;
    int32_t parameter1;
    int32_t parameter2;
} generic_button;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    int16_t type;
    int16_t image_group;
    int16_t image_offset;
    int16_t reserved_1;
    button_handler left_click_handler;
    button_handler right_click_handler;
    uint8_t enabled;
    uint8_t pressed;
    uint8_t focused;
    uint8_t animation;
    int32_t parameter1;
    int32_t parameter2;
} image_button;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t image_id;
    int16_t size;
    button_handler handler;
    uint8_t pressed;
    uint8_t held_countdown;
    uint8_t repeats;
    uint8_t type;
    int32_t parameter1;
    int32_t parameter2;
} arrow_button;

#endif
