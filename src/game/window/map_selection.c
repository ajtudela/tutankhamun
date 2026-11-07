// The map selection: the custom maps of the maps folder or the campaign's missions, the panel of the selected one
// (climate, size, invasions, goals, a difficulty rating by map_rating_brackets, the player's records of a campaign
// mission), start and back.
//
// The panel's counts are approximate:
// - the rating counts the requests of the events from 1 on (event 0 is not looked at);
// - the invasions counted leave out those only a chain of kingdom rating events starts.

#include "window/map_selection.h"
#include "city/gods.h"
#include "core/calc.h"
#include "core/text.h"
#include "editor/editor_data.h"
#include "figure/action.h"
#include "game/game_data.h"
#include "game/scenario_start.h"
#include "graphics/button_border.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "io/game_file.h"
#include "platform/platform_file.h"
#include "scenario/campaign.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_internal.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "text/event_text.h"
#include "text/lang.h"
#include "text/text_lookup.h"
#include "window/file_dialog.h"
#include "window/file_list.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/mission_flow.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum map_selection_text {
    MAP_TEXT_LOCKED = 21,
    MAP_TEXT_CLIMATE_FIRST = 77,
    MAP_TEXT_INVASIONS_NONE = 112,
    MAP_TEXT_SIZE_FIRST = 121,
    MAP_TEXT_GOALS = 127,
    MAP_TEXT_GOAL_CULTURE = 129,
    MAP_TEXT_GOAL_PROSPERITY = 130,
    MAP_TEXT_GOAL_KINGDOM = 132,
    MAP_TEXT_GOAL_POPULATION = 133,
    MAP_TEXT_GOAL_TIME_LIMIT = 134,
    MAP_TEXT_GOAL_SURVIVAL = 135,
    MAP_TEXT_START = 136,
    MAP_TEXT_EDITOR_OPEN = 214,
    MAP_TEXT_CAMPAIGN_HINT_2 = 215,
    MAP_TEXT_DIFFICULTY = 216,
    MAP_TEXT_CAMPAIGN_HINT_1 = 217,
    MAP_TEXT_TOGGLE_WHILE_INFORMATION = 220,
    MAP_TEXT_TOGGLE_WHILE_RECORDS = 221,
    PLAYER_RANK_TEXT_FIRST_RATING = 22,
    HOUSE_LEVEL_TEXT_PLURAL = 20,
    OPEN_PLAY_TEXT_GROUP = 145,
    MONUMENT_NAMES_GROUP = 198,
    MISSION_DESCRIPTIONS_GROUP = 297,
    MISSION_RECORD_TEXT_GROUP = 298,
    MISSION_RECORD_TEXT_CULTURE = 0,
    MISSION_RECORD_TEXT_PROSPERITY = 1,
    MISSION_RECORD_TEXT_KINGDOM = 3,
    MISSION_RECORD_TEXT_POPULATION = 4,
    MISSION_RECORD_TEXT_TREASURY = 5,
    MISSION_RECORD_TEXT_YEARS = 6,
    MISSION_RECORD_TEXT_DIFFICULTY = 7,
    MISSION_RECORD_TEXT_SCORE = 8,
    MISSION_RECORD_TEXT_YEARS_POSTFIX = 9,
    NO_MISSION_RECORD_TEXT_GROUP = 305,
    CAMPAIGN_TITLE_GROUP = 293,
    CAMPAIGN_TITLE_TEXT = 6,
    MISSION_LIST_TEXT_GROUP = 294,
    MISSION_LIST_TEXT_BACK = 38,
    MISSION_LIST_TEXT_PROCEED = 39,
    MISSION_LIST_TEXT_EMPTY = 40,
    MAP_TEXT_LENGTH = 200,
};

enum map_selection_layout {
    MAP_LIST_ROWS_CAMPAIGN = 13,
    MAP_LIST_ROWS_CUSTOM = 15,
    MAP_LIST_TOP_CAMPAIGN = 252,
    MAP_LIST_TOP_CUSTOM = 220,
    MAP_ROW_HEIGHT = 16,
    MAP_LIST_BUTTONS = 2,
    SCROLLBAR_HEIGHT_LESS = 76,
    SCROLLBAR_DOT_IMAGE = 39,
    MAP_NAME_SIZE = 64,
    MAP_PATH_SIZE = 100,
    MAP_EXTENSION_LENGTH = 3,
    MONUMENTS_SHOWN = 3,
    CLEOPATRA_FIRST_BACKGROUND = 19,
    FULL_SCREEN_WIDTH = 1024,
    FULL_SCREEN_HEIGHT = 768,
    EVENT_CHAIN_DEPTH = 7,
};

