// The functions of figure/figure_phrase.c that other files call.

#ifndef FIGURE_FIGURE_PHRASE_H
#define FIGURE_FIGURE_PHRASE_H

#include "figure/figure_type.h"
#include <stdbool.h>
#include <stdint.h>

enum phrase_group {
    PHRASE_GROUP_OWN = 0,
    PHRASE_GROUP_LABOR = 1,
    PHRASE_GROUP_MILITARY = 2,
    PHRASE_GROUP_SHIPS = 3,
    PHRASE_GROUP_KINGDOM = 5,
    PHRASE_GROUP_TRADE = 6,
    PHRASE_GROUP_HEALTH = 8,
    PHRASE_GROUP_ENTERTAINMENT = 10,
    PHRASE_GROUP_RELIGION = 11,
    PHRASE_GROUP_TAXES = 12,
    PHRASE_GROUP_CITY_LIFE = 13,
    PHRASE_GROUP_MONUMENTS = 14,
};

enum city_problem {
    CITY_PROBLEM_HEALTH = 0,
    CITY_PROBLEM_FOOD = 1,
    CITY_PROBLEM_INVASION = 2,
    CITY_PROBLEM_WORKER_SHORTAGE = 3,
    CITY_PROBLEM_GODS = 4,
    CITY_PROBLEM_KINGDOM = 5,
    CITY_PROBLEM_UNEMPLOYMENT = 6,
    CITY_PROBLEM_ENTERTAINMENT = 7,
    CITY_PROBLEM_COUNT = 8,
    CITY_PROBLEM_NOTHING_TO_COMPLAIN = 8,
    CITY_PROBLEM_NONE = 9,
};

enum figure_phrase_rule {
    FIGURE_PHRASE_NONE = -1,
    FIGURE_PHRASE_FAR_DISTANCE = 25,
};

enum trade_ship_dock_activity {
    TRADE_SHIP_NONE = 0,
    TRADE_SHIP_BUYING = 1,
    TRADE_SHIP_SELLING = 2,
};

enum figure_speech_code {
    FIGURE_SPEECH_BY_SOUND_TYPE = -4,
    FIGURE_SPEECH_BY_TEMPLE_GOD = -3,
    FIGURE_SPEECH_BY_CLIMATE = -2,
    FIGURE_SPEECH_NONE = -1,
    FIGURE_SPEECH_FIRST = 201,
    FIGURE_SPEECH_HUNTER_ANTELOPE = 208,
    FIGURE_SPEECH_HUNTER_OSTRICH = 209,
    FIGURE_SPEECH_HUNTER_BIRDS = 210,
    FIGURE_SPEECH_PRIEST_OSIRIS = 230,
};

extern int32_t figure_speech_sounds[FIGURE_TYPE_COUNT];

int32_t figure_phrase_city_weights(uint8_t *weights, uint8_t *none);
int32_t figure_phrase_pick_city_problem(void);
int32_t figure_trade_ship_dock_activity(int32_t figure_id);
int32_t figure_phrase_sound_id(int32_t figure_id);
void figure_phrase_play(void);
void figure_phrase_determine(int32_t figure_id);
void window_figure_info_button_city_problem(void);

#endif
