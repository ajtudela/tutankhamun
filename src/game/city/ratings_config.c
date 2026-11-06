// The gameplay config table rating_rules.json: rating_rules (city/ratings.h), the kingdom rating bonus and penalty of a
// milestone.

#include "city/ratings.h"
#include "core/config.h"

static const config_name rating_rule_names[] = { { "kingdom_milestone_bonus", RATING_RULE_KINGDOM_MILESTONE_BONUS },
    { "kingdom_milestone_penalty", RATING_RULE_KINGDOM_MILESTONE_PENALTY }, { 0, 0 } };

static const config_field rating_rule_fields[] = {
    { "value", rating_rules, 1, 1, -100, 100, 0 },
};

static const config_storage rating_rule_storage[] = {
    { rating_rules, sizeof(rating_rules) },
};

const config_table rating_rule_table = {
    "rating_rules.json",
    "rule",
    rating_rule_names,
    RATING_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(rating_rule_fields, rating_rule_storage),
};
