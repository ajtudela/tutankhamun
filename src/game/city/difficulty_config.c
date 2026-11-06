// The gameplay config table difficulty.json: the four difficulty arrays as one record per level (the bailout and
// starting treasury, the tax percentage, the starting favor, the base sentiment and the enemy strength).

#include "city/difficulty.h"
#include "core/config.h"
#include "game/game_data.h"

#define DIFFICULTY_ENEMY_PERCENTAGE_OFFSET 5

static const config_field difficulty_fields[] = {
    { "treasury_percentage", difficulty_treasury_percentage, 1, 1, 0, 10000, 0 },
    { "tax_percentage", difficulty_tax_percentages, 1, 1, 0, 10000, 0 },
    { "starting_favor", difficulty_starting_favor, 1, 1, 0, 100, 0 },
    { "sentiment", difficulty_sentiment_base_table, 1, 1, 0, 100, 0 },
    { "enemies_percentage", difficulty_sentiment_base_table + DIFFICULTY_ENEMY_PERCENTAGE_OFFSET, 1, 1, 0, 10000, 0 },
};

static const config_storage difficulty_storage[] = {
    { difficulty_treasury_percentage, sizeof(int32_t) * DIFFICULTY_COUNT },
    { difficulty_tax_percentages, sizeof(int32_t) * DIFFICULTY_COUNT },
    { difficulty_starting_favor, sizeof(int32_t) * DIFFICULTY_COUNT },
    { difficulty_sentiment_base_table, sizeof(int32_t) * (2 * DIFFICULTY_COUNT + 1) },
};

const config_table difficulty_table = {
    "difficulty.json",
    "level",
    config_difficulty_names,
    DIFFICULTY_COUNT,
    CONFIG_TABLE_COLUMNS(difficulty_fields, difficulty_storage),
};
