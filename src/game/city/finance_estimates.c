// City finance and rating helpers outside the tick: the financial advisor's estimates and totals of this year, the
// cost of construction, the palace check, the prosperity lost after a bailout, the culture rating of the ratings
// advisor, the advisors' causes (no immigration, missing health, education, entertainment and religion), and the gift
// to the pharaoh from the personal savings.
//
// The interest estimate looks up the temple complex's god without using it.

#include "city/finance_estimates.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/house_evolve.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "city/finance.h"
#include "city/ratings.h"
#include "city/sentiment.h"
#include "game/game_data.h"
#include "window/window_data.h"

#include <stdbool.h>

enum {
    GIFTS_RAISING_RATING = 4,
    GIFT_IDS = 4,
};

void city_finance_estimate_taxes(void)
{
    city_player *player = city_player_current();
    player->monthly_collected_plebs = 0;
    player->monthly_collected_nobles = 0;
    int32_t highest_id = building_highest_id;
    for (int32_t id = 1; id <= highest_id; id++) {
        const building *house = building_get(id);
        if (house->state != BUILDING_STATE_IN_USE || !house->house_size || !house->house_tax_coverage) {
            continue;
        }
        int32_t tax = house->house_population * house_models[house->subtype].tax_multiplier;
        if (house->subtype >= HOUSE_LEVEL_COMMON_MANOR) {
            player->monthly_collected_nobles += tax;
        } else {
            player->monthly_collected_plebs += tax;
        }
    }
    int32_t nobles = city_finance_apply_tax_rate(player, player->monthly_collected_nobles);
    int32_t plebs = city_finance_apply_tax_rate(player, player->monthly_collected_plebs);
    int32_t months_left = MONTHS_PER_YEAR - game_time_month;
    player->taxes_this_year = player->yearly_collected_plebs + player->yearly_collected_nobles;
    player->estimated_tax_income = ((plebs + nobles) * months_left) + player->taxes_this_year;
}

void city_finance_estimate_wages(void)
{
    city_player *player = city_player_current();
    int32_t monthly = ((player->wages * player->workers_employed) / WORKERS_PER_WAGE_UNIT) / MONTHS_PER_YEAR;
    player->wages_this_year = player->wages_so_far;
    player->estimated_wages = ((MONTHS_PER_YEAR - game_time_month) * monthly) + player->wages_so_far;
}

void city_finance_estimate_interest(void)
{
    city_player *player = city_player_current();
    if (player->has_temple_complex) {
        building_temple_god(player->temple_complex_building_id);
    }
    player->interest_this_year = player->interest_so_far;
}

void city_finance_estimate_salary(void)
{
    city_player *player = city_player_current();
    player->salary_this_year = player->salary_so_far;
}

void city_finance_calculate_totals(void)
{
    city_player *player = city_player_current();
    player->income_this_year = player->gold_this_year + player->donated_this_year + player->taxes_this_year + player->exports_this_year;
    player->expenses_this_year = player->requests_and_festivals_this_year + player->stolen_this_year + player->salary_this_year + player->interest_this_year +
        player->construction_this_year + player->wages_this_year + player->imports_this_year;
    player->net_in_out_last_year = player->income_last_year - player->expenses_last_year;
    player->net_in_out_this_year = player->income_this_year - player->expenses_this_year;
    player->balance_this_year = player->balance_last_year + (player->income_this_year - player->expenses_this_year);
}

void city_ratings_store_culture_lowest(void)
{
    city_player_current()->culture_explanation = culture_lowest_index;
}

void city_ratings_update_culture_advisor(void)
{
    city_ratings_update_culture();
    city_ratings_store_culture_lowest();
}

void city_ratings_reduce_prosperity_after_bailout(void)
{
    city_player *player = city_player_current();
    player->rating_prosperity -= finance_rules[FINANCE_RULE_BAILOUT_PROSPERITY_LOSS];
    if (player->rating_prosperity < 0) {
        player->rating_prosperity = 0;
    }
    player->prosperity_explanation = PROSPERITY_REASON_TRIBUTE_NOT_PAID;
}

bool city_buildings_has_palace(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        if (building_get(id)->state == BUILDING_STATE_IN_USE && building_is_palace(id)) {
            return 1;
        }
    }
    return 0;
}

void city_finance_process_construction(int32_t player_id, int32_t amount)
{
    city_player *player = city_player_get(player_id);
    if (player->in_use == 1 && editor_active != 1) {
        player->treasury -= amount;
        player->construction_this_year += amount;
    }
}

