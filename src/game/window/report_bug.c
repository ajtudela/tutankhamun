// The bug report form (report_bug.h). Its two text fields are keyboard capture slots 8 and 9 of the game's own text
// input (window/text_input.c: letters, digits, space, "-" and ", . ? !"); the description is drawn wrapped over the
// lines of its box with the cursor on its line. The game's texts have none of the form's words, so they are in the
// code. Right click, Esc or "Cancel" closes it; the result of the sending stays shown until it closes.

#include "window/report_bug.h"
#include "graphics/button.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/image_draw.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/screenshot_png.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "input/keyboard_capture.h"
#include "platform/report.h"
#include "widget/sidebar_buttons.h"
#include "widget/widget_data.h"
#include "window/text_input.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int32_t g_cursor_x;

enum report_bug_layout {
    FORM_WIDTH_BLOCKS = 32,
    FORM_HEIGHT_BLOCKS = 22,
    FORM_WIDTH = FORM_WIDTH_BLOCKS * 16,
    FORM_HEIGHT = FORM_HEIGHT_BLOCKS * 16,
    MARGIN = 16,
    KIND_Y = 48,
    KIND_WIDTH = 100,
    BUTTON_HEIGHT = 26,
    TITLE_LABEL_Y = 84,
    TITLE_BOX_Y = 100,
    BOX_BLOCKS = 30,
    BOX_WIDTH = BOX_BLOCKS * 16,
    TEXT_INSET = 8,
    TEXT_WIDTH = BOX_WIDTH - 2 * TEXT_INSET,
    TITLE_BOX_BLOCKS = 2,
    BODY_LABEL_Y = 140,
    BODY_BOX_Y = 156,
    BODY_BOX_BLOCKS = 8,
    BODY_LINES = 7,
    LINE_HEIGHT = 16,
    STATUS_Y = 292,
    ACTION_Y = 314,
    ACTION_WIDTH = 160,
    CURSOR_WIDTH_AT_END = 6,
};

enum report_bug_fields {
    FIELD_TITLE = 8,
    FIELD_BODY = 9,
    TITLE_MAX = 80,
    BODY_MAX = 400,
    BODY_MIN = 10,
    BODY_CAPTURE_WIDTH = 100000,
};

// under the mission briefing's button (window_data.c: x 116, y 404, 43 x 45), a little to the left of it, framed like the game's buttons
enum report_bug_sidebar {
    SIDEBAR_BUTTON_X = 111,
    SIDEBAR_BUTTON_Y = 453,
    SIDEBAR_BUTTON_MIN_WIDTH = 43,
    SIDEBAR_BUTTON_PADDING = 0,
    SIDEBAR_BUTTON_GROUP = 378,
};

static const char text_heading[] = "Report a bug or an idea";
static const char text_bug[] = "Bug";
static const char text_idea[] = "Idea";
static const char text_title[] = "Title";
static const char text_body[] = "What happened, or what would you like?";
static const char text_send[] = "Send";
static const char text_cancel[] = "Cancel";
static const char text_need_title[] = "Please enter a title.";
static const char text_need_body[] = "Please write a description of at least 10 characters.";
static const char text_sending[] = "Sending...";
static const char text_sent[] = "Sent, thank you! The screenshot and the log went with it.";
static const char text_failed[] = "Could not send it";

static struct {
    char title[TITLE_MAX + 2];
    char body[BODY_MAX + 2];
    int32_t idea;
    int32_t return_window;
    uint8_t *shot;
    size_t shot_size;
    const char *status;
    int32_t status_is_result;
} form;

// ---- the sidebar's button ----

static int32_t sidebar_button_shown(void)
{
    return report_supported() && g_sidebar_collapsed == 0;
}

static int32_t inside(int32_t x, int32_t y, int32_t width, int32_t height);

// the frame around the icon: x, y, width, height on the screen
static void sidebar_button_box(int32_t *box)
{
    const image_header *header = image_header_get(image_group(SIDEBAR_BUTTON_GROUP));
    int32_t width = header->width + 2 * SIDEBAR_BUTTON_PADDING;
    box[0] = city_view_width_expanded + SIDEBAR_BUTTON_X;
    box[1] = SIDEBAR_TOP + SIDEBAR_BUTTON_Y;
    box[2] = width < SIDEBAR_BUTTON_MIN_WIDTH ? SIDEBAR_BUTTON_MIN_WIDTH : width;
    box[3] = header->height + 2 * SIDEBAR_BUTTON_PADDING;
}

void report_bug_sidebar_button_draw(void)
{
    if (!sidebar_button_shown()) {
        return;
    }
    int32_t box[4];
    sidebar_button_box(box);
    if (box[1] + box[3] > g_screen_height) {
        return;
    }
    const image_header *header = image_header_get(image_group(SIDEBAR_BUTTON_GROUP));
    image_draw(image_group(SIDEBAR_BUTTON_GROUP), box[0] + (box[2] - header->width) / 2, box[1] + SIDEBAR_BUTTON_PADDING);
    button_border_draw(box[0], box[1], box[2], box[3], inside(box[0], box[1], box[2], box[3]));
}

