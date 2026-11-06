// The mission flow windows (mission_flow.c): the cleanup after a lost mission, how the game was started
// (game_is_replay) and the mission options of text group 307 ('1' at 0 keeps the savings, '1' at 2 the legions, 'A'
// or 'V' at 4 places the monuments of earlier missions, '1' at 6 stores them).

#ifndef WINDOW_MISSION_FLOW_H
#define WINDOW_MISSION_FLOW_H

#include "graphics/button.h"
#include <stdbool.h>
#include <stdint.h>

enum game_play_mode {
    GAME_PLAY_CUSTOM_SCENARIO = 0,
    GAME_PLAY_CAMPAIGN = 1,
    GAME_PLAY_CAMPAIGN_FROM_PERIOD = 2,
    GAME_PLAY_MISSION_REPLAY = 3,
};

enum mission_options {
    MISSION_OPTIONS_GROUP = 307,
    MISSION_OPTION_KEEP_SAVINGS = 0,
    MISSION_OPTION_KEEP_LEGIONS = 2,
    MISSION_OPTION_MONUMENTS = 4,
    MISSION_OPTION_STORE_MONUMENTS = 6,
    MISSION_MONUMENTS_ALEXANDER = 'A',
    MISSION_MONUMENTS_VALLEY = 'V',
};

int32_t victory_count_finished_monuments(void);
void city_victory_lost_cleanup(void);
void mission_records_apply(void);
void mission_briefing_set_started(void);
void window_city_intro_draw_background(void);
void mission_records_store(void);
void mission_selection_start(void);
void victory_button_continue_years(void);
void defeat_button_menu(void);
void defeat_button_replay(void);
void defeat_button_easier(void);
void mission_end_advance(void);
void window_victory_draw(void);
void window_briefing_draw(void);
void string_trim(char *text);
bool mission_replay_available(void);
void mission_replay(void);
generic_button *defeat_buttons(void);

#endif
