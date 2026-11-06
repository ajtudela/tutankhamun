// The gameplay config table of the figure movement (core/config.h): movement_rules.json, the most damage a blocked
// enemy does to a building, wall or gatehouse, and where and how far a roamer looks for its first road.

#include "core/config.h"
#include "figure/movement.h"

static const config_name movement_rule_names[] = {
    { "building_max_damage", MOVEMENT_RULE_BUILDING_MAX_DAMAGE },
    { "wall_max_damage", MOVEMENT_RULE_WALL_MAX_DAMAGE },
    { "gatehouse_max_damage", MOVEMENT_RULE_GATEHOUSE_MAX_DAMAGE },
    { "roam_start_distance", MOVEMENT_RULE_ROAM_START_DISTANCE },
    { "roam_road_radius", MOVEMENT_RULE_ROAM_ROAD_RADIUS },
    { 0, 0 },
};

static const config_field movement_rule_fields[] = {
    { "value", movement_rules, 1, 1, 0, 10000, 0 },
};

static const config_storage movement_rule_storage[] = {
    { movement_rules, sizeof(movement_rules) },
};

const config_table movement_rule_table = {
    "movement_rules.json",
    "rule",
    movement_rule_names,
    MOVEMENT_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(movement_rule_fields, movement_rule_storage),
};