void city_ratings_advisor_reasons(void)
{
    city_player *player = city_player_current();
    int32_t low_mood_cause = player->sentiment_low_mood_cause;
    if (low_mood_cause == LOW_MOOD_CAUSE_LACK_OF_FOOD) {
        player->no_immigration_cause = NO_IMMIGRATION_NO_FOOD;
    } else if (low_mood_cause == LOW_MOOD_CAUSE_LACK_OF_JOBS) {
        player->no_immigration_cause = NO_IMMIGRATION_NO_JOBS;
    } else if (low_mood_cause == LOW_MOOD_CAUSE_HIGH_TAXES) {
        player->no_immigration_cause = NO_IMMIGRATION_HIGH_TAXES;
    } else if (low_mood_cause == LOW_MOOD_CAUSE_LOW_WAGES) {
        player->no_immigration_cause = NO_IMMIGRATION_LOW_WAGES;
    } else if (low_mood_cause == LOW_MOOD_CAUSE_SLUMS) {
        player->no_immigration_cause = NO_IMMIGRATION_FOOD_FACTOR;
    } else {
        player->no_immigration_cause = NO_IMMIGRATION_LOW_MOOD;
    }
    player->health_demand = HEALTH_DEMAND_NONE;
    int32_t largest = 0;
    if (player->houses_missing_courthouse > 0 || player->houses_missing_any_courthouse > 0) {
        if (player->houses_missing_courthouse < player->houses_missing_any_courthouse) {
            player->health_demand = HEALTH_DEMAND_COURTHOUSE_NONE;
            largest = player->houses_missing_any_courthouse;
        } else {
            player->health_demand = HEALTH_DEMAND_COURTHOUSE;
            largest = player->houses_missing_courthouse;
        }
    }
    if (largest < player->houses_missing_dentist) {
        player->health_demand = HEALTH_DEMAND_DENTIST;
        largest = player->houses_missing_dentist;
    }
    if (largest < player->houses_missing_health) {
        player->health_demand = HEALTH_DEMAND_HEALTH;
    }
    player->education_demand = EDUCATION_DEMAND_NONE;
    int32_t more_education = player->houses_missing_more_education;
    int32_t education = player->houses_missing_education;
    if (education < more_education) {
        player->education_demand = EDUCATION_DEMAND_MORE;
    } else if (more_education < education) {
        player->education_demand = EDUCATION_DEMAND_SOME;
    } else if (more_education != 0 || education != 0) {
        player->education_demand = EDUCATION_DEMAND_BOTH;
    }
    player->entertainment_demand = ENTERTAINMENT_DEMAND_NONE;
    if (player->houses_missing_more_entertainment < player->houses_missing_entertainment) {
        player->entertainment_demand = ENTERTAINMENT_DEMAND_SOME;
    } else if (player->houses_missing_more_entertainment != 0) {
        player->entertainment_demand = ENTERTAINMENT_DEMAND_MORE;
    }
    player->religion_demand = RELIGION_DEMAND_NONE;
    largest = 0;
    if (player->houses_missing_religion > 0) {
        player->religion_demand = RELIGION_DEMAND_FIRST_GOD;
        largest = player->houses_missing_religion;
    }
    if (largest < player->houses_missing_second_religion) {
        player->religion_demand = RELIGION_DEMAND_SECOND_GOD;
        largest = player->houses_missing_second_religion;
    }
    if (largest < player->houses_missing_third_religion) {
        player->religion_demand = RELIGION_DEMAND_THIRD_GOD;
    }
}

void city_kingdom_send_gift_to_pharaoh(void)
{
    city_player *player = city_player_current();
    int32_t size = player->selected_gift_size;
    int32_t is_known_size = (size >= GIFT_MODEST) && (size <= GIFT_LAVISH);
    int32_t cost = player->gift_cost[(size == GIFT_MODEST || size == GIFT_GENEROUS) ? size : GIFT_LAVISH];
    if (player->personal_savings < cost) {
        return;
    }
    static const int8_t kingdom_gain[GIFTS_RAISING_RATING][GIFT_SIZES] = { { 3, 5, 10 }, { 1, 3, 5 }, { 0, 1, 3 }, { 0, 0, 1 } };
    int32_t gifts = player->gift_overdose_penalty;
    int32_t index = (gifts < 1) ? 0 : gifts;
    if (index < GIFTS_RAISING_RATING) {
        player->gift_overdose_penalty = index + 1;
        if (is_known_size) {
            city_ratings_change_kingdom(kingdom_gain[index][size]);
        }
    }
    player->months_since_gift = 0;
    if (is_known_size) {
        player->gift_id[size]++;
    }
    for (int32_t gift = GIFT_MODEST; gift < GIFT_SIZES; gift++) {
        if (player->gift_id[gift] >= GIFT_IDS) {
            player->gift_id[gift] = 0;
        }
    }
    player->personal_savings -= cost;
}
