// City ratings, updated monthly: culture, kingdom, monument and (every third month) prosperity, each 0..100, with the
// explanations the ratings advisor shows. A kingdom rating fixed by the scenario stays at 50.
//
// A culture category gets its bonus when its points plus 30 are below the lowest points, that is 30 below the lowest
// rather than above it.

#include "city/ratings.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/house_evolve.h"
#include "building/monument_city.h"
#include "building/temple_complex.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "city/city_player.h"
#include "game/game_data.h"
#include "window/window_data.h"

int32_t culture_bonus;

enum {
    RATING_MAX = 100,
    KINGDOM_RATING_FIXED = 50,
    CULTURE_RECORDS = 11,
    CULTURE_BANDS = 7,
    CULTURE_BONUS_MARGIN = 30,
    CULTURE_BONUS_POINTS = 5,
};

int32_t rating_rules[RATING_RULE_COUNT] = {
    [RATING_RULE_KINGDOM_MILESTONE_BONUS] = 5,
    [RATING_RULE_KINGDOM_MILESTONE_PENALTY] = 2,
};

typedef struct {
    int32_t coverage_index;
    int32_t points[CULTURE_BANDS];
    int32_t limits[CULTURE_BANDS - 1];
} culture_record;

static const culture_record *culture_record_get(int32_t index)
{
    return &((const culture_record *) culture_records)[index];
}

static int32_t clamp_rating(int32_t rating)
{
    if (rating < 0) {
        return 0;
    }
    return (rating > RATING_MAX) ? RATING_MAX : rating;
}

void city_ratings_culture_lowest(void)
{
    culture_lowest_points = 100;
    for (int32_t i = 0; i < CULTURE_RECORDS; i++) {
        const culture_record *r = culture_record_get(i);
        int32_t coverage = culture_coverage[r->coverage_index];
        int32_t points;
        if (coverage <= r->limits[0]) {
            points = r->points[0];
        } else if (coverage < r->limits[1]) {
            points = r->points[1];
        } else if (coverage < r->limits[2]) {
            points = r->points[2];
        } else if (coverage < r->limits[3]) {
            points = r->points[3];
        } else if (coverage < r->limits[4]) {
            points = r->points[4];
        } else if (coverage < r->limits[5]) {
            points = r->points[5];
        } else {
            points = r->points[6];
        }
        if (points < culture_lowest_points) {
            culture_lowest_points = points;
            culture_lowest_index = i;
        }
    }
}

void city_ratings_culture_bonus(void)
{
    culture_bonus = 0;
    for (int32_t i = 0; i < CULTURE_RECORDS; i++) {
        const culture_record *r = culture_record_get(i);
        int32_t coverage = culture_coverage[r->coverage_index];
        int32_t points;
        if (coverage < 1) {
            points = r->points[0];
        } else if (coverage < 20) {
            points = r->points[1];
        } else if (coverage < 40) {
            points = r->points[2];
        } else if (coverage < 60) {
            points = r->points[3];
        } else if (coverage < 80) {
            points = r->points[4];
        } else if (coverage < 100) {
            points = r->points[5];
        } else {
            points = r->points[6];
        }
        if ((points + CULTURE_BONUS_MARGIN) < culture_lowest_points) {
            culture_bonus += CULTURE_BONUS_POINTS;
        }
    }
}

void city_ratings_update_culture(void)
{
    city_ratings_culture_lowest();
    city_ratings_culture_bonus();
    int32_t rating = culture_lowest_points + culture_bonus;
    if (rating >= RATING_MAX) {
        rating = RATING_MAX;
    } else if (rating < 0) {
        rating = 0;
    }
    city_player *player = city_player_current();
    player->rating_culture = rating;
    player->culture_explanation = culture_lowest_index;
}

void city_ratings_prosperity_cap(void)
{
    int32_t total = 0;
    int32_t people = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_UNUSED && house->house_size != 0) {
            total += house_models[house->subtype].prosperity * house->house_population;
            people += house->house_population;
        }
    }
    city_player_current()->prosperity_max = (people > 0) ? (total / people) : 0;
}

