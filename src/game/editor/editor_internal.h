// The scenario editor's shared header: the colours of the editor's texts and frames (green: a point is set, red:
// missing, orange: the tool's name and the focused row, grey: a tool without a brush) and the number prefixes it draws.

#ifndef EDITOR_EDITOR_INTERNAL_H
#define EDITOR_EDITOR_INTERNAL_H

#include "graphics/text_draw.h"
#include <stdint.h>

enum editor_color {
    EDITOR_COLOR_GREEN = 0x7E0,
    EDITOR_COLOR_GREY = 0x4208,
    EDITOR_COLOR_RED = 0xF800,
    EDITOR_COLOR_ORANGE = 0xFAC1,
};

#endif
