// City ratings (city/ratings.c, city/monument_rating.c): the monthly updates, the explanations of the ratings advisor
// and the explanation codes; rating_rules holds the tunable values.

#ifndef CITY_RATINGS_H
#define CITY_RATINGS_H

#include <stdint.h>

enum prosperity_trend {
    PROSPERITY_RISING = 0,
    PROSPERITY_FALLING = 1,
    PROSPERITY_SAME = 2,
};

enum prosperity_reason {
    PROSPERITY_REASON_NONE = 0,
    PROSPERITY_REASON_AT_CAP = 3,
    PROSPERITY_REASON_FINANCES = 4,
    PROSPERITY_REASON_UNEMPLOYMENT = 5,
    PROSPERITY_REASON_WAGES = 6,
    PROSPERITY_REASON_TOO_MANY_HUTS = 7,
    PROSPERITY_REASON_TRIBUTE_NOT_PAID = 8,
    PROSPERITY_REASON_WORKERS_NEEDED = 10,
    PROSPERITY_REASON_TRADE = 11,
};

enum kingdom_change {
    KINGDOM_FELL = 0,
    KINGDOM_SAME = 1,
    KINGDOM_ROSE = 2,
};

enum kingdom_explanation {
    KINGDOM_EXPLANATION_FELL = 0,
    KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_8 = 1,
    KINGDOM_EXPLANATION_TRIBUTE_UNPAID_3_YEARS = 2,
    KINGDOM_EXPLANATION_REQUESTS_IGNORED_MANY = 3,
    KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_5 = 4,
    KINGDOM_EXPLANATION_TRIBUTE_UNPAID_2_YEARS = 5,
    KINGDOM_EXPLANATION_REQUESTS_IGNORED_SOME = 6,
    KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_3 = 7,
    KINGDOM_EXPLANATION_TRIBUTE_UNPAID = 8,
    KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_2 = 9,
    KINGDOM_EXPLANATION_MILESTONE_MISSED = 10,
    KINGDOM_EXPLANATION_SALARY_ABOVE_RANK = 11,
    KINGDOM_EXPLANATION_ROSE = 12,
    KINGDOM_EXPLANATION_SAME = 13,
};

enum peace_explanation {
    PEACE_EXPLANATION_BELOW_10 = 0,
    PEACE_EXPLANATION_BELOW_30 = 1,
    PEACE_EXPLANATION_BELOW_60 = 2,
    PEACE_EXPLANATION_BELOW_90 = 3,
    PEACE_EXPLANATION_BELOW_100 = 4,
    PEACE_EXPLANATION_FULL = 5,
    PEACE_EXPLANATION_RIOTERS = 6,
    PEACE_EXPLANATION_KINGDOM_SOLDIERS = 7,
    PEACE_EXPLANATION_ENEMIES = 8,
};

enum burial_goal_divisor {
    BURIAL_GOAL_MATERIAL_DIVISOR = 32,
    BURIAL_GOAL_GOODS_DIVISOR = 16,
};

enum rating_rule {
    RATING_RULE_KINGDOM_MILESTONE_BONUS,
    RATING_RULE_KINGDOM_MILESTONE_PENALTY,
    RATING_RULE_COUNT,
};

extern int32_t rating_rules[RATING_RULE_COUNT];

void city_ratings_change_kingdom(int32_t amount);
void city_ratings_update(int32_t is_yearly_update);
void city_ratings_update_culture(void);
void city_ratings_culture_lowest(void);
void city_ratings_culture_bonus(void);
void city_ratings_update_kingdom(int32_t is_yearly_update);
void city_ratings_update_monument(void);
void city_ratings_prosperity_cap(void);
void city_ratings_update_prosperity(void);
void city_ratings_prosperity_explanation(void);
void city_ratings_peace_explanation(void);
void city_ratings_kingdom_clear_yearly(void);
void city_ratings_kingdom_explanation(void);
double city_ratings_calculate_monument(int32_t maximum);
int32_t city_burial_goal_divisor(int32_t resource);

#endif
