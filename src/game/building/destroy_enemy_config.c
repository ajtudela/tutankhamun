// The gameplay config table of the enemy theft (core/config.h): per difficulty the share of the savings or the
// palace money enemies take when they destroy a mansion or a palace.

#include "building/destroy_enemy.h"
#include "city/difficulty.h"
#include "core/config.h"

static const config_field enemy_theft_percentage_fields[] = {
    { "percent", enemy_theft_percentages, 1, 1, 0, 100, 0 },
};

static const config_storage enemy_theft_percentage_storage[] = {
    { enemy_theft_percentages, sizeof(int32_t) * DIFFICULTY_COUNT },
};

const config_table enemy_theft_percentage_table = {
    "enemy_theft_percentages.json",
    "difficulty",
    config_difficulty_names,
    DIFFICULTY_COUNT,
    CONFIG_TABLE_COLUMNS(enemy_theft_percentage_fields, enemy_theft_percentage_storage),
};
