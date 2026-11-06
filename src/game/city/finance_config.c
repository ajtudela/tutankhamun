// The gameplay config tables finance_rules.json (finance_rules, the tunable values of city/finance.c) and
// debt_kingdom_penalties.json (per difficulty level the kingdom rating change of each of the first ten years in debt).

#include "city/city_data.h"
#include "city/difficulty.h"
#include "city/finance.h"
#include "core/config.h"

static const config_name finance_rule_names[] = { { "house_tax_bonus_percentage", FINANCE_RULE_HOUSE_TAX_BONUS_PERCENTAGE },
    { "ra_complex_interest_reduction", FINANCE_RULE_RA_COMPLEX_INTEREST_REDUCTION }, { "salary_min_treasury", FINANCE_RULE_SALARY_MIN_TREASURY },
    { "tribute_profit_percentage", FINANCE_RULE_TRIBUTE_PROFIT_PERCENTAGE }, { "bailout_prosperity_loss", FINANCE_RULE_BAILOUT_PROSPERITY_LOSS }, { 0, 0 } };

static const config_field finance_rule_fields[] = {
    { "value", finance_rules, 1, 1, -100000, 100000, 0 },
};

static const config_storage finance_rule_storage[] = {
    { finance_rules, sizeof(finance_rules) },
};

const config_table finance_rule_table = {
    "finance_rules.json",
    "rule",
    finance_rule_names,
    FINANCE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(finance_rule_fields, finance_rule_storage),
};

static const config_field debt_kingdom_penalty_fields[] = {
    { "penalty_by_years_in_debt", debt_kingdom_penalties, DEBT_PENALTY_YEARS, DEBT_PENALTY_YEARS, -100, 0, 0 },
};

static const config_storage debt_kingdom_penalty_storage[] = {
    { debt_kingdom_penalties, sizeof(int32_t) * DEBT_PENALTY_YEARS *DIFFICULTY_COUNT },
};

const config_table debt_kingdom_penalty_table = {
    "debt_kingdom_penalties.json",
    "level",
    config_difficulty_names,
    DIFFICULTY_COUNT,
    CONFIG_TABLE_COLUMNS(debt_kingdom_penalty_fields, debt_kingdom_penalty_storage),
};