int32_t report_bug_sidebar_button_handle(void)
{
    if (!sidebar_button_shown()) {
        return 0;
    }
    int32_t box[4];
    sidebar_button_box(box);
    if (box[1] + box[3] > g_screen_height || !inside(box[0], box[1], box[2], box[3])) {
        return 0;
    }
    if (mouse_left_went_up) {
        report_bug_show();
    }
    return 1;
}

// ---- the form ----

static int32_t form_x(void)
{
    return (g_screen_width - FORM_WIDTH) / 2;
}

static int32_t form_y(void)
{
    return (g_screen_height - FORM_HEIGHT) / 2;
}

static int32_t inside(int32_t x, int32_t y, int32_t width, int32_t height)
{
    return mouse_x >= x && mouse_x < x + width && mouse_y >= y && mouse_y < y + height;
}

static int32_t font_white(void)
{
    return image_group(IMAGE_GROUP_FONT_WHITE);
}

static int32_t font_black(void)
{
    return image_group(IMAGE_GROUP_FONT_BLACK);
}

static void focus_field(int32_t field)
{
    keyboard_capture_set(field);
    keyboard_end();
}

void report_bug_show(void)
{
    free(form.shot);
    form.shot = 0;
    form.shot_size = 0;
    // the screen still shows the city: the screenshot goes with the report
    graphics_screen_png(&form.shot, &form.shot_size);
    form.return_window = window_current_id;
    if (form.status_is_result || !form.status) {
        form.status = 0;
        form.status_is_result = 0;
    }
    report_forget();
    keyboard_start_capture(FIELD_TITLE, form.title, TITLE_MAX, TEXT_WIDTH, 0, font_white());
    keyboard_start_capture(FIELD_BODY, form.body, BODY_MAX, BODY_CAPTURE_WIDTH, 0, font_white());
    focus_field(FIELD_TITLE);
    keyboard_accepted = 0;
    window_current_id = WINDOW_REPORT_BUG;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void report_bug_close(void)
{
    free(form.shot);
    form.shot = 0;
    form.shot_size = 0;
    keyboard_field_index = 0;
    window_current_id = (uint8_t) form.return_window;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static void send(void)
{
    if (report_current_state() == REPORT_SENDING) {
        return;
    }
    if (!form.title[0]) {
        form.status = text_need_title;
        focus_field(FIELD_TITLE);
        return;
    }
    if (strlen(form.body) < BODY_MIN) {
        form.status = text_need_body;
        focus_field(FIELD_BODY);
        return;
    }
    uint8_t *shot = form.shot;
    form.shot = 0;
    if (report_send(form.idea ? "idea" : "bug", form.title, form.body, shot, form.shot_size)) {
        form.status = text_sending;
    } else {
        form.status = text_failed;
    }
    form.status_is_result = 1;
}

// the end of the body's line that starts at start: the last space that fits, or as many letters as fit
static int32_t line_end(const char *text, int32_t start)
{
    char line[BODY_MAX + 2];
    int32_t length = (int32_t) strlen(text);
    int32_t last_space = -1;
    int32_t end = start;
    while (end < length) {
        int32_t n = end - start + 1;
        memcpy(line, text + start, (size_t) n);
        line[n] = 0;
        if (text_get_width((const uint8_t *) line, font_white()) > TEXT_WIDTH) {
            break;
        }
        if (text[end] == ' ') {
            last_space = end;
        }
        end++;
    }
    if (end < length && last_space >= start) {
        return last_space + 1;
    }
    return end > start ? end : start + 1;
}

static int32_t width_of(const char *text, int32_t from, int32_t to)
{
    char part[BODY_MAX + 2];
    int32_t n = to - from;
    if (n <= 0) {
        return 0;
    }
    memcpy(part, text + from, (size_t) n);
    part[n] = 0;
    return text_get_width((const uint8_t *) part, font_white());
}

static void draw_body(int32_t x, int32_t y)
{
    int32_t length = (int32_t) strlen(form.body);
    int32_t cursor = capture_cursor_position[FIELD_BODY];
    int32_t start = 0;
    for (int32_t line = 0; line < BODY_LINES; line++) {
        int32_t end = start < length ? line_end(form.body, start) : start;
        char text[BODY_MAX + 2];
        memcpy(text, form.body + start, (size_t) (end - start));
        text[end - start] = 0;
        int32_t line_y = y + line * LINE_HEIGHT;
        text_draw((const uint8_t *) text, x, line_y, font_white(), 0);
        int32_t last_line = end >= length;
        if (keyboard_field_index == FIELD_BODY && cursor >= start && (cursor < end || last_line)) {
            g_cursor_x = width_of(form.body, start, cursor);
            text_cursor_y = 0;
            g_cursor_width = CURSOR_WIDTH_AT_END;
            graphics_draw_text_cursor(x, line_y + 1, COLOR_WHITE);
        }
        if (last_line) {
            break;
        }
        start = end;
    }
}

static void draw_button(int32_t x, int32_t y, int32_t width, const char *label, int32_t selected)
{
    int32_t hovered = inside(x, y, width, BUTTON_HEIGHT);
    button_border_draw(x, y, width, BUTTON_HEIGHT, hovered || selected);
    int32_t font = selected ? image_group(IMAGE_GROUP_FONT_RED) : font_black();
    text_draw_centered((const uint8_t *) label, x, y + 8, width, font, 0);
}

void report_bug_draw(void)
{
    int32_t x = form_x();
    int32_t y = form_y();
    outer_panel_draw(x, y, FORM_WIDTH_BLOCKS, FORM_HEIGHT_BLOCKS);
    text_draw_centered((const uint8_t *) text_heading, x, y + 16, FORM_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0);

    draw_button(x + MARGIN, y + KIND_Y, KIND_WIDTH, text_bug, !form.idea);
    draw_button(x + MARGIN + KIND_WIDTH + 8, y + KIND_Y, KIND_WIDTH, text_idea, form.idea);

    text_draw((const uint8_t *) text_title, x + MARGIN, y + TITLE_LABEL_Y, font_black(), 0);
    inner_panel_draw(x + MARGIN, y + TITLE_BOX_Y, BOX_BLOCKS, TITLE_BOX_BLOCKS);
    text_cursor_reset();
    text_draw((const uint8_t *) form.title, x + MARGIN + TEXT_INSET, y + TITLE_BOX_Y + TEXT_INSET + 2, font_white(), 0);
    if (keyboard_field_index == FIELD_TITLE) {
        graphics_draw_text_cursor(x + MARGIN + TEXT_INSET, y + TITLE_BOX_Y + TEXT_INSET + 3, COLOR_WHITE);
    }

    text_draw((const uint8_t *) text_body, x + MARGIN, y + BODY_LABEL_Y, font_black(), 0);
    inner_panel_draw(x + MARGIN, y + BODY_BOX_Y, BOX_BLOCKS, BODY_BOX_BLOCKS);
    draw_body(x + MARGIN + TEXT_INSET, y + BODY_BOX_Y + TEXT_INSET);

    if (form.status) {
        char status[160];
        if (form.status == text_failed && report_failed_status() > 0) {
            snprintf(status, sizeof(status), "%s (the server answered %d).", text_failed, report_failed_status());
        } else if (form.status == text_failed) {
            snprintf(status, sizeof(status), "%s: no answer from the server.", text_failed);
        } else {
            snprintf(status, sizeof(status), "%s", form.status);
        }
        text_draw_centered((const uint8_t *) status, x, y + STATUS_Y, FORM_WIDTH, font_black(), 0);
    }
    draw_button(x + MARGIN, y + ACTION_Y, ACTION_WIDTH, text_send, 0);
    draw_button(x + FORM_WIDTH - MARGIN - ACTION_WIDTH, y + ACTION_Y, ACTION_WIDTH, text_cancel, 0);
}

static void follow_sending(void)
{
    if (!form.status_is_result) {
        return;
    }
    report_state state = report_current_state();
    if (state == REPORT_SENT && form.status != text_sent) {
        form.status = text_sent;
        form.title[0] = 0;
        form.body[0] = 0;
        focus_field(FIELD_TITLE);
    } else if (state == REPORT_FAILED) {
        form.status = text_failed;
    }
}

void report_bug_handle_input(void)
{
    follow_sending();
    int32_t field = keyboard_field_index == FIELD_BODY ? FIELD_BODY : FIELD_TITLE;
    keyboard_capture_set(field);
    if (keyboard_accepted) {
        keyboard_accepted = 0;
        if (field == FIELD_TITLE) {
            focus_field(FIELD_BODY);
        }
    }
    if (mouse_right_went_up) {
        report_bug_close();
        return;
    }
    if (!mouse_left_went_up) {
        return;
    }
    int32_t x = form_x();
    int32_t y = form_y();
    if (inside(x + MARGIN, y + KIND_Y, KIND_WIDTH, BUTTON_HEIGHT)) {
        form.idea = 0;
    } else if (inside(x + MARGIN + KIND_WIDTH + 8, y + KIND_Y, KIND_WIDTH, BUTTON_HEIGHT)) {
        form.idea = 1;
    } else if (inside(x + MARGIN, y + TITLE_BOX_Y, BOX_WIDTH, TITLE_BOX_BLOCKS * 16)) {
        focus_field(FIELD_TITLE);
    } else if (inside(x + MARGIN, y + BODY_BOX_Y, BOX_WIDTH, BODY_BOX_BLOCKS * 16)) {
        focus_field(FIELD_BODY);
    } else if (inside(x + MARGIN, y + ACTION_Y, ACTION_WIDTH, BUTTON_HEIGHT)) {
        send();
    } else if (inside(x + FORM_WIDTH - MARGIN - ACTION_WIDTH, y + ACTION_Y, ACTION_WIDTH, BUTTON_HEIGHT)) {
        report_bug_close();
    }
}