enum map_rating_weights {
    RATING_WEIGHT_KINGDOM = 1,
    RATING_WEIGHT_CULTURE = 2,
    RATING_WEIGHT_PROSPERITY = 2,
    RATING_WEIGHT_MONUMENT = 20,
};

map_rating_bracket map_rating_brackets[MAP_RATING_BRACKET_COUNT] = {
    [MAP_RATING_POPULATION_NONE] = { 500, 0 },
    [MAP_RATING_POPULATION_SMALL] = { 2000, 20 },
    [MAP_RATING_POPULATION_MEDIUM] = { 4000, 50 },
    [MAP_RATING_POPULATION_LARGE] = { 8000, 80 },
    [MAP_RATING_POPULATION_HUGE] = { INT32_MAX, 100 },
    [MAP_RATING_INVASIONS_NONE] = { 0, 0 },
    [MAP_RATING_INVASIONS_FEW] = { 2, 40 },
    [MAP_RATING_INVASIONS_SOME] = { 4, 60 },
    [MAP_RATING_INVASIONS_MANY] = { 10, 80 },
    [MAP_RATING_INVASIONS_MOST] = { INT32_MAX, 100 },
    [MAP_RATING_REQUESTS_FEW] = { 5, 20 },
    [MAP_RATING_REQUESTS_SOME] = { 10, 40 },
    [MAP_RATING_REQUESTS_MANY] = { 20, 60 },
    [MAP_RATING_REQUESTS_MORE] = { 30, 80 },
    [MAP_RATING_REQUESTS_MOST] = { INT32_MAX, 100 },
    [MAP_RATING_GODS_NONE] = { 0, 0 },
    [MAP_RATING_GODS_FEW] = { 2, 20 },
    [MAP_RATING_GODS_SOME] = { 5, 50 },
    [MAP_RATING_GODS_MANY] = { 8, 80 },
    [MAP_RATING_GODS_MOST] = { INT32_MAX, 100 },
    [MAP_RATING_RANK_0] = { 100, 0 },
    [MAP_RATING_RANK_1] = { 200, 1 },
    [MAP_RATING_RANK_2] = { 400, 2 },
    [MAP_RATING_RANK_3] = { 600, 3 },
    [MAP_RATING_RANK_4] = { 900, 4 },
    [MAP_RATING_RANK_5] = { 1300, 5 },
    [MAP_RATING_RANK_6] = { 1700, 6 },
    [MAP_RATING_RANK_7] = { 2100, 7 },
    [MAP_RATING_RANK_8] = { 2500, 8 },
    [MAP_RATING_RANK_9] = { INT32_MAX, 9 },
};

static int32_t bracket_of(int32_t first, int32_t last, int32_t value)
{
    int32_t bracket = first;
    while (bracket < last && value > map_rating_brackets[bracket].max) {
        bracket++;
    }
    return bracket;
}

static int32_t bracket_value(int32_t first, int32_t last, int32_t value)
{
    return map_rating_brackets[bracket_of(first, last, value)].value;
}

static const int32_t map_widths[] = { 56, 84, 112, 140, 170 };

static bool showing_campaign(void)
{
    return map_file_list.campaign == 1;
}

static int32_t list_rows(void)
{
    return showing_campaign() ? MAP_LIST_ROWS_CAMPAIGN : MAP_LIST_ROWS_CUSTOM;
}

static int32_t list_top(void)
{
    return showing_campaign() ? MAP_LIST_TOP_CAMPAIGN : MAP_LIST_TOP_CUSTOM;
}

static int32_t scrollbar_height(void)
{
    return list_rows() * MAP_ROW_HEIGHT - SCROLLBAR_HEIGHT_LESS;
}

static scenario_event *event_at(int32_t id)
{
    if (id < 0 || id >= scenario_events.count) {
        return 0;
    }
    return &scenario_events.events[id];
}

static char *list_name(int32_t index)
{
    return map_list_entry_name(&map_file_list, index);
}

static int32_t list_kind(int32_t index)
{
    return map_list_entry_kind(&map_file_list, index);
}

