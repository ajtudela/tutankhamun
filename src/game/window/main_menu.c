// The main menu: the logo window, the menu background and the front menu with its buttons (new game, the web site or
// Free play, the editor, high scores, exit) and the play mode's link buttons; the player's game menu, the mission
// selection of a campaign period, the player name input and the campaign queries they use. A period is open when a
// played mission of it has no follow-up link; Cleopatra's periods 5..8 are always open.
//
// The replay selection counts no period as done: every period stays open there.

#include "window/main_menu.h"
#include "core/static_init.h"
#include "editor/editor_data.h"
#include "game/game_data.h"
#include "graphics/button_border.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "io/file.h"
#include "io/game_file.h"
#include "io/io_data.h"
#include "platform/platform_dialog.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "platform/platform_shell.h"
#include "platform/platform_window.h"
#include "scenario/campaign_load.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "text/event_text.h"
#include "text/lang.h"
#include "text/text_lookup.h"
#include "widget/widget_data.h"
#include "window/file_dialog_buttons.h"
#include "window/file_list.h"
#include "window/free_play.h"
#include "window/high_scores.h"
#include "window/intermezzo.h"
#include "window/main_menu_links.h"
#include "window/map_selection.h"
#include "window/message_box.h"
#include "window/player_selection.h"
#include "window/text_input.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum campaign_periods {
    CAMPAIGN_FIRST_CLEOPATRA_PERIOD = 5,
};

enum main_menu_layout {
    GAME_MENU_ENTRIES = 5,
    GAME_MENU_CONTINUE = 0,
    GAME_MENU_NEW_CAMPAIGN = 1,
    GAME_MENU_LOAD = 2,
    MISSION_SELECTION_PERIOD_BUTTONS = 9,
    MISSION_SELECTION_BACK = 9,
    CUSTOM_SCENARIO_MAP = 2,
    LABEL_ANIMATION_MENU = 6,
    LABEL_FULLY_GROWN = 100,
    FULL_SCREEN_WIDTH = 1024,
    FULL_SCREEN_HEIGHT = 768,
};

enum front_menu_button {
    FRONT_MENU_NEW_GAME = 0,
    FRONT_MENU_WEB_SITE = 1,
    FRONT_MENU_EDITOR = 2,
    FRONT_MENU_HIGH_SCORES = 3,
    FRONT_MENU_EXIT = 4,
    FRONT_MENU_LABEL_DY = 6,
};

bool campaign_has_choices(campaign *self)
{
    int32_t index = (int8_t) settings_data.previous_campaign_link;
    if (index < 0 || index >= self->link_count) {
        return false;
    }
    return self->links[index].choice_count > 1;
}

int32_t campaign_link_of_mission(campaign *self, int32_t mission_id)
{
    for (int32_t i = 0; i < self->link_count; i++) {
        if (self->links[i].mission_id == mission_id) {
            return i;
        }
    }
    return -1;
}

bool campaign_period_valid(campaign *self, int32_t period)
{
    return period >= 0 && period < CAMPAIGN_PERIODS;
}

bool campaign_period_open(campaign *self, int32_t period)
{
    if (period >= CAMPAIGN_FIRST_CLEOPATRA_PERIOD) {
        return true;
    }
    for (int32_t i = 0; i < self->link_count; i++) {
        const campaign_link *link = &self->links[i];
        if (link->period == period && link->played != 0 && campaign_find_link(&campaign_object.campaign, period, i, 0) < 0) {
            return true;
        }
    }
    return false;
}

bool campaign_any_mission_played(campaign *self)
{
    for (int32_t i = 0; i < self->link_count; i++) {
        if (self->links[i].played != 0) {
            return true;
        }
    }
    return false;
}

bool campaign_any_period_open(campaign *self)
{
    for (int32_t period = 0; period < CAMPAIGN_PERIODS; period++) {
        if (campaign_period_open(self, period)) {
            return true;
        }
    }
    return false;
}

int32_t campaign_mark_all_played(campaign *self)
{
    int32_t i = 0;
    while (i < self->link_count) {
        self->links[i].played = 1;
        i++;
    }
    return i;
}

void mission_selection_select(void)
{
    window_redraw_request = 1;
    mission_selected = button_parameter;
}

int32_t mission_selection_button_count(void)
{
    if (mission_select_mode == MISSION_SELECTION_NEW_GAME) {
        return mission_selected == campaign_period_reached;
    }
    return 2;
}

