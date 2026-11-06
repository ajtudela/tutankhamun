// The options windows' draws: display, popup messages, difficulty, speed, sound, and the display mode check (a text on
// a cleared screen with an OK button). In a play without a script the display options are the list of resolutions.

#include "window/options_windows.h"
#include "graphics/arrow_buttons.h"
#include "graphics/color.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "window/display_resolution_list.h"
#include "window/intermezzo.h"
#include "window/window_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum options_text {
    DISPLAY_OPTIONS_TEXT_GROUP = 42,
    DISPLAY_OPTIONS_TEXT_TITLE = 0,
    DISPLAY_OPTIONS_TEXT_WINDOWED = 1,
    DISPLAY_OPTIONS_TEXT_FULLSCREEN = 2,
    GAME_OPTIONS_TEXT_GROUP = 310,
    GAME_OPTIONS_TEXT_TITLE = 0,
    GAME_OPTIONS_TEXT_HINT = 1,
    GAME_OPTIONS_TEXT_BELOW_1 = 14,
    GAME_OPTIONS_TEXT_BELOW_2 = 15,
    DIFFICULTY_TEXT_TITLE = 0,
    DIFFICULTY_TEXT_HINT = 8,
    SPEED_OPTIONS_TEXT_GROUP = 45,
    SPEED_OPTIONS_TEXT_TITLE = 0,
    SPEED_OPTIONS_TEXT_CANCEL = 1,
    SPEED_OPTIONS_TEXT_GAME_SPEED = 2,
    SPEED_OPTIONS_TEXT_SCROLL_SPEED = 3,
    SPEED_OPTIONS_TEXT_OK = 4,
    SOUND_OPTIONS_TEXT_GROUP = 46,
    SOUND_OPTIONS_TEXT_TITLE = 0,
    SOUND_OPTIONS_TEXT_MUSIC_OFF = 1,
    SOUND_OPTIONS_TEXT_SPEECH_OFF = 3,
    SOUND_OPTIONS_TEXT_EFFECTS_OFF = 5,
    SOUND_OPTIONS_TEXT_CITY_OFF = 7,
    SOUND_OPTIONS_TEXT_CANCEL = 9,
    SOUND_OPTIONS_TEXT_HEAD_SOUND = 10,
    SOUND_OPTIONS_TEXT_HEAD_VOLUME = 11,
    SOUND_OPTIONS_TEXT_OK = 12,
    DISPLAY_MODE_CHECK_TEXT_GROUP = 146,
};

enum options_labels {
    OPTIONS_LABEL_DISPLAY = 2,
    OPTIONS_LABEL_SOUND = 3,
    OPTIONS_LABEL_SPEED = 4,
    OPTIONS_LABEL_GAME = 15,
    OPTIONS_LABEL_GROWN = 50,
    LABEL_FOCUSED = 1,
    LABEL_NORMAL = 2,
};

enum options_layout {
    DISPLAY_OPTIONS_ROWS = 5,
    GAME_OPTIONS_TOGGLES = 12,
    GAME_OPTIONS_BELOW_FIRST_ROW = 13,
    GAME_OPTIONS_BELOW_LAST_ROW = 14,
    SOUND_OPTIONS_TOGGLES = 4,
    SOUND_OPTIONS_ROW_OK = 5,
    SOUND_OPTIONS_ROW_CANCEL = 6,
    SPEED_OPTIONS_ROW_OK = 1,
    SPEED_OPTIONS_ROW_CANCEL = 2,
    DIFFICULTY_ARROW_BUTTONS = 2,
    ROW_HEIGHT = 24,
    SOUND_ROW_HEIGHT = 30,
};

static int32_t row_color(int32_t row)
{
    return focus_button_id == row ? COLOR_ORANGE : COLOR_BLUE;
}

static void draw_label(int32_t label, int32_t x, int32_t y, int32_t width_blocks, int32_t row)
{
    label_draw_animated_ui(
        label, ui_origin_x + x, ui_origin_y + y, width_blocks, 0, focus_button_id == row ? LABEL_FOCUSED : LABEL_NORMAL, ui_origin_x, ui_origin_y);
}

static void draw_centered(int32_t group, int32_t text, int32_t x, int32_t y, int32_t width, int32_t font, int32_t color)
{
    lang_text_draw_centered_ui(group, text, ui_origin_x + x, ui_origin_y + y, width, font, color, ui_origin_x, ui_origin_y);
}

