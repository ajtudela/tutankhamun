// The config tables of the building info window, described over their C tables: the panels' help messages, the
// festival square, house crime, legion damage and ship hull text rules, the advisor offers per building type and per
// house evolve text, the pyramid style rules, and the window's heights and placements per screen size.

#include "building/building_type.h"
#include "building/house_evolve.h"
#include "core/config.h"
#include "window/advisors.h"
#include "window/building_info.h"
#include "window/building_info_advisors.h"
#include "window/building_info_house.h"
#include "window/building_info_military.h"
#include "window/building_info_monuments.h"
#include "window/building_info_panels.h"
#include "window/building_info_religion.h"
#include "window/building_info_ships.h"
#include <stddef.h>

static const config_name building_info_help_names[] = {
    { "natives", BUILDING_INFO_HELP_NATIVES },
    { "mission_post", BUILDING_INFO_HELP_MISSION_POST },
    { "scribal_school", BUILDING_INFO_HELP_SCRIBAL_SCHOOL },
    { "library", BUILDING_INFO_HELP_LIBRARY },
    { "architect_post", BUILDING_INFO_HELP_ARCHITECT_POST },
    { "police_station", BUILDING_INFO_HELP_POLICE_STATION },
    { "firehouse", BUILDING_INFO_HELP_FIREHOUSE },
    { "water_lift", BUILDING_INFO_HELP_WATER_LIFT },
    { "water_supply", BUILDING_INFO_HELP_WATER_SUPPLY },
    { "well", BUILDING_INFO_HELP_WELL },
    { "apothecary", BUILDING_INFO_HELP_APOTHECARY },
    { "physician", BUILDING_INFO_HELP_PHYSICIAN },
    { "dentist", BUILDING_INFO_HELP_DENTIST },
    { "mortuary", BUILDING_INFO_HELP_MORTUARY },
    { "palace", BUILDING_INFO_HELP_PALACE },
    { "courthouse", BUILDING_INFO_HELP_COURTHOUSE },
    { "tax_collector", BUILDING_INFO_HELP_TAX_COLLECTOR },
    { "mansion", BUILDING_INFO_HELP_MANSION },
    { "ruins", BUILDING_INFO_HELP_RUINS },
    { "gardens", BUILDING_INFO_HELP_GARDENS },
    { "walls", BUILDING_INFO_HELP_WALLS },
    { "irrigation_ditch", BUILDING_INFO_HELP_IRRIGATION_DITCH },
    { "plaza", BUILDING_INFO_HELP_PLAZA },
    { "statue", BUILDING_INFO_HELP_STATUE },
    { "bandstand", BUILDING_INFO_HELP_BANDSTAND },
    { "booth", BUILDING_INFO_HELP_BOOTH },
    { "senet_house", BUILDING_INFO_HELP_SENET_HOUSE },
    { "zoo", BUILDING_INFO_HELP_ZOO },
    { "pavilion", BUILDING_INFO_HELP_PAVILION },
    { "performer_school", BUILDING_INFO_HELP_PERFORMER_SCHOOL },
    { "oracle", BUILDING_INFO_HELP_ORACLE },
    { "temple", BUILDING_INFO_HELP_TEMPLE },
    { "festival_square", BUILDING_INFO_HELP_FESTIVAL_SQUARE },
    { "bazaar", BUILDING_INFO_HELP_BAZAAR },
    { "granary", BUILDING_INFO_HELP_GRANARY },
    { "storage_yard", BUILDING_INFO_HELP_STORAGE_YARD },
    { "house", BUILDING_INFO_HELP_HOUSE },
    { "work_camp", BUILDING_INFO_HELP_WORK_CAMP },
    { "grain_farm", BUILDING_INFO_HELP_GRAIN_FARM },
    { "barley_farm", BUILDING_INFO_HELP_BARLEY_FARM },
    { "lettuce_farm", BUILDING_INFO_HELP_LETTUCE_FARM },
    { "fig_farm", BUILDING_INFO_HELP_FIG_FARM },
    { "pomegranate_farm", BUILDING_INFO_HELP_POMEGRANATE_FARM },
    { "chickpea_farm", BUILDING_INFO_HELP_CHICKPEA_FARM },
    { "flax_farm", BUILDING_INFO_HELP_FLAX_FARM },
    { "henna_farm", BUILDING_INFO_HELP_HENNA_FARM },
    { "reed_gatherer", BUILDING_INFO_HELP_REED_GATHERER },
    { "cattle_ranch", BUILDING_INFO_HELP_CATTLE_RANCH },
    { "stone_quarry", BUILDING_INFO_HELP_STONE_QUARRY },
    { "quarry", BUILDING_INFO_HELP_QUARRY },
    { "gold_mine", BUILDING_INFO_HELP_GOLD_MINE },
    { "copper_mine", BUILDING_INFO_HELP_COPPER_MINE },
    { "gemstone_mine", BUILDING_INFO_HELP_GEMSTONE_MINE },
    { "wood_cutter", BUILDING_INFO_HELP_WOOD_CUTTER },
    { "clay_pit", BUILDING_INFO_HELP_CLAY_PIT },
    { "brewery", BUILDING_INFO_HELP_BREWERY },
    { "weaver", BUILDING_INFO_HELP_WEAVER },
    { "papyrus_maker", BUILDING_INFO_HELP_PAPYRUS_MAKER },
    { "lamp_maker", BUILDING_INFO_HELP_LAMP_MAKER },
    { "paint_maker", BUILDING_INFO_HELP_PAINT_MAKER },
    { "chariot_maker", BUILDING_INFO_HELP_CHARIOT_MAKER },
    { "brickworks", BUILDING_INFO_HELP_BRICKWORKS },
    { "weaponsmith", BUILDING_INFO_HELP_WEAPONSMITH },
    { "jeweler", BUILDING_INFO_HELP_JEWELER },
    { "potter", BUILDING_INFO_HELP_POTTER },
    { "carpenters_guild", BUILDING_INFO_HELP_CARPENTERS_GUILD },
    { "bricklayers_guild", BUILDING_INFO_HELP_BRICKLAYERS_GUILD },
    { "stonemasons_guild", BUILDING_INFO_HELP_STONEMASONS_GUILD },
    { "artisans_guild", BUILDING_INFO_HELP_ARTISANS_GUILD },
    { "hunting_lodge", BUILDING_INFO_HELP_HUNTING_LODGE },
    { "legion", BUILDING_INFO_HELP_LEGION },
    { "fort", BUILDING_INFO_HELP_FORT },
    { "gatehouse", BUILDING_INFO_HELP_GATEHOUSE },
    { "roadblock", BUILDING_INFO_HELP_ROADBLOCK },
    { "tower", BUILDING_INFO_HELP_TOWER },
    { "military_academy", BUILDING_INFO_HELP_MILITARY_ACADEMY },
    { "recruiter", BUILDING_INFO_HELP_RECRUITER },
    { "transport_ship", BUILDING_INFO_HELP_TRANSPORT_SHIP },
    { "warship", BUILDING_INFO_HELP_WARSHIP },
    { "shipwright", BUILDING_INFO_HELP_SHIPWRIGHT },
    { "dock", BUILDING_INFO_HELP_DOCK },
    { "fishing_wharf", BUILDING_INFO_HELP_FISHING_WHARF },
    { "transport_wharf", BUILDING_INFO_HELP_TRANSPORT_WHARF },
    { "warship_wharf", BUILDING_INFO_HELP_WARSHIP_WHARF },
    { "ferry_landing", BUILDING_INFO_HELP_FERRY_LANDING },
    { "sphinx", BUILDING_INFO_HELP_SPHINX },
    { "obelisk", BUILDING_INFO_HELP_OBELISK },
    { "sun_temple", BUILDING_INFO_HELP_SUN_TEMPLE },
    { "library_monument", BUILDING_INFO_HELP_LIBRARY_MONUMENT },
    { "abu_simbel", BUILDING_INFO_HELP_ABU_SIMBEL },
    { "caesareum", BUILDING_INFO_HELP_CAESAREUM },
    { "lighthouse", BUILDING_INFO_HELP_LIGHTHOUSE },
    { "royal_tomb", BUILDING_INFO_HELP_ROYAL_TOMB },
    { "mausoleum", BUILDING_INFO_HELP_MAUSOLEUM },
    { "window", BUILDING_INFO_HELP_WINDOW },
    { "road", BUILDING_INFO_HELP_ROAD },
    { "bridge", BUILDING_INFO_HELP_BRIDGE },
    { "figure", BUILDING_INFO_HELP_FIGURE },
    { "figure_from_shipwreck_on", BUILDING_INFO_HELP_FIGURE_FROM_SHIPWRECK_ON },
    { 0, 0 },
};