static void set_full_screen_offsets(void)
{
    image_view_x = (FULL_SCREEN_WIDTH - g_screen_width) / 2;
    image_view_width = g_screen_width;
    image_view_y = (FULL_SCREEN_HEIGHT - g_screen_height) / 2;
    image_view_height = g_screen_height;
}

static void draw_title_without_player_title(int32_t text, int32_t x, int32_t y, int32_t width)
{
    char expanded[MENU_TEXT_LENGTH];
    const char *source = lang_get_string(GAME_MENU_TEXT_GROUP, text);
    int32_t title = scenario_data.player_title_id;
    scenario_data.player_title_id = 0;
    event_text_expand(source, expanded, MENU_TEXT_LENGTH);
    scenario_data.player_title_id = title;
    text_draw_centered((const uint8_t *) expanded, x, y, width, image_group(IMAGE_GROUP_FONT_LARGE), COLOR_BLACK);
}

static int32_t period_background(int32_t period)
{
    static const int32_t cleopatra_images[4] = { 4, 2, 5, 7 };
    if (period < CAMPAIGN_FIRST_CLEOPATRA_PERIOD) {
        return image_group(IMAGE_GROUP_MISSION_BACKGROUNDS) + period;
    }
    int32_t cleopatra = period - CAMPAIGN_FIRST_CLEOPATRA_PERIOD;
    return image_group(IMAGE_GROUP_CLEOPATRA_BACKGROUNDS) + cleopatra_images[cleopatra < 3 ? cleopatra : 3];
}

static void draw_period_description(void)
{
    lang_text_draw_centered(MISSION_SELECTION_TEXT_GROUP, mission_selected * MISSION_SELECTION_TEXTS_PER_PERIOD, ui_origin_x + 339, ui_origin_y + 60, 283,
        image_group(IMAGE_GROUP_FONT_LARGE_B), COLOR_BLACK);
    int32_t index;
    if (mission_select_mode == MISSION_SELECTION_NEW_GAME) {
        if (mission_selected < campaign_period_reached) {
            index = MISSION_SELECTION_TEXT_DONE;
        } else {
            index = (mission_selected != campaign_period_reached) + MISSION_SELECTION_TEXT_OPEN;
        }
    } else {
        index = MISSION_SELECTION_TEXT_OPEN;
    }
    lang_text_draw_multiline(MISSION_SELECTION_TEXT_GROUP, index + (mission_selected * MISSION_SELECTION_TEXTS_PER_PERIOD), ui_origin_x + 347,
        ui_origin_y + 116, 269, image_group(IMAGE_GROUP_FONT_GREEN), COLOR_BLACK);
}

static bool period_closed(int32_t i)
{
    return mission_select_mode == MISSION_SELECTION_NEW_GAME && i != campaign_period_reached;
}

static bool draw_period_buttons(void)
{
    bool selected_open = false;
    for (int32_t i = 0; i < MISSION_SELECTION_PERIOD_BUTTONS; i++) {
        const generic_button *button = &mission_selection_tables.mission_buttons[i];
        int32_t width = button->x_end - button->x;
        int32_t focused = focus_button_id == i + 1;
        bool closed = period_closed(i);
        if (i == mission_selected) {
            selected_open = !closed;
        }
        int32_t font = image_group(IMAGE_GROUP_FONT_BLACK);
        if (closed) {
            font = image_group(IMAGE_GROUP_FONT_RED);
            focused = 0;
        }
        large_label_draw_animated(LABEL_ANIMATION_MENU, button->x + ui_origin_x, button->y + ui_origin_y, width / 16, focused);
        lang_text_draw_centered_rect(PERIOD_NAMES_GROUP, i, &button->x, font, 0, 0, 6);
    }
    return selected_open;
}

static void draw_back_and_proceed(void)
{
    static const int32_t texts[2] = { MISSION_SELECTION_TEXT_BACK, MISSION_SELECTION_TEXT_PROCEED };
    for (int32_t b = 0; b < 2; b++) {
        int32_t focused = b == 1 || focus_button_id == b + 10;
        const generic_button *button = &mission_selection_tables.mission_buttons[MISSION_SELECTION_BACK + b];
        button_border_draw_rect(&button->x, focused, 0, 0);
        lang_text_draw_centered_rect(MISSION_SELECTION_TEXT_GROUP, texts[b], &button->x, image_group(IMAGE_GROUP_FONT_BLACK), 0, 4, 7);
    }
}

