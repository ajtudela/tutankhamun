// RGB565 colours the windows pass to the text and rectangle drawing (0 draws a text in its font's own colour),
// and the RGB555 value of one of them.

#ifndef GRAPHICS_COLOR_H
#define GRAPHICS_COLOR_H

enum color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 0x2BF,
    COLOR_GREY = 0x8410,
    COLOR_LIGHT_YELLOW = 0xFF3A,
    COLOR_LIGHT_YELLOW_RGB555 = 0x7F9A,
    COLOR_RED = 0xF800,
    COLOR_ORANGE = 0xFAC1,
    COLOR_WHITE = 0xFFFF,
};

#endif
