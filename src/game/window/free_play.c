// The free play window, a state of the front menu: every mission of the Pharaoh and Cleopatra campaigns grouped by
// period, with the map selection's panel for the chosen one, the difficulty arrows, back and start. A mission started
// here plays on its own: the campaign's records, high scores and stored monuments are left as they were, and its end
// returns to the front menu. Only a play without a script offers it; its strings are the port's own.
//
// The image button table ends with a zeroed record, which the button input reads.

#include "window/free_play.h"
#include "core/calc.h"
#include "game/scenario_start.h"
#include "graphics/button.h"
#include "graphics/generic_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "io/game_file.h"
#include "scenario/campaign.h"
#include "scenario/campaign_load.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "text/lang.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/map_selection.h"
#include "window/mission_flow.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum free_play_layout {
    FREE_PLAY_LIST_ROWS = 15,
    FREE_PLAY_LIST_TOP = 220,
    FREE_PLAY_LIST_LEFT = 24,
    FREE_PLAY_LIST_RIGHT = 280,
    FREE_PLAY_ROW_HEIGHT = 16,
    FREE_PLAY_MISSION_INDENT = 12,
    FREE_PLAY_LIST_WIDTH = FREE_PLAY_LIST_RIGHT - FREE_PLAY_LIST_LEFT,
    FREE_PLAY_SCROLLBAR_HEIGHT = FREE_PLAY_LIST_ROWS * FREE_PLAY_ROW_HEIGHT - 76,
    FREE_PLAY_SCROLL_DOT_IMAGE = 39,
    FREE_PLAY_IMAGE_BUTTONS = 4,
    FREE_PLAY_DIFFICULTY_ARROWS = 2,
    FREE_PLAY_FULL_SCREEN_WIDTH = 1024,
    FREE_PLAY_FULL_SCREEN_HEIGHT = 768,
    FREE_PLAY_NAME_LENGTH = 80,
};

enum free_play_campaign {
    FREE_PLAY_FIRST_PHARAOH_PERIOD = 0,
    FREE_PLAY_FIRST_CLEOPATRA_PERIOD = 5,
    FREE_PLAY_MAX_ROWS = CAMPAIGN_MAX_LINKS + CAMPAIGN_PERIODS + 2,
};

enum free_play_map_text {
    FREE_PLAY_MAP_TEXT_START = 136,
};

enum free_play_row_kind {
    FREE_PLAY_ROW_CAMPAIGN,
    FREE_PLAY_ROW_PERIOD,
    FREE_PLAY_ROW_MISSION,
    FREE_PLAY_ROW_ALTERNATIVE,
};

typedef struct free_play_row {
    int32_t kind;
    int32_t period;
    int32_t mission;
    int32_t text_id;
} free_play_row;

typedef struct free_play_settings {
    int8_t mission_id;
    uint8_t campaign;
    int8_t campaign_link;
    uint8_t previous_campaign_link;
    uint8_t play_mode;
    int32_t custom_scenario;
    int32_t starting_favor;
    int32_t savings;
    int32_t mission_count;
} free_play_settings;

static const char *const free_play_strings[FREE_PLAY_TEXT_COUNT] = {
    [FREE_PLAY_TEXT_BUTTON] = "Free play",
    [FREE_PLAY_TEXT_PHARAOH] = "Pharaoh",
    [FREE_PLAY_TEXT_CLEOPATRA] = "Cleopatra",
    [FREE_PLAY_TEXT_ALTERNATIVE] = "or ",
};

static int32_t allowed;
static int32_t shown;
static int32_t running;
static free_play_settings kept_settings;
static free_play_row rows[FREE_PLAY_MAX_ROWS];
static int32_t row_count;
static int32_t scroll;
static int32_t selected = -1;
static int32_t focused_row;

static void free_play_scroll(void);
static void free_play_back(void);
static void free_play_start(void);
static void free_play_select(void);

static image_button image_buttons[FREE_PLAY_IMAGE_BUTTONS + 1] = {
    { 282, 215, 34, 34, IMAGE_BUTTON_ANIMATED, 365, 8, 0, free_play_scroll, button_none, 1, 0, 0, 0, SCROLL_ARROW_UP, 0 },
    { 282, 435, 34, 34, IMAGE_BUTTON_ANIMATED, 365, 12, 0, free_play_scroll, button_none, 1, 0, 0, 0, SCROLL_ARROW_DOWN, 0 },
    { 335, 440, 27, 27, IMAGE_BUTTON_ANIMATED, 365, 4, 0, free_play_back, button_none, 1, 0, 0, 0, 1, 0 },
    { 600, 440, 27, 27, IMAGE_BUTTON_ANIMATED, 461, 0, 0, free_play_start, button_none, 1, 0, 0, 0, 1, 0 },
};

static generic_button row_buttons[FREE_PLAY_LIST_ROWS];