static const config_field building_info_help_message_fields[] = {
    { "message", building_info_help_messages, 1, 1, 0, 1000, 0 },
};

static const config_storage building_info_help_message_storage[] = {
    { building_info_help_messages, sizeof(building_info_help_messages) },
};

const config_table building_info_help_message_table = {
    "building_info_help_messages.json",
    "panel",
    building_info_help_names,
    BUILDING_INFO_HELP_COUNT,
    CONFIG_TABLE_COLUMNS(building_info_help_message_fields, building_info_help_message_storage),
};

static const config_name festival_square_rule_names[] = {
    { "months_few", FESTIVAL_SQUARE_RULE_MONTHS_FEW },
    { "months_quite_some_time", FESTIVAL_SQUARE_RULE_MONTHS_QUITE_SOME_TIME },
    { "months_over_a_year", FESTIVAL_SQUARE_RULE_MONTHS_OVER_A_YEAR },
    { "months_almost_two_years", FESTIVAL_SQUARE_RULE_MONTHS_ALMOST_TWO_YEARS },
    { "months_over_two_years", FESTIVAL_SQUARE_RULE_MONTHS_OVER_TWO_YEARS },
    { "months_good_old_days", FESTIVAL_SQUARE_RULE_MONTHS_GOOD_OLD_DAYS },
    { "months_effect_order_now", FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ORDER_NOW },
    { "months_effect_about_time", FESTIVAL_SQUARE_RULE_MONTHS_EFFECT_ABOUT_TIME },
    { 0, 0 },
};

