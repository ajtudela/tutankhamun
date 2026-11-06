// The gameplay config tables of the scenario editor (core/config.h): burial provision goods and the monuments that
// need them, the goods offered in the empire editor's goods slot, the terrain brush tiles, the new map sizes and the
// empire map regions.

#include "city/resource_type.h"
#include "core/config.h"
#include "editor/editor_data.h"
#include "editor/editor_monuments.h"
#include "editor/editor_new_scenario.h"
#include "editor/empire_editor_object_panel.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "window/window_data.h"

static const config_field burial_provision_fields[] = {
    { "resource", burial_provision_resources, 1, 1, RESOURCE_MIN, RESOURCE_MAX - 1, config_resource_names },
};

static const config_storage burial_provision_storage[] = {
    { burial_provision_resources, sizeof(burial_provision_resources) },
};

const config_table burial_provision_table = {
    "burial_provision_resources.json",
    "row",
    0,
    BURIAL_PROVISION_ROWS,
    CONFIG_TABLE_COLUMNS(burial_provision_fields, burial_provision_storage),
};

static const config_field monument_burial_provision_fields[] = {
    { "burial_provisions", monument_burial_provisions, 1, 1, 0, 1, 0 },
};

static const config_storage monument_burial_provision_storage[] = {
    { monument_burial_provisions, sizeof(monument_burial_provisions) },
};

const config_table monument_burial_provision_table = {
    "monument_burial_provisions.json",
    "monument",
    config_monument_def_names,
    MONUMENT_DEFINITIONS,
    CONFIG_TABLE_COLUMNS(monument_burial_provision_fields, monument_burial_provision_storage),
};

static const config_field goods_slot_offered_fields[] = {
    { "offered", empire_editor_goods_slot_offered, 1, 1, 0, 1, 0 },
};

static const config_storage goods_slot_offered_storage[] = {
    { empire_editor_goods_slot_offered, sizeof(empire_editor_goods_slot_offered) },
};

const config_table empire_editor_goods_slot_table = {
    "empire_editor_goods_slot_goods.json",
    "resource",
    config_resource_names,
    RESOURCE_MAX,
    CONFIG_TABLE_COLUMNS(goods_slot_offered_fields, goods_slot_offered_storage),
};

enum editor_brush_table {
    EDITOR_BRUSH_TILES = 41,
    EDITOR_BRUSH_TILE_STRIDE = 2,
    EDITOR_BRUSH_REACH = 4,
};

static const config_field editor_brush_tile_fields[] = {
    { "dx", &editor_brush_tiles[0], EDITOR_BRUSH_TILE_STRIDE, 1, -EDITOR_BRUSH_REACH, EDITOR_BRUSH_REACH, 0 },
    { "dy", &editor_brush_tiles[1], EDITOR_BRUSH_TILE_STRIDE, 1, -EDITOR_BRUSH_REACH, EDITOR_BRUSH_REACH, 0 },
};

static const config_storage editor_brush_tile_storage[] = {
    { editor_brush_tiles, sizeof(editor_brush_tiles) },
};

const config_table editor_brush_tile_table = {
    "editor_brush_tiles.json",
    "tile",
    0,
    EDITOR_BRUSH_TILES,
    CONFIG_TABLE_COLUMNS(editor_brush_tile_fields, editor_brush_tile_storage),
};

#define NEW_MAP_SIZE_STRIDE ((int32_t) (sizeof(new_map_size) / sizeof(int32_t)))
#define REGION_INFO_STRIDE ((int32_t) (sizeof(pharaohmap_info) / sizeof(int32_t)))
#define REGION_POSITION_STRIDE ((int32_t) (sizeof(empire_map_region_position) / sizeof(int32_t)))

enum editor_config_limits {
    GRID_OFFSET_MAX = GRID_SIZE * GRID_SIZE,
    PHARAOHMAPS_PIXELS_MAX = 1000,
};

static const config_field new_map_size_fields[] = {
    { "width", &new_map_sizes[0].width, NEW_MAP_SIZE_STRIDE, 1, 0, GRID_SIZE, 0 },
    { "height", &new_map_sizes[0].height, NEW_MAP_SIZE_STRIDE, 1, 0, GRID_SIZE, 0 },
    { "grid_start", &new_map_sizes[0].grid_start, NEW_MAP_SIZE_STRIDE, 1, 0, GRID_OFFSET_MAX, 0 },
    { "grid_border_size", &new_map_sizes[0].grid_border_size, NEW_MAP_SIZE_STRIDE, 1, 0, GRID_SIZE, 0 },
};

static const config_storage new_map_size_storage[] = {
    { &new_map_sizes[0], NEW_MAP_SIZES * sizeof(new_map_size) },
};

const config_table new_map_size_table = {
    "new_map_sizes.json",
    "size",
    0,
    NEW_MAP_SIZES,
    CONFIG_TABLE_COLUMNS(new_map_size_fields, new_map_size_storage),
};

static const config_field empire_map_region_fields[] = {
    { "map_id", &empire_map_region_infos[0].map_id, REGION_INFO_STRIDE, 1, 0, EMPIRE_MAP_REGIONS - 1, 0 },
    { "grid_width", &empire_map_region_infos[0].grid_width, REGION_INFO_STRIDE, 1, 0, GRID_SIZE, 0 },
    { "grid_height", &empire_map_region_infos[0].grid_height, REGION_INFO_STRIDE, 1, 0, GRID_SIZE, 0 },
    { "grid_start", &empire_map_region_infos[0].grid_start, REGION_INFO_STRIDE, 1, 0, GRID_OFFSET_MAX, 0 },
    { "grid_border_size", &empire_map_region_infos[0].grid_border_size, REGION_INFO_STRIDE, 1, 0, GRID_SIZE, 0 },
    { "picture_x", &empire_map_region_positions[0].x, REGION_POSITION_STRIDE, 1, 0, PHARAOHMAPS_PIXELS_MAX, 0 },
    { "picture_y", &empire_map_region_positions[0].y, REGION_POSITION_STRIDE, 1, 0, PHARAOHMAPS_PIXELS_MAX, 0 },
};

static const config_storage empire_map_region_storage[] = {
    { &empire_map_region_infos[0], EMPIRE_MAP_REGIONS * sizeof(pharaohmap_info) },
    { &empire_map_region_positions[0], EMPIRE_MAP_REGIONS * sizeof(empire_map_region_position) },
};

const config_table empire_map_region_table = {
    "empire_map_regions.json",
    "region",
    0,
    EMPIRE_MAP_REGIONS,
    CONFIG_TABLE_COLUMNS(empire_map_region_fields, empire_map_region_storage),
};
