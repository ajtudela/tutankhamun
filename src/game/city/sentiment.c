// City sentiment: twice a month every inhabited house's sentiment moves by the city factors (taxes, wages,
// unemployment) and its own (food, huts), and the city sentiment is their average; every day the house happiness
// drifts towards it; and the migration follows it. The values a player may tune are sentiment_rules.
//
// Three things the migration and the crime wave depend on:
// - in a city below the factors' population the factors are cleared at the first inhabited house, so the houses after
//   it and the averages see 0;
// - the migration update reads the population limit and the soldier counts of the current player, not the given one;
// - the house happiness is the same field the crime wave reads as the house's crime.

#include "city/sentiment.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/construction_load.h"
#include "building/house_evolve.h"
#include "building/house_population.h"
#include "building/temple_complex.h"
#include "city/city_internal.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/population.h"
#include "game/game_data.h"
#include "window/window_data.h"

enum sentiment_bound {
    SENTIMENT_MAX = 100,
    HOUSE_HAPPINESS_MAX = 1000,
};

enum tax_model_shape {
    TAXED_SHARE_STEPS = 11,
};

int32_t sentiment_rules[SENTIMENT_RULE_COUNT] = {
    [SENTIMENT_RULE_NO_HOUSES_SENTIMENT] = 60,
    [SENTIMENT_RULE_HAPPINESS_NEUTRAL_SENTIMENT] = 80,
    [SENTIMENT_RULE_HAPPINESS_DRIFT_DIVISOR] = 5,
    [SENTIMENT_RULE_FACTORS_MIN_POPULATION] = 300,
    [SENTIMENT_RULE_WARNING_MIN_POPULATION] = 300,
    [SENTIMENT_RULE_SMALL_CITY_POPULATION] = 200,
    [SENTIMENT_RULE_EMPTY_HOUSE_BONUS] = 10,
    [SENTIMENT_RULE_MAX_MONTHS_WITHOUT_FOOD] = 3,
    [SENTIMENT_RULE_BAST_ORACLE_BONUS] = 10,
    [SENTIMENT_RULE_RA_ALTAR_HAPPINESS_PENALTY] = 2,
    [SENTIMENT_RULE_MIGRATION_MAX_POPULATION] = 200000,
    [SENTIMENT_RULE_MIGRATION_MIN_EMIGRATION_POPULATION] = 101,
    [SENTIMENT_RULE_MIGRATION_BASE_AMOUNT] = 12,
    [SENTIMENT_RULE_MIGRATION_HOLD_UPDATES] = 2,
    [SENTIMENT_RULE_MIGRATION_MAX_SOLDIERS] = 3,
};

static int32_t low_mood_warning(int32_t cause)
{
    return WARNING_UNHAPPINESS_LACK_OF_FOOD + (cause - LOW_MOOD_CAUSE_LACK_OF_FOOD);
}

int32_t city_sentiment_food_factor(void)
{
    const city_player *player = city_player_current();
    int32_t huts_percentage = calc_percentage(player->people_in_huts, player->population);
    if (player->people_in_manors_and_up > 0) {
        if (huts_percentage > 56) {
            return 0;
        }
        if (huts_percentage > 39) {
            return -3;
        }
        if (huts_percentage > 25) {
            return -4;
        }
        return (huts_percentage > 9) ? -5 : -6;
    }
    if (player->people_in_residences_and_up < 1) {
        if (huts_percentage > 39) {
            return 0;
        }
        if (huts_percentage > 25) {
            return -1;
        }
        return (huts_percentage > 9) ? -2 : -3;
    }
    if (huts_percentage > 56) {
        return 0;
    }
    if (huts_percentage > 39) {
        return -2;
    }
    if (huts_percentage > 25) {
        return -3;
    }
    return (huts_percentage > 9) ? -4 : -5;
}