static const config_field festival_square_rule_fields[] = {
    { "value", festival_square_rules, 1, 1, 0, 1000, 0 },
};

static const config_storage festival_square_rule_storage[] = {
    { festival_square_rules, sizeof(festival_square_rules) },
};

const config_table festival_square_rule_table = {
    "festival_square_rules.json",
    "rule",
    festival_square_rule_names,
    FESTIVAL_SQUARE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(festival_square_rule_fields, festival_square_rule_storage),
};

static const config_name house_crime_text_rule_names[] = {
    { "no_crime", HOUSE_CRIME_TEXT_RULE_NO_CRIME },
    { "peaceful", HOUSE_CRIME_TEXT_RULE_PEACEFUL },
    { "some_crime", HOUSE_CRIME_TEXT_RULE_SOME_CRIME },
    { "becoming_a_problem", HOUSE_CRIME_TEXT_RULE_BECOMING_A_PROBLEM },
    { "high_crime", HOUSE_CRIME_TEXT_RULE_HIGH_CRIME },
    { "lawless", HOUSE_CRIME_TEXT_RULE_LAWLESS },
    { 0, 0 },
};

static const config_field house_crime_text_rule_fields[] = {
    { "sentiment", house_crime_text_rules, 1, 1, 0, 100, 0 },
};

static const config_storage house_crime_text_rule_storage[] = {
    { house_crime_text_rules, sizeof(house_crime_text_rules) },
};

const config_table house_crime_text_rule_table = {
    "house_crime_text_rules.json",
    "rule",
    house_crime_text_rule_names,
    HOUSE_CRIME_TEXT_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(house_crime_text_rule_fields, house_crime_text_rule_storage),
};

static const config_name advisor_request_names[] = {
    { "none", ADVISOR_REQUEST_NONE },
    { "food", ADVISOR_REQUEST_FOOD },
    { "goods", ADVISOR_REQUEST_GOODS },
    { 0, 0 },
};

#define ADVISOR_OFFER_STRIDE ((int32_t) (sizeof(building_info_advisor_offer) / sizeof(int32_t)))
#define advisor_offer_column(field) (&building_info_advisor_offers[0].field)

