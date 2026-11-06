// The difficulty levels of the game, the index of the difficulty tables (treasury and tax percentages, starting favor,
// base sentiment and enemy strength).

#ifndef CITY_DIFFICULTY_H
#define CITY_DIFFICULTY_H

typedef enum difficulty_level {
    DIFFICULTY_VERY_EASY = 0,
    DIFFICULTY_EASY = 1,
    DIFFICULTY_NORMAL = 2,
    DIFFICULTY_HARD = 3,
    DIFFICULTY_VERY_HARD = 4,
    DIFFICULTY_COUNT = 5
} difficulty_level;

#endif
