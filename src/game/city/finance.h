// City finance (city/finance.c): the monthly and yearly finance steps, the tribute, the money shares of palaces and
// mansions; finance_rules holds the tunable values.

#ifndef CITY_FINANCE_H
#define CITY_FINANCE_H

#include "city/city_data.h"
#include "city/city_player.h"

#include <assert.h>
#include <stdint.h>

enum {
    MONEY_SHARE_ROWS = 3,
    MONEY_SHARE_BUILDING_TYPES = 5,
};

enum {
    DEBT_PENALTY_YEARS = 10,
    WORKERS_PER_WAGE_UNIT = 10,
};

enum finance_rule {
    FINANCE_RULE_HOUSE_TAX_BONUS_PERCENTAGE,
    FINANCE_RULE_RA_COMPLEX_INTEREST_REDUCTION,
    FINANCE_RULE_SALARY_MIN_TREASURY,
    FINANCE_RULE_TRIBUTE_PROFIT_PERCENTAGE,
    FINANCE_RULE_BAILOUT_PROSPERITY_LOSS,
    FINANCE_RULE_COUNT,
};

extern int32_t finance_rules[FINANCE_RULE_COUNT];

enum mansion_share_row {
    MANSION_SHARE_PERSONAL = 0,
    MANSION_SHARE_FAMILY = 1,
    MANSION_SHARE_DYNASTY = 2,
};

enum palace_share_row {
    PALACE_SHARE_TAX_COLLECTORS = 0,
    PALACE_SHARE_COURTHOUSE = 1,
    PALACE_SHARE_PALACES = 2,
};

typedef struct {
    int32_t building_type_count;
    int32_t building_types[MONEY_SHARE_BUILDING_TYPES];
    int32_t weight;
} money_share_row;

static_assert(sizeof(money_share_row) == 0x1C, "money_share_row is a finance share table row of 28 bytes");

static inline const money_share_row *mansion_shares(void)
{
    return (const money_share_row *) mansion_share_table;
}

static inline const money_share_row *palace_shares(void)
{
    return (const money_share_row *) palace_share_table;
}

void city_finance_collect_monthly_taxes(void);
void city_finance_pay_monthly_wages(void);
void city_finance_pay_monthly_interest(void);
void city_finance_pay_monthly_salary(void);
void city_finance_update_debt_months(void);
void city_finance_handle_year_change(void);
void city_finance_year_change_wages(void);
void city_finance_year_change_trade(void);
void city_finance_year_change_construction(void);
void city_finance_year_change_interest(void);
void city_finance_year_change_salary(void);
void city_finance_year_change_sundries(void);
void city_finance_year_change_requests_festivals(void);
void city_finance_calculate_tribute(void);
void city_finance_pay_tribute(void);
int32_t building_is_mansion(int32_t building_id);
int32_t building_is_palace(int32_t building_id);
int32_t building_is_palace_or_mansion(int32_t building_id);
int32_t city_finance_apply_tax_rate(const city_player *player, int32_t amount);
int32_t city_finance_money_share(int32_t money, int32_t index, const money_share_row *table);
int32_t city_finance_building_money_value(int32_t building_id);

#endif
