// City sentiment (city/sentiment.c): the city and house sentiment, the house happiness and the migration;
// sentiment_rules holds the tunable values.

#ifndef CITY_SENTIMENT_H
#define CITY_SENTIMENT_H

#include <stdint.h>

enum low_mood_cause {
    LOW_MOOD_CAUSE_NONE = 0,
    LOW_MOOD_CAUSE_LACK_OF_FOOD = 1,
    LOW_MOOD_CAUSE_LACK_OF_JOBS = 2,
    LOW_MOOD_CAUSE_HIGH_TAXES = 3,
    LOW_MOOD_CAUSE_LOW_WAGES = 4,
    LOW_MOOD_CAUSE_SLUMS = 5,
};

enum sentiment_rule {
    SENTIMENT_RULE_NO_HOUSES_SENTIMENT,
    SENTIMENT_RULE_HAPPINESS_NEUTRAL_SENTIMENT,
    SENTIMENT_RULE_HAPPINESS_DRIFT_DIVISOR,
    SENTIMENT_RULE_FACTORS_MIN_POPULATION,
    SENTIMENT_RULE_WARNING_MIN_POPULATION,
    SENTIMENT_RULE_SMALL_CITY_POPULATION,
    SENTIMENT_RULE_EMPTY_HOUSE_BONUS,
    SENTIMENT_RULE_MAX_MONTHS_WITHOUT_FOOD,
    SENTIMENT_RULE_BAST_ORACLE_BONUS,
    SENTIMENT_RULE_RA_ALTAR_HAPPINESS_PENALTY,
    SENTIMENT_RULE_MIGRATION_MAX_POPULATION,
    SENTIMENT_RULE_MIGRATION_MIN_EMIGRATION_POPULATION,
    SENTIMENT_RULE_MIGRATION_BASE_AMOUNT,
    SENTIMENT_RULE_MIGRATION_HOLD_UPDATES,
    SENTIMENT_RULE_MIGRATION_MAX_SOLDIERS,
    SENTIMENT_RULE_COUNT,
};

extern int32_t sentiment_rules[SENTIMENT_RULE_COUNT];

void city_migration_update(int32_t player_id);
void city_sentiment_change_happiness(int32_t amount);
int32_t city_sentiment_food_factor(void);
void city_sentiment_update(void);
void city_sentiment_update_house_happiness(void);

#endif
