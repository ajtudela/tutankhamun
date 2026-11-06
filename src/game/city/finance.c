// City finance of the day tick: each month the taxes, the wages, the interest on a debt, the player's salary and the
// months in debt (a full year in debt costs kingdom rating); each year this year's sums become last year's and the
// tribute is paid. Also the money value a mansion, palace, tax collector or courthouse shows. The tunable values are
// finance_rules and debt_kingdom_penalties, read from their config files at start.
//
// The tribute estimate of the finance advisor computes an amount and then stores 0.

#include "city/finance.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "city/gods.h"
#include "city/message.h"
#include "game/game_data.h"
#include "window/window_data.h"

enum {
    WAGE_DIVISOR = WORKERS_PER_WAGE_UNIT * MONTHS_PER_YEAR,
};

int32_t finance_rules[FINANCE_RULE_COUNT] = {
    [FINANCE_RULE_HOUSE_TAX_BONUS_PERCENTAGE] = 110,
    [FINANCE_RULE_RA_COMPLEX_INTEREST_REDUCTION] = 5,
    [FINANCE_RULE_SALARY_MIN_TREASURY] = -5000,
    [FINANCE_RULE_TRIBUTE_PROFIT_PERCENTAGE] = 25,
    [FINANCE_RULE_BAILOUT_PROSPERITY_LOSS] = 3,
};

static int32_t debt_kingdom_penalty(int32_t years, int32_t difficulty)
{
    return debt_kingdom_penalties[years - 1 + (difficulty * DEBT_PENALTY_YEARS)];
}

int32_t city_finance_apply_tax_rate(const city_player *player, int32_t amount)
{
    return calc_adjust_with_percentage(calc_adjust_with_percentage(amount, player->tax_percentage), difficulty_tax_percentages[settings_data.difficulty]);
}

void city_finance_collect_monthly_taxes(void)
{
    city_player *player = city_player_current();
    player->taxed_plebs = 0;
    player->taxed_nobles = 0;
    player->untaxed_plebs = 0;
    player->untaxed_nobles = 0;
    player->monthly_uncollected_plebs = 0;
    player->monthly_collected_plebs = 0;
    player->monthly_uncollected_nobles = 0;
    player->monthly_collected_nobles = 0;
    for (int32_t level = 0; level < HOUSE_LEVEL_COUNT; level++) {
        player->population_per_level[level] = 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *house = building_get(id);
        if (house->state != BUILDING_STATE_IN_USE || house->house_size == 0) {
            continue;
        }
        int32_t level = house->subtype;
        int32_t is_noble = level >= HOUSE_LEVEL_COMMON_MANOR;
        int32_t tax = house_models[level].tax_multiplier;
        if (house->data.house.tax_bonus) {
            tax = calc_adjust_with_percentage(tax, finance_rules[FINANCE_RULE_HOUSE_TAX_BONUS_PERCENTAGE]);
        }
        int32_t people = house->house_population;
        player->population_per_level[level] += people;
        if (house->house_tax_coverage) {
            if (is_noble) {
                player->taxed_nobles += people;
            } else {
                player->taxed_plebs += people;
            }
            int32_t amount = people * tax;
            house->data.house.tax_income += difficulty_tax_percentages[settings_data.difficulty] * player->tax_percentage * amount;
            if (is_noble) {
                player->monthly_collected_nobles += amount;
            } else {
                player->monthly_collected_plebs += amount;
            }
        } else if (is_noble) {
            player->untaxed_nobles += people;
            player->monthly_uncollected_nobles += people * tax;
        } else {
            player->untaxed_plebs += people;
            player->monthly_uncollected_plebs += people * tax;
        }
    }
    int32_t nobles = city_finance_apply_tax_rate(player, player->monthly_collected_nobles);
    player->yearly_collected_nobles += nobles;
    int32_t plebs = city_finance_apply_tax_rate(player, player->monthly_collected_plebs);
    player->yearly_collected_plebs += plebs;
    player->yearly_uncollected_nobles += city_finance_apply_tax_rate(player, player->monthly_uncollected_nobles);
    player->yearly_uncollected_plebs += city_finance_apply_tax_rate(player, player->monthly_uncollected_plebs);
    player->treasury += nobles + plebs;
    player->percentage_taxed_plebs = calc_percentage(player->taxed_plebs, player->untaxed_plebs + player->taxed_plebs);
    player->percentage_taxed_nobles = calc_percentage(player->taxed_nobles, player->untaxed_nobles + player->taxed_nobles);
    player->percentage_taxed_people = calc_percentage(
        player->taxed_plebs + player->taxed_nobles, player->untaxed_nobles + player->untaxed_plebs + player->taxed_plebs + player->taxed_nobles);
}

void city_finance_pay_monthly_wages(void)
{
    city_player *player = city_player_current();
    int32_t wages = (player->workers_employed * player->wages) / WAGE_DIVISOR;
    player->treasury -= wages;
    player->wages_so_far += wages;
    player->wage_rate_paid_this_year += player->wages;
}