void window_mission_selection_draw(void)
{
    set_full_screen_offsets();
    window_ui_redraw = 1;
    int32_t y = mission_select_mode == MISSION_SELECTION_NEW_GAME ? ui_origin_y + 60 : ui_origin_y + 56;
    image_draw(period_background(mission_selected), ui_origin_x + 78, y);
    draw_title_without_player_title(
        mission_select_mode == MISSION_SELECTION_NEW_GAME ? GAME_MENU_TEXT_TITLE : GAME_MENU_TEXT_TITLE_ALL_PERIODS, ui_origin_x + 20, ui_origin_y + 17, 600);
    if (campaign_period_valid(&campaign_object.campaign, mission_selected)) {
        draw_period_description();
    }
    uint8_t selected_open = draw_period_buttons();
    if (mission_select_mode == MISSION_SELECTION_ALL_PERIODS || mission_select_mode == MISSION_SELECTION_NEW_GAME) {
        lang_text_draw_multiline(MISSION_SELECTION_TEXT_GROUP, MISSION_SELECTION_TEXT_HELP, ui_origin_x + 30, ui_origin_y + 266, 144, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
        lang_text_draw_multiline(
            MISSION_SELECTION_TEXT_GROUP, MISSION_SELECTION_TEXT_HELP_2, ui_origin_x + 30, ui_origin_y + 366, 144, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
    }
    if (mission_select_mode == MISSION_SELECTION_ALL_PERIODS) {
        draw_back_and_proceed();
    }
    if (selected_open) {
        if (mission_select_mode == MISSION_SELECTION_NEW_GAME) {
            lang_text_draw(MISSION_SELECTION_TEXT_GROUP, MISSION_SELECTION_TEXT_BEGIN, ui_origin_x + 420, ui_origin_y + 445, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
        } else {
            lang_text_draw(MISSION_SELECTION_TEXT_GROUP, MISSION_SELECTION_TEXT_SELECTED, ui_origin_x + 550, ui_origin_y + 445, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
            lang_text_draw(LANG_GROUP_EDITOR, MISSION_SELECTION_TEXT_SELECTED_HINT, ui_origin_x + 380, ui_origin_y + 445, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
        }
    }
    window_redraw_request = 1;
}

void window_mission_selection_draw_background(void)
{
    image_draw(image_group(IMAGE_GROUP_MISSION_FRAME), ui_origin_x - 192, ui_origin_y - 144);
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = LABEL_FULLY_GROWN;
    window_ui_redraw = 1;
}

static void open_map_selection(int32_t kind, int32_t campaign_maps)
{
    editor_active = 0;
    overlay_show_all = 0;
    map_file_list.selection_kind = kind;
    map_file_list.campaign = campaign_maps;
    window_map_selection_show();
}

void mission_selection_proceed(void)
{
    if (window_current_id == WINDOW_MISSION_SELECTION && mission_select_mode == MISSION_SELECTION_ALL_PERIODS) {
        open_map_selection(MAP_SELECTION_CAMPAIGN, 1);
        window_redraw_request = 1;
    }
}

void window_mission_selection_show_all(void)
{
    if (window_current_id != WINDOW_MISSION_SELECTION) {
        window_current_id = WINDOW_MISSION_SELECTION;
        mission_select_mode = MISSION_SELECTION_ALL_PERIODS;
        window_redraw_request = 1;
    }
}

static void play_menu_music(void)
{
    if (sound_digital_ok != 0 && sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_MUSIC) == 0 && settings_data.sound_music_enabled != 0) {
        sound_lib_play_stream(&sound_manager, setup_mp3_path, SOUND_STREAM_MUSIC, 0);
    }
}

static void draw_game_menu_entry(int32_t entry, int32_t text)
{
    const generic_button *button = &game_menu_buttons[entry];
    lang_text_draw_centered_rect(GAME_MENU_TEXT_GROUP, text, &button->x, image_group(IMAGE_GROUP_FONT_BLACK), 0, 4, 7);
    button_border_draw_rect(&button->x, focus_button_id == entry + 1, 0, 0);
}

void window_main_menu_draw_foreground(void)
{
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_SPEECH);
    play_menu_music();
    for (int32_t i = 0; i < GAME_MENU_ENTRIES; i++) {
        if (i == GAME_MENU_CONTINUE && !menu_can_continue) {
            draw_game_menu_entry(GAME_MENU_CONTINUE, GAME_MENU_TEXT_START);
            continue;
        }
        if ((i == GAME_MENU_NEW_CAMPAIGN && !menu_can_start) || (i == GAME_MENU_LOAD && !menu_has_saves)) {
            continue;
        }
        draw_game_menu_entry(i, i);
    }
}

void window_main_menu_draw_background(void)
{
    if (window_redraw_request == 1) {
        image_draw(image_group(IMAGE_GROUP_MAIN_MENU_BACKGROUND), ui_origin_x, ui_origin_y);
    }
    outer_panel_draw(ui_origin_x + 128, ui_origin_y + 40, 24, 21);
    draw_title_without_player_title(GAME_MENU_TEXT_TITLE, ui_origin_x + 140, ui_origin_y + 60, 368);
    window_main_menu_draw_foreground();
}

static bool player_has_saved_games(void)
{
    file_list list;
    char path[FILE_PATH_SIZE];
    file_list_construct(&list);
    strcpy(path, save_folder_path);
    strcat(path, settings_data.player_name);
    strcat(path, save_file_pattern);
    file_list_list_dir(&list, path, 0);
    bool has_saves = list.count > 0;
    file_list_destroy(&list);
    return has_saves;
}

void window_main_menu_show(void)
{
    menu_period_open = campaign_any_period_open(&campaign_object.campaign);
    menu_can_start = 1;
    menu_can_continue = 0;
    menu_has_saves = 0;
    if (campaign_object.campaign.saved_game_path[0] != 0) {
        FILE *file = fopen(campaign_object.campaign.saved_game_path, file_mode_read_binary);
        if (file) {
            fclose(file);
            menu_can_continue = 1;
        }
    }
    menu_has_saves = player_has_saved_games();
    free_play_forget();
    window_current_id = WINDOW_MAIN_MENU;
    window_redraw_request = 1;
}

void main_menu_button_continue(void)
{
    sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    if (menu_can_continue) {
        editor_active = 0;
        overlay_show_all = 0;
        game_file_load_saved_game(campaign_object.campaign.saved_game_path);
        window_current_id = WINDOW_CITY;
        window_redraw_request = 1;
        return;
    }
    mission_selected = 0;
    campaign_period_reached = 0;
    mission_select_mode = MISSION_SELECTION_NEW_GAME;
    window_current_id = WINDOW_MISSION_SELECTION;
    window_redraw_request = 1;
}

void main_menu_button_campaign(void)
{
    if (!menu_can_start) {
        return;
    }
    if (!campaign_any_mission_played(&campaign_object.campaign)) {
        const char *title = lang_get_string(LANG_GROUP_POPUP, POPUP_NEW_CAMPAIGN);
        const char *text = lang_get_string(LANG_GROUP_POPUP, POPUP_NEW_CAMPAIGN + 1);
        if (window_message_box_show(title, text, PLATFORM_BOX_YES_NO) == PLATFORM_ANSWER_NO) {
            return;
        }
    }
    if (menu_period_open) {
        window_current_id = WINDOW_MISSION_SELECTION;
        mission_select_mode = MISSION_SELECTION_ALL_PERIODS;
        window_redraw_request = 1;
        return;
    }
    open_map_selection(MAP_SELECTION_CAMPAIGN, 1);
    window_redraw_request = 1;
}

void main_menu_button_load(void)
{
    if (menu_has_saves) {
        editor_active = 0;
        overlay_show_all = 0;
        file_menu_load_game();
        file_dialog_return_window.id = WINDOW_MAIN_MENU;
    }
}

void main_menu_button_custom(void)
{
    settings_data.is_custom_scenario = CUSTOM_SCENARIO_MAP;
    open_map_selection(MAP_SELECTION_CUSTOM, 0);
}

void main_menu_button_back(void)
{
    window_player_selection_show();
}

void window_name_input_draw_foreground(void)
{
    outer_panel_draw(ui_origin_x + 128, ui_origin_y + 160, 24, 8);
    lang_text_draw_centered_ui(NAME_INPUT_TITLE_GROUP, 0, ui_origin_x + 128, ui_origin_y + 172, 384, image_group(IMAGE_GROUP_FONT_LARGE), COLOR_BLACK, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(LANG_GROUP_WINDOW_HINTS, NAME_INPUT_TEXT_HINT, ui_origin_x + 395, ui_origin_y + 255, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK, ui_origin_x, ui_origin_y);
    int32_t focused = focus_button_id == 1;
    int32_t color = focused ? COLOR_RED : COLOR_BLACK;
    button_border_draw_ui(ui_origin_x + 352, ui_origin_y + 246, 25, 25, focused, ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(
        LANG_GROUP_WINDOW_HINTS, NAME_INPUT_TEXT_OK, ui_origin_x + 145, ui_origin_y + 256, 180, image_group(IMAGE_GROUP_FONT_BLACK), color, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(ui_origin_x + 160, ui_origin_y + 208, 20, 2, ui_origin_x, ui_origin_y);
    text_cursor_reset();
    text_draw((const uint8_t *) settings_data.player_name, ui_origin_x + 176, ui_origin_y + 216, image_group(IMAGE_GROUP_FONT_WHITE), COLOR_BLACK);
    graphics_draw_text_cursor(ui_origin_x + 176, ui_origin_y + 217, COLOR_WHITE);
}

void main_menu_button_click(void)
{
    uint32_t button = (uint32_t) button_parameter;
    settings_data.mission_id = 0;
    empire_editor_route_dragging = 1;
    switch (button) {
        case FRONT_MENU_NEW_GAME:
            window_player_selection_show();
            g_sidebar_collapsed = 0;
            return;
        case FRONT_MENU_WEB_SITE:
            if (free_play_allowed()) {
                window_free_play_show();
                return;
            }
            platform_window_minimize(platform_main_window);
            platform_shell_open(platform_main_window, "open", "www.activision.com");
            return;
        case FRONT_MENU_EDITOR:
            settings_data.is_custom_scenario = CUSTOM_SCENARIO_MAP;
            editor_active = 1;
            settings_data.paused = 0;
            g_sidebar_collapsed = 0;
            map_file_list.campaign = 0;
            map_file_list.selection_kind = MAP_SELECTION_CUSTOM;
            window_map_selection_show();
            return;
        case FRONT_MENU_HIGH_SCORES:
            highscore_load();
            window_current_id = WINDOW_HIGH_SCORES;
            return;
        case FRONT_MENU_EXIT:
            platform_message_post(platform_main_window, PLATFORM_MSG_KEY_DOWN, PLATFORM_KEY_ESCAPE, 0);
            return;
        default:
            return;
    }
}

void logo_window_draw(void)
{
    play_menu_music();
    image_draw(image_group(IMAGE_GROUP_LOGO), ui_origin_x, ui_origin_y);
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, STARTUP_TEXT_CLICK, (display_width - 320) / 2, ui_origin_y + 462, 320, image_group(IMAGE_GROUP_FONT_SMALL), COLOR_WHITE, 0, 0);
}

void menu_background_draw(void)
{
    image_draw(image_group(IMAGE_GROUP_MENU_BACKGROUND), ui_origin_x, ui_origin_y);
    main_menu_links_keep_background();
    label_animation[LABEL_SLOT_REFRESH_COUNTDOWN] = LABEL_FULLY_GROWN;
}

static void draw_front_menu_label(int32_t button, const generic_button *entry)
{
    if (button == FRONT_MENU_WEB_SITE && free_play_allowed()) {
        text_draw_centered((const uint8_t *) free_play_text(FREE_PLAY_TEXT_BUTTON), entry->x + ui_origin_x, entry->y + ui_origin_y + FRONT_MENU_LABEL_DY,
            entry->x_end - entry->x, image_group(IMAGE_GROUP_FONT_BLACK), COLOR_BLACK);
        return;
    }
    lang_text_draw_centered_rect(FRONT_MENU_TEXT_GROUP, (int32_t) main_menu_button_labels[button], &entry->x, image_group(IMAGE_GROUP_FONT_BLACK), 0, 0, FRONT_MENU_LABEL_DY);
}

void front_menu_draw(void)
{
    if (window_free_play_shown()) {
        window_free_play_draw();
        return;
    }
    for (int32_t i = 0; i < GAME_MENU_ENTRIES; i++) {
        const generic_button *entry = &main_menu_buttons[i];
        int32_t width = entry->x_end - entry->x;
        large_label_draw_animated(LABEL_ANIMATION_MENU, entry->x + ui_origin_x, entry->y + ui_origin_y, width / 16, focus_button_id == i + 1);
        draw_front_menu_label(i, entry);
    }
    main_menu_links_draw();
}