static const config_field building_info_advisor_offer_fields[] = {
    { "advisors", building_info_advisor_offers[0].advisors, ADVISOR_OFFER_STRIDE, BUILDING_INFO_FIXED_ADVISORS, 0, ADVISOR_COUNT - 1, config_advisor_names },
    { "labor", advisor_offer_column(labor), ADVISOR_OFFER_STRIDE, 1, 0, 1, 0 },
    { "burial_provisions", advisor_offer_column(burial_provisions), ADVISOR_OFFER_STRIDE, 1, 0, 1, 0 },
    { "monument_material", advisor_offer_column(monument_material), ADVISOR_OFFER_STRIDE, 1, 0, RESOURCE_MAX - 1, config_resource_names },
    { "kingdom_request", advisor_offer_column(kingdom_request), ADVISOR_OFFER_STRIDE, 1, 0, ADVISOR_REQUEST_GOODS, advisor_request_names },
    { "monument_workers", advisor_offer_column(monument_workers), ADVISOR_OFFER_STRIDE, 1, 0, 1, 0 },
    { "house_evolve", advisor_offer_column(house_evolve), ADVISOR_OFFER_STRIDE, 1, 0, 1, 0 },
};

static const config_storage building_info_advisor_offer_storage[] = {
    { building_info_advisor_offers, sizeof(building_info_advisor_offers) },
};

const config_table building_info_advisor_offer_table = {
    "building_info_advisor_offers.json",
    "type",
    config_building_type_names,
    BUILDING_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(building_info_advisor_offer_fields, building_info_advisor_offer_storage),
};

