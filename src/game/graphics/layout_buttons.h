// The functions of layout_buttons.c that other files call.

#ifndef GRAPHICS_LAYOUT_BUTTONS_H
#define GRAPHICS_LAYOUT_BUTTONS_H

#include "graphics/ui_layout_types.h"
#include <stdint.h>

void ui_layout_clear_highlight(void);
void ui_layout_apply_buttons(uint8_t *buttons, const ui_layout_entry *node);
int32_t ui_layout_buttons(int32_t x, int32_t y, void *buttons, int32_t count, int32_t focus);

#endif
