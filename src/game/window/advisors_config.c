// The config tables of the advisor windows: advisor_help_messages.json (the message each advisor's help button opens)
// and advisor_comment_rules.json (the thresholds of the chief advisor's comments), described over their C tables.

#include "core/config.h"
#include "window/advisor_comments.h"
#include "window/advisors.h"

const config_name config_advisor_names[] = {
    { "none", ADVISOR_NONE },
    { "labor", ADVISOR_LABOR },
    { "military", ADVISOR_MILITARY },
    { "fleet", ADVISOR_FLEET },
    { "kingdom", ADVISOR_KINGDOM },
    { "ratings", ADVISOR_RATINGS },
    { "trade", ADVISOR_TRADE },
    { "population", ADVISOR_POPULATION },
    { "health", ADVISOR_HEALTH },
    { "education", ADVISOR_EDUCATION },
    { "entertainment", ADVISOR_ENTERTAINMENT },
    { "religion", ADVISOR_RELIGION },
    { "financial", ADVISOR_FINANCIAL },
    { "chief", ADVISOR_CHIEF },
    { "monuments", ADVISOR_MONUMENTS },
    { 0, 0 },
};

static const config_field advisor_help_message_fields[] = {
    { "message", advisor_help_messages, 1, 1, 0, 1000, 0 },
};

static const config_storage advisor_help_message_storage[] = {
    { advisor_help_messages, sizeof(advisor_help_messages) },
};

const config_table advisor_help_message_table = {
    "advisor_help_messages.json",
    "advisor",
    config_advisor_names,
    ADVISOR_COUNT,
    CONFIG_TABLE_COLUMNS(advisor_help_message_fields, advisor_help_message_storage),
};

static const config_name advisor_comment_rule_names[] = {
    { "sentiment_low", ADVISOR_COMMENT_RULE_SENTIMENT_LOW },
    { "sentiment_fair", ADVISOR_COMMENT_RULE_SENTIMENT_FAIR },
    { "migration_max_armed_figures", ADVISOR_COMMENT_RULE_MIGRATION_MAX_ARMED_FIGURES },
    { "migration_newcomers_strong", ADVISOR_COMMENT_RULE_MIGRATION_NEWCOMERS_STRONG },
    { "migration_percentage_strong", ADVISOR_COMMENT_RULE_MIGRATION_PERCENTAGE_STRONG },
    { "employment_balanced_unemployment", ADVISOR_COMMENT_RULE_EMPLOYMENT_BALANCED_UNEMPLOYMENT },
    { "employment_balanced_workers_needed", ADVISOR_COMMENT_RULE_EMPLOYMENT_BALANCED_WORKERS_NEEDED },
    { "unemployment_urgent", ADVISOR_COMMENT_RULE_UNEMPLOYMENT_URGENT },
    { "unemployment_serious", ADVISOR_COMMENT_RULE_UNEMPLOYMENT_SERIOUS },
    { "unemployment_important", ADVISOR_COMMENT_RULE_UNEMPLOYMENT_IMPORTANT },
    { "labor_shortage_urgent", ADVISOR_COMMENT_RULE_LABOR_SHORTAGE_URGENT },
    { "labor_shortage_serious", ADVISOR_COMMENT_RULE_LABOR_SHORTAGE_SERIOUS },
    { "food_stocks_low_percent", ADVISOR_COMMENT_RULE_FOOD_STOCKS_LOW_PERCENT },
    { "food_supply_plenty_percent", ADVISOR_COMMENT_RULE_FOOD_SUPPLY_PLENTY_PERCENT },
    { "food_supply_enough_percent", ADVISOR_COMMENT_RULE_FOOD_SUPPLY_ENOUGH_PERCENT },
    { "food_supply_barely_enough_percent", ADVISOR_COMMENT_RULE_FOOD_SUPPLY_BARELY_ENOUGH_PERCENT },
    { "food_supply_slightly_short_percent", ADVISOR_COMMENT_RULE_FOOD_SUPPLY_SLIGHTLY_SHORT_PERCENT },
    { "food_supply_short_percent", ADVISOR_COMMENT_RULE_FOOD_SUPPLY_SHORT_PERCENT },
    { "angry_god_mood", ADVISOR_COMMENT_RULE_ANGRY_GOD_MOOD },
    { "very_angry_god_mood", ADVISOR_COMMENT_RULE_VERY_ANGRY_GOD_MOOD },
    { "happy_god_mood", ADVISOR_COMMENT_RULE_HAPPY_GOD_MOOD },
    { "thefts_many", ADVISOR_COMMENT_RULE_THEFTS_MANY },
    { "thefts_several", ADVISOR_COMMENT_RULE_THEFTS_SEVERAL },
    { "thefts_some", ADVISOR_COMMENT_RULE_THEFTS_SOME },
    { "requests_counted", ADVISOR_COMMENT_RULE_REQUESTS_COUNTED },
    { "requests_some_max", ADVISOR_COMMENT_RULE_REQUESTS_SOME_MAX },
    { "requests_several_max", ADVISOR_COMMENT_RULE_REQUESTS_SEVERAL_MAX },
    { 0, 0 },
};

static const config_field advisor_comment_rule_fields[] = {
    { "value", advisor_comment_rules, 1, 1, 0, 100000, 0 },
};

static const config_storage advisor_comment_rule_storage[] = {
    { advisor_comment_rules, sizeof(advisor_comment_rules) },
};

const config_table advisor_comment_rule_table = {
    "advisor_comment_rules.json",
    "rule",
    advisor_comment_rule_names,
    ADVISOR_COMMENT_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(advisor_comment_rule_fields, advisor_comment_rule_storage),
};