bool map_list_fill(map_list *self, const char *pattern, int32_t keep)
{
    if (map_file_list.campaign == 0) {
        return file_list_list_dir(&self->files, pattern, (char) keep);
    }
    self->files.count = 0;
    for (int32_t mission = 0; mission < campaign_object.campaign.mission_name_count; mission++) {
        self->missions[self->files.count] = mission;
        self->files.count++;
    }
    return true;
}

int32_t map_list_entry_mission(map_list *self, int32_t index)
{
    if (index >= 0 && index < self->files.count) {
        return self->missions[index];
    }
    return -1;
}

int32_t map_list_entry_kind(map_list *self, int32_t index)
{
    if (index >= 0 && index < self->files.count) {
        return map_file_list.campaign != 0 ? MAP_ENTRY_CAMPAIGN : MAP_ENTRY_CUSTOM;
    }
    return MAP_ENTRY_NONE;
}

char *map_list_entry_name(map_list *self, int32_t index)
{
    if (index < 0 || index >= self->files.count) {
        return 0;
    }
    if (map_file_list.campaign == 0) {
        return self->files.offsets[index] + self->files.names;
    }
    return campaign_object.campaign.mission_names[self->missions[index]];
}

void map_selection_toggle_kind(void)
{
    window_redraw_request = 1;
    map_file_list.selection_kind = map_file_list.selection_kind != MAP_SELECTION_CUSTOM ? MAP_SELECTION_CUSTOM : MAP_SELECTION_CAMPAIGN;
}

image_button *map_selection_row_buttons(void)
{
    return showing_campaign() ? map_selection_button_data.campaign_image_buttons : map_selection_button_data.custom_image_buttons;
}

generic_button *map_selection_toggle_buttons(void)
{
    return &map_selection_button_data.kind_toggle;
}

image_button *map_selection_image_buttons(void)
{
    return map_selection_button_data.difficulty_option_arrows;
}

int32_t map_selection_has_image_buttons(void)
{
    if (editor_active == 1 || (showing_campaign() && map_file_list.selection_kind == MAP_SELECTION_CAMPAIGN)) {
        return 0;
    }
    return 1;
}

int32_t map_selection_image_button_count(void)
{
    return list_kind(map_file_list.selected) != MAP_ENTRY_LOCKED ? 4 : 3;
}

generic_button *map_selection_buttons(void)
{
    return showing_campaign() ? map_selection_button_data.campaign_list : map_selection_button_data.custom_list;
}

int32_t map_selection_button_count(void)
{
    return MAP_SELECTION_LIST_BUTTONS;
}

bool scenario_event_has_chain(const scenario_event *self)
{
    (void) self;
    return true;
}

bool scenario_event_type_chains(const scenario_event *self)
{
    return self->type == EVENT_TYPE_REQUEST || self->type == EVENT_TYPE_GIFT_FROM_PHARAOH;
}

bool scenario_event_has_followup(const scenario_event *self)
{
    return self->type == EVENT_TYPE_REQUEST && self->subtype != 0 && self->subtype <= 2;
}

void map_selection_draw_scroll_thumb(void)
{
    int32_t over = map_file_list.total - list_rows();
    if (over <= 0) {
        return;
    }
    int32_t percentage;
    if (map_file_list.scroll <= 0) {
        percentage = 0;
    } else if (list_rows() + map_file_list.scroll >= map_file_list.total) {
        percentage = 100;
    } else {
        percentage = calc_percentage(map_file_list.scroll, over);
    }
    int32_t offset = calc_adjust_with_percentage(scrollbar_height(), percentage);
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + SCROLLBAR_DOT_IMAGE, ui_origin_x + 289, ui_origin_y + offset + list_top() + 25);
}

static bool chains_to(scenario_event *event, int32_t id)
{
    return scenario_event_is_chain_target(event, id);
}

void event_chain_mark(scenario_event *event, int32_t depth)
{
    if (depth >= EVENT_CHAIN_DEPTH) {
        return;
    }
    int32_t next = depth + 1;
    for (;;) {
        event->chain_marked = 1;
        if (scenario_event_has_chain(event) && chains_to(event, event->on_completed)) {
            event_chain_mark(event_at(event->on_completed), next);
        }
        if (scenario_event_type_chains(event) && chains_to(event, event->on_refusal)) {
            event_chain_mark(event_at(event->on_refusal), next);
        }
        if (scenario_event_type_chains(event) && chains_to(event, event->on_too_late)) {
            event_chain_mark(event_at(event->on_too_late), next);
        }
        if (!scenario_event_has_followup(event) || !chains_to(event, event->on_defeat)) {
            return;
        }
        event = event_at(event->on_defeat);
        next++;
        if (next > EVENT_CHAIN_DEPTH) {
            return;
        }
    }
}