static const config_name evolve_text_names[] = {
    { "EVOLVE_TEXT_DEVOLVING_DESIRABILITY", EVOLVE_TEXT_DEVOLVING_DESIRABILITY },
    { "EVOLVE_TEXT_NO_WELL", EVOLVE_TEXT_NO_WELL },
    { "EVOLVE_TEXT_NO_WATER_SUPPLY", EVOLVE_TEXT_NO_WATER_SUPPLY },
    { "EVOLVE_TEXT_NO_ENTERTAINMENT", EVOLVE_TEXT_NO_ENTERTAINMENT },
    { "EVOLVE_TEXT_ENTERTAINMENT_BELOW_10", EVOLVE_TEXT_ENTERTAINMENT_BELOW_10 },
    { "EVOLVE_TEXT_ENTERTAINMENT_BELOW_25", EVOLVE_TEXT_ENTERTAINMENT_BELOW_25 },
    { "EVOLVE_TEXT_ENTERTAINMENT_BELOW_50", EVOLVE_TEXT_ENTERTAINMENT_BELOW_50 },
    { "EVOLVE_TEXT_ENTERTAINMENT_BELOW_80", EVOLVE_TEXT_ENTERTAINMENT_BELOW_80 },
    { "EVOLVE_TEXT_ENTERTAINMENT_HIGH", EVOLVE_TEXT_ENTERTAINMENT_HIGH },
    { "EVOLVE_TEXT_FOOD_TYPES_1", EVOLVE_TEXT_FOOD_TYPES_1 },
    { "EVOLVE_TEXT_FOOD_TYPES_2", EVOLVE_TEXT_FOOD_TYPES_2 },
    { "EVOLVE_TEXT_FOOD_TYPES_3", EVOLVE_TEXT_FOOD_TYPES_3 },
    { "EVOLVE_TEXT_NO_EDUCATION", EVOLVE_TEXT_NO_EDUCATION },
    { "EVOLVE_TEXT_NO_LIBRARY", EVOLVE_TEXT_NO_LIBRARY },
    { "EVOLVE_TEXT_NO_SCHOOL", EVOLVE_TEXT_NO_SCHOOL },
    { "EVOLVE_TEXT_NO_HIGHER_EDUCATION", EVOLVE_TEXT_NO_HIGHER_EDUCATION },
    { "EVOLVE_TEXT_COURTHOUSE", EVOLVE_TEXT_COURTHOUSE },
    { "EVOLVE_TEXT_POTTERY", EVOLVE_TEXT_POTTERY },
    { "EVOLVE_TEXT_RELIGION_1", EVOLVE_TEXT_RELIGION_1 },
    { "EVOLVE_TEXT_RELIGION_2", EVOLVE_TEXT_RELIGION_2 },
    { "EVOLVE_TEXT_RELIGION_3", EVOLVE_TEXT_RELIGION_3 },
    { "EVOLVE_TEXT_DENTIST", EVOLVE_TEXT_DENTIST },
    { "EVOLVE_TEXT_HEALTH_1", EVOLVE_TEXT_HEALTH_1 },
    { "EVOLVE_TEXT_HEALTH_2_HAS_PHYSICIAN", EVOLVE_TEXT_HEALTH_2_HAS_PHYSICIAN },
    { "EVOLVE_TEXT_HEALTH_2", EVOLVE_TEXT_HEALTH_2 },
    { "EVOLVE_TEXT_LINEN", EVOLVE_TEXT_LINEN },
    { "EVOLVE_TEXT_BEER", EVOLVE_TEXT_BEER },
    { "EVOLVE_TEXT_NEXT_LEVEL", EVOLVE_TEXT_NEXT_LEVEL },
    { "EVOLVE_TEXT_NEXT_NO_WELL", EVOLVE_TEXT_NEXT_NO_WELL },
    { "EVOLVE_TEXT_NEXT_NO_WATER_SUPPLY", EVOLVE_TEXT_NEXT_NO_WATER_SUPPLY },
    { "EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT", EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT },
    { "EVOLVE_TEXT_NEXT_FOOD_TYPES_1", EVOLVE_TEXT_NEXT_FOOD_TYPES_1 },
    { "EVOLVE_TEXT_NEXT_FOOD_TYPES_2", EVOLVE_TEXT_NEXT_FOOD_TYPES_2 },
    { "EVOLVE_TEXT_NEXT_FOOD_TYPES_3", EVOLVE_TEXT_NEXT_FOOD_TYPES_3 },
    { "EVOLVE_TEXT_NEXT_NO_EDUCATION", EVOLVE_TEXT_NEXT_NO_EDUCATION },
    { "EVOLVE_TEXT_NEXT_NO_LIBRARY", EVOLVE_TEXT_NEXT_NO_LIBRARY },
    { "EVOLVE_TEXT_NEXT_NO_SCHOOL", EVOLVE_TEXT_NEXT_NO_SCHOOL },
    { "EVOLVE_TEXT_NEXT_COURTHOUSE", EVOLVE_TEXT_NEXT_COURTHOUSE },
    { "EVOLVE_TEXT_NEXT_POTTERY", EVOLVE_TEXT_NEXT_POTTERY },
    { "EVOLVE_TEXT_NEXT_RELIGION_1", EVOLVE_TEXT_NEXT_RELIGION_1 },
    { "EVOLVE_TEXT_NEXT_RELIGION_2", EVOLVE_TEXT_NEXT_RELIGION_2 },
    { "EVOLVE_TEXT_NEXT_RELIGION_3", EVOLVE_TEXT_NEXT_RELIGION_3 },
    { "EVOLVE_TEXT_NEXT_DENTIST", EVOLVE_TEXT_NEXT_DENTIST },
    { "EVOLVE_TEXT_NEXT_HEALTH_1", EVOLVE_TEXT_NEXT_HEALTH_1 },
    { "EVOLVE_TEXT_NEXT_HEALTH_2_HAS_PHYSICIAN", EVOLVE_TEXT_NEXT_HEALTH_2_HAS_PHYSICIAN },
    { "EVOLVE_TEXT_NEXT_HEALTH_2", EVOLVE_TEXT_NEXT_HEALTH_2 },
    { "EVOLVE_TEXT_NEXT_LINEN", EVOLVE_TEXT_NEXT_LINEN },
    { "EVOLVE_TEXT_NEXT_BEER", EVOLVE_TEXT_NEXT_BEER },
    { "EVOLVE_TEXT_TOP_LEVEL", EVOLVE_TEXT_TOP_LEVEL },
    { "EVOLVE_TEXT_EVOLVING", EVOLVE_TEXT_EVOLVING },
    { "EVOLVE_TEXT_NEXT_DESIRABILITY_E", EVOLVE_TEXT_NEXT_DESIRABILITY_E },
    { "EVOLVE_TEXT_EVOLVING_NO_SPACE", EVOLVE_TEXT_EVOLVING_NO_SPACE },
    { "EVOLVE_TEXT_NEXT_JEWELRY", EVOLVE_TEXT_NEXT_JEWELRY },
    { "EVOLVE_TEXT_SECOND_JEWELRY", EVOLVE_TEXT_SECOND_JEWELRY },
    { "EVOLVE_TEXT_JEWELRY", EVOLVE_TEXT_JEWELRY },
    { "EVOLVE_TEXT_NEXT_SECOND_JEWELRY", EVOLVE_TEXT_NEXT_SECOND_JEWELRY },
    { 0, 0 },
};