static int32_t last_year_income(const city_player *player)
{
    return player->gold_last_year + player->donated_last_year + player->taxes_last_year + player->exports_last_year;
}

void city_ratings_update_prosperity(void)
{
    int32_t wage_bonus = city_temple_complex_wage_bonus();
    city_player *player = city_player_current();
    int32_t change = 0;
    int32_t construction = player->construction_last_year;
    int32_t income = last_year_income(player);
    int32_t expenses = player->salary_last_year + player->interest_last_year + player->wages_last_year + player->imports_last_year + construction;
    if ((income - expenses) >= 1) {
        change = 5;
    } else if (((construction / 2) - expenses + income) >= 1) {
        change = 3;
    } else if ((income - expenses) < 0) {
        change = -8;
    } else if ((construction - expenses + income) < 0) {
        change = -5;
    }
    if (player->imports_last_year < player->exports_last_year) {
        change++;
    } else if (player->exports_last_year < player->imports_last_year) {
        change--;
    }
    if (player->workers_needed < 1) {
        int32_t unemployment = player->unemployment_percentage;
        if (unemployment < 5) {
            change += 2;
        } else if (unemployment >= 10) {
            if (unemployment < 15) {
                change -= 1;
            } else if (unemployment < 20) {
                change -= 2;
            } else {
                change -= 3;
            }
        }
    } else {
        change--;
    }
    if (player->food_types_eaten >= 0 && player->food_types_eaten < 2) {
        change--;
    }
    int32_t wages = (player->wage_rate_paid_last_year / MONTHS_PER_YEAR) - player->wages_kingdom - wage_bonus;
    if (wages >= 6) {
        change += 2;
    } else if (wages >= 1) {
        change += 1;
    } else if (wages < -10) {
        change -= 3;
    } else if (wages < -5) {
        change -= 2;
    } else if (wages < 0) {
        change -= 1;
    }
    int32_t huts_share = calc_percentage(player->people_in_huts_and_shanties, player->population);
    int32_t manors_share = calc_percentage(player->people_in_manors_and_up, player->population);
    if (huts_share > 95) {
        change--;
    }
    if (manors_share > 10) {
        change += 2;
    }
    if (player->tribute_not_paid_last_year) {
        change--;
    }
    if (player->senet_shows > 0) {
        change++;
    }
    if (city_has_completed_monument()) {
        change++;
    }
    int32_t rating = player->rating_prosperity + change;
    player->rating_prosperity = rating;
    if (player->prosperity_max < rating) {
        player->rating_prosperity = player->prosperity_max;
    }
    player->rating_prosperity = clamp_rating(player->rating_prosperity);
    city_ratings_prosperity_explanation();
}