int32_t map_selection_count_invasions(void)
{
    int32_t count = scenario_events.count;
    for (int32_t i = 0; i < count; i++) {
        event_at(i)->chain_marked = 0;
    }
    for (int32_t i = 0; i < count; i++) {
        scenario_event *event = event_at(i);
        if (event->flags & EVENT_FLAG_BY_FAVOUR) {
            event_chain_mark(event, 1);
        }
    }
    int32_t invasions = 0;
    for (scenario_event *event = scenario_invasion_next(&scenario_events, 0, 0); event;
        event = scenario_invasion_next(&scenario_events, event, 0)) {
        if (event->chain_marked == 0) {
            invasions++;
        }
    }
    return invasions;
}

static int32_t count_requests(void)
{
    int32_t requests = 0;
    for (int32_t i = 1; i < scenario_events.count; i++) {
        if (scenario_events.events[i].type == EVENT_TYPE_REQUEST) {
            requests++;
        }
    }
    return requests;
}

static int32_t count_gods(void)
{
    int32_t gods = 0;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        if (scenario_data.god_status[god] == GOD_STATUS_LOCAL) {
            gods++;
        } else if (scenario_data.god_status[god] == GOD_STATUS_PATRON) {
            gods += 2;
        }
    }
    return gods;
}

int32_t map_selection_rating(void)
{
    int32_t rating = scenario_data.win_kingdom * RATING_WEIGHT_KINGDOM + scenario_data.win_prosperity * RATING_WEIGHT_PROSPERITY +
        scenario_data.win_monument * RATING_WEIGHT_MONUMENT + scenario_data.win_culture * RATING_WEIGHT_CULTURE;
    if (scenario_data.win_population_enabled != 0) {
        rating += bracket_value(MAP_RATING_POPULATION_NONE, MAP_RATING_POPULATION_HUGE, scenario_data.win_population);
    }
    rating += bracket_value(MAP_RATING_INVASIONS_NONE, MAP_RATING_INVASIONS_MOST, map_selection_count_invasions());
    rating += bracket_value(MAP_RATING_REQUESTS_FEW, MAP_RATING_REQUESTS_MOST, count_requests());
    rating += bracket_value(MAP_RATING_GODS_NONE, MAP_RATING_GODS_MOST, count_gods());
    return rating;
}

static void draw_goal(int32_t value, int32_t text, int32_t x, int32_t y, int32_t font)
{
    g_text_width = 0;
    text_draw_number(value, NUMBER_PREFIX_NONE, postfix_none, x + 90, y, font, 0);
    lang_text_draw(LANG_GROUP_EDITOR, text, g_text_width + x + 90, y, font, 0);
}

static void draw_record(int32_t text, int32_t value, int32_t x, int32_t y, int32_t font)
{
    g_text_width = 0;
    const char *label = lang_get_string(MISSION_RECORD_TEXT_GROUP, text);
    text_draw_number_prefixed_centered(value, label, empty_text, g_text_width + x + 25, y, 270, font, 0);
}

static int32_t map_size_text(void)
{
    for (int32_t i = 0; i < (int32_t) (sizeof(map_widths) / sizeof(map_widths[0])); i++) {
        if (scenario_data.map_width == map_widths[i]) {
            return MAP_TEXT_SIZE_FIRST + i;
        }
    }
    return MAP_TEXT_SIZE_FIRST + (int32_t) (sizeof(map_widths) / sizeof(map_widths[0]));
}

static int32_t invasions_text(int32_t invasions)
{
    return MAP_TEXT_INVASIONS_NONE + bracket_of(MAP_RATING_INVASIONS_NONE, MAP_RATING_INVASIONS_MOST, invasions) - MAP_RATING_INVASIONS_NONE;
}

static int32_t rating_rank(int32_t rating)
{
    return bracket_value(MAP_RATING_RANK_0, MAP_RATING_RANK_9, rating);
}

