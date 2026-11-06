// The gameplay config table of the work camp laborers (core/config.h): laborer_rules.json, the laborers' loads, waits,
// walk limits, the camp's spawning by worker percentage, the farm search and the work done in a row.

#include "core/config.h"
#include "figure/laborer.h"

static const config_name laborer_rule_names[] = {
    { "load", LABORER_RULE_LOAD },
    { "yard_wait_ticks", LABORER_RULE_YARD_WAIT_TICKS },
    { "ramp_route_retry_ticks", LABORER_RULE_RAMP_ROUTE_RETRY_TICKS },
    { "ramp_route_give_up_ticks", LABORER_RULE_RAMP_ROUTE_GIVE_UP_TICKS },
    { "delivered_wait_ticks", LABORER_RULE_DELIVERED_WAIT_TICKS },
    { "ferry_wait_ticks", LABORER_RULE_FERRY_WAIT_TICKS },
    { "max_roam_ticks", LABORER_RULE_MAX_ROAM_TICKS },
    { "farm_walk_ticks", LABORER_RULE_FARM_WALK_TICKS },
    { "home_road_radius", LABORER_RULE_HOME_ROAD_RADIUS },
    { "max_out", LABORER_RULE_MAX_OUT },
    { "labor_seeker_houses", LABORER_RULE_LABOR_SEEKER_HOUSES },
    { "spawn_delay_full", LABORER_RULE_SPAWN_DELAY_FULL },
    { "spawn_delay_75_percent", LABORER_RULE_SPAWN_DELAY_75_PERCENT },
    { "spawn_delay_50_percent", LABORER_RULE_SPAWN_DELAY_50_PERCENT },
    { "spawn_delay_25_percent", LABORER_RULE_SPAWN_DELAY_25_PERCENT },
    { "spawn_delay_low", LABORER_RULE_SPAWN_DELAY_LOW },
    { "farm_search_distance", LABORER_RULE_FARM_SEARCH_DISTANCE },
    { "farm_busy_labor_days", LABORER_RULE_FARM_BUSY_LABOR_DAYS },
    { "farm_labor_days", LABORER_RULE_FARM_LABOR_DAYS },
    { "camp_labor_days", LABORER_RULE_CAMP_LABOR_DAYS },
    { "level_tiles_in_a_row", LABORER_RULE_LEVEL_TILES_IN_A_ROW },
    { "monument_work_searches", LABORER_RULE_MONUMENT_WORK_SEARCHES },
    { 0, 0 },
};

static const config_field laborer_rule_fields[] = {
    { "value", laborer_rules, 1, 1, 0, 10000, 0 },
};

static const config_storage laborer_rule_storage[] = {
    { laborer_rules, sizeof(laborer_rules) },
};

const config_table laborer_rule_table = {
    "laborer_rules.json",
    "rule",
    laborer_rule_names,
    LABORER_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(laborer_rule_fields, laborer_rule_storage),
};
