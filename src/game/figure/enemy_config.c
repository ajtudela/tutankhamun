// The gameplay config tables of the enemy soldiers (core/config.h): enemy_cart_offsets.json (per direction where an
// enemy chariot's cart is drawn) and enemy_foot_rules.json (per foot enemy type its speed factor and attack sound).

#include "core/config.h"
#include "figure/enemy.h"
#include "figure/figure_type.h"

static const config_field enemy_cart_offset_fields[] = {
    { "dx", &enemy_cart_offsets[0].dx, (int32_t) (sizeof(enemy_cart_offset) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dy", &enemy_cart_offsets[0].dy, (int32_t) (sizeof(enemy_cart_offset) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
};

static const config_storage enemy_cart_offset_storage[] = {
    { enemy_cart_offsets, sizeof(enemy_cart_offsets) },
};

const config_table enemy_cart_offset_table = {
    "enemy_cart_offsets.json",
    "direction",
    config_direction_names,
    DIRECTION_COUNT,
    CONFIG_TABLE_COLUMNS(enemy_cart_offset_fields, enemy_cart_offset_storage),
};

static const config_name enemy_foot_names[] = {
    { "FIGURE_ENEMY_ARCHER", ENEMY_FOOT_ARCHER },
    { "FIGURE_ENEMY_INFANTRY", ENEMY_FOOT_INFANTRY },
    { "FIGURE_ENEMY_CHARIOT", ENEMY_FOOT_CHARIOT },
    { "FIGURE_ENEMY_EGYPTIAN_INFANTRY", ENEMY_FOOT_EGYPTIAN_INFANTRY },
    { "FIGURE_ENEMY_EGYPTIAN_ARCHER", ENEMY_FOOT_EGYPTIAN_ARCHER },
    { "FIGURE_ENEMY_EGYPTIAN_CHARIOT", ENEMY_FOOT_EGYPTIAN_CHARIOT },
    { "FIGURE_BEDOUIN", ENEMY_FOOT_BEDOUIN },
    { 0, 0 },
};

static const config_field enemy_foot_rule_fields[] = {
    { "speed_factor", &enemy_foot_rules[0].speed_factor, (int32_t) (sizeof(enemy_foot_rule) / sizeof(int32_t)), 1, 1, UINT8_MAX, 0 },
    { "attack_sound", &enemy_foot_rules[0].attack_sound, (int32_t) (sizeof(enemy_foot_rule) / sizeof(int32_t)), 1, ENEMY_NO_ATTACK_SOUND, 32, 0 },
};

static const config_storage enemy_foot_rule_storage[] = {
    { enemy_foot_rules, sizeof(enemy_foot_rules) },
};

const config_table enemy_foot_rule_table = {
    "enemy_foot_rules.json",
    "figure_type",
    enemy_foot_names,
    ENEMY_FOOT_KINDS,
    CONFIG_TABLE_COLUMNS(enemy_foot_rule_fields, enemy_foot_rule_storage),
};
