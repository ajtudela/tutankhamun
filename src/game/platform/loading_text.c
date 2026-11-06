// The text line of the loading screen (game_setup draws it before and after each frame of a stage): a white box with a
// black border in the frame buffer, and the text itself with GDI straight on the desktop's device context. The box is
// centred on the game screen, 10% of the free height below the middle (24 pixels higher when the game does not fill
// the desktop); the text is centred the same way on the desktop, which is where it lands only in full screen. An empty
// text draws nothing.

#include "platform/loading_text.h"

#include <stdint.h>
#include "core/text.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "platform/platform_data.h"
#include "platform/platform_window.h"

enum loading_text_box {
    BOX_MARGIN_X = 25,
    BOX_MARGIN_Y = 10,
    BOX_BORDER = 1,
    BOX_WINDOWED_RAISE = 24,
    BOX_BELOW_MIDDLE_PERCENT = 50,
    BOX_BORDER_COLOR = 0,
    BOX_COLOR = 0xFFFF,
    TEXT_BACKGROUND_COLOR = 0xFFFFFF,
};

static void fill_around_text(int32_t x, int32_t y, int32_t width, int32_t height, int32_t margin_x, int32_t margin_y, int32_t color)
{
    graphics_fill_rect(x - margin_x, y - margin_y, width + (margin_x * 2), height + (margin_y * 2), color);
}

void platform_draw_loading_text(const char *text)
{
    int32_t length = string_length(text);
    if (length <= 0) {
        return;
    }
    void *dc = platform_desktop_text_begin();
    if (!dc) {
        return;
    }
    int32_t width;
    int32_t height;
    platform_desktop_text_size(dc, text, length, &width, &height);
    int32_t text_x = (desktop_width - width) / 2;
    int32_t box_x = (g_screen_width - width) / 2;
    int32_t box_y = (g_screen_height - height) / 2;
    int32_t below = (box_y * BOX_BELOW_MIDDLE_PERCENT) / 100;
    int32_t text_y = ((desktop_height - height) / 2) + below;
    box_y += below;
    if (desktop_height != g_screen_height) {
        box_y -= BOX_WINDOWED_RAISE;
    }
    fill_around_text(box_x, box_y, width, height, BOX_MARGIN_X + BOX_BORDER, BOX_MARGIN_Y + BOX_BORDER, BOX_BORDER_COLOR);
    fill_around_text(box_x, box_y, width, height, BOX_MARGIN_X, BOX_MARGIN_Y, BOX_COLOR);
    platform_desktop_text_draw(dc, text_x, text_y, text, length, TEXT_BACKGROUND_COLOR);
    platform_desktop_text_end(dc);
}
