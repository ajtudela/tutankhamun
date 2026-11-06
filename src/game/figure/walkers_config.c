// The gameplay config table of the walkers (core/config.h): walker_rules.json, the roam lengths, the entertainers'
// walk to a venue, the firemen's and constables' searches, the fire fight, the ferry wait and the festival trips.

#include "core/config.h"
#include "figure/walkers.h"

static const config_name walker_rule_names[] = {
    { "roamer_roam_ticks", WALKER_RULE_ROAMER_ROAM_TICKS },
    { "patrician_roam_ticks", WALKER_RULE_PATRICIAN_ROAM_TICKS },
    { "missionary_roam_ticks", WALKER_RULE_MISSIONARY_ROAM_TICKS },
    { "tax_collector_roam_ticks", WALKER_RULE_TAX_COLLECTOR_ROAM_TICKS },
    { "entertainer_roam_ticks", WALKER_RULE_ENTERTAINER_ROAM_TICKS },
    { "engineer_roam_ticks", WALKER_RULE_ENGINEER_ROAM_TICKS },
    { "fireman_roam_ticks", WALKER_RULE_FIREMAN_ROAM_TICKS },
    { "law_walker_roam_ticks", WALKER_RULE_LAW_WALKER_ROAM_TICKS },
    { "market_trader_empty_turn_ticks", WALKER_RULE_MARKET_TRADER_EMPTY_TURN_TICKS },
    { "entertainer_venue_walk_ticks", WALKER_RULE_ENTERTAINER_VENUE_WALK_TICKS },
    { "home_road_radius", WALKER_RULE_HOME_ROAD_RADIUS },
    { "ferry_wait_ticks", WALKER_RULE_FERRY_WAIT_TICKS },
    { "fire_search_ticks", WALKER_RULE_FIRE_SEARCH_TICKS },
    { "fire_search_distance", WALKER_RULE_FIRE_SEARCH_DISTANCE },
    { "fire_fight_ticks", WALKER_RULE_FIRE_FIGHT_TICKS },
    { "fire_out_duration", WALKER_RULE_FIRE_OUT_DURATION },
    { "enemy_search_ticks", WALKER_RULE_ENEMY_SEARCH_TICKS },
    { "enemy_search_distance", WALKER_RULE_ENEMY_SEARCH_DISTANCE },
    { "festival_trips", WALKER_RULE_FESTIVAL_TRIPS },
    { 0, 0 },
};

static const config_field walker_rule_fields[] = {
    { "value", walker_rules, 1, 1, 1, 10000, 0 },
};

static const config_storage walker_rule_storage[] = {
    { walker_rules, sizeof(walker_rules) },
};

const config_table walker_rule_table = {
    "walker_rules.json",
    "rule",
    walker_rule_names,
    WALKER_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(walker_rule_fields, walker_rule_storage),
};
