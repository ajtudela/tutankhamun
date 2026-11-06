// The win criteria window (editor_win_criteria.c): its functions, buttons, rows and layout.

#ifndef EDITOR_EDITOR_WIN_CRITERIA_H
#define EDITOR_EDITOR_WIN_CRITERIA_H

#include <stdint.h>

enum win_criteria_button {
    WIN_CRITERIA_BUTTON_CULTURE = 1,
    WIN_CRITERIA_BUTTON_CULTURE_VALUE = 2,
    WIN_CRITERIA_BUTTON_PROSPERITY = 3,
    WIN_CRITERIA_BUTTON_PROSPERITY_VALUE = 4,
    WIN_CRITERIA_BUTTON_MONUMENTS = 6,
    WIN_CRITERIA_BUTTON_KINGDOM = 7,
    WIN_CRITERIA_BUTTON_KINGDOM_VALUE = 8,
    WIN_CRITERIA_BUTTON_TIME_LIMIT = 9,
    WIN_CRITERIA_BUTTON_TIME_LIMIT_YEARS = 10,
    WIN_CRITERIA_BUTTON_SURVIVAL = 11,
    WIN_CRITERIA_BUTTON_SURVIVAL_YEARS = 12,
    WIN_CRITERIA_BUTTON_POPULATION = 13,
    WIN_CRITERIA_BUTTON_POPULATION_VALUE = 14,
    WIN_CRITERIA_BUTTON_OPEN_PLAY = 15,
    WIN_CRITERIA_BUTTON_OPEN_PLAY_ID = 16,
    WIN_CRITERIA_BUTTON_HOUSES = 17,
    WIN_CRITERIA_BUTTON_HOUSES_LEVEL = 18,
};

enum win_criteria_row {
    WIN_CRITERIA_ROW_OPEN_PLAY = 0,
    WIN_CRITERIA_ROW_HOUSES = 1,
    WIN_CRITERIA_ROW_CULTURE = 2,
    WIN_CRITERIA_ROW_PROSPERITY = 3,
    WIN_CRITERIA_ROW_MONUMENTS = 4,
    WIN_CRITERIA_ROW_KINGDOM = 5,
    WIN_CRITERIA_ROW_TIME_LIMIT = 6,
    WIN_CRITERIA_ROW_SURVIVAL = 7,
    WIN_CRITERIA_ROW_POPULATION = 8,
};

enum win_criteria_layout {
    WIN_CRITERIA_FIRST_ROW = 60,
    WIN_CRITERIA_ROW_SPACING = 40,
    WIN_CRITERIA_TEXT_DY = 9,
    WIN_CRITERIA_LABEL_X = 66,
    WIN_CRITERIA_TOGGLE_X = 316,
    WIN_CRITERIA_TOGGLE_WIDTH = 80,
    WIN_CRITERIA_VALUE_X = 416,
    WIN_CRITERIA_VALUE_WIDTH = 180,
    WIN_CRITERIA_BUTTON_HEIGHT = 30,
    WIN_CRITERIA_PROSPERITY_OFF_PANEL = 1600,
};

void window_editor_win_criteria_draw(void);
void editor_criteria_open_play_toggle(void);
void editor_criteria_open_play_id(void);
void editor_criteria_houses(void);
void editor_criteria_houses_level(void);
void editor_criteria_goal_toggle(void);
void editor_criteria_goal(void);
void editor_criteria_time_limit_toggle(void);
void editor_criteria_time_limit_years(void);
void editor_criteria_survival_toggle(void);
void editor_criteria_survival_years(void);
void editor_criteria_population_toggle(void);
void editor_criteria_population(void);

#endif
