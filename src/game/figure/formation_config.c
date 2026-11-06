// The gameplay config table of the legions' morale (core/config.h): formation_morale_rules.json, the morale losses of
// a legion by the share of its soldiers that died.

#include "core/config.h"
#include "figure/formation.h"

static const config_name formation_morale_rule_names[] = {
    { "new_legion", FORMATION_MORALE_RULE_NEW_LEGION },
    { "charioteers_max", FORMATION_MORALE_RULE_CHARIOTEERS_MAX },
    { "trained_charioteers_max", FORMATION_MORALE_RULE_TRAINED_CHARIOTEERS_MAX },
    { "kingdom_soldiers_max", FORMATION_MORALE_RULE_KINGDOM_SOLDIERS_MAX },
    { "foot_soldiers_max", FORMATION_MORALE_RULE_FOOT_SOLDIERS_MAX },
    { "trained_foot_soldiers_max", FORMATION_MORALE_RULE_TRAINED_FOOT_SOLDIERS_MAX },
    { "enemy_max", FORMATION_MORALE_RULE_ENEMY_MAX },
    { "enemy_13_max", FORMATION_MORALE_RULE_ENEMY_13_MAX },
    { "at_fort_gain", FORMATION_MORALE_RULE_AT_FORT_GAIN },
    { "away_loss", FORMATION_MORALE_RULE_AWAY_LOSS },
    { "months_away_without_loss", FORMATION_MORALE_RULE_MONTHS_AWAY_WITHOUT_LOSS },
    { "low", FORMATION_MORALE_RULE_LOW },
    { "very_low", FORMATION_MORALE_RULE_VERY_LOW },
    { "rout_swing", FORMATION_MORALE_RULE_ROUT_SWING },
    { "death_share_1", FORMATION_MORALE_RULE_DEATH_SHARE_1 },
    { "death_share_2", FORMATION_MORALE_RULE_DEATH_SHARE_2 },
    { "death_share_3", FORMATION_MORALE_RULE_DEATH_SHARE_3 },
    { "death_share_4", FORMATION_MORALE_RULE_DEATH_SHARE_4 },
    { "death_share_5", FORMATION_MORALE_RULE_DEATH_SHARE_5 },
    { "death_loss_1", FORMATION_MORALE_RULE_DEATH_LOSS_1 },
    { "death_loss_2", FORMATION_MORALE_RULE_DEATH_LOSS_2 },
    { "death_loss_3", FORMATION_MORALE_RULE_DEATH_LOSS_3 },
    { "death_loss_4", FORMATION_MORALE_RULE_DEATH_LOSS_4 },
    { "death_loss_5", FORMATION_MORALE_RULE_DEATH_LOSS_5 },
    { "death_loss_6", FORMATION_MORALE_RULE_DEATH_LOSS_6 },
    { 0, 0 },
};

static const config_field formation_morale_rule_fields[] = {
    { "value", formation_morale_rules, 1, 1, 0, 100, 0 },
};

static const config_storage formation_morale_rule_storage[] = {
    { formation_morale_rules, sizeof(formation_morale_rules) },
};

const config_table formation_morale_rule_table = {
    "formation_morale_rules.json",
    "rule",
    formation_morale_rule_names,
    FORMATION_MORALE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(formation_morale_rule_fields, formation_morale_rule_storage),
};
