// The mission end dialog (victory_dialog.c): its draw, the buttons a lost mission offers and the dialog's texts.

#ifndef WINDOW_VICTORY_DIALOG_H
#define WINDOW_VICTORY_DIALOG_H

#include <stdint.h>

enum victory_dialog_text {
    VICTORY_TEXT_VICTORY = 0,
    VICTORY_TEXT_CLICK_TO_PROCEED = 2,
    VICTORY_TEXT_PROCEED = 3,
    VICTORY_TEXT_CONTINUE_2_YEARS = 4,
    VICTORY_TEXT_CONTINUE_5_YEARS = 5,
    VICTORY_TEXT_NEW_GAME = 6,
    VICTORY_TEXT_CAMPAIGN_COMPLETE = 8,
    VICTORY_TEXT_CAMPAIGN_COMPLETE_MESSAGE = 9,
    VICTORY_TEXT_REPLAY_MISSION = 37,
    VICTORY_TEXT_LOWER_DIFFICULTY = 40,
};

enum defeat_options {
    DEFEAT_OPTIONS_NEW_GAME = 1,
    DEFEAT_OPTIONS_REPLAY = 2,
    DEFEAT_OPTIONS_LOWER_DIFFICULTY = 3,
};

int32_t city_defeat_options(void);
void victory_dialog_draw_background(void);
void victory_dialog_draw_foreground(void);

#endif
