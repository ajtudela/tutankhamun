// The high scores ("Eternal Houses of Egypt") and their file save\highscore.jas: a mission record per campaign
// mission, the window listing them with a scroll bar and arrows, and the scoring of a won mission, written when it is
// new or beaten (a cheated victory and a free play mission record nothing).

#include "window/high_scores.h"
#include "building/monument_city.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "game/game_data.h"
#include "game/player_record.h"
#include "graphics/button.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "io/file.h"
#include "scenario/campaign.h"
#include "scenario/scenario_data.h"
#include "text/lang.h"
#include "window/free_play.h"
#include "window/mission_flow.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum high_scores_text {
    HIGH_SCORES_TEXT_TITLE = 0,
    HIGH_SCORES_TEXT_MISSION = 1,
    HIGH_SCORES_TEXT_BEST_FAMILY = 2,
    HIGH_SCORES_TEXT_DIFFICULTY = 3,
    HIGH_SCORES_TEXT_SCORE = 4,
    HIGH_SCORES_TEXT_HOUSE_OF = 5,
    HIGH_SCORES_TEXT_NO_HIGH_SCORES = 6,
};

enum high_scores_layout {
    HIGH_SCORES_LINES = 20,
    HIGH_SCORES_LINE_HEIGHT = 16,
    HIGH_SCORES_LIST_Y = 104,
    HIGH_SCORES_HEADER_Y = 80,
    HIGH_SCORES_COLUMN_MISSION = 48,
    HIGH_SCORES_COLUMN_FAMILY = 224,
    HIGH_SCORES_COLUMN_DIFFICULTY = 368,
    HIGH_SCORES_COLUMN_SCORE = 512,
    HIGH_SCORES_TITLE_X = 20,
    HIGH_SCORES_TITLE_WIDTH = 608,
    HIGH_SCORES_SCROLLBAR_LEFT = 568,
    HIGH_SCORES_SCROLLBAR_X = 584,
    HIGH_SCORES_SCROLLBAR_RIGHT = 600,
    HIGH_SCORES_SCROLLBAR_TOP = 128,
    HIGH_SCORES_SCROLLBAR_DOT_TOP = 130,
    HIGH_SCORES_SCROLLBAR_BOTTOM = 401,
    HIGH_SCORES_SCROLLBAR_RANGE = 257,
    HIGH_SCORES_SCROLLBAR_DOT_IMAGE = 39,
};

static mission_record *high_score_record(int32_t mission)
{
    return &highscores.missions[mission];
}

void high_scores_scroll(void)
{
    if (hs_count <= HIGH_SCORES_LINES) {
        return;
    }
    if (button_parameter == SCROLL_ARROW_UP) {
        if (hs_scroll > 0) {
            window_redraw_request = 2;
            hs_scroll--;
        }
        return;
    }
    if (button_parameter == SCROLL_ARROW_DOWN) {
        if ((hs_scroll + HIGH_SCORES_LINES) >= hs_count) {
            return;
        }
        hs_scroll++;
    }
    window_redraw_request = 2;
}

void high_scores_draw_scrollbar_dot(void)
{
    if ((hs_count - HIGH_SCORES_LINES) <= 0) {
        return;
    }
    int32_t percentage;
    if (hs_scroll < 1) {
        percentage = 0;
    } else if ((hs_scroll + HIGH_SCORES_LINES) < hs_count) {
        percentage = calc_percentage(hs_scroll, hs_count - HIGH_SCORES_LINES);
    } else {
        percentage = 100;
    }
    int32_t offset = calc_adjust_with_percentage(HIGH_SCORES_SCROLLBAR_RANGE, percentage);
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + HIGH_SCORES_SCROLLBAR_DOT_IMAGE, ui_origin_x + HIGH_SCORES_SCROLLBAR_X,
        ui_origin_y + HIGH_SCORES_SCROLLBAR_DOT_TOP + offset);
}