void city_ratings_prosperity_explanation(void)
{
    int32_t wage_bonus = city_temple_complex_wage_bonus();
    city_player *player = city_player_current();
    uint32_t severity = 0;
    int32_t income = last_year_income(player);
    int32_t expenses = player->construction_last_year + player->salary_last_year + player->interest_last_year + player->wages_last_year +
        player->imports_last_year;
    int32_t change = 0;
    if ((income - expenses) >= 1) {
        change = 5;
    } else if (((player->construction_this_year / 2) - expenses + income) >= 1) {
        change = 3;
    } else if ((income - expenses) < 0) {
        severity = 8;
        change = -8;
    } else if ((player->construction_this_year - expenses + income) < 0) {
        severity = 5;
        change = -5;
    }
    enum prosperity_reason reason = PROSPERITY_REASON_FINANCES;
    if (player->food_types_eaten >= 0 && player->food_types_eaten < 2) {
        change--;
    }
    if (player->imports_last_year < player->exports_last_year) {
        change++;
    } else if (player->exports_last_year < player->imports_last_year) {
        if (severity == 0) {
            severity = 1;
            reason = PROSPERITY_REASON_TRADE;
        }
        change--;
    } else if (severity == 0) {
        severity = 1;
        reason = PROSPERITY_REASON_TRADE;
    }
    if (player->workers_needed < 1) {
        int32_t unemployment = player->unemployment_percentage;
        if (unemployment < 5) {
            change += 2;
        } else if (unemployment < 10) {
            if (severity == 0) {
                severity = 1;
                reason = PROSPERITY_REASON_UNEMPLOYMENT;
            }
        } else if (unemployment < 15) {
            change -= 1;
            if (severity == 0) {
                severity = 1;
                reason = PROSPERITY_REASON_UNEMPLOYMENT;
            }
        } else if (unemployment < 20) {
            change -= 2;
            if (severity < 2) {
                severity = 2;
                reason = PROSPERITY_REASON_UNEMPLOYMENT;
            }
        } else {
            change -= 3;
            if (severity < 3) {
                severity = 3;
                reason = PROSPERITY_REASON_UNEMPLOYMENT;
            }
        }
    } else {
        if (severity == 0) {
            severity = 1;
            reason = PROSPERITY_REASON_WORKERS_NEEDED;
        }
        change--;
    }
    int32_t wages = (player->wage_rate_paid_last_year / MONTHS_PER_YEAR) - player->wages_kingdom - wage_bonus;
    if (wages >= 6) {
        change += 2;
    } else if (wages >= 1) {
        change += 1;
    } else if (wages < -10) {
        if (severity < 3) {
            severity = 3;
            reason = PROSPERITY_REASON_WAGES;
        }
        change -= 3;
    } else if (wages < -5) {
        change -= 2;
        if (severity < 2) {
            severity = 2;
            reason = PROSPERITY_REASON_WAGES;
        }
    } else if (wages < 0) {
        change -= 1;
        if (severity == 0) {
            severity = 1;
            reason = PROSPERITY_REASON_WAGES;
        }
    }
    int32_t huts_share = calc_percentage(player->people_in_huts_and_shanties, player->population);
    int32_t manors_share = calc_percentage(player->people_in_manors_and_up, player->population);
    if (huts_share > 95) {
        if (severity == 0) {
            severity = 1;
            reason = PROSPERITY_REASON_TOO_MANY_HUTS;
        }
        change--;
    }
    if (manors_share > 10) {
        change += 2;
    }
    if (player->tribute_not_paid_last_year) {
        change--;
        if (severity == 0) {
            severity = 1;
            reason = PROSPERITY_REASON_TRIBUTE_NOT_PAID;
        }
    }
    if (player->senet_shows > 0) {
        change++;
    }
    if (city_has_completed_monument()) {
        change++;
    }
    if (change == 0 || (player->rating_prosperity < 1 && game_time_year == scenario_data.start.year)) {
        player->prosperity_explanation = PROSPERITY_REASON_NONE;
        player->prosperity_trend = PROSPERITY_SAME;
        return;
    }
    if (change < 1) {
        player->prosperity_trend = PROSPERITY_FALLING;
        player->prosperity_explanation = reason;
        return;
    }
    player->prosperity_trend = PROSPERITY_RISING;
    int32_t max = player->prosperity_max;
    if ((player->rating_prosperity + change) <= max) {
        player->prosperity_explanation = (severity == 0) ? PROSPERITY_REASON_NONE : reason;
        return;
    }
    if (player->rating_prosperity <= max) {
        player->prosperity_trend = (max <= player->rating_prosperity) ? PROSPERITY_SAME : PROSPERITY_RISING;
        player->prosperity_explanation = PROSPERITY_REASON_AT_CAP;
        return;
    }
    player->prosperity_trend = PROSPERITY_FALLING;
    player->prosperity_explanation = PROSPERITY_REASON_AT_CAP;
}

