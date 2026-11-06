// The gameplay config tables of the military (core/config.h): military_rules.json (the combat balance, the
// sentries', ballistas', warships' and soldiers' timings and ranges, the criminals' waits and thefts),
// military_difficulty.json (per difficulty a thief's share and a robbed tomb's rating cost), chariot_cart_offsets.json,
// transport_escapes.json and soldier_alternative_spots.json.

#include "city/difficulty.h"
#include "core/config.h"
#include "figure/military.h"

static const config_name military_rule_names[] = {
    { "melee_hit_ticks", MILITARY_RULE_MELEE_HIT_TICKS },
    { "flank_attack_bonus", MILITARY_RULE_FLANK_ATTACK_BONUS },
    { "line_attack_bonus", MILITARY_RULE_LINE_ATTACK_BONUS },
    { "mop_up_attack_bonus", MILITARY_RULE_MOP_UP_ATTACK_BONUS },
    { "line_defense_bonus", MILITARY_RULE_LINE_DEFENSE_BONUS },
    { "mop_up_defense_penalty", MILITARY_RULE_MOP_UP_DEFENSE_PENALTY },
    { "facing_away_defense_penalty", MILITARY_RULE_FACING_AWAY_DEFENSE_PENALTY },
    { "kingdom_soldier_line_bonus", MILITARY_RULE_KINGDOM_SOLDIER_LINE_BONUS },
    { "max_experience", MILITARY_RULE_MAX_EXPERIENCE },
    { "charge_min_steps", MILITARY_RULE_CHARGE_MIN_STEPS },
    { "charge_max_steps", MILITARY_RULE_CHARGE_MAX_STEPS },
    { "charge_attack_factor", MILITARY_RULE_CHARGE_ATTACK_FACTOR },
    { "javelin_damage", MILITARY_RULE_JAVELIN_DAMAGE },
    { "bolt_damage", MILITARY_RULE_BOLT_DAMAGE },
    { "missile_attack_timeout", MILITARY_RULE_MISSILE_ATTACK_TIMEOUT },
    { "missile_facing_away_penalty", MILITARY_RULE_MISSILE_FACING_AWAY_PENALTY },
    { "missile_single_line_bonus", MILITARY_RULE_MISSILE_SINGLE_LINE_BONUS },
    { "missile_mop_up_penalty", MILITARY_RULE_MISSILE_MOP_UP_PENALTY },
    { "missile_docked_warship_bonus", MILITARY_RULE_MISSILE_DOCKED_WARSHIP_BONUS },
    { "target_priority_enemy_soldier", MILITARY_RULE_TARGET_PRIORITY_ENEMY_SOLDIER },
    { "target_priority_animal", MILITARY_RULE_TARGET_PRIORITY_ANIMAL },
    { "target_priority_loaded_transport", MILITARY_RULE_TARGET_PRIORITY_LOADED_TRANSPORT },
    { "target_priority_transport", MILITARY_RULE_TARGET_PRIORITY_TRANSPORT },
    { "target_priority_warship", MILITARY_RULE_TARGET_PRIORITY_WARSHIP },
    { "sentry_rest_ticks", MILITARY_RULE_SENTRY_REST_TICKS },
    { "sentry_search_ticks", MILITARY_RULE_SENTRY_SEARCH_TICKS },
    { "sentry_shoot_distance", MILITARY_RULE_SENTRY_SHOOT_DISTANCE },
    { "ballista_search_ticks", MILITARY_RULE_BALLISTA_SEARCH_TICKS },
    { "ballista_aim_ticks", MILITARY_RULE_BALLISTA_AIM_TICKS },
    { "ballista_reload_ticks", MILITARY_RULE_BALLISTA_RELOAD_TICKS },
    { "warship_hunt_range", MILITARY_RULE_WARSHIP_HUNT_RANGE },
    { "warship_patrol_extra_range", MILITARY_RULE_WARSHIP_PATROL_EXTRA_RANGE },
    { "warship_patrol_radius", MILITARY_RULE_WARSHIP_PATROL_RADIUS },
    { "ship_damaged_percentage", MILITARY_RULE_SHIP_DAMAGED_PERCENTAGE },
    { "transport_evade_ticks", MILITARY_RULE_TRANSPORT_EVADE_TICKS },
    { "transport_evade_range", MILITARY_RULE_TRANSPORT_EVADE_RANGE },
    { "soldier_engage_radius", MILITARY_RULE_SOLDIER_ENGAGE_RADIUS },
    { "soldier_mop_up_range", MILITARY_RULE_SOLDIER_MOP_UP_RANGE },
    { "soldier_attack_formation_range", MILITARY_RULE_SOLDIER_ATTACK_FORMATION_RANGE },
    { "criminal_wait_ticks", MILITARY_RULE_CRIMINAL_WAIT_TICKS },
    { "criminal_speed_factor", MILITARY_RULE_CRIMINAL_SPEED_FACTOR },
    { "cart_thief_flee_ticks", MILITARY_RULE_CART_THIEF_FLEE_TICKS },
    { "cart_theft_amount", MILITARY_RULE_CART_THEFT_AMOUNT },
    { "tomb_robber_killed_rating", MILITARY_RULE_TOMB_ROBBER_KILLED_RATING },
    { 0, 0 },
};