void free_play_allow(int32_t allow)
{
    allowed = allow;
}

int32_t free_play_allowed(void)
{
    return allowed;
}

const char *free_play_text(int32_t id)
{
    if (id < 0 || id >= FREE_PLAY_TEXT_COUNT) {
        return "";
    }
    return free_play_strings[id];
}

int32_t window_free_play_shown(void)
{
    return shown;
}

int32_t free_play_running(void)
{
    return running && game_is_replay == GAME_PLAY_MISSION_REPLAY;
}

static void keep_settings(void)
{
    kept_settings.mission_id = settings_data.mission_id;
    kept_settings.campaign = settings_data.campaign;
    kept_settings.campaign_link = settings_data.campaign_link;
    kept_settings.previous_campaign_link = settings_data.previous_campaign_link;
    kept_settings.play_mode = game_is_replay;
    kept_settings.custom_scenario = settings_data.is_custom_scenario;
    kept_settings.starting_favor = settings_data.starting_favor;
    kept_settings.savings = settings_data.personal_savings_last_mission;
    kept_settings.mission_count = settings_data.current_mission_id;
}

static void restore_settings(void)
{
    settings_data.mission_id = kept_settings.mission_id;
    settings_data.campaign = kept_settings.campaign;
    settings_data.campaign_link = kept_settings.campaign_link;
    settings_data.previous_campaign_link = kept_settings.previous_campaign_link;
    game_is_replay = kept_settings.play_mode;
    settings_data.is_custom_scenario = kept_settings.custom_scenario;
    settings_data.starting_favor = kept_settings.starting_favor;
    settings_data.personal_savings_last_mission = kept_settings.savings;
    settings_data.current_mission_id = kept_settings.mission_count;
}

static int32_t link_has_mission(const campaign_link *link)
{
    return link->mission_id >= 0 && link->mission_id < campaign_object.campaign.mission_name_count;
}

static void find_choice_partners(int32_t *partner)
{
    for (int32_t i = 0; i < campaign_object.campaign.link_count; i++) {
        partner[i] = -1;
    }
    for (int32_t i = 0; i < campaign_object.campaign.link_count; i++) {
        const campaign_link *link = &campaign_object.campaign.links[i];
        if (link->choice_count < CAMPAIGN_LINK_CHOICES) {
            continue;
        }
        int32_t first = campaign_find_link(&campaign_object.campaign, link->period, i, 0);
        int32_t second = campaign_find_link(&campaign_object.campaign, link->period, i, 1);
        if (first >= 0 && second >= 0 && first != second) {
            partner[first] = second;
            partner[second] = first;
        }
    }
}

static void add_row(int32_t kind, int32_t period, int32_t mission, int32_t text_id)
{
    if (row_count >= FREE_PLAY_MAX_ROWS) {
        return;
    }
    rows[row_count].kind = kind;
    rows[row_count].period = period;
    rows[row_count].mission = mission;
    rows[row_count].text_id = text_id;
    row_count++;
}

static void add_period_missions(int32_t period, const int32_t *partner, uint8_t *listed)
{
    for (int32_t i = 0; i < campaign_object.campaign.link_count; i++) {
        const campaign_link *link = &campaign_object.campaign.links[i];
        if (link->period != period || !link_has_mission(link) || listed[link->mission_id]) {
            continue;
        }
        listed[link->mission_id] = 1;
        int32_t second_of_choice = partner[i] >= 0 && partner[i] < i;
        add_row(second_of_choice ? FREE_PLAY_ROW_ALTERNATIVE : FREE_PLAY_ROW_MISSION, period, link->mission_id, -1);
    }
}

static void build_rows(void)
{
    int32_t partner[CAMPAIGN_MAX_LINKS];
    uint8_t listed[CAMPAIGN_MAX_MISSIONS];
    memset(listed, 0, sizeof(listed));
    find_choice_partners(partner);
    row_count = 0;
    for (int32_t period = 0; period < CAMPAIGN_PERIODS; period++) {
        if (period == FREE_PLAY_FIRST_PHARAOH_PERIOD) {
            add_row(FREE_PLAY_ROW_CAMPAIGN, period, -1, FREE_PLAY_TEXT_PHARAOH);
        } else if (period == FREE_PLAY_FIRST_CLEOPATRA_PERIOD) {
            add_row(FREE_PLAY_ROW_CAMPAIGN, period, -1, FREE_PLAY_TEXT_CLEOPATRA);
        }
        add_row(FREE_PLAY_ROW_PERIOD, period, -1, -1);
        add_period_missions(period, partner, listed);
    }
}