static const config_field house_evolve_advisor_fields[] = {
    { "advisor", house_evolve_advisors, 1, 1, 0, ADVISOR_COUNT - 1, config_advisor_names },
};

static const config_storage house_evolve_advisor_storage[] = {
    { house_evolve_advisors, sizeof(house_evolve_advisors) },
};

const config_table house_evolve_advisor_table = {
    "house_evolve_advisors.json",
    "evolve_text",
    evolve_text_names,
    EVOLVE_TEXT_COUNT,
    CONFIG_TABLE_COLUMNS(house_evolve_advisor_fields, house_evolve_advisor_storage),
};

static const config_name legion_damage_text_rule_names[] = {
    { "perfect", LEGION_DAMAGE_TEXT_RULE_PERFECT },
    { "very_good", LEGION_DAMAGE_TEXT_RULE_VERY_GOOD },
    { "good", LEGION_DAMAGE_TEXT_RULE_GOOD },
    { "average", LEGION_DAMAGE_TEXT_RULE_AVERAGE },
    { "poor", LEGION_DAMAGE_TEXT_RULE_POOR },
    { "very_poor", LEGION_DAMAGE_TEXT_RULE_VERY_POOR },
    { 0, 0 },
};

static const config_field legion_damage_text_rule_fields[] = {
    { "damage_percentage", legion_damage_text_rules, 1, 1, 0, 100, 0 },
};

static const config_storage legion_damage_text_rule_storage[] = {
    { legion_damage_text_rules, sizeof(legion_damage_text_rules) },
};

const config_table legion_damage_text_rule_table = {
    "legion_damage_text_rules.json",
    "rule",
    legion_damage_text_rule_names,
    LEGION_DAMAGE_TEXT_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(legion_damage_text_rule_fields, legion_damage_text_rule_storage),
};

static const config_name ship_hull_text_rule_names[] = {
    { "weak", SHIP_HULL_TEXT_RULE_WEAK },
    { "fair", SHIP_HULL_TEXT_RULE_FAIR },
    { "average", SHIP_HULL_TEXT_RULE_AVERAGE },
    { "good", SHIP_HULL_TEXT_RULE_GOOD },
    { "strong", SHIP_HULL_TEXT_RULE_STRONG },
    { 0, 0 },
};

#define SHIP_HULL_RULE_STRIDE ((int32_t) (sizeof(ship_hull_text_rule) / sizeof(int32_t)))

static const config_field ship_hull_text_rule_fields[] = {
    { "transport", &ship_hull_text_rules[0].transport, SHIP_HULL_RULE_STRIDE, 1, 0, 100, 0 },
    { "warship", &ship_hull_text_rules[0].warship, SHIP_HULL_RULE_STRIDE, 1, 0, 100, 0 },
};

static const config_storage ship_hull_text_rule_storage[] = {
    { ship_hull_text_rules, sizeof(ship_hull_text_rules) },
};

const config_table ship_hull_text_rule_table = {
    "ship_hull_text_rules.json",
    "rule",
    ship_hull_text_rule_names,
    SHIP_HULL_TEXT_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(ship_hull_text_rule_fields, ship_hull_text_rule_storage),
};

static const config_name pyramid_style_rule_names[] = {
    { "other", PYRAMID_STYLE_RULE_OTHER },
    { "bent", PYRAMID_STYLE_RULE_BENT },
    { "mudbrick", PYRAMID_STYLE_RULE_MUDBRICK },
    { "stepped", PYRAMID_STYLE_RULE_STEPPED },
    { "pyramid", PYRAMID_STYLE_RULE_PYRAMID },
    { "mastaba", PYRAMID_STYLE_RULE_MASTABA },
    { 0, 0 },
};