static void draw_background_picture(int32_t base_x, int32_t base_y)
{
    if (editor_active == 1) {
        image_draw(image_group(IMAGE_GROUP_EDITOR_BACKGROUND), 0, 0);
    } else if (map_file_list.campaign == 0) {
        image_draw(image_group(IMAGE_GROUP_MAP_BACKGROUND), base_x - 192, base_y - 144);
    } else {
        image_draw(image_group(IMAGE_GROUP_MISSION_FRAME), base_x - 192, base_y - 144);
    }
}

void map_selection_draw_scenario_picture(int32_t base_x, int32_t image_y)
{
    if (scenario_data.image < CLEOPATRA_FIRST_BACKGROUND) {
        image_draw(image_group(IMAGE_GROUP_MISSION_BACKGROUNDS) + scenario_data.image, base_x + 78, image_y);
    } else {
        image_draw(scenario_data.image + image_group(IMAGE_GROUP_CLEOPATRA_BACKGROUNDS) - CLEOPATRA_FIRST_BACKGROUND, base_x + 78, image_y);
    }
}

static void draw_campaign_title(void)
{
    char text[MAP_TEXT_LENGTH];
    const char *title_text = lang_get_string(CAMPAIGN_TITLE_GROUP, CAMPAIGN_TITLE_TEXT);
    int32_t title = scenario_data.player_title_id;
    scenario_data.player_title_id = 0;
    event_text_expand(title_text, text, MAP_TEXT_LENGTH);
    scenario_data.player_title_id = title;
    text_draw_centered((const uint8_t *) text, ui_origin_x + 20, ui_origin_y + 17, 600, image_group(IMAGE_GROUP_FONT_LARGE), 0);
}

static void draw_goals(int32_t x, int32_t y, int32_t label_x, int32_t font)
{
    lang_text_draw_centered(LANG_GROUP_EDITOR, MAP_TEXT_GOALS, label_x, y + 197, 260, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    if (scenario_data.win_culture_enabled) {
        draw_goal(scenario_data.win_culture, MAP_TEXT_GOAL_CULTURE, x, y + 225, font);
    }
    if (scenario_data.win_prosperity_enabled) {
        draw_goal(scenario_data.win_prosperity, MAP_TEXT_GOAL_PROSPERITY, x, y + 241, font);
    }
    if (scenario_data.win_kingdom_enabled) {
        draw_goal(scenario_data.win_kingdom, MAP_TEXT_GOAL_KINGDOM, x, y + 257, font);
    }
    if (scenario_data.win_population_enabled != 0) {
        draw_goal(scenario_data.win_population, MAP_TEXT_GOAL_POPULATION, x, y + 273, font);
    }
    if (scenario_data.win_houses != 0) {
        g_text_width = 0;
        text_draw_number(scenario_data.win_houses, NUMBER_PREFIX_NONE, postfix_none, x + 90, y + 289, font, 0);
        int32_t plural = scenario_data.win_houses > 1 ? HOUSE_LEVEL_TEXT_PLURAL : 0;
        lang_text_draw(LANG_GROUP_HOUSE_LEVELS, scenario_data.win_houses_level + plural, g_text_width + x + 90, y + 289, font, 0);
    }
    if (scenario_data.survival != 0) {
        draw_goal(scenario_data.survival_years, MAP_TEXT_GOAL_SURVIVAL, x, y + 305, font);
    }
    if (scenario_data.time_limit != 0) {
        draw_goal(scenario_data.time_limit_years, MAP_TEXT_GOAL_TIME_LIMIT, x, y + 305, font);
    }
    int32_t monument_y = y + 321;
    for (int32_t i = 0; i < MONUMENTS_SHOWN; i++, monument_y += MAP_ROW_HEIGHT) {
        g_text_width = 0;
        if (scenario_data.monument_def_ids[i] != 0) {
            lang_text_draw_centered(MONUMENT_NAMES_GROUP, scenario_data.monument_def_ids[i], x + 30, monument_y, 260, font, 0);
        }
    }
}

void map_selection_draw_map_information(int32_t x, int32_t y, int32_t label_x, int32_t font)
{
    lang_text_draw_centered(LANG_GROUP_EDITOR, scenario_data.climate_type + MAP_TEXT_CLIMATE_FIRST, label_x, y + 105, 260, font, 0);
    lang_text_draw_centered(LANG_GROUP_EDITOR, map_size_text(), label_x, y + 125, 260, font, 0);
    lang_text_draw_centered(LANG_GROUP_EDITOR, invasions_text(map_selection_count_invasions()), label_x, y + 145, 260, font, 0);
    int32_t rank = rating_rank(map_selection_rating());
    lang_text_draw_centered(LANG_GROUP_PLAYER_RANKS, rank + PLAYER_RANK_TEXT_FIRST_RATING, label_x, y + 165, 260, font, 0);
    if (scenario_data.open_play != 0) {
        lang_text_draw_multiline(OPEN_PLAY_TEXT_GROUP, 0, x + 25, y + 205, 260, font, 0);
    } else {
        draw_goals(x, y, label_x, font);
    }
    if (editor_active == 0) {
        g_text_width = 0;
        lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_DIFFICULTY, x + 90, y + 372, font, 0);
        lang_text_draw(LANG_GROUP_DIFFICULTY, settings_data.difficulty + 1, g_text_width + x + 90, y + 372, font, 0);
    }
}

