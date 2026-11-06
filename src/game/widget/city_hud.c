// What the city window draws over the city each frame: the mission line (won, time left, survival), the cheat
// and event debug lines, the paused panel and the fast-forward note at the end of the year.

#include "widget/city_hud.h"
#include "city/city_player.h"
#include "city/difficulty.h"
#include "figure/action.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "text/lang.h"
#include "widget/city_view_rotate.h"
#include "widget/top_menu.h"
#include "widget/widget_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum startup_text {
    STARTUP_TEXT_ACCELERATED_TIME = 5,
    STARTUP_TEXT_RIGHT_CLICK_TO_CONTINUE = 6,
};

enum hud_color {
    HUD_COLOR_TEXT = 0,
    HUD_COLOR_ALERT = 0xF800,
    HUD_COLOR_DEBUG = 0xFFFF,
    HUD_COLOR_TITLE_SHADOW_RGB565 = 0xFF3A,
    HUD_COLOR_TITLE_SHADOW = 0x7F9A,
    HUD_COLOR_MISSION_BACKGROUND_MASK = 0x1FE3,
};

enum hud_extra_years {
    HUD_EXTRA_YEARS_VERY_EASY = 7,
    HUD_EXTRA_YEARS_EASY = 2,
};

enum hud_layout {
    PAUSED_PANEL_WIDTH = 448,
    PAUSED_PANEL_BLOCKS = 28,
    FAST_FORWARD_NOTE_X = 128,
    FAST_FORWARD_NOTE_WIDTH = 384,
};

static int32_t months_left(int32_t end_year)
{
    int32_t years = (end_year > (game_time_year + 1)) ? (end_year - game_time_year - 1) : 0;
    return (((years * 3) + 3) * 4) - game_time_month;
}

static void draw_title(int32_t index)
{
    int32_t font = image_group(IMAGE_GROUP_FONT_SMALL);
    g_text_width = 0;
    int16_t shadow = (int16_t) ((display_mode == DISPLAY_MODE_RGB565) ? HUD_COLOR_TITLE_SHADOW_RGB565 : HUD_COLOR_TITLE_SHADOW);
    lang_text_draw_ui(LANG_GROUP_TOP_MENU, index, 8, 47, font, HUD_COLOR_TEXT, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_TOP_MENU, index, 7, 46, font, shadow, ui_origin_x, ui_origin_y);
}