static void build_row_buttons(void)
{
    for (int32_t i = 0; i < FREE_PLAY_LIST_ROWS; i++) {
        generic_button *button = &row_buttons[i];
        memset(button, 0, sizeof(*button));
        button->x = FREE_PLAY_LIST_LEFT;
        button->y = (int16_t) (FREE_PLAY_LIST_TOP + i * FREE_PLAY_ROW_HEIGHT);
        button->x_end = FREE_PLAY_LIST_RIGHT;
        button->y_end = (int16_t) (button->y + FREE_PLAY_ROW_HEIGHT);
        button->left_click_handler = free_play_select;
        button->right_click_handler = button_none;
        button->type = GENERIC_BUTTON_PRESS;
        button->parameter1 = i;
    }
}

static int32_t row_is_mission(int32_t index)
{
    if (index < 0 || index >= row_count) {
        return 0;
    }
    return rows[index].kind == FREE_PLAY_ROW_MISSION || rows[index].kind == FREE_PLAY_ROW_ALTERNATIVE;
}

static const char *mission_name(int32_t mission)
{
    return campaign_object.campaign.mission_names[mission];
}

static void select_row(int32_t index)
{
    if (!row_is_mission(index)) {
        return;
    }
    selected = index;
    int8_t mission = (int8_t) rows[index].mission;
    settings_data.mission_id = mission;
    game_file_load_campaign_mission(0, 1);
    settings_data.mission_id = mission;
    strcpy(scenario_map_name, mission_name(mission));
    window_redraw_request = 1;
}

static int32_t first_mission_row(void)
{
    for (int32_t i = 0; i < row_count; i++) {
        if (row_is_mission(i)) {
            return i;
        }
    }
    return -1;
}

void window_free_play_show(void)
{
    keep_settings();
    build_rows();
    build_row_buttons();
    scroll = 0;
    focused_row = 0;
    shown = 1;
    editor_active = 0;
    overlay_show_all = 0;
    select_row(row_is_mission(selected) ? selected : first_mission_row());
    window_redraw_request = 1;
}

static void close_window(void)
{
    shown = 0;
    window_current_id = WINDOW_FRONT_MENU;
    window_redraw_request = 1;
}

static void free_play_back(void)
{
    restore_settings();
    close_window();
}

static void free_play_select(void)
{
    select_row(scroll + button_parameter);
}

static int32_t scroll_max(void)
{
    return row_count > FREE_PLAY_LIST_ROWS ? row_count - FREE_PLAY_LIST_ROWS : 0;
}

static void free_play_scroll(void)
{
    if (button_parameter == SCROLL_ARROW_UP && scroll > 0) {
        scroll--;
    } else if (button_parameter == SCROLL_ARROW_DOWN && scroll < scroll_max()) {
        scroll++;
    }
    window_redraw_request = 1;
}

static int32_t selected_period(void)
{
    return rows[selected].period;
}

static void free_play_start(void)
{
    if (!row_is_mission(selected)) {
        return;
    }
    int8_t mission = (int8_t) rows[selected].mission;
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    shown = 0;
    running = 1;
    editor_active = 0;
    overlay_show_all = 0;
    settings_data.is_custom_scenario = 0;
    settings_data.mission_id = mission;
    settings_data.campaign = (uint8_t) selected_period();
    game_is_replay = GAME_PLAY_MISSION_REPLAY;
    strcpy(scenario_map_name, mission_name(mission));
    game_file_start_scenario(scenario_map_name, 1);
    mission_start_intermezzo();
    window_redraw_request = 1;
}

void free_play_leave(void)
{
    running = 0;
    restore_settings();
    close_window();
}

void free_play_forget(void)
{
    running = 0;
}

static int32_t row_font(int32_t index, int32_t screen_row)
{
    if (focused_row == screen_row + 1 || index == selected) {
        return image_group(IMAGE_GROUP_FONT_WHITE);
    }
    return image_group(IMAGE_GROUP_FONT_GREEN);
}

static void draw_mission_row(int32_t index, int32_t screen_row, int32_t y)
{
    char text[FREE_PLAY_NAME_LENGTH];
    text[0] = 0;
    if (rows[index].kind == FREE_PLAY_ROW_ALTERNATIVE) {
        strcpy(text, free_play_text(FREE_PLAY_TEXT_ALTERNATIVE));
    }
    strncat(text, mission_name(rows[index].mission), FREE_PLAY_NAME_LENGTH - strlen(text) - 1);
    text_draw((const uint8_t *) text, ui_origin_x + FREE_PLAY_LIST_LEFT + FREE_PLAY_MISSION_INDENT, y, row_font(index, screen_row), 0);
}