void high_scores_draw_list(void)
{
    if (hs_count == 0) {
        lang_text_draw(
            LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_NO_HIGH_SCORES, ui_origin_x + HIGH_SCORES_COLUMN_MISSION, ui_origin_y + HIGH_SCORES_LIST_Y, image_group(IMAGE_GROUP_FONT_GREEN), 0);
        return;
    }
    int32_t line = 0;
    for (int32_t i = 0; i < campaign_object.campaign.mission_name_count; i++) {
        int32_t mission = hs_scroll + i;
        const mission_record *record = high_score_record(mission);
        const char *name = campaign_object.campaign.mission_names[mission];
        if ((record->used != 0) && (name != 0)) {
            int32_t y = line + ui_origin_y + HIGH_SCORES_LIST_Y;
            text_draw((const uint8_t *) name, ui_origin_x + HIGH_SCORES_COLUMN_MISSION, y, image_group(IMAGE_GROUP_FONT_SMALL), 0);
            g_text_width = 0;
            lang_text_draw(LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_HOUSE_OF, ui_origin_x + HIGH_SCORES_COLUMN_FAMILY, y, image_group(IMAGE_GROUP_FONT_SMALL), 0);
            text_draw((const uint8_t *) record->player_name, g_text_width + ui_origin_x + HIGH_SCORES_COLUMN_FAMILY, y, image_group(IMAGE_GROUP_FONT_SMALL), 0);
            lang_text_draw(LANG_GROUP_DIFFICULTY, record->difficulty + 1, ui_origin_x + HIGH_SCORES_COLUMN_DIFFICULTY, y, image_group(IMAGE_GROUP_FONT_SMALL), 0);
            text_draw_number(record->score, NUMBER_PREFIX_NONE, postfix_none, ui_origin_x + HIGH_SCORES_COLUMN_SCORE, y, image_group(IMAGE_GROUP_FONT_SMALL), 0);
            line += HIGH_SCORES_LINE_HEIGHT;
        }
        if (line >= (HIGH_SCORES_LINES * HIGH_SCORES_LINE_HEIGHT)) {
            return;
        }
    }
}

