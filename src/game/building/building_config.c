// The gameplay config table of the building properties (core/config.h): per building type its size, whether it
// is fire proof, its image group and offset and its labor category.

#include "building/building_data.h"
#include "building/building_type.h"
#include "city/labor.h"
#include "core/config.h"

#define BUILDING_PROPERTY_VALUES 5

#define building_property_column(index) (building_properties + (index))

static const config_field building_property_fields[] = {
    { "size", building_property_column(0), BUILDING_PROPERTY_VALUES, 1, 0, 10, 0 },
    { "fire_proof", building_property_column(1), BUILDING_PROPERTY_VALUES, 1, 0, 1, 0 },
    { "image_group", building_property_column(2), BUILDING_PROPERTY_VALUES, 1, 0, 10000, 0 },
    { "image_offset", building_property_column(3), BUILDING_PROPERTY_VALUES, 1, -1, 10000, 0 },
    { "labor_category", building_property_column(4), BUILDING_PROPERTY_VALUES, 1, -1, LABOR_CATEGORY_COUNT - 1, 0 },
};

static const config_storage building_property_storage[] = {
    { building_properties, sizeof(int32_t) * BUILDING_PROPERTY_VALUES *BUILDING_TYPE_COUNT },
};

const config_table building_property_table = {
    "building_properties.json",
    "type",
    config_building_type_names,
    BUILDING_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(building_property_fields, building_property_storage),
};
