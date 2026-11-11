// The version line of the front menu, bottom left in the small font as in Akhenaten ("version_number" of
// ui_main_menu.js), and the update's line above it: the download with its percentage, the build ready with
// "Restart now" (lighter under the mouse; a click closes the game and starts the new program), or a failed download.
// The game's texts have none of these lines, so they are in the code. A new line asks for the whole menu to be drawn
// again, so the old text does not stay under it.

#include "window/main_menu_version.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "platform/updater.h"
#include "platform/version.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdio.h>

enum main_menu_version_layout {
    VERSION_X = 16,
    VERSION_BOTTOM = 20,
    STATUS_ABOVE = 16,
    RESTART_GAP = 8,
    LINK_HEIGHT = 12,
};

static const char restart_text[] = "Restart now";

static int32_t drawn_state = -1;
static int32_t drawn_progress = -1;

static int32_t version_y(void)
{
    return g_screen_height - VERSION_BOTTOM;
}

static int32_t status_y(void)
{
    return version_y() - STATUS_ABOVE;
}

static int32_t font(void)
{
    return image_group(IMAGE_GROUP_FONT_SMALL);
}

static void status_text(updater_state state, char *text, size_t size)
{
    text[0] = 0;
    switch (state) {
        case UPDATER_DOWNLOADING:
            snprintf(text, size, "Downloading the update 0.1.%d: %d%%", updater_new_build(), updater_progress_percent());
            break;
        case UPDATER_READY:
            snprintf(text, size, "The update 0.1.%d is ready, it is installed when the game closes.", updater_new_build());
            break;
        case UPDATER_FAILED:
            snprintf(text, size, "The update 0.1.%d could not be downloaded, see the log.", updater_new_build());
            break;
        default:
            break;
    }
}

static int32_t restart_left(void)
{
    char text[128];
    status_text(UPDATER_READY, text, sizeof(text));
    return VERSION_X + text_get_width((const uint8_t *) text, font()) + RESTART_GAP;
}

static int32_t restart_under_mouse(void)
{
    if (updater_current_state() != UPDATER_READY) {
        return 0;
    }
    int32_t left = restart_left();
    int32_t width = text_get_width((const uint8_t *) restart_text, font());
    return mouse_x >= left && mouse_x < left + width && mouse_y >= status_y() - 2 && mouse_y < status_y() + LINK_HEIGHT;
}

void main_menu_version_draw(void)
{
    updater_state state = updater_current_state();
    int32_t progress = state == UPDATER_DOWNLOADING ? updater_progress_percent() : 0;
    if (drawn_state != (int32_t) state || drawn_progress != progress) {
        if (drawn_state >= 0) {
            window_redraw_request = WINDOW_REDRAW_ALL;
        }
        drawn_state = (int32_t) state;
        drawn_progress = progress;
    }
    text_draw((const uint8_t *) version_text(), VERSION_X, version_y(), font(), COLOR_WHITE);
    char text[128];
    status_text(state, text, sizeof(text));
    if (!text[0]) {
        return;
    }
    text_draw((const uint8_t *) text, VERSION_X, status_y(), font(), COLOR_WHITE);
    if (state == UPDATER_READY) {
        int32_t color = COLOR_WHITE;
        if (restart_under_mouse()) {
            color = display_mode == DISPLAY_MODE_RGB565 ? COLOR_LIGHT_YELLOW : COLOR_LIGHT_YELLOW_RGB555;
        }
        text_draw((const uint8_t *) restart_text, restart_left(), status_y(), font(), color);
    }
}

int32_t main_menu_version_handle_mouse(void)
{
    if (!mouse_left_went_up || !restart_under_mouse()) {
        return 0;
    }
    updater_restart_request();
    game_quit_request = 1;
    return 1;
}
