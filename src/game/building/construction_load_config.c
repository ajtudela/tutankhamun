// The gameplay config table of the migration warning (core/config.h): the population it starts from, the
// missions in which some causes stay quiet, and its thresholds.

#include "building/construction_load.h"
#include "core/config.h"

static const config_name migration_warning_rule_names[] = {
    { "min_population", MIGRATION_WARNING_RULE_MIN_POPULATION },
    { "quiet_missions", MIGRATION_WARNING_RULE_QUIET_MISSIONS },
    { "max_armed_figures", MIGRATION_WARNING_RULE_MAX_ARMED_FIGURES },
    { "few_newcomers", MIGRATION_WARNING_RULE_FEW_NEWCOMERS },
    { "low_percentage", MIGRATION_WARNING_RULE_LOW_PERCENTAGE },
    { 0, 0 },
};

static const config_field migration_warning_rule_fields[] = {
    { "value", migration_warning_rules, 1, 1, 0, 30000, 0 },
};

static const config_storage migration_warning_rule_storage[] = {
    { migration_warning_rules, sizeof(migration_warning_rules) },
};

const config_table migration_warning_rule_table = {
    "migration_warning_rules.json",
    "rule",
    migration_warning_rule_names,
    MIGRATION_WARNING_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(migration_warning_rule_fields, migration_warning_rule_storage),
};
