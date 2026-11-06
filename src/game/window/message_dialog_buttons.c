// The buttons of the message dialog: the scroll arrows (up or down within the text, the links cleared, a full redraw)
// and back (the previous message of the history at its scroll position).

#include "window/message_dialog_buttons.h"
#include "graphics/button.h"
#include "graphics/rich_text.h"
#include "window/window_data.h"
#include <stdint.h>

void message_dialog_scroll_arrow(void)
{
    if (button_parameter == SCROLL_ARROW_UP) {
        if (rich_scroll_pos > 0) {
            rich_scroll_pos = rich_scroll_pos - 1;
        }
    } else if (button_parameter == SCROLL_ARROW_DOWN) {
        if (rich_scroll_pos < dialog_scroll_max) {
            rich_scroll_pos = rich_scroll_pos + 1;
        }
    }
    rich_text_clear_links();
    dialog_scroll_dragging = 0;
    window_redraw_request = 2;
}

void message_dialog_back(void)
{
    int32_t count = dialog_history_count;
    if (count <= 0) {
        return;
    }
    int32_t last = count - 1;
    dialog_text_id = dialog_history_id[last];
    int32_t scroll = dialog_history_scroll[last];
    dialog_history_count = last;
    rich_scroll_pos = scroll;
    rich_text_clear_links();
    rich_line_count = 0;
    dialog_scroll_dragging = 0;
    window_redraw_request = 1;
}
