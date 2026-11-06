// The scenario dialogs (editor_scenario_settings.c): their functions and buttons, the eras of the start year, the
// steps of the flood settings (season, duration, quality) and the sizes of the name lists.

#ifndef EDITOR_EDITOR_SCENARIO_SETTINGS_H
#define EDITOR_EDITOR_SCENARIO_SETTINGS_H

#include <stdint.h>

enum start_year_button {
    START_YEAR_BUTTON_ERA = 1,
    START_YEAR_BUTTON_YEAR = 2,
};

enum settings_button {
    SETTINGS_BUTTON_START_DATE = 1,
    SETTINGS_BUTTON_INITIAL_FUNDS = 2,
    SETTINGS_BUTTON_PHARAOH_GIFT = 3,
    SETTINGS_BUTTON_MILESTONE_25 = 4,
    SETTINGS_BUTTON_MILESTONE_50 = 5,
    SETTINGS_BUTTON_MILESTONE_75 = 6,
    SETTINGS_BUTTON_INITIAL_RANK = 7,
    SETTINGS_BUTTON_INTEREST_RATE = 8,
    SETTINGS_BUTTON_CURRENT_PHARAOH = 9,
    SETTINGS_BUTTON_PLAYER_INCARNATION = 10,
};

enum scenario_era {
    ERA_BC = 0,
    ERA_AD = 1,
};

enum flood_setting_steps {
    FLOOD_SEASON_STEP_DAYS = 15,
    FLOOD_SEASON_FIRST_STEP = 10,
    FLOOD_SEASON_LAST_STEP = 7,
    FLOOD_DURATION_STEP_DAYS = 30,
    FLOOD_DURATION_FIRST_MONTHS = 2,
    FLOOD_DURATION_LAST_MONTHS = 4,
    FLOOD_QUALITY_STEP = 20,
    FLOOD_QUALITY_PERFECT = 100,
};

enum flood_settings_rows {
    FLOOD_SETTINGS_ROWS = 3,
};

enum trade_amounts_layout {
    TRADE_AMOUNTS_PER_ROW = 8,
    TRADE_AMOUNTS_ROWS = 2,
};

enum settings_list_size {
    PLAYER_RANK_COUNT = 11,
    PHARAOH_NAME_COUNT = 63,
    PLAYER_INCARNATION_COUNT = 31,
};

void window_editor_flood_draw(void);
void window_editor_settings_draw(void);
void window_editor_start_year_draw(void);
void window_editor_trade_amounts_draw(void);
void editor_start_year_button(void);
void editor_start_year_toggle_era(void);
void editor_flood_season_next(void);
void editor_flood_duration_next(void);
void editor_flood_quality_next(void);
void editor_settings_funds(void);
void editor_settings_bailout(void);
void editor_settings_interest(void);
void editor_settings_empire_map(void);
void editor_settings_player_title(void);
void editor_settings_start_year(void);
void editor_settings_milestone(void);
void editor_settings_pharaoh_name(void);
void editor_trade_amount_next(void);

#endif