static void draw_percent(int32_t value, int32_t x, int32_t y)
{
    text_draw_number_ui(value, NUMBER_PREFIX_NONE, pct_postfix, ui_origin_x + x, ui_origin_y + y, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_display_options_draw(void)
{
    if (display_resolution_list_allowed()) {
        display_resolution_list_draw();
        return;
    }
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    outer_panel_draw(ui_origin_x + 96, ui_origin_y + 80, 18, 12);
    for (int32_t row = 1; row <= DISPLAY_OPTIONS_ROWS; row++) {
        draw_label(OPTIONS_LABEL_DISPLAY, 128, 112 + row * ROW_HEIGHT, 14, row);
    }
    draw_centered(DISPLAY_OPTIONS_TEXT_GROUP, DISPLAY_OPTIONS_TEXT_TITLE, 128, 94, 224, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    int32_t mode_text = settings_data.fullscreen != 0 ? DISPLAY_OPTIONS_TEXT_FULLSCREEN : DISPLAY_OPTIONS_TEXT_WINDOWED;
    draw_centered(DISPLAY_OPTIONS_TEXT_GROUP, mode_text, 128, 140, 224, font, row_color(1));
    for (int32_t row = 2; row <= DISPLAY_OPTIONS_ROWS; row++) {
        draw_centered(DISPLAY_OPTIONS_TEXT_GROUP, row + 1, 128, 116 + row * ROW_HEIGHT, 224, font, row_color(row));
    }
    graphics_reset_clip_rectangle();
    label_animation[LABEL_SLOT_DISPLAY_OPTIONS] = OPTIONS_LABEL_GROWN;
}

void window_game_options_draw(void)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    outer_panel_draw(ui_origin_x + 32, ui_origin_y + 38, 26, 28);
    for (int32_t row = 1; row <= GAME_OPTIONS_TOGGLES; row++) {
        draw_label(OPTIONS_LABEL_GAME, 128, 92 + row * ROW_HEIGHT, 14, row);
    }
    for (int32_t row = GAME_OPTIONS_BELOW_FIRST_ROW; row <= GAME_OPTIONS_BELOW_LAST_ROW; row++) {
        draw_label(OPTIONS_LABEL_GAME, 128, 116 + row * ROW_HEIGHT, 14, row);
    }
    draw_centered(GAME_OPTIONS_TEXT_GROUP, GAME_OPTIONS_TEXT_TITLE, 48, 60, 384, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    draw_centered(GAME_OPTIONS_TEXT_GROUP, GAME_OPTIONS_TEXT_HINT, 48, 95, 384, font, 0);
    for (int32_t row = 1; row <= GAME_OPTIONS_TOGGLES; row++) {
        int32_t on = (game_options_flags & (1u << row)) != 0;
        draw_centered(GAME_OPTIONS_TEXT_GROUP, row + 1, 128, 96 + row * ROW_HEIGHT, 224, on ? image_group(IMAGE_GROUP_FONT_RED) : image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
    draw_centered(GAME_OPTIONS_TEXT_GROUP, GAME_OPTIONS_TEXT_BELOW_1, 128, 432, 224, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    draw_centered(GAME_OPTIONS_TEXT_GROUP, GAME_OPTIONS_TEXT_BELOW_2, 128, 456, 224, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    label_animation[LABEL_SLOT_GAME_OPTIONS] = OPTIONS_LABEL_GROWN;
}

void window_difficulty_options_draw(void)
{
    outer_panel_draw(ui_origin_x + 48, ui_origin_y + 80, 24, 12);
    draw_centered(LANG_GROUP_DIFFICULTY, DIFFICULTY_TEXT_TITLE, 48, 94, 384, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    draw_centered(LANG_GROUP_DIFFICULTY, settings_data.difficulty + 1, 80, 174, 224, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    arrow_buttons_draw(ui_origin_x + 288, ui_origin_y + 112, difficulty_arrows.arrows, DIFFICULTY_ARROW_BUTTONS);
    draw_centered(LANG_GROUP_DIFFICULTY, DIFFICULTY_TEXT_HINT, 48, 246, 384, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

void window_speed_options_draw(void)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    outer_panel_draw(ui_origin_x + 96, ui_origin_y + 80, 18, 14);
    draw_label(OPTIONS_LABEL_SPEED, 144, 232, 12, SPEED_OPTIONS_ROW_OK);
    draw_label(OPTIONS_LABEL_SPEED, 144, 262, 12, SPEED_OPTIONS_ROW_CANCEL);
    draw_centered(SPEED_OPTIONS_TEXT_GROUP, SPEED_OPTIONS_TEXT_TITLE, 128, 92, 224, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    draw_centered(SPEED_OPTIONS_TEXT_GROUP, SPEED_OPTIONS_TEXT_OK, 128, 236, 224, font, row_color(SPEED_OPTIONS_ROW_OK));
    draw_centered(SPEED_OPTIONS_TEXT_GROUP, SPEED_OPTIONS_TEXT_CANCEL, 128, 266, 224, font, row_color(SPEED_OPTIONS_ROW_CANCEL));
    lang_text_draw_ui(SPEED_OPTIONS_TEXT_GROUP, SPEED_OPTIONS_TEXT_GAME_SPEED, ui_origin_x + 144, ui_origin_y + 146, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_percent(settings_data.game_speed, 296, 146);
    lang_text_draw_ui(SPEED_OPTIONS_TEXT_GROUP, SPEED_OPTIONS_TEXT_SCROLL_SPEED, ui_origin_x + 144, ui_origin_y + 182, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_percent(settings_data.scroll_speed, 296, 182);
    graphics_reset_clip_rectangle();
    label_animation[LABEL_SLOT_SPEED_OPTIONS] = OPTIONS_LABEL_GROWN;
}

static void draw_sound_row(int32_t off_text, uint8_t enabled, int32_t volume, int32_t y, int32_t font)
{
    draw_centered(SOUND_OPTIONS_TEXT_GROUP, off_text + (enabled != 0), 64, y, 224, font, 0);
    draw_percent(volume, 370, y);
}

void window_sound_options_draw(void)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    outer_panel_draw(ui_origin_x + 48, ui_origin_y + 80, 24, 18);
    for (int32_t row = 1; row <= SOUND_OPTIONS_TOGGLES; row++) {
        draw_label(OPTIONS_LABEL_SOUND, 64, 132 + row * SOUND_ROW_HEIGHT, 14, row);
    }
    draw_label(OPTIONS_LABEL_SOUND, 144, 296, 12, SOUND_OPTIONS_ROW_OK);
    draw_label(OPTIONS_LABEL_SOUND, 144, 326, 12, SOUND_OPTIONS_ROW_CANCEL);
    draw_centered(SOUND_OPTIONS_TEXT_GROUP, SOUND_OPTIONS_TEXT_TITLE, 96, 92, 288, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    draw_centered(SOUND_OPTIONS_TEXT_GROUP, SOUND_OPTIONS_TEXT_OK, 128, 300, 224, font, row_color(SOUND_OPTIONS_ROW_OK));
    draw_centered(SOUND_OPTIONS_TEXT_GROUP, SOUND_OPTIONS_TEXT_CANCEL, 128, 330, 224, font, row_color(SOUND_OPTIONS_ROW_CANCEL));
    lang_text_draw_ui(SOUND_OPTIONS_TEXT_GROUP, SOUND_OPTIONS_TEXT_HEAD_SOUND, ui_origin_x + 112, ui_origin_y + 142, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(SOUND_OPTIONS_TEXT_GROUP, SOUND_OPTIONS_TEXT_HEAD_VOLUME, ui_origin_x + 336, ui_origin_y + 142, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    draw_sound_row(SOUND_OPTIONS_TEXT_MUSIC_OFF, settings_data.sound_music_enabled, settings_data.sound_music_volume, 166, font);
    draw_sound_row(SOUND_OPTIONS_TEXT_SPEECH_OFF, settings_data.sound_speech_enabled, settings_data.sound_speech_volume, 196, font);
    draw_sound_row(SOUND_OPTIONS_TEXT_EFFECTS_OFF, settings_data.sound_effects_enabled, settings_data.sound_effects_volume, 226, font);
    draw_sound_row(SOUND_OPTIONS_TEXT_CITY_OFF, settings_data.sound_city_enabled, settings_data.sound_city_volume, 256, font);
    graphics_reset_clip_rectangle();
    label_animation[LABEL_SLOT_SOUND_OPTIONS] = OPTIONS_LABEL_GROWN;
}

void window_3e_draw_background(void)
{
    int32_t x = ui_origin_x + 80;
    int32_t y = ui_origin_y + 64;
    outer_panel_draw(x, y, 30, 10);
    lang_text_draw_multiline_ui(DISPLAY_MODE_CHECK_TEXT_GROUP, 0, x + 32, y + 30, 416, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}

void window_3e_draw_foreground(void)
{
    int32_t x = ui_origin_x + 240;
    int32_t y = ui_origin_y + 64;
    button_border_draw_ui(x, y + 112, 160, 32, focus_button_id == 1, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_YES_NO, LANG_OK, x, y + 123, 160, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
}