void city_sentiment_update(void)
{
    city_player *player = city_player_current();
    int32_t wage_bonus = city_temple_complex_wage_bonus();
    int32_t people = house_population_calculate_people_per_type(city_current_player);
    if (people < player->population) {
        city_population_remove_from_census(city_current_player, player->population - people);
    }
    int32_t sentiment_food_types = 0;
    int32_t sentiment_huts = 0;
    int32_t sentiment_wages = 0;
    int32_t taxed = 0;
    if (player->percentage_taxed_people != 0) {
        taxed = (player->percentage_taxed_people + 10) / 10;
        if (taxed > 10) {
            taxed = 10;
        }
    }
    int32_t sentiment_taxes = tax_model[taxed + (player->tax_percentage * TAXED_SHARE_STEPS)];
    int32_t wage_difference = player->wages - player->wages_kingdom - wage_bonus;
    if (wage_difference < 0) {
        sentiment_wages = wage_difference / 2;
        if (sentiment_wages == 0) {
            sentiment_wages = -1;
        }
    } else if (wage_difference > 7) {
        sentiment_wages = 4;
    } else if (wage_difference > 4) {
        sentiment_wages = 3;
    } else if (wage_difference > 1) {
        sentiment_wages = 2;
    } else if (wage_difference > 0) {
        sentiment_wages = 1;
    }
    player->sentiment_wages = player->wages;
    int32_t unemployment = player->unemployment_percentage;
    int32_t sentiment_unemployment;
    if (unemployment > 25) {
        sentiment_unemployment = -3;
    } else if (unemployment > 17) {
        sentiment_unemployment = -2;
    } else if (unemployment > 10) {
        sentiment_unemployment = -1;
    } else {
        sentiment_unemployment = unemployment < 5;
    }
    player->sentiment_unemployment = unemployment;
    int32_t huts_penalty_now;
    if (player->sentiment_food_toggle) {
        huts_penalty_now = city_sentiment_food_factor();
        player->sentiment_food_toggle = 0;
    } else {
        huts_penalty_now = 0;
        player->sentiment_food_toggle = 1;
    }
    int32_t houses_needing_food = 0;
    int32_t houses = 0;
    int32_t food_types_total = 0;
    int32_t huts_total = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_IN_USE || house->house_size == 0) {
            continue;
        }
        if (house->house_population == 0) {
            house->sentiment.house = (int8_t) (difficulty_sentiment_base_table[settings_data.difficulty] + sentiment_rules[SENTIMENT_RULE_EMPTY_HOUSE_BONUS]);
            continue;
        }
        if (player->population < sentiment_rules[SENTIMENT_RULE_FACTORS_MIN_POPULATION]) {
            sentiment_food_types = 0;
            sentiment_unemployment = 0;
            sentiment_taxes = 0;
            sentiment_wages = 0;
            sentiment_huts = 0;
            if (player->population < sentiment_rules[SENTIMENT_RULE_SMALL_CITY_POPULATION]) {
                house->sentiment.house = (int8_t) (difficulty_sentiment_base_table[settings_data.difficulty] + sentiment_rules[SENTIMENT_RULE_EMPTY_HOUSE_BONUS]);
            } else {
                house->sentiment.house = (int8_t) difficulty_sentiment_base_table[settings_data.difficulty];
            }
            house->crime_risk = 0;
            continue;
        }
        int32_t level = house->subtype;
        house->crime_risk += house_models[level].crime_growth;
        house->sentiment.house = (int8_t) (house->sentiment.house + sentiment_unemployment + sentiment_wages + sentiment_taxes);
        houses++;
        sentiment_food_types = 0;
        sentiment_huts = 0;
        if (house_models[level].food_types == 0) {
            huts_total += huts_penalty_now;
            house->house_months_without_food = 0;
            sentiment_huts = huts_penalty_now;
        } else {
            houses_needing_food++;
            if (house->data.house.food_types >= 2) {
                sentiment_food_types = 2;
                house->house_months_without_food = 0;
                food_types_total += 2;
            } else if (house->data.house.food_types == 1) {
                sentiment_food_types = 1;
                food_types_total += 1;
                house->house_months_without_food = 0;
            } else {
                uint8_t months = (uint8_t) (house->house_months_without_food + 1);
                house->house_months_without_food = months;
                if (months > sentiment_rules[SENTIMENT_RULE_MAX_MONTHS_WITHOUT_FOOD]) {
                    house->house_months_without_food = (uint8_t) sentiment_rules[SENTIMENT_RULE_MAX_MONTHS_WITHOUT_FOOD];
                }
                if (house->house_months_without_food >= 3) {
                    sentiment_food_types = -3;
                    food_types_total -= 3;
                } else if (house->house_months_without_food >= 2) {
                    sentiment_food_types = -2;
                    food_types_total -= 2;
                } else {
                    sentiment_food_types = -1;
                    food_types_total -= 1;
                }
            }
        }
        int8_t sentiment = (int8_t) (house->sentiment.house + sentiment_huts + sentiment_food_types);
        house->sentiment.house = sentiment;
        if (sentiment < 0) {
            house->sentiment.house = 0;
        }
        if (house->sentiment.house > SENTIMENT_MAX) {
            house->sentiment.house = SENTIMENT_MAX;
        }
    }
    if (houses_needing_food != 0) {
        sentiment_food_types = food_types_total / houses_needing_food;
    }
    if (houses != 0) {
        sentiment_huts = huts_total / houses;
    }
    int32_t total = 0;
    int32_t count = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state == BUILDING_STATE_IN_USE && house->house_size != 0 && house->house_population != 0) {
            count++;
            total += house->sentiment.house;
        }
    }
    if (count == 0) {
        player->sentiment = sentiment_rules[SENTIMENT_RULE_NO_HOUSES_SENTIMENT];
    } else {
        player->sentiment = total / count;
        if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_BAST &&
            building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ORACLE)) {
            int32_t sentiment = player->sentiment + sentiment_rules[SENTIMENT_RULE_BAST_ORACLE_BONUS];
            if (sentiment >= SENTIMENT_MAX) {
                sentiment = SENTIMENT_MAX;
            }
            player->sentiment = sentiment;
        }
    }
    int32_t worst = 0;
    if (sentiment_food_types < worst) {
        player->sentiment_low_mood_cause = LOW_MOOD_CAUSE_LACK_OF_FOOD;
        worst = sentiment_food_types;
    }
    if (sentiment_unemployment < worst) {
        player->sentiment_low_mood_cause = LOW_MOOD_CAUSE_LACK_OF_JOBS;
        worst = sentiment_unemployment;
    }
    if (sentiment_taxes < worst) {
        player->sentiment_low_mood_cause = LOW_MOOD_CAUSE_HIGH_TAXES;
        worst = sentiment_taxes;
    }
    if (sentiment_wages < worst) {
        player->sentiment_low_mood_cause = LOW_MOOD_CAUSE_LOW_WAGES;
        worst = sentiment_wages;
    }
    if (sentiment_huts < worst) {
        player->sentiment_low_mood_cause = LOW_MOOD_CAUSE_SLUMS;
    }
    if ((settings_data.mission_id >= 2 || settings_data.is_custom_scenario != 0) && (player->sentiment_previous_value / 10) != (player->sentiment / 10) &&
        player->population > sentiment_rules[SENTIMENT_RULE_WARNING_MIN_POPULATION]) {
        int32_t sentiment = player->sentiment;
        int32_t warning;
        int32_t cause = 0;
        if (sentiment <= 0) {
            warning = WARNING_LOATHED_IN_CITY;
            if (player->sentiment_low_mood_cause != LOW_MOOD_CAUSE_NONE) {
                cause = low_mood_warning(player->sentiment_low_mood_cause);
            }
        } else if (sentiment >= SENTIMENT_MAX) {
            warning = WARNING_PEOPLE_IDOLIZE_YOU;
        } else {
            warning = (sentiment / 10) + WARNING_PEOPLE_VERY_ANGRY;
            if (sentiment < 50 && player->sentiment_low_mood_cause != LOW_MOOD_CAUSE_NONE) {
                cause = low_mood_warning(player->sentiment_low_mood_cause);
            }
        }
        if (warning != 0) {
            city_warning_show(warning);
        }
        if (cause != 0) {
            city_warning_show(cause);
        }
    }
    player->sentiment_previous_value = player->sentiment;
}