void city_finance_pay_monthly_interest(void)
{
    city_player *player = city_player_current();
    int32_t rate = scenario_data.interest_rate;
    if (player->has_temple_complex && building_temple_god(player->temple_complex_building_id) == GOD_RA) {
        int32_t reduction = finance_rules[FINANCE_RULE_RA_COMPLEX_INTEREST_REDUCTION];
        rate = ((rate - reduction) < 0) ? 0 : (rate - reduction);
    }
    if (player->treasury < 0) {
        int32_t interest = calc_adjust_with_percentage(-player->treasury, rate);
        player->treasury -= interest / MONTHS_PER_YEAR;
        player->interest_so_far += interest / MONTHS_PER_YEAR;
    }
}

void city_finance_pay_monthly_salary(void)
{
    city_player *player = city_player_current();
    if (player->treasury > finance_rules[FINANCE_RULE_SALARY_MIN_TREASURY] && player->mansion_placed != 0) {
        player->salary_so_far += player->salary_amount;
        player->personal_savings += player->salary_amount;
        player->treasury -= player->salary_amount;
    }
}

void city_finance_update_debt_months(void)
{
    city_player *player = city_player_current();
    if (player->treasury >= 0) {
        player->months_in_debt = 0;
        player->years_in_debt = 0;
        return;
    }
    int32_t months = ++player->months_in_debt;
    if (months == 0 || (months % MONTHS_PER_YEAR) != 0) {
        return;
    }
    int32_t years = ++player->years_in_debt;
    if (years >= DEBT_PENALTY_YEARS) {
        years = DEBT_PENALTY_YEARS;
    }
    city_ratings_change_kingdom(debt_kingdom_penalty(years, settings_data.difficulty));
    city_message_use_popup = 1;
    city_message_post(MESSAGE_CITY_IN_DEBT, 0, 0, NULL, 0);
}

void city_finance_handle_year_change(void)
{
    city_player *player = city_player_current();
    player->taxes_last_year = player->yearly_collected_nobles + player->yearly_collected_plebs;
    player->yearly_collected_plebs = 0;
    player->yearly_collected_nobles = 0;
    player->yearly_uncollected_plebs = 0;
    player->yearly_uncollected_nobles = 0;
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *house = building_get(id);
        if (house->state == BUILDING_STATE_IN_USE && house->house_size != 0) {
            house->data.house.tax_income = 0;
        }
    }
}

void city_finance_year_change_wages(void)
{
    city_player *player = city_player_current();
    player->wages_last_year = player->wages_so_far;
    player->wages_so_far = 0;
    player->wage_rate_paid_last_year = player->wage_rate_paid_this_year;
    player->wage_rate_paid_this_year = 0;
}

void city_finance_year_change_trade(void)
{
    city_player *player = city_player_current();
    player->exports_last_year = player->exports_this_year;
    player->exports_this_year = 0;
    player->imports_last_year = player->imports_this_year;
    player->imports_this_year = 0;
}

void city_finance_year_change_construction(void)
{
    city_player *player = city_player_current();
    player->construction_last_year = player->construction_this_year;
    player->construction_this_year = 0;
}

void city_finance_year_change_interest(void)
{
    city_player *player = city_player_current();
    player->interest_last_year = player->interest_so_far;
    player->interest_so_far = 0;
}

void city_finance_year_change_salary(void)
{
    city_player *player = city_player_current();
    player->salary_last_year = player->salary_so_far;
    player->salary_so_far = 0;
}

void city_finance_year_change_sundries(void)
{
    city_player *player = city_player_current();
    player->stolen_last_year = player->stolen_this_year;
    player->stolen_this_year = 0;
    player->palace_robberies_last_year = player->palace_robberies;
    player->palace_robberies = 0;
    player->savings_lost = 0;
}

void city_finance_year_change_requests_festivals(void)
{
    city_player *player = city_player_current();
    player->requests_and_festivals_last_year = player->requests_and_festivals_this_year;
    player->requests_and_festivals_this_year = 0;
}

static int32_t tribute_by_population(int32_t population)
{
    if (population >= 5001) {
        return 500;
    }
    if (population >= 3001) {
        return 400;
    }
    if (population >= 2001) {
        return 300;
    }
    if (population >= 1001) {
        return 225;
    }
    if (population >= 501) {
        return 150;
    }
    return 50;
}

void city_finance_calculate_tribute(void)
{
    city_player *player = city_player_current();
    if (player->treasury <= 0) {
        player->tribute_estimate = 0;
        return;
    }
    int32_t population = player->population;
    if (player->net_in_out_this_year > 0) {
        player->tribute_estimate = tribute_by_population(population);
        int32_t share = calc_adjust_with_percentage(player->net_in_out_this_year, finance_rules[FINANCE_RULE_TRIBUTE_PROFIT_PERCENTAGE]);
        if (share > player->tribute_estimate) {
            player->tribute_estimate = share;
        }
    } else if (population > 2000) {
        player->tribute_estimate = 200;
    } else if (population > 1000) {
        player->tribute_estimate = 100;
    } else {
        player->tribute_estimate = 0;
    }
    player->tribute_estimate = 0;
}