static void draw_mission_records(int32_t x, int32_t y, int32_t font)
{
    int32_t mission = settings_data.mission_id;
    if (campaign_object.campaign.mission_records[mission].score == 0) {
        lang_text_draw_multiline(NO_MISSION_RECORD_TEXT_GROUP, 0, x + 25, y + 105, 260, font, 0);
        return;
    }
    lang_text_draw_multiline(MISSION_DESCRIPTIONS_GROUP, mission, x + 25, y + 105, 270, font, 0);
    const mission_record *record = &campaign_object.campaign.mission_records[settings_data.mission_id];
    if (scenario_data.win_culture_enabled) {
        draw_record(MISSION_RECORD_TEXT_CULTURE, record->rating_culture, x, y + 265, font);
    }
    if (scenario_data.win_prosperity_enabled) {
        draw_record(MISSION_RECORD_TEXT_PROSPERITY, record->rating_prosperity, x, y + 281, font);
    }
    if (scenario_data.win_kingdom_enabled) {
        draw_record(MISSION_RECORD_TEXT_KINGDOM, record->rating_kingdom, x, y + 297, font);
    }
    if (scenario_data.win_population_enabled != 0) {
        draw_record(MISSION_RECORD_TEXT_POPULATION, record->population, x, y + 313, font);
    }
    draw_record(MISSION_RECORD_TEXT_TREASURY, record->treasury, x, y + 329, font);
    g_text_width = 0;
    const char *difficulty_label = lang_get_string(MISSION_RECORD_TEXT_GROUP, MISSION_RECORD_TEXT_DIFFICULTY);
    const char *difficulty = lang_get_string(LANG_GROUP_DIFFICULTY, record->difficulty + 1);
    text_draw_prefixed_centered(difficulty_label, difficulty, g_text_width + x + 15, y + 345, 270, font, 0);
    g_text_width = 0;
    const char *years_label = lang_get_string(MISSION_RECORD_TEXT_GROUP, MISSION_RECORD_TEXT_YEARS);
    const char *years = lang_get_string(MISSION_RECORD_TEXT_GROUP, MISSION_RECORD_TEXT_YEARS_POSTFIX);
    text_draw_number_prefixed_centered(record->months / MONTHS_PER_YEAR, years_label, years, g_text_width + x + 25, y + 361, 270, font, 0);
    draw_record(MISSION_RECORD_TEXT_SCORE, record->score, x, y + 377, image_group(IMAGE_GROUP_FONT_WHITE));
}

static void draw_start_hint(int32_t x, int32_t y)
{
    if (list_kind(map_file_list.selected) == MAP_ENTRY_LOCKED) {
        lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_LOCKED, x + 32, y + 426, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    } else if (editor_active != 0) {
        lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_EDITOR_OPEN, x + 170, y + 426, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    } else if (showing_campaign()) {
        lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_CAMPAIGN_HINT_1, x + 60, y + 426, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_CAMPAIGN_HINT_2, x + 140, y + 426, image_group(IMAGE_GROUP_FONT_BLACK), 0);
        int32_t toggle_text = map_file_list.selection_kind != MAP_SELECTION_CUSTOM ? MAP_TEXT_TOGGLE_WHILE_RECORDS : MAP_TEXT_TOGGLE_WHILE_INFORMATION;
        lang_text_draw_centered(LANG_GROUP_EDITOR, toggle_text, ui_origin_x + 354, ui_origin_y + 414, 250, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    } else {
        lang_text_draw(LANG_GROUP_EDITOR, MAP_TEXT_START, x + 185, y + 426, image_group(IMAGE_GROUP_FONT_BLACK), 0);
    }
}

