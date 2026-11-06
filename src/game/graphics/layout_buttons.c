// Buttons under the interface layout overrides: the first draw of a window moves its generic buttons by the
// layout, later draws frame the focused button while the layout editor marks buttons.

#include "graphics/layout_buttons.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/panel_draw.h"
#include "graphics/ui_layout.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t layout_focus_index;

enum layout_highlight_panel {
    LAYOUT_HIGHLIGHT_PANEL_BLOCKS = 6,
};

void ui_layout_clear_highlight(void)
{
    outer_panel_draw(0, 0, LAYOUT_HIGHLIGHT_PANEL_BLOCKS, LAYOUT_HIGHLIGHT_PANEL_BLOCKS);
}

void ui_layout_apply_buttons(uint8_t *buttons, const ui_layout_entry *node)
{
    generic_button *button_table = (generic_button *) buttons;
    while (node) {
        generic_button *button = &button_table[node->key_x - 1];
        button->x = (int16_t) node->x;
        button->y = (int16_t) node->y;
        button->x_end = (int16_t) (node->width + node->x);
        button->y_end = (int16_t) (node->height + node->y);
        node = node->next;
    }
}

static void mark_focused_button(int32_t x, int32_t y, generic_button *button, int32_t focus)
{
    ui_layout_clear_highlight();
    layout_focus_button = &button->x;
    layout_focus_index = focus;
    ui_layout_marked_x = button->x;
    ui_layout_marked_y = button->y;
    ui_layout_marked_width = button->x_end - button->x;
    ui_layout_marked_key_x = focus;
    ui_layout_marked_height = button->y_end - button->y;
    ui_layout_marked_key_y = 0;
    button_border_draw(button->x + x, button->y + y, button->x_end - button->x, button->y_end - button->y, COLOR_WHITE);
}

int32_t ui_layout_buttons(int32_t x, int32_t y, void *buttons, int32_t count, int32_t focus)
{
    (void) count;
    ui_layout_window *window = &ui_layout_lists[ui_layout_window_key(window_current_id)];
    if (window->buttons_applied == 0) {
        ui_layout_apply_buttons(buttons, window->buttons);
        window->buttons_applied = 1;
        return 0;
    }
    if (focus == 0) {
        return 0;
    }
    if (ui_layout_edit_mode == UI_LAYOUT_BUTTON) {
        mark_focused_button(x, y, &((generic_button *) buttons)[focus - 1], focus);
    }
    return focus;
}