void window_city_draw_paused_and_time_left(void)
{
    const city_player *player = city_player_current();
    if (construction_placing_royal_tomb != 0) {
        draw_title(TOP_MENU_TEXT_PLACE_TOMBS_OVER_CLIFFS);
    } else if (player->mission_has_won != 0) {
        draw_title(TOP_MENU_TEXT_MISSION_ALREADY_WON);
    } else if (scenario_data.open_play == 0) {
        if (scenario_data.time_limit != 0) {
            int32_t end_year = scenario_max_year;
            if (player->difficulty == DIFFICULTY_VERY_EASY) {
                end_year += HUD_EXTRA_YEARS_VERY_EASY;
            } else if (player->difficulty == DIFFICULTY_EASY) {
                end_year += HUD_EXTRA_YEARS_EASY;
            }
            int32_t months = months_left(end_year);
            graphics_and_rect(1, 40, 230, 20, HUD_COLOR_MISSION_BACKGROUND_MASK);
            int32_t font = image_group(IMAGE_GROUP_FONT_SMALL);
            for (int32_t dx = 0; dx <= 1; dx++) {
                int32_t x = dx + 7;
                for (int32_t y = 45; y <= 47; y += 2) {
                    g_text_width = 0;
                    lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_MONTHS_TO_COMPLETE_MISSION, x, y, font, HUD_COLOR_TEXT, ui_origin_x, ui_origin_y);
                    text_draw_number_ui(months, NUMBER_PREFIX_NONE, postfix_none, g_text_width + x, y, font, HUD_COLOR_TEXT, ui_origin_x, ui_origin_y);
                }
            }
            g_text_width = 0;
            lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_MONTHS_TO_COMPLETE_MISSION, 7, 46, font, HUD_COLOR_ALERT, ui_origin_x, ui_origin_y);
            text_draw_number_ui(months, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 7, 46, font, HUD_COLOR_ALERT, ui_origin_x, ui_origin_y);
        } else if (scenario_data.survival != 0) {
            int32_t months = months_left(scenario_max_year);
            graphics_and_rect(1, 42, 200, 18, HUD_COLOR_MISSION_BACKGROUND_MASK);
            g_text_width = 0;
            lang_text_draw_ui(LANG_GROUP_TOP_MENU, TOP_MENU_TEXT_MONTHS_UNTIL_VICTORY, 2, 46, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_ALERT, ui_origin_x, ui_origin_y);
            text_draw_number_ui(months, NUMBER_PREFIX_NONE, postfix_none, g_text_width + 2, 46, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_ALERT, ui_origin_x, ui_origin_y);
        }
    }
    g_text_width = 0;
    if (cheat_allow_all == 1) {
        text_draw(str_allow_all, g_text_width + 10, 61, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG);
    }
    if (debug_show_happiness != 0) {
        text_draw_number_ui(player->sentiment, NUMBER_PREFIX_NONE, hud_happiness_label, 2, 80, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_TEXT, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->sentiment, NUMBER_PREFIX_NONE, hud_happiness_label, 1, 81, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->prosperity_max, NUMBER_PREFIX_NONE, hud_max_prosperity_label, 2, 100, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_TEXT, ui_origin_x, ui_origin_y);
        text_draw_number_ui(player->prosperity_max, NUMBER_PREFIX_NONE, hud_max_prosperity_label, 1, 101, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG, ui_origin_x, ui_origin_y);
    }
    if (debug_event_kind != DEBUG_EVENT_NONE) {
        int32_t saved_x = g_text_width;
        g_text_width = 0;
        lang_text_draw_ui(LANG_GROUP_GOD_NAMES, debug_event_text, 0, 121, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG, ui_origin_x, ui_origin_y);
        const uint8_t *kind = 0;
        switch (debug_event_kind) {
            case DEBUG_EVENT_MAJOR_GOOD:
                kind = str_major_good;
                break;
            case DEBUG_EVENT_MINOR_GOOD:
                kind = str_minor_good;
                break;
            case DEBUG_EVENT_MAJOR_BAD:
                kind = str_major_bad;
                break;
            case DEBUG_EVENT_MINOR_BAD:
                kind = str_minor_bad;
                break;
        }
        if (kind) {
            text_draw(kind, g_text_width + 1, 121, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG);
        }
        text_draw(str_event, g_text_width, 101, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG);
        text_draw_number_ui(debug_event_level, NUMBER_PREFIX_NONE, hud_level_label, g_text_width + 2, 121, image_group(IMAGE_GROUP_FONT_SMALL), HUD_COLOR_DEBUG,
            ui_origin_x, ui_origin_y);
        g_text_width = saved_x;
    }
    if (settings_data.paused != 0) {
        int32_t x = (city_view_get_viewport_width_pixels() - PAUSED_PANEL_WIDTH) / 2;
        outer_panel_draw(x, 40, PAUSED_PANEL_BLOCKS, 3);
        lang_text_draw_centered_ui(
            LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_GAME_PAUSED, x, 58, PAUSED_PANEL_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), HUD_COLOR_TEXT, ui_origin_x, ui_origin_y);
    }
}

void window_fast_forward_note_draw(void)
{
    int32_t x = ui_origin_x + FAST_FORWARD_NOTE_X;
    outer_panel_draw(x, ui_origin_y + 160, 24, 7);
    lang_text_draw_centered(LANG_GROUP_STARTUP, STARTUP_TEXT_ACCELERATED_TIME, x, ui_origin_y + 188, FAST_FORWARD_NOTE_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), HUD_COLOR_TEXT);
    lang_text_draw_centered(
        LANG_GROUP_STARTUP, STARTUP_TEXT_RIGHT_CLICK_TO_CONTINUE, x, ui_origin_y + 220, FAST_FORWARD_NOTE_WIDTH, image_group(IMAGE_GROUP_FONT_GREEN), HUD_COLOR_TEXT);
}
