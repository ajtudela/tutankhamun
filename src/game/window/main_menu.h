// The main menu's draws and buttons and the campaign queries (main_menu.c).

#ifndef WINDOW_MAIN_MENU_H
#define WINDOW_MAIN_MENU_H

#include "scenario/campaign.h"
#include <stdbool.h>
#include <stdint.h>

enum main_menu_text {
    GAME_MENU_TEXT_GROUP = 293,
    GAME_MENU_TEXT_TITLE = 5,
    GAME_MENU_TEXT_TITLE_ALL_PERIODS = 6,
    GAME_MENU_TEXT_START = 7,
    MISSION_SELECTION_TEXT_GROUP = 294,
    MISSION_SELECTION_TEXTS_PER_PERIOD = 4,
    MISSION_SELECTION_TEXT_DONE = 1,
    MISSION_SELECTION_TEXT_OPEN = 2,
    MISSION_SELECTION_TEXT_BEGIN = 36,
    MISSION_SELECTION_TEXT_SELECTED = 37,
    MISSION_SELECTION_TEXT_BACK = 38,
    MISSION_SELECTION_TEXT_PROCEED = 39,
    MISSION_SELECTION_TEXT_HELP = 41,
    MISSION_SELECTION_TEXT_HELP_2 = 42,
    MISSION_SELECTION_TEXT_SELECTED_HINT = 217,
    PERIOD_NAMES_GROUP = 27,
    FRONT_MENU_TEXT_GROUP = 30,
    NAME_INPUT_TITLE_GROUP = 31,
    NAME_INPUT_TEXT_HINT = 5,
    STARTUP_TEXT_CLICK = 7,
    NAME_INPUT_TEXT_OK = 8,
    POPUP_NEW_CAMPAIGN = 141,
    MENU_TEXT_LENGTH = 200,
};

enum mission_selection_mode {
    MISSION_SELECTION_NEW_GAME = 0,
    MISSION_SELECTION_ALL_PERIODS = 1,
};

void window_main_menu_draw_foreground(void);
void logo_window_draw(void);
void menu_background_draw(void);
void front_menu_draw(void);
void main_menu_button_click(void);
void window_main_menu_show(void);
void window_main_menu_draw_background(void);
void main_menu_button_continue(void);
void main_menu_button_campaign(void);
void main_menu_button_back(void);
void window_mission_selection_draw(void);
void window_mission_selection_draw_background(void);
void mission_selection_select(void);
void mission_selection_proceed(void);
void window_mission_selection_show_all(void);
void window_name_input_draw_foreground(void);
void main_menu_button_load(void);
void main_menu_button_custom(void);
int32_t mission_selection_button_count(void);
bool campaign_has_choices(campaign *self);
int32_t campaign_link_of_mission(campaign *self, int32_t mission_id);
bool campaign_period_valid(campaign *self, int32_t period);
bool campaign_period_open(campaign *self, int32_t period);
bool campaign_any_mission_played(campaign *self);
bool campaign_any_period_open(campaign *self);
int32_t campaign_mark_all_played(campaign *self);

#endif