static void draw_row(int32_t index, int32_t screen_row)
{
    int32_t y = ui_origin_y + FREE_PLAY_LIST_TOP + screen_row * FREE_PLAY_ROW_HEIGHT;
    const free_play_row *row = &rows[index];
    switch (row->kind) {
        case FREE_PLAY_ROW_CAMPAIGN:
            text_draw_centered((const uint8_t *) free_play_text(row->text_id), ui_origin_x + FREE_PLAY_LIST_LEFT, y, FREE_PLAY_LIST_WIDTH, image_group(IMAGE_GROUP_FONT_WHITE), 0);
            break;
        case FREE_PLAY_ROW_PERIOD:
            lang_text_draw(PERIOD_NAMES_GROUP, row->period, ui_origin_x + FREE_PLAY_LIST_LEFT, y, image_group(IMAGE_GROUP_FONT_WHITE), 0);
            break;
        default:
            draw_mission_row(index, screen_row, y);
            break;
    }
}

static void draw_list(void)
{
    inner_panel_draw(ui_origin_x + 15, ui_origin_y + FREE_PLAY_LIST_TOP - 15, 16, FREE_PLAY_LIST_ROWS + 1);
    for (int32_t screen_row = 0; screen_row < FREE_PLAY_LIST_ROWS; screen_row++) {
        int32_t index = scroll + screen_row;
        if (index >= row_count) {
            break;
        }
        draw_row(index, screen_row);
    }
}

static void draw_scroll_thumb(void)
{
    int32_t over = scroll_max();
    if (over <= 0) {
        return;
    }
    int32_t offset = calc_adjust_with_percentage(FREE_PLAY_SCROLLBAR_HEIGHT, calc_percentage(scroll, over));
    image_draw(image_group(IMAGE_GROUP_PANEL_BUTTON) + FREE_PLAY_SCROLL_DOT_IMAGE, ui_origin_x + 289, ui_origin_y + offset + FREE_PLAY_LIST_TOP + 25);
}

static void draw_selected_mission(void)
{
    int32_t x = ui_origin_x + 320;
    int32_t y = ui_origin_y + 20;
    int32_t label_x = x + 15;
    map_selection_draw_scenario_picture(ui_origin_x, ui_origin_y + 36);
    text_draw_centered((const uint8_t *) scenario_map_name, label_x, y + 45, 260, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    text_draw_centered((const uint8_t *) scenario_data.name, label_x, y + 65, 260, image_group(IMAGE_GROUP_FONT_WHITE), 0);
    lang_text_draw_year(1, scenario_data.start.year, x + 90, y + 85, image_group(IMAGE_GROUP_FONT_GREEN), 0);
    map_selection_draw_map_information(x, y, label_x, image_group(IMAGE_GROUP_FONT_GREEN));
    // The start-mission arrow sits at x + 280: end the hint 8 px before it.
    int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
    int32_t width = lang_text_get_width(LANG_GROUP_EDITOR, FREE_PLAY_MAP_TEXT_START, font);
    lang_text_draw(LANG_GROUP_EDITOR, FREE_PLAY_MAP_TEXT_START, x + 272 - width, y + 426, font, 0);
}

void window_free_play_draw(void)
{
    image_view_x = (FREE_PLAY_FULL_SCREEN_WIDTH - g_screen_width) / 2;
    image_view_y = (FREE_PLAY_FULL_SCREEN_HEIGHT - g_screen_height) / 2;
    image_view_width = g_screen_width;
    image_view_height = g_screen_height;
    window_ui_redraw = 1;
    image_draw(image_group(IMAGE_GROUP_MAP_BACKGROUND), ui_origin_x - 192, ui_origin_y - 144);
    inner_panel_draw(ui_origin_x + 286, ui_origin_y + FREE_PLAY_LIST_TOP + 22, 2, FREE_PLAY_LIST_ROWS - 3);
    draw_list();
    draw_scroll_thumb();
    if (row_is_mission(selected)) {
        draw_selected_mission();
    }
    image_buttons_draw(ui_origin_x, ui_origin_y, image_buttons, FREE_PLAY_IMAGE_BUTTONS);
    image_buttons_draw(ui_origin_x, ui_origin_y, map_selection_image_buttons(), FREE_PLAY_DIFFICULTY_ARROWS);
}

int32_t window_free_play_handle_input(void)
{
    if (!shown) {
        return 0;
    }
    if (image_buttons_handle_mouse(ui_origin_x, ui_origin_y, image_buttons, FREE_PLAY_IMAGE_BUTTONS) != 0) {
        return 1;
    }
    if (image_buttons_handle_mouse(ui_origin_x, ui_origin_y, map_selection_image_buttons(), FREE_PLAY_DIFFICULTY_ARROWS) != 0) {
        return 1;
    }
    focused_row = generic_buttons_get_focus(ui_origin_x, ui_origin_y, row_buttons, FREE_PLAY_LIST_ROWS);
    if (generic_buttons_handle_mouse(ui_origin_x, ui_origin_y, row_buttons, FREE_PLAY_LIST_ROWS) != 0) {
        return 1;
    }
    if (mouse_right_went_up != 0) {
        free_play_back();
    }
    return 1;
}