#define PYRAMID_STYLE_RULE_STRIDE ((int32_t) (sizeof(pyramid_style_rule) / sizeof(int32_t)))

static const config_field pyramid_style_rule_fields[] = {
    { "help_message", &pyramid_style_rules[0].help_message, PYRAMID_STYLE_RULE_STRIDE, 1, 0, 1000, 0 },
    { "main_material", &pyramid_style_rules[0].main_material, PYRAMID_STYLE_RULE_STRIDE, 1, 0, RESOURCE_MAX - 1, config_resource_names },
    { "casing_material", &pyramid_style_rules[0].casing_material, PYRAMID_STYLE_RULE_STRIDE, 1, -1, RESOURCE_MAX - 1, config_resource_names },
    { "stonemasons", &pyramid_style_rules[0].stonemasons, PYRAMID_STYLE_RULE_STRIDE, 1, 0, 1, 0 },
    { "bricklayers", &pyramid_style_rules[0].bricklayers, PYRAMID_STYLE_RULE_STRIDE, 1, 0, 1, 0 },
};

static const config_storage pyramid_style_rule_storage[] = {
    { pyramid_style_rules, sizeof(pyramid_style_rules) },
};

const config_table pyramid_style_rule_table = {
    "pyramid_style_rules.json",
    "style",
    pyramid_style_rule_names,
    PYRAMID_STYLE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(pyramid_style_rule_fields, pyramid_style_rule_storage),
};

static const config_name building_info_height_class_names[] = {
    { "large", BUILDING_INFO_HEIGHT_LARGE },
    { "standard", BUILDING_INFO_HEIGHT_STANDARD },
    { "tall", BUILDING_INFO_HEIGHT_TALL },
    { "bandstand", BUILDING_INFO_HEIGHT_BANDSTAND },
    { "short", BUILDING_INFO_HEIGHT_SHORT },
    { "farm", BUILDING_INFO_HEIGHT_FARM },
    { 0, 0 },
};

static const config_field building_info_window_height_fields[] = {
    { "blocks", building_info_window_heights, 1, 1, 1, 48, 0 },
};

static const config_storage building_info_window_height_storage[] = {
    { building_info_window_heights, sizeof(building_info_window_heights) },
};

const config_table building_info_window_height_table = {
    "building_info_window_heights.json",
    "height_class",
    building_info_height_class_names,
    BUILDING_INFO_HEIGHT_CLASSES,
    CONFIG_TABLE_COLUMNS(building_info_window_height_fields, building_info_window_height_storage),
};

static const config_name building_info_screen_size_names[] = {
    { "640x480", BUILDING_INFO_SCREEN_640 },
    { "800x600", BUILDING_INFO_SCREEN_800 },
    { "1024x768", BUILDING_INFO_SCREEN_1024 },
    { 0, 0 },
};

#define WINDOW_PLACEMENT_STRIDE ((int32_t) (sizeof(building_info_window_placement) / sizeof(int32_t)))

static const config_field building_info_window_placement_fields[] = {
    { "display_height", &building_info_window_placements[0].display_height, WINDOW_PLACEMENT_STRIDE, 1, 0, 4096, 0 },
    { "x", &building_info_window_placements[0].x, WINDOW_PLACEMENT_STRIDE, 1, 0, 4096, 0 },
    { "x_collapsed", &building_info_window_placements[0].x_collapsed, WINDOW_PLACEMENT_STRIDE, 1, 0, 4096, 0 },
    { "low_mouse_y", &building_info_window_placements[0].low_mouse_y, WINDOW_PLACEMENT_STRIDE, 1, 0, 4096, 0 },
    { "bottom_y", &building_info_window_placements[0].bottom_y, WINDOW_PLACEMENT_STRIDE, 1, 0, 4096, 0 },
};

static const config_storage building_info_window_placement_storage[] = {
    { building_info_window_placements, sizeof(building_info_window_placements) },
};

const config_table building_info_window_placement_table = {
    "building_info_window_placements.json",
    "screen",
    building_info_screen_size_names,
    BUILDING_INFO_SCREEN_SIZES,
    CONFIG_TABLE_COLUMNS(building_info_window_placement_fields, building_info_window_placement_storage),
};
