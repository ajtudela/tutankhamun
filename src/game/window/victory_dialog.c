// The mission end dialog over a won or lost city: the panel with title and subtitle, and its buttons as large labels.
// A won mission offers "Proceed" and, except in the first two missions of the first campaign, "Continue governing"
// for 2 or 5 more years; a lost one "New Game", "Replay mission" and "Lower Difficulty" as city_defeat_options allows.
//
// A lost mission draws the "Victory" title and "Click to proceed" too.

#include "window/victory_dialog.h"
#include "city/city_player.h"
#include "city/kingdom.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "text/lang.h"
#include "window/window_image_group.h"
#include <stdint.h>

uint8_t label_focus;

enum victory_dialog_layout {
    VICTORY_DIALOG_LEFT = 48,
    VICTORY_DIALOG_TOP = 128,
    VICTORY_DIALOG_WIDTH_BLOCKS = 34,
    VICTORY_DIALOG_HEIGHT_BLOCKS = 15,
    VICTORY_DIALOG_WIDTH = VICTORY_DIALOG_WIDTH_BLOCKS * 16,
    VICTORY_DIALOG_CENTERED_SCREEN_WIDTH = 1024,
    VICTORY_DIALOG_CENTERED_SCREEN_HEIGHT = 768,
    VICTORY_BUTTON_LABEL = 6,
    VICTORY_BUTTON_TEXT_DY = 6,
};

int32_t city_defeat_options(void)
{
    if (!city_lost_by_population) {
        if (!mission_lost_can_replay) {
            return DEFEAT_OPTIONS_NEW_GAME;
        }
        if ((scenario_data.time_limit != 0) && (city_player_current()->difficulty > 0)) {
            return DEFEAT_OPTIONS_LOWER_DIFFICULTY;
        }
    }
    return mission_lost_can_replay ? DEFEAT_OPTIONS_REPLAY : DEFEAT_OPTIONS_NEW_GAME;
}

void victory_dialog_draw_background(void)
{
    victory_x = ui_origin_x + VICTORY_DIALOG_LEFT;
    victory_y = ui_origin_y + VICTORY_DIALOG_TOP;
    image_view_x = (VICTORY_DIALOG_CENTERED_SCREEN_WIDTH - g_screen_width) / 2;
    image_view_y = (VICTORY_DIALOG_CENTERED_SCREEN_HEIGHT - g_screen_height) / 2;
    image_view_width = g_screen_width;
    image_view_height = g_screen_height;
    outer_panel_draw(victory_x, victory_y, VICTORY_DIALOG_WIDTH_BLOCKS, VICTORY_DIALOG_HEIGHT_BLOCKS);
    lang_text_draw_centered(LANG_GROUP_MISSION_GOALS, VICTORY_TEXT_VICTORY, victory_x, victory_y + 16, VICTORY_DIALOG_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    lang_text_draw_centered(LANG_GROUP_MISSION_GOALS, VICTORY_TEXT_CLICK_TO_PROCEED, victory_x, victory_y + 47, VICTORY_DIALOG_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void draw_button(int32_t button, int32_t dx, int32_t dy, int32_t width_blocks, int32_t text, int32_t font)
{
    label_focus = focus_button_id == button;
    large_label_draw_animated(VICTORY_BUTTON_LABEL, victory_x + dx, victory_y + dy, width_blocks, label_focus);
    lang_text_draw_centered(LANG_GROUP_MISSION_GOALS, text, victory_x + dx, victory_y + dy + VICTORY_BUTTON_TEXT_DY, width_blocks * 16, font, 0);
}

void victory_dialog_draw_foreground(void)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    victory_x = ui_origin_x + VICTORY_DIALOG_LEFT;
    victory_y = ui_origin_y + VICTORY_DIALOG_TOP;
    if (city_victory_state == VICTORY_STATE_WON) {
        draw_button(1, 32, 112, 30, VICTORY_TEXT_PROCEED, font);
        if ((settings_data.mission_id >= 2) || (settings_data.is_custom_scenario != 0) || (settings_data.campaign != 0)) {
            draw_button(2, 32, 144, 30, VICTORY_TEXT_CONTINUE_2_YEARS, font);
            draw_button(3, 32, 176, 30, VICTORY_TEXT_CONTINUE_5_YEARS, font);
        }
    } else if (city_defeat_options() == DEFEAT_OPTIONS_NEW_GAME) {
        draw_button(1, 64, 96, 26, VICTORY_TEXT_NEW_GAME, image_group(IMAGE_GROUP_FONT_BLACK));
    } else if (city_defeat_options() == DEFEAT_OPTIONS_REPLAY) {
        draw_button(1, 64, 96, 9, VICTORY_TEXT_NEW_GAME, image_group(IMAGE_GROUP_FONT_BLACK));
        draw_button(2, 352, 96, 9, VICTORY_TEXT_REPLAY_MISSION, image_group(IMAGE_GROUP_FONT_BLACK));
    } else {
        draw_button(1, 32, 96, 9, VICTORY_TEXT_LOWER_DIFFICULTY, image_group(IMAGE_GROUP_FONT_BLACK));
        draw_button(2, 208, 96, 9, VICTORY_TEXT_NEW_GAME, image_group(IMAGE_GROUP_FONT_BLACK));
        draw_button(3, 384, 96, 9, VICTORY_TEXT_REPLAY_MISSION, image_group(IMAGE_GROUP_FONT_BLACK));
    }
}