static const config_field military_rule_fields[] = {
    { "value", military_rules, 1, 1, 0, 10000, 0 },
};

static const config_storage military_rule_storage[] = {
    { military_rules, sizeof(military_rules) },
};

const config_table military_rule_table = {
    "military_rules.json",
    "rule",
    military_rule_names,
    MILITARY_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(military_rule_fields, military_rule_storage),
};

static const config_field military_difficulty_fields[] = {
    { "theft_percentage", &military_difficulties[0].theft_percentage, (int32_t) (sizeof(military_difficulty) / sizeof(int32_t)), 1, 0, 100, 0 },
    { "tomb_robbery_rating", &military_difficulties[0].tomb_robbery_rating, (int32_t) (sizeof(military_difficulty) / sizeof(int32_t)), 1, -100, 0, 0 },
};

static const config_storage military_difficulty_storage[] = {
    { military_difficulties, sizeof(military_difficulties) },
};

const config_table military_difficulty_table = {
    "military_difficulty.json",
    "difficulty",
    config_difficulty_names,
    DIFFICULTY_COUNT,
    CONFIG_TABLE_COLUMNS(military_difficulty_fields, military_difficulty_storage),
};

static const config_field chariot_cart_offset_fields[] = {
    { "dx", &chariot_cart_offsets[0].dx, (int32_t) (sizeof(military_offset) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dy", &chariot_cart_offsets[0].dy, (int32_t) (sizeof(military_offset) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
};

static const config_storage chariot_cart_offset_storage[] = {
    { chariot_cart_offsets, sizeof(chariot_cart_offsets) },
};

const config_table chariot_cart_offset_table = {
    "chariot_cart_offsets.json",
    "direction",
    config_direction_names,
    DIRECTION_COUNT,
    CONFIG_TABLE_COLUMNS(chariot_cart_offset_fields, chariot_cart_offset_storage),
};

static const config_field transport_escape_fields[] = {
    { "dx", &transport_escapes[0].dx[0], (int32_t) (sizeof(transport_escape) / sizeof(int32_t)), TRANSPORT_ESCAPE_POINTS, -30, 30, 0 },
    { "dy", &transport_escapes[0].dy[0], (int32_t) (sizeof(transport_escape) / sizeof(int32_t)), TRANSPORT_ESCAPE_POINTS, -30, 30, 0 },
};

static const config_storage transport_escape_storage[] = {
    { transport_escapes, sizeof(transport_escapes) },
};

const config_table transport_escape_table = {
    "transport_escapes.json",
    "direction",
    config_direction_names,
    DIRECTION_COUNT,
    CONFIG_TABLE_COLUMNS(transport_escape_fields, transport_escape_storage),
};

static const config_field soldier_alternative_spot_fields[] = {
    { "dx", &soldier_alternative_spots[0].dx, (int32_t) (sizeof(military_offset) / sizeof(int32_t)), 1, -10, 10, 0 },
    { "dy", &soldier_alternative_spots[0].dy, (int32_t) (sizeof(military_offset) / sizeof(int32_t)), 1, -10, 10, 0 },
};

static const config_storage soldier_alternative_spot_storage[] = {
    { soldier_alternative_spots, sizeof(soldier_alternative_spots) },
};

const config_table soldier_alternative_spot_table = {
    "soldier_alternative_spots.json",
    "spot",
    0,
    SOLDIER_ALTERNATIVE_SPOTS,
    CONFIG_TABLE_COLUMNS(soldier_alternative_spot_fields, soldier_alternative_spot_storage),
};