void window_map_selection_draw(void)
{
    image_view_x = (FULL_SCREEN_WIDTH - g_screen_width) / 2;
    image_view_y = (FULL_SCREEN_HEIGHT - g_screen_height) / 2;
    image_view_width = g_screen_width;
    int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
    mouse_right_went_up = 0;
    mouse_left_went_up = 0;
    int32_t base_x = ui_origin_x;
    int32_t base_y = ui_origin_y;
    int32_t x = base_x + 320;
    int32_t y = base_y + 20;
    image_view_height = g_screen_height;
    if (window_redraw_request == 1) {
        draw_background_picture(base_x, base_y);
    }
    window_ui_redraw = 1;
    inner_panel_draw(ui_origin_x + 286, ui_origin_y + list_top() + 22, 2, list_rows() - 3);
    map_selection_draw_list();
    map_selection_draw_scroll_thumb();
    map_selection_draw_scenario_picture(base_x, showing_campaign() ? base_y + 56 : base_y + 36);
    if (showing_campaign()) {
        draw_campaign_title();
    }
    int32_t label_x = x + 15;
    text_draw_centered((const uint8_t *) scenario_map_name, label_x, y + 45, 260, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    text_draw_centered((const uint8_t *) scenario_data.name, label_x, y + 65, 260, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    lang_text_draw_year(1, scenario_data.start.year, x + 90, y + 85, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    if (map_file_list.selection_kind == MAP_SELECTION_CUSTOM) {
        map_selection_draw_map_information(x, y, label_x, font);
    } else {
        draw_mission_records(x, y, font);
    }
    draw_start_hint(x, y);
}

static const int16_t *list_button_rect(int32_t index)
{
    return (const int16_t *) &map_selection_buttons()[index];
}

static void draw_mission_list_buttons(void)
{
    static const int32_t texts[MAP_LIST_BUTTONS] = { MISSION_LIST_TEXT_BACK, MISSION_LIST_TEXT_PROCEED };
    for (int32_t i = 0; i < MAP_LIST_BUTTONS; i++) {
        int32_t index = list_rows() + i;
        int32_t focused = i == 0 || focus_button_id == index + 1;
        button_border_draw_rect(list_button_rect(index), focused, 0, 0);
        lang_text_draw_centered_rect(MISSION_LIST_TEXT_GROUP, texts[i], list_button_rect(index), image_group(IMAGE_GROUP_FONT_BLACK), 0, 4, 7);
    }
    if (map_file_list.files.count == 0) {
        lang_text_draw_multiline(MISSION_LIST_TEXT_GROUP, MISSION_LIST_TEXT_EMPTY, ui_origin_x + 24, ui_origin_y + list_top(), 235, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    }
}

static int32_t list_row_font(int32_t row, int32_t index)
{
    if (focus_button_id == row) {
        return image_group(IMAGE_GROUP_FONT_WHITE);
    }
    if (focus_button_id == 0 && index == map_file_list.selected) {
        return image_group(IMAGE_GROUP_FONT_WHITE);
    }
    if (list_kind(index) == MAP_ENTRY_LOCKED) {
        return image_group(IMAGE_GROUP_FONT_RED);
    }
    return image_group(IMAGE_GROUP_FONT_GREEN);
}

void map_selection_draw_list(void)
{
    inner_panel_draw(ui_origin_x + 15, ui_origin_y + list_top() - 15, 16, list_rows() + 1);
    for (int32_t row = 1; row <= list_rows(); row++) {
        int32_t index = map_file_list.scroll + row - 1;
        const char *name = list_name(index);
        if (name == 0) {
            continue;
        }
        text_draw((const uint8_t *) name, ui_origin_x + 24, ui_origin_y + row * MAP_ROW_HEIGHT + list_top() - MAP_ROW_HEIGHT, list_row_font(row, index), 0);
    }
    if (showing_campaign()) {
        draw_mission_list_buttons();
    }
    int32_t focused = focus_button_id_secondary != 0;
    if (showing_campaign()) {
        button_border_draw_rect((const int16_t *) map_selection_toggle_buttons(), focused, 350, 410);
    }
}

void map_selection_scroll(void)
{
    if (map_file_list.total <= list_rows()) {
        return;
    }
    if (button_parameter == 0) {
        if (map_file_list.scroll > 0) {
            window_redraw_request = 1;
            map_file_list.scroll--;
        }
        return;
    }
    if (button_parameter == 1) {
        if (map_file_list.scroll + list_rows() >= map_file_list.total) {
            return;
        }
        map_file_list.scroll++;
    }
    window_redraw_request = 1;
}

int32_t map_selection_handle_scrollbar(void)
{
    if (map_file_list.total <= list_rows() || mouse_left_down == 0) {
        return 0;
    }
    int32_t over = map_file_list.total - list_rows();
    int32_t height = scrollbar_height();
    int32_t top = list_top();
    if (mouse_x < ui_origin_x + 280 || mouse_x > ui_origin_x + 312) {
        return 0;
    }
    if (mouse_y < ui_origin_y + top + 25 || mouse_y > ui_origin_y + top + height + 50) {
        return 0;
    }
    int32_t at = mouse_y - ui_origin_y - top - 25;
    if (at > height) {
        at = height;
    }
    map_file_list.scroll = calc_adjust_with_percentage(over, calc_percentage(at, height));
    window_redraw_request = 1;
    return 1;
}

void map_selection_start(void)
{
    uint8_t replay = 0;
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    int32_t kind = list_kind(map_file_list.selected);
    if (kind == MAP_ENTRY_CUSTOM) {
        game_is_replay = GAME_PLAY_CUSTOM_SCENARIO;
    } else if (kind == MAP_ENTRY_CAMPAIGN) {
        replay = 1;
        game_is_replay = GAME_PLAY_MISSION_REPLAY;
    } else {
        return;
    }
    game_file_start_scenario(scenario_map_name, replay);
    if (game_is_replay == GAME_PLAY_MISSION_REPLAY) {
        mission_start_intermezzo();
        window_redraw_request = 1;
        return;
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
}

void map_selection_back(void)
{
    if (editor_active != 0) {
        window_current_id = WINDOW_FRONT_MENU;
        window_redraw_request = 1;
        return;
    }
    window_main_menu_show();
}

static void select_custom_map(void)
{
    char path[MAP_PATH_SIZE];
    memcpy(file_extension, map_file_extension, MAP_EXTENSION_LENGTH);
    file_append_extension(scenario_map_name);
    strcpy(path, maps_directory);
    strcat(path, scenario_map_name);
    game_file_load_scenario_data(path, 1);
    file_remove_extension(scenario_map_name);
}

static void select_campaign_mission(const char *name)
{
    int8_t mission = (int8_t) map_list_entry_mission(&map_file_list, map_file_list.selected);
    settings_data.mission_id = mission;
    game_file_load_campaign_mission(0, 1);
    settings_data.mission_id = mission;
    strcpy(scenario_map_name, name);
    file_remove_extension(scenario_map_name);
}

void map_selection_select(void)
{
    if (button_parameter >= map_file_list.total) {
        return;
    }
    map_file_list.selected = button_parameter + map_file_list.scroll;
    const char *name = list_name(map_file_list.selected);
    if (name == 0) {
        return;
    }
    memset(scenario_map_name, 0, MAP_NAME_SIZE);
    strcpy(scenario_map_name, name);
    file_remove_extension(scenario_map_name);
    int32_t kind = list_kind(map_file_list.selected);
    if (kind == MAP_ENTRY_CUSTOM) {
        select_custom_map();
    } else if (kind == MAP_ENTRY_CAMPAIGN || kind == MAP_ENTRY_LOCKED) {
        select_campaign_mission(name);
    } else {
        return;
    }
    window_redraw_request = 1;
}

void map_selection_load_list(const char *pattern)
{
    map_list_fill(&map_file_list, pattern, 0);
    map_file_list.total = map_file_list.files.count;
    if (map_file_list.campaign != 0) {
        return;
    }
    for (int32_t i = 0; i < map_file_list.total; i++) {
        file_remove_extension(list_name(i));
    }
    file_list_sort(&map_file_list.files, platform_text_compare_nocase);
}

void window_map_selection_show(void)
{
    window_current_id = WINDOW_MAP_SELECTION;
    map_selection_load_list(map_file_pattern);
    file_dialog_error_frames = 0;
    map_file_list.scroll = 0;
    file_dialog_type = FILE_DIALOG_LOAD;
    button_parameter = map_file_list.selected;
    map_selection_select();
    window_redraw_request = 1;
}
