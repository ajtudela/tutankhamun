// The gameplay config table culture_rules.json: the records and columns of culture_rules (city/culture.h), the tunable
// values of the culture coverage and rating.

#include "city/culture.h"
#include "core/config.h"

static const config_name culture_rule_names[] = {
    { "jugglers_per_venue", CULTURE_RULE_JUGGLERS_PER_VENUE },
    { "musicians_per_venue", CULTURE_RULE_MUSICIANS_PER_VENUE },
    { "dancers_per_venue", CULTURE_RULE_DANCERS_PER_VENUE },
    { "senet_house_people", CULTURE_RULE_SENET_HOUSE_PEOPLE },
    { "zoo_people", CULTURE_RULE_ZOO_PEOPLE },
    { "temple_complex_people", CULTURE_RULE_TEMPLE_COMPLEX_PEOPLE },
    { "local_temple_people", CULTURE_RULE_LOCAL_TEMPLE_PEOPLE },
    { "local_shrine_people", CULTURE_RULE_LOCAL_SHRINE_PEOPLE },
    { "patron_temple_people", CULTURE_RULE_PATRON_TEMPLE_PEOPLE },
    { "patron_shrine_people", CULTURE_RULE_PATRON_SHRINE_PEOPLE },
    { "school_children", CULTURE_RULE_SCHOOL_CHILDREN },
    { "library_people", CULTURE_RULE_LIBRARY_PEOPLE },
    { "booth_no_show_weight", CULTURE_RULE_BOOTH_NO_SHOW_WEIGHT },
    { "bandstand_no_show_weight", CULTURE_RULE_BANDSTAND_NO_SHOW_WEIGHT },
    { "pavilion_no_show_weight", CULTURE_RULE_PAVILION_NO_SHOW_WEIGHT },
    { "senet_no_show_weight", CULTURE_RULE_SENET_NO_SHOW_WEIGHT },
    { "zoo_no_show_weight", CULTURE_RULE_ZOO_NO_SHOW_WEIGHT },
    { "small_festival_people_per_deben", CULTURE_RULE_SMALL_FESTIVAL_PEOPLE_PER_DEBEN },
    { "small_festival_base_cost", CULTURE_RULE_SMALL_FESTIVAL_BASE_COST },
    { "large_festival_people_per_deben", CULTURE_RULE_LARGE_FESTIVAL_PEOPLE_PER_DEBEN },
    { "large_festival_base_cost", CULTURE_RULE_LARGE_FESTIVAL_BASE_COST },
    { "grand_festival_people_per_deben", CULTURE_RULE_GRAND_FESTIVAL_PEOPLE_PER_DEBEN },
    { "grand_festival_base_cost", CULTURE_RULE_GRAND_FESTIVAL_BASE_COST },
    { "grand_festival_people_per_beer_step", CULTURE_RULE_GRAND_FESTIVAL_PEOPLE_PER_BEER_STEP },
    { "grand_festival_beer_step", CULTURE_RULE_GRAND_FESTIVAL_BEER_STEP },
    { 0, 0 },
};

static const config_field culture_rule_fields[] = {
    { "value", culture_rules, 1, 1, 1, 1000000, 0 },
};

static const config_storage culture_rule_storage[] = {
    { culture_rules, sizeof(culture_rules) },
};

const config_table culture_rule_table = {
    "culture_rules.json",
    "rule",
    culture_rule_names,
    CULTURE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(culture_rule_fields, culture_rule_storage),
};
