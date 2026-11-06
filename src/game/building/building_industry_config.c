// The gameplay config tables of the industries (core/config.h): the harvest months of each crop, the production
// of each building type and the industry rules.

#include "building/building_data.h"
#include "building/building_industry.h"
#include "city/resource_type.h"
#include "core/config.h"

static const config_field farm_harvest_month_fields[] = {
    { "months", farm_harvest_months, FARM_HARVESTS_PER_CROP, FARM_HARVESTS_PER_CROP, -1, 11, 0 },
};

static const config_storage farm_harvest_month_storage[] = {
    { farm_harvest_months, sizeof(int32_t) * (FARM_HARVESTS_PER_CROP * FARM_CROP_ROWS) },
};

const config_table farm_harvest_month_table = {
    "farm_harvest_months.json",
    "crop",
    0,
    FARM_CROP_ROWS,
    CONFIG_TABLE_COLUMNS(farm_harvest_month_fields, farm_harvest_month_storage),
};

static const config_name industry_input_names[] = {
    { "INPUT_NONE", INDUSTRY_INPUT_NONE },
    { "INPUT_FLAX", INDUSTRY_INPUT_FLAX },
    { "INPUT_UNUSED_2", INDUSTRY_INPUT_UNUSED_2 },
    { "INPUT_STONE", INDUSTRY_INPUT_STONE },
    { "INPUT_TIMBER", INDUSTRY_INPUT_TIMBER },
    { "INPUT_CLAY", INDUSTRY_INPUT_CLAY },
    { "INPUT_LIMESTONE", INDUSTRY_INPUT_LIMESTONE },
    { "INPUT_BRICKS", INDUSTRY_INPUT_BRICKS },
    { "INPUT_WEAPONS", INDUSTRY_INPUT_WEAPONS },
    { "INPUT_BARLEY", INDUSTRY_INPUT_BARLEY },
    { "INPUT_GEMS", INDUSTRY_INPUT_GEMS },
    { "INPUT_REEDS", INDUSTRY_INPUT_REEDS },
    { "INPUT_PAPYRUS", INDUSTRY_INPUT_PAPYRUS },
    { "INPUT_STRAW", INDUSTRY_INPUT_STRAW },
    { "INPUT_CHARIOTS", INDUSTRY_INPUT_CHARIOTS },
    { "INPUT_GOLD", INDUSTRY_INPUT_GOLD },
    { "INPUT_LINEN", INDUSTRY_INPUT_LINEN },
    { "INPUT_COPPER", INDUSTRY_INPUT_COPPER },
    { "INPUT_SANDSTONE", INDUSTRY_INPUT_SANDSTONE },
    { "INPUT_BEER", INDUSTRY_INPUT_BEER },
    { "INPUT_HENNA", INDUSTRY_INPUT_HENNA },
    { "INPUT_MEAT", INDUSTRY_INPUT_MEAT },
    { "INPUT_GAMEMEAT", INDUSTRY_INPUT_GAMEMEAT },
    { "INPUT_OIL", INDUSTRY_INPUT_OIL },
    { "INPUT_POTTERY", INDUSTRY_INPUT_POTTERY },
    { "INPUT_PAINT", INDUSTRY_INPUT_PAINT },
    { "INPUT_LAMPS", INDUSTRY_INPUT_LAMPS },
    { 0, 0 },
};

enum building_production_layout {
    BUILDING_PRODUCTION_STRIDE = sizeof(building_production) / sizeof(int32_t),
};

static const config_field building_production_fields[] = {
    { "output", &building_productions[0].output, BUILDING_PRODUCTION_STRIDE, 1, 0, RESOURCE_MAX - 1, config_resource_names },
    { "input", &building_productions[0].input, BUILDING_PRODUCTION_STRIDE, 1, 0, INDUSTRY_INPUT_COUNT - 1, industry_input_names },
    { "second_input", &building_productions[0].second_input, BUILDING_PRODUCTION_STRIDE, 1, 0, INDUSTRY_INPUT_COUNT - 1, industry_input_names },
    { "counted_as", &building_productions[0].counted_as, BUILDING_PRODUCTION_STRIDE, 1, 0, RESOURCE_MAX - 1, config_resource_names },
    { "takes_input", &building_productions[0].takes_input, BUILDING_PRODUCTION_STRIDE, 1, 0, 1, 0 },
    { "input_stock_max", &building_productions[0].input_stock_max, BUILDING_PRODUCTION_STRIDE, 1, 0, 1000000, 0 },
};

static const config_storage building_production_storage[] = {
    { building_productions, sizeof(building_productions) },
};

const config_table building_production_table = {
    "building_production.json",
    "type",
    config_building_type_names,
    BUILDING_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(building_production_fields, building_production_storage),
};

static const config_name industry_rule_names[] = {
    { "progress_done", INDUSTRY_RULE_PROGRESS_DONE },
    { "input_load", INDUSTRY_RULE_INPUT_LOAD },
    { "brickworks_straw_load", INDUSTRY_RULE_BRICKWORKS_STRAW_LOAD },
    { "input_stock_default", INDUSTRY_RULE_INPUT_STOCK_DEFAULT },
    { 0, 0 },
};

static const config_field industry_rule_fields[] = {
    { "value", industry_rules, 1, 1, 1, 10000, 0 },
};

static const config_storage industry_rule_storage[] = {
    { industry_rules, sizeof(industry_rules) },
};

const config_table industry_rule_table = {
    "industry_rules.json",
    "rule",
    industry_rule_names,
    INDUSTRY_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(industry_rule_fields, industry_rule_storage),
};
