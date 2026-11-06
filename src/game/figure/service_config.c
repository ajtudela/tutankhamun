// The gameplay config table of the service walkers (core/config.h): service_rules.json, the coverage radii, access
// values, risk limits and market amounts.

#include "core/config.h"
#include "figure/service.h"

static const config_name service_rule_names[] = {
    { "radius", SERVICE_RULE_RADIUS },
    { "missionary_radius", SERVICE_RULE_MISSIONARY_RADIUS },
    { "house_access", SERVICE_RULE_HOUSE_ACCESS },
    { "house_tax_access", SERVICE_RULE_HOUSE_TAX_ACCESS },
    { "risk_seen_start", SERVICE_RULE_RISK_SEEN_START },
    { "crime_risk_max", SERVICE_RULE_CRIME_RISK_MAX },
    { "houses_covered_max", SERVICE_RULE_HOUSES_COVERED_MAX },
    { "seen_fade", SERVICE_RULE_SEEN_FADE },
    { "market_demand", SERVICE_RULE_MARKET_DEMAND },
    { "food_per_person", SERVICE_RULE_FOOD_PER_PERSON },
    { "food_stock_per_person", SERVICE_RULE_FOOD_STOCK_PER_PERSON },
    { "large_house_goods_factor", SERVICE_RULE_LARGE_HOUSE_GOODS_FACTOR },
    { "goods_stock_factor", SERVICE_RULE_GOODS_STOCK_FACTOR },
    { 0, 0 },
};

static const config_field service_rule_fields[] = {
    { "value", service_rules, 1, 1, 0, 1000, 0 },
};

static const config_storage service_rule_storage[] = {
    { service_rules, sizeof(service_rules) },
};

const config_table service_rule_table = {
    "service_rules.json",
    "rule",
    service_rule_names,
    SERVICE_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(service_rule_fields, service_rule_storage),
};