void window_high_scores_draw_background(void)
{
    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 30);
    inner_panel_draw(ui_origin_x + 32, ui_origin_y + 96, 34, 22);
    lang_text_draw_centered(
        LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_TITLE, ui_origin_x + HIGH_SCORES_TITLE_X, ui_origin_y + 32, HIGH_SCORES_TITLE_WIDTH, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    inner_panel_draw(ui_origin_x + 579, ui_origin_y + 123, 2, 19);
    lang_text_draw(LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_MISSION, ui_origin_x + 80, ui_origin_y + HIGH_SCORES_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(
        LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_BEST_FAMILY, ui_origin_x + HIGH_SCORES_COLUMN_FAMILY, ui_origin_y + HIGH_SCORES_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(
        LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_DIFFICULTY, ui_origin_x + HIGH_SCORES_COLUMN_DIFFICULTY, ui_origin_y + HIGH_SCORES_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    lang_text_draw(LANG_GROUP_HIGH_SCORES, HIGH_SCORES_TEXT_SCORE, ui_origin_x + HIGH_SCORES_COLUMN_SCORE, ui_origin_y + HIGH_SCORES_HEADER_Y, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    high_scores_draw_list();
    high_scores_draw_scrollbar_dot();
    lang_text_draw_centered(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, ui_origin_x + HIGH_SCORES_TITLE_X, ui_origin_y + 452,
        HIGH_SCORES_TITLE_WIDTH, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    window_redraw_request = 1;
}

int32_t high_scores_handle_scrollbar(void)
{
    if (hs_count <= HIGH_SCORES_LINES) {
        return 0;
    }
    if (mouse_left_down == 0) {
        return 0;
    }
    int32_t hidden = hs_count - HIGH_SCORES_LINES;
    if ((mouse_x < (ui_origin_x + HIGH_SCORES_SCROLLBAR_LEFT)) || (mouse_x > (ui_origin_x + HIGH_SCORES_SCROLLBAR_RIGHT))) {
        return 0;
    }
    if ((mouse_y < (ui_origin_y + HIGH_SCORES_SCROLLBAR_TOP)) || (mouse_y > (ui_origin_y + HIGH_SCORES_SCROLLBAR_BOTTOM))) {
        return 0;
    }
    int32_t at = mouse_y - ui_origin_y - HIGH_SCORES_SCROLLBAR_TOP;
    if (at > HIGH_SCORES_SCROLLBAR_RANGE) {
        at = HIGH_SCORES_SCROLLBAR_RANGE;
    }
    hs_scroll = calc_adjust_with_percentage(hidden, calc_percentage(at, HIGH_SCORES_SCROLLBAR_RANGE));
    window_redraw_request = 2;
    return 1;
}

bool highscore_write_file(void *self, const char *path)
{
    FILE *file = fopen(path, file_mode_write_binary);
    if (!file) {
        return false;
    }
    size_t written = fwrite(self, sizeof(mission_record_table), 1, file);
    fclose(file);
    return written == 1;
}

bool highscore_load(void)
{
    hs_count = 0;
    if (!highscore_read_file(&highscores, str_highscore_file)) {
        return false;
    }
    for (int32_t i = 0; i < campaign_object.campaign.mission_name_count; i++) {
        if (high_score_record(i)->used != 0) {
            hs_count++;
        }
    }
    return true;
}

void highscore_set_entry(int32_t mission, int32_t monuments, int32_t score, int32_t months)
{
    const city_player *player = city_player_current();
    mission_record *record = high_score_record(mission);
    record->rating_culture = player->rating_culture;
    record->difficulty = player->difficulty;
    record->treasury = player->treasury;
    record->population = player->population;
    record->rating_kingdom = player->rating_kingdom;
    record->mission = mission;
    record->used = 1;
    record->monuments = monuments;
    strcpy(record->player_name, settings_data.player_name);
    record->rating_prosperity = player->rating_prosperity;
    record->score = score;
    record->months = months;
    highscore_write_file(&highscores, str_highscore_file);
}

void highscore_record_mission(void)
{
    hs_result = HIGH_SCORE_RESULT_NONE;
    if (settings_data.is_custom_scenario != 0 || free_play_running()) {
        return;
    }
    int32_t years = scenario_data.start.year - game_time_year;
    if (years < 0) {
        years = -years;
    }
    int32_t months = game_time_month + (years * 12);
    int32_t score = city_victory_score(months);
    int32_t monuments = victory_count_finished_monuments();
    if (city_victory_cheated != 0) {
        city_victory_cheated = 0;
        hs_result = HIGH_SCORE_RESULT_CHEATED;
        return;
    }
    highscore_read_file(&highscores, str_highscore_file);
    int32_t mission = settings_data.mission_id;
    const mission_record *record = high_score_record(mission);
    if (record->used == 0) {
        hs_result = HIGH_SCORE_RESULT_NEW;
        highscore_set_entry(mission, monuments, score, months);
    } else if (_stricmp(record->player_name, settings_data.player_name) == 0) {
        if (record->score >= score) {
            hs_result = HIGH_SCORE_RESULT_OWN_KEPT;
        } else {
            hs_result = HIGH_SCORE_RESULT_OWN_BEATEN;
            highscore_set_entry(mission, monuments, score, months);
        }
    } else if (record->score >= score) {
        hs_result = HIGH_SCORE_RESULT_OTHER_KEPT;
    } else {
        hs_result = HIGH_SCORE_RESULT_OTHER_BEATEN;
        strcpy(high_score_previous_name, record->player_name);
        highscore_set_entry(mission, monuments, score, months);
    }
    if (campaign_object.campaign.mission_records[mission].score < score) {
        player_record_mission(mission, monuments, score, months);
    }
}
