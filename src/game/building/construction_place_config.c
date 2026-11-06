// The gameplay config table of building placement (core/config.h): the credit limit, the most forts, warship
// wharves and docks, and the materials some monuments take from the storage yards.

#include "building/construction_place.h"
#include "core/config.h"

static const config_name construction_place_rule_names[] = {
    { "out_of_credit_treasury", CONSTRUCTION_PLACE_RULE_OUT_OF_CREDIT_TREASURY },
    { "max_forts", CONSTRUCTION_PLACE_RULE_MAX_FORTS },
    { "max_warship_wharves", CONSTRUCTION_PLACE_RULE_MAX_WARSHIP_WHARVES },
    { "max_docks", CONSTRUCTION_PLACE_RULE_MAX_DOCKS },
    { "library_papyrus", CONSTRUCTION_PLACE_RULE_LIBRARY_PAPYRUS },
    { "small_obelisk_granite", CONSTRUCTION_PLACE_RULE_SMALL_OBELISK_GRANITE },
    { "large_obelisk_granite", CONSTRUCTION_PLACE_RULE_LARGE_OBELISK_GRANITE },
    { "sun_temple_sandstone", CONSTRUCTION_PLACE_RULE_SUN_TEMPLE_SANDSTONE },
    { "mausoleum_sandstone", CONSTRUCTION_PLACE_RULE_MAUSOLEUM_SANDSTONE },
    { 0, 0 },
};

static const config_field construction_place_rule_fields[] = {
    { "value", construction_place_rules, 1, 1, -100000, 100000, 0 },
};

static const config_storage construction_place_rule_storage[] = {
    { construction_place_rules, sizeof(construction_place_rules) },
};

const config_table construction_place_rule_table = {
    "construction_place_rules.json",
    "rule",
    construction_place_rule_names,
    CONSTRUCTION_PLACE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(construction_place_rule_fields, construction_place_rule_storage),
};