void city_sentiment_update_house_happiness(void)
{
    const city_player *player = city_player_current();
    int32_t sentiment = player->sentiment;
    int32_t bonus = 0;
    if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_RA &&
        building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ALTAR)) {
        bonus = -sentiment_rules[SENTIMENT_RULE_RA_ALTAR_HAPPINESS_PENALTY];
    }
    int32_t drift = (sentiment_rules[SENTIMENT_RULE_HAPPINESS_NEUTRAL_SENTIMENT] - sentiment) / sentiment_rules[SENTIMENT_RULE_HAPPINESS_DRIFT_DIVISOR];
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state == BUILDING_STATE_IN_USE && house->house_size != 0 && house->house_population != 0) {
            int32_t happiness = house->crime_risk + bonus + drift;
            house->crime_risk = happiness;
            if (happiness < 0) {
                house->crime_risk = 0;
            }
            if (house->crime_risk > HOUSE_HAPPINESS_MAX) {
                house->crime_risk = HOUSE_HAPPINESS_MAX;
            }
        }
    }
}

void city_sentiment_change_happiness(int32_t amount)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *house = building_get(id);
        if (house->state == BUILDING_STATE_IN_USE && house->house_size != 0) {
            int8_t sentiment = (int8_t) (house->sentiment.house + (int8_t) amount);
            house->sentiment.house = sentiment;
            if (sentiment < 0) {
                house->sentiment.house = 0;
            }
            if (house->sentiment.house > SENTIMENT_MAX) {
                house->sentiment.house = SENTIMENT_MAX;
            }
        }
    }
}

