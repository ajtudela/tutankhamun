#include "widget/touch_pause.h"
#include "city/message.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_present.h"
#include "graphics/graphics_primitives.h"
#include "input/input_data.h"
#include "widget/touch_construction.h"
#include "widget/widget_data.h"
#include "window/report_bug.h"
#include "window/window_data.h"

enum touch_pause_layout {
    PAUSE_WIDTH = 43,
    PAUSE_GAP = 6,
    BAR_WIDTH = 6,
    BAR_GAP = 6,
    ICON_HEIGHT = 18,
    TRIANGLE_WIDTH = 13,
};

static int32_t shown(void)
{
    return touch_construction_enabled && editor_active == 0 && g_sidebar_collapsed == 0;
}

static int32_t pause_box(int32_t *box)
{
    report_bug_sidebar_button_box(box);
    box[0] -= PAUSE_WIDTH + PAUSE_GAP;
    box[2] = PAUSE_WIDTH;
    return box[1] + box[3] <= g_screen_height;
}

static int32_t inside(const int32_t *box)
{
    return mouse_x >= box[0] && mouse_x < box[0] + box[2] && mouse_y >= box[1] && mouse_y < box[1] + box[3];
}

void touch_pause_button_draw(void)
{
    int32_t box[4];
    if (!shown() || !pause_box(box)) {
        return;
    }
    int32_t top = box[1] + (box[3] - ICON_HEIGHT) / 2;
    if (settings_data.paused) {
        // a play triangle: the tap resumes the game
        int32_t left = box[0] + (box[2] - TRIANGLE_WIDTH) / 2 + 1;
        for (int32_t row = 0; row < ICON_HEIGHT; row++) {
            int32_t reach = row < ICON_HEIGHT / 2 ? row : ICON_HEIGHT - 1 - row;
            graphics_draw_line(left, top + row, left + reach + reach / 2, top + row, COLOR_BLACK);
        }
    } else {
        int32_t left = box[0] + (box[2] - 2 * BAR_WIDTH - BAR_GAP) / 2;
        graphics_fill_rect(left, top, BAR_WIDTH, ICON_HEIGHT, COLOR_BLACK);
        graphics_fill_rect(left + BAR_WIDTH + BAR_GAP, top, BAR_WIDTH, ICON_HEIGHT, COLOR_BLACK);
    }
    button_border_draw(box[0], box[1], box[2], box[3], inside(box));
}

int32_t touch_pause_button_handle(void)
{
    int32_t box[4];
    if (!shown() || !pause_box(box) || !inside(box)) {
        return 0;
    }
    if (mouse_left_went_up) {
        settings_data.paused = settings_data.paused == 0;
        city_warning_clear_all();
    }
    return 1;
}