void city_ratings_peace_explanation(void)
{
    city_player *player = city_player_current();
    enum peace_explanation explanation;
    if (player->enemies != 0) {
        explanation = PEACE_EXPLANATION_ENEMIES;
    } else if (player->kingdom_soldiers != 0) {
        explanation = PEACE_EXPLANATION_KINGDOM_SOLDIERS;
    } else if (player->rioters != 0) {
        explanation = PEACE_EXPLANATION_RIOTERS;
    } else if (player->rating_peace < 10) {
        explanation = PEACE_EXPLANATION_BELOW_10;
    } else if (player->rating_peace < 30) {
        explanation = PEACE_EXPLANATION_BELOW_30;
    } else if (player->rating_peace < 60) {
        explanation = PEACE_EXPLANATION_BELOW_60;
    } else if (player->rating_peace < 90) {
        explanation = PEACE_EXPLANATION_BELOW_90;
    } else if (player->rating_peace < 100) {
        explanation = PEACE_EXPLANATION_BELOW_100;
    } else {
        explanation = PEACE_EXPLANATION_FULL;
    }
    player->peace_explanation = explanation;
}

void city_ratings_kingdom_clear_yearly(void)
{
    city_player *player = city_player_current();
    player->monument_num_criminals = 0;
    player->monument_num_rioters = 0;
    player->destroyed_buildings = 0;
    player->rating_yearly_counters[0] = 0;
    player->rating_yearly_counters[1] = 0;
    player->rating_yearly_counters[2] = 0;
    player->rating_yearly_counters[3] = 0;
}

void city_ratings_kingdom_explanation(void)
{
    city_player *player = city_player_current();
    player->kingdom_salary_penalty = 0;
    int32_t late = player->salary_rank - player->player_rank;
    if (player->player_rank == 0) {
        if (late >= 1) {
            player->kingdom_salary_penalty = late;
        }
    } else if (late >= 1) {
        player->kingdom_salary_penalty = late + 1;
    }
    city_ratings_peace_explanation();
    int32_t penalty = player->kingdom_salary_penalty;
    enum kingdom_explanation explanation;
    if (penalty > 7) {
        explanation = KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_8;
    } else if (player->tribute_not_paid_total_years > 2) {
        explanation = KINGDOM_EXPLANATION_TRIBUTE_UNPAID_3_YEARS;
    } else if (player->kingdom_ignored_request_penalty > 4) {
        explanation = KINGDOM_EXPLANATION_REQUESTS_IGNORED_MANY;
    } else if (penalty > 4) {
        explanation = KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_5;
    } else if (player->tribute_not_paid_total_years > 1) {
        explanation = KINGDOM_EXPLANATION_TRIBUTE_UNPAID_2_YEARS;
    } else if (player->kingdom_ignored_request_penalty > 2) {
        explanation = KINGDOM_EXPLANATION_REQUESTS_IGNORED_SOME;
    } else if (penalty > 2) {
        explanation = KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_3;
    } else if (player->tribute_not_paid_last_year != 0) {
        explanation = KINGDOM_EXPLANATION_TRIBUTE_UNPAID;
    } else if (penalty > 1) {
        explanation = KINGDOM_EXPLANATION_SALARY_ABOVE_RANK_BY_2;
    } else if (player->kingdom_milestone_penalty != 0) {
        explanation = KINGDOM_EXPLANATION_MILESTONE_MISSED;
    } else if (penalty != 0) {
        explanation = KINGDOM_EXPLANATION_SALARY_ABOVE_RANK;
    } else if (player->kingdom_change == KINGDOM_ROSE) {
        explanation = KINGDOM_EXPLANATION_ROSE;
    } else if (player->kingdom_change == KINGDOM_SAME) {
        explanation = KINGDOM_EXPLANATION_SAME;
    } else {
        explanation = KINGDOM_EXPLANATION_FELL;
    }
    player->kingdom_explanation = explanation;
}

static int32_t milestone_percentage(void)
{
    int32_t start = scenario_data.start.year;
    if (game_time_year == (start + scenario_data.milestone25_year)) {
        return 25;
    }
    if (game_time_year == (scenario_data.milestone50_year + start)) {
        return 50;
    }
    if (game_time_year == (start + scenario_data.milestone75_year)) {
        return 75;
    }
    return 0;
}