void city_finance_pay_tribute(void)
{
    city_player *player = city_player_current();
    player->donated_last_year = player->donated_this_year;
    player->donated_this_year = 0;
    player->gold_last_year = player->gold_this_year;
    player->gold_this_year = 0;
    player->tribute_not_paid_last_year = 0;
    int32_t income = player->taxes_last_year + player->exports_last_year + player->gold_last_year + player->donated_last_year;
    int32_t expenses = player->salary_last_year + player->interest_last_year + player->construction_last_year + player->wages_last_year +
        player->imports_last_year;
    if (player->player_rank == PLAYER_RANK_PHARAOH) {
        player->tribute_not_paid_last_year = 0;
        player->tribute_not_paid_total_years = 0;
        player->tribute_paid = 0;
    } else if (player->treasury < 1) {
        player->tribute_not_paid_last_year = 1;
        player->tribute_not_paid_total_years++;
        player->tribute_paid = 0;
    } else {
        player->tribute_not_paid_total_years = 0;
        if (expenses < income) {
            player->tribute_paid = tribute_by_population(player->population);
            int32_t tribute = calc_adjust_with_percentage(income - expenses, finance_rules[FINANCE_RULE_TRIBUTE_PROFIT_PERCENTAGE]);
            if (tribute > player->tribute_paid) {
                player->tribute_paid = tribute;
            }
        } else if (player->population > 2000) {
            player->tribute_paid = 200;
        } else if (player->population > 1000) {
            player->tribute_paid = 100;
        } else {
            player->tribute_paid = 0;
        }
    }
    player->treasury -= player->tribute_paid;
    int32_t paid = player->tribute_paid;
    city_finance_calculate_tribute();
    player->balance_last_year = player->treasury;
    player->income_last_year = income;
    player->expenses_last_year = expenses + paid;
}

int32_t building_is_mansion(int32_t building_id)
{
    int32_t type = building_get(building_id)->type;
    return (type >= BUILDING_PERSONAL_MANSION) && (type <= BUILDING_DYNASTY_MANSION);
}

static int32_t keeps_city_money(int32_t type)
{
    switch (type) {
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
        case BUILDING_COURTHOUSE:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            return 1;
        default:
            return 0;
    }
}

int32_t building_is_palace(int32_t building_id)
{
    return keeps_city_money(building_get(building_id)->type);
}

int32_t building_is_palace_or_mansion(int32_t building_id)
{
    int32_t type = building_get(building_id)->type;
    return (type >= BUILDING_PERSONAL_MANSION && type <= BUILDING_DYNASTY_MANSION) || keeps_city_money(type);
}

int32_t city_finance_money_share(int32_t money, int32_t index, const money_share_row *table)
{
    if (money < 1) {
        return 0;
    }
    int32_t buildings[MONEY_SHARE_ROWS];
    for (int32_t i = 0; i < MONEY_SHARE_ROWS; i++) {
        const money_share_row *row = &table[i];
        buildings[i] = 0;
        for (int32_t j = 0; j < row->building_type_count; j++) {
            buildings[i] += building_count_total[row->building_types[j]];
        }
    }
    int32_t total = (table[2].weight * buildings[2]) + (table[1].weight * buildings[1]) + (table[0].weight * buildings[0]);
    if (total == 0) {
        return 0;
    }
    return crt_ftol_double(((double) table[index].weight / (double) total) * (double) money);
}

int32_t city_finance_building_money_value(int32_t building_id)
{
    const city_player *player = city_player_current();
    building *b = building_get(building_id);
    if (b->state != BUILDING_STATE_IN_USE) {
        return 0;
    }
    switch (b->type) {
        case BUILDING_PERSONAL_MANSION:
            return city_finance_money_share(player->personal_savings, MANSION_SHARE_PERSONAL, mansion_shares());
        case BUILDING_FAMILY_MANSION:
            return city_finance_money_share(player->personal_savings, MANSION_SHARE_FAMILY, mansion_shares());
        case BUILDING_DYNASTY_MANSION:
            return city_finance_money_share(player->personal_savings, MANSION_SHARE_DYNASTY, mansion_shares());
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            return city_finance_money_share(player->treasury, PALACE_SHARE_PALACES, palace_shares());
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
            return city_finance_money_share(player->treasury, PALACE_SHARE_TAX_COLLECTORS, palace_shares());
        case BUILDING_COURTHOUSE:
            return city_finance_money_share(player->treasury, PALACE_SHARE_COURTHOUSE, palace_shares());
        default:
            return 0;
    }
}