void city_migration_update(int32_t player_id)
{
    const city_player *current = city_player_current();
    city_player *player = city_player_get(player_id);
    int32_t sentiment = player->sentiment;
    int32_t previous = player->migration_percentage;
    int32_t percentage;
    if (sentiment >= 71) {
        percentage = 100;
    } else if (sentiment >= 61) {
        percentage = 75;
    } else if (sentiment >= 50) {
        percentage = 50;
    } else if (sentiment >= 41) {
        percentage = 0;
    } else if (sentiment >= 31) {
        percentage = -10;
    } else {
        percentage = (sentiment < 21) ? -50 : -25;
    }
    player->migration_percentage = percentage;
    player->emigration_amount_per_batch = 0;
    player->immigration_amount_per_batch = 0;
    if (current->population >= sentiment_rules[SENTIMENT_RULE_MIGRATION_MAX_POPULATION]) {
        player->migration_percentage = 0;
        return;
    }
    if (((current->kingdom_soldiers + current->enemies) > sentiment_rules[SENTIMENT_RULE_MIGRATION_MAX_SOLDIERS]) && player->migration_percentage > 0) {
        player->migration_percentage = 0;
        return;
    }
    if (percentage >= 1) {
        if (player->immigration_duration != 0) {
            player->immigration_duration--;
            return;
        }
        player->immigration_amount_per_batch = calc_adjust_with_percentage(sentiment_rules[SENTIMENT_RULE_MIGRATION_BASE_AMOUNT], percentage);
        player->emigration_duration = sentiment_rules[SENTIMENT_RULE_MIGRATION_HOLD_UPDATES];
    } else if (percentage < 0) {
        if (player->emigration_duration != 0) {
            player->emigration_duration--;
            return;
        }
        if (player->population < sentiment_rules[SENTIMENT_RULE_MIGRATION_MIN_EMIGRATION_POPULATION]) {
            return;
        }
        player->emigration_amount_per_batch = calc_adjust_with_percentage(sentiment_rules[SENTIMENT_RULE_MIGRATION_BASE_AMOUNT], -percentage);
        player->immigration_duration = sentiment_rules[SENTIMENT_RULE_MIGRATION_HOLD_UPDATES];
    }
    if (previous != player->migration_percentage) {
        city_migration_post_message(0);
    }
}
