// The gameplay config table sentiment_rules.json: sentiment_rules (city/sentiment.h), the tunable values of the city's
// sentiment.

#include "city/sentiment.h"
#include "core/config.h"

static const config_name sentiment_rule_names[] = {
    { "no_houses_sentiment", SENTIMENT_RULE_NO_HOUSES_SENTIMENT },
    { "happiness_neutral_sentiment", SENTIMENT_RULE_HAPPINESS_NEUTRAL_SENTIMENT },
    { "happiness_drift_divisor", SENTIMENT_RULE_HAPPINESS_DRIFT_DIVISOR },
    { "factors_min_population", SENTIMENT_RULE_FACTORS_MIN_POPULATION },
    { "warning_min_population", SENTIMENT_RULE_WARNING_MIN_POPULATION },
    { "small_city_population", SENTIMENT_RULE_SMALL_CITY_POPULATION },
    { "empty_house_bonus", SENTIMENT_RULE_EMPTY_HOUSE_BONUS },
    { "max_months_without_food", SENTIMENT_RULE_MAX_MONTHS_WITHOUT_FOOD },
    { "bast_oracle_bonus", SENTIMENT_RULE_BAST_ORACLE_BONUS },
    { "ra_altar_happiness_penalty", SENTIMENT_RULE_RA_ALTAR_HAPPINESS_PENALTY },
    { "migration_max_population", SENTIMENT_RULE_MIGRATION_MAX_POPULATION },
    { "migration_min_emigration_population", SENTIMENT_RULE_MIGRATION_MIN_EMIGRATION_POPULATION },
    { "migration_base_amount", SENTIMENT_RULE_MIGRATION_BASE_AMOUNT },
    { "migration_hold_updates", SENTIMENT_RULE_MIGRATION_HOLD_UPDATES },
    { "migration_max_soldiers", SENTIMENT_RULE_MIGRATION_MAX_SOLDIERS },
    { 0, 0 },
};

static const config_field sentiment_rule_fields[] = {
    { "value", sentiment_rules, 1, 1, 1, 1000000, 0 },
};

static const config_storage sentiment_rule_storage[] = {
    { sentiment_rules, sizeof(sentiment_rules) },
};

const config_table sentiment_rule_table = {
    "sentiment_rules.json",
    "rule",
    sentiment_rule_names,
    SENTIMENT_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(sentiment_rule_fields, sentiment_rule_storage),
};
