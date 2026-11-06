// Types of the interface layout: a layout entry, the per-window lists of texts, buttons and frames, and the element
// kinds the layout editor marks.

#ifndef GRAPHICS_UI_LAYOUT_TYPES_H
#define GRAPHICS_UI_LAYOUT_TYPES_H

#include <stdint.h>

enum ui_layout_kind {
    UI_LAYOUT_TEXT = 1,
    UI_LAYOUT_BUTTON = 2,
    UI_LAYOUT_FRAME = 3,
    UI_LAYOUT_PANEL = 4,
};

enum ui_layout_size {
    UI_LAYOUT_KEYS = 120,
    UI_LAYOUT_COLUMNS = 6,
};

typedef struct ui_layout_entry {
    int32_t key_x;
    int32_t key_y;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    struct ui_layout_entry *next;
} ui_layout_entry;

typedef struct ui_layout_window {
    ui_layout_entry *texts;
    ui_layout_entry *buttons;
    ui_layout_entry *frames;
    int32_t buttons_applied;
} ui_layout_window;

typedef ui_layout_window ui_layout_window_table[UI_LAYOUT_KEYS];

#endif
