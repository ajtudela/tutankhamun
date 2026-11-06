// The gameplay config table kingdom_salaries.json: salary_by_rank, the salary of each rank.

#include "city/city_data.h"
#include "core/config.h"

static const config_field kingdom_salary_fields[] = {
    { "salary", salary_by_rank, 1, 1, 0, 100000, 0 },
};

static const config_storage kingdom_salary_storage[] = {
    { salary_by_rank, sizeof(salary_by_rank) },
};

const config_table kingdom_salary_table = {
    "kingdom_salaries.json",
    "rank",
    0,
    CONFIG_COUNT(salary_by_rank),
    CONFIG_TABLE_COLUMNS(kingdom_salary_fields, kingdom_salary_storage),
};
