// The gameplay config table of the construction guild workers (core/config.h): guild_worker_rules.json, the roaming,
// waits and task searches of the workers, the load a worker uses up and the carpenters' guild progress.

#include "core/config.h"
#include "figure/construction_worker.h"

static const config_name guild_worker_rule_names[] = {
    { "max_roam_ticks", GUILD_WORKER_RULE_MAX_ROAM_TICKS },
    { "ferry_wait_ticks", GUILD_WORKER_RULE_FERRY_WAIT_TICKS },
    { "ghost_ticks", GUILD_WORKER_RULE_GHOST_TICKS },
    { "task_search_base_ticks", GUILD_WORKER_RULE_TASK_SEARCH_BASE_TICKS },
    { "task_search_random_ticks", GUILD_WORKER_RULE_TASK_SEARCH_RANDOM_TICKS },
    { "guild_road_radius", GUILD_WORKER_RULE_GUILD_ROAD_RADIUS },
    { "mausoleum_head_start_ticks", GUILD_WORKER_RULE_MAUSOLEUM_HEAD_START_TICKS },
    { "abu_simbel_spots_in_a_row", GUILD_WORKER_RULE_ABU_SIMBEL_SPOTS_IN_A_ROW },
    { "load", GUILD_WORKER_RULE_LOAD },
    { "carpenter_progress", GUILD_WORKER_RULE_CARPENTER_PROGRESS },
    { 0, 0 },
};

static const config_field guild_worker_rule_fields[] = {
    { "value", guild_worker_rules, 1, 1, 1, 10000, 0 },
};

static const config_storage guild_worker_rule_storage[] = {
    { guild_worker_rules, sizeof(guild_worker_rules) },
};

const config_table guild_worker_rule_table = {
    "guild_worker_rules.json",
    "rule",
    guild_worker_rule_names,
    GUILD_WORKER_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(guild_worker_rule_fields, guild_worker_rule_storage),
};