static int32_t milestone_goals_reached(const city_player *player, int32_t percentage)
{
    if (scenario_data.win_culture_enabled && player->rating_culture < calc_adjust_with_percentage(scenario_data.win_culture, percentage)) {
        return 0;
    }
    if (scenario_data.win_prosperity_enabled && player->rating_prosperity < calc_adjust_with_percentage(scenario_data.win_prosperity, percentage)) {
        return 0;
    }
    if (scenario_data.win_population_enabled && player->population < calc_adjust_with_percentage(scenario_data.win_population, percentage)) {
        return 0;
    }
    if (scenario_data.win_monument_enabled && player->rating_monument < calc_adjust_with_percentage(scenario_data.win_monument, percentage)) {
        return 0;
    }
    if (scenario_data.win_kingdom_enabled && player->rating_kingdom < calc_adjust_with_percentage(scenario_data.win_kingdom, percentage)) {
        return 0;
    }
    return 1;
}

static void update_kingdom_yearly(city_player *player)
{
    player->kingdom_salary_penalty = 0;
    player->kingdom_milestone_penalty = 0;
    player->kingdom_ignored_request_penalty = 0;
    if (settings_data.mission_id >= 2 || settings_data.is_custom_scenario != 0) {
        player->rating_kingdom--;
    }
    if (player->tribute_not_paid_last_year == 0) {
        if (player->expenses_last_year < player->income_last_year) {
            player->rating_kingdom++;
        }
    } else if (player->tribute_not_paid_total_years <= 1) {
        player->rating_kingdom -= 3;
    } else if (player->tribute_not_paid_total_years <= 2) {
        player->rating_kingdom -= 5;
    } else {
        player->rating_kingdom -= 8;
    }
    int32_t late = player->salary_rank - player->player_rank;
    if (player->player_rank == 0) {
        if (late > 0) {
            player->rating_kingdom -= late;
            player->kingdom_salary_penalty = late;
        }
    } else if (late > 0) {
        player->rating_kingdom -= late;
        player->kingdom_salary_penalty = late + 1;
    } else if (late < 0) {
        player->rating_kingdom++;
    }
    int32_t percentage = milestone_percentage();
    if (percentage) {
        if (milestone_goals_reached(player, percentage)) {
            player->rating_kingdom += rating_rules[RATING_RULE_KINGDOM_MILESTONE_BONUS];
        } else {
            int32_t penalty = rating_rules[RATING_RULE_KINGDOM_MILESTONE_PENALTY];
            player->rating_kingdom -= penalty;
            player->kingdom_milestone_penalty = penalty;
        }
    }
    city_ratings_kingdom_clear_yearly();
    int32_t rating = player->rating_kingdom;
    if (player->kingdom_last_year < rating) {
        player->kingdom_change = KINGDOM_ROSE;
    } else if (player->kingdom_last_year == rating) {
        player->kingdom_change = KINGDOM_SAME;
    } else {
        player->kingdom_change = KINGDOM_FELL;
    }
    player->kingdom_last_year = player->rating_kingdom;
}

void city_ratings_update_kingdom(int32_t is_yearly_update)
{
    city_player *player = city_player_current();
    if (scenario_data.open_play != 0) {
        player->rating_kingdom = KINGDOM_RATING_FIXED;
        return;
    }
    if (++player->months_since_gift >= MONTHS_PER_YEAR) {
        player->gift_overdose_penalty = 0;
    }
    if (is_yearly_update) {
        update_kingdom_yearly(player);
    }
    player->rating_kingdom = clamp_rating(player->rating_kingdom);
    city_ratings_kingdom_explanation();
}

void city_ratings_update_monument(void)
{
    city_player *player = city_player_current();
    player->rating_monument = clamp_rating(crt_ftol_double(city_ratings_calculate_monument(0)));
    player->monument_explanation = 0;
}

void city_ratings_update(int32_t is_yearly_update)
{
    city_ratings_update_culture();
    city_ratings_update_kingdom(is_yearly_update);
    city_ratings_update_monument();
    city_ratings_prosperity_cap();
    if (game_time_month == 0 || game_time_month == 3 || game_time_month == 6 || game_time_month == 9) {
        city_ratings_update_prosperity();
    }
}

void city_ratings_change_kingdom(int32_t amount)
{
    city_player *player = city_player_current();
    player->rating_kingdom = clamp_rating(player->rating_kingdom + amount);
}
