// The config table of the map selection: map_rating_brackets.json (per bracket the largest count and its points for
// the population goal, invasions, requests and gods, and the rank the rating shows), described over its C table.

#include "core/config.h"
#include "window/map_selection.h"

static const config_name map_rating_bracket_names[] = {
    { "population_none", MAP_RATING_POPULATION_NONE },
    { "population_small", MAP_RATING_POPULATION_SMALL },
    { "population_medium", MAP_RATING_POPULATION_MEDIUM },
    { "population_large", MAP_RATING_POPULATION_LARGE },
    { "population_huge", MAP_RATING_POPULATION_HUGE },
    { "invasions_none", MAP_RATING_INVASIONS_NONE },
    { "invasions_few", MAP_RATING_INVASIONS_FEW },
    { "invasions_some", MAP_RATING_INVASIONS_SOME },
    { "invasions_many", MAP_RATING_INVASIONS_MANY },
    { "invasions_most", MAP_RATING_INVASIONS_MOST },
    { "requests_few", MAP_RATING_REQUESTS_FEW },
    { "requests_some", MAP_RATING_REQUESTS_SOME },
    { "requests_many", MAP_RATING_REQUESTS_MANY },
    { "requests_more", MAP_RATING_REQUESTS_MORE },
    { "requests_most", MAP_RATING_REQUESTS_MOST },
    { "gods_none", MAP_RATING_GODS_NONE },
    { "gods_few", MAP_RATING_GODS_FEW },
    { "gods_some", MAP_RATING_GODS_SOME },
    { "gods_many", MAP_RATING_GODS_MANY },
    { "gods_most", MAP_RATING_GODS_MOST },
    { "rank_0", MAP_RATING_RANK_0 },
    { "rank_1", MAP_RATING_RANK_1 },
    { "rank_2", MAP_RATING_RANK_2 },
    { "rank_3", MAP_RATING_RANK_3 },
    { "rank_4", MAP_RATING_RANK_4 },
    { "rank_5", MAP_RATING_RANK_5 },
    { "rank_6", MAP_RATING_RANK_6 },
    { "rank_7", MAP_RATING_RANK_7 },
    { "rank_8", MAP_RATING_RANK_8 },
    { "rank_9", MAP_RATING_RANK_9 },
    { 0, 0 },
};

static const config_field map_rating_bracket_fields[] = {
    { "max", &map_rating_brackets[0].max, (int32_t) (sizeof(map_rating_bracket) / sizeof(int32_t)), 1, 0, INT32_MAX, 0 },
    { "value", &map_rating_brackets[0].value, (int32_t) (sizeof(map_rating_bracket) / sizeof(int32_t)), 1, 0, 10000, 0 },
};

static const config_storage map_rating_bracket_storage[] = {
    { map_rating_brackets, sizeof(map_rating_brackets) },
};

const config_table map_rating_bracket_table = {
    "map_rating_brackets.json",
    "bracket",
    map_rating_bracket_names,
    MAP_RATING_BRACKET_COUNT,
    CONFIG_TABLE_COLUMNS(map_rating_bracket_fields, map_rating_bracket_storage),
};
