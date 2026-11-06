// Placing buildings with the build tool (building/construction_place.c) and what the construction files share:
// the building model and property records by type, the drag tools, the placement rules (credit limit, most forts,
// wharves and docks, the monuments' materials) and the site sizes of the checks.

#ifndef BUILDING_CONSTRUCTION_PLACE_H
#define BUILDING_CONSTRUCTION_PLACE_H

#include "building/building.h"
#include "building/building_data.h"
#include "city/city_player_types.h"
#include "city/message.h"
#include "map/grid.h"
#include "map/terrain.h"
#include <stdbool.h>
#include <stdint.h>

enum construction_place_rule {
    CONSTRUCTION_PLACE_RULE_OUT_OF_CREDIT_TREASURY,
    CONSTRUCTION_PLACE_RULE_MAX_FORTS,
    CONSTRUCTION_PLACE_RULE_MAX_WARSHIP_WHARVES,
    CONSTRUCTION_PLACE_RULE_MAX_DOCKS,
    CONSTRUCTION_PLACE_RULE_LIBRARY_PAPYRUS,
    CONSTRUCTION_PLACE_RULE_SMALL_OBELISK_GRANITE,
    CONSTRUCTION_PLACE_RULE_LARGE_OBELISK_GRANITE,
    CONSTRUCTION_PLACE_RULE_SUN_TEMPLE_SANDSTONE,
    CONSTRUCTION_PLACE_RULE_MAUSOLEUM_SANDSTONE,
    CONSTRUCTION_PLACE_RULE_COUNT,
};

enum construction_site_size {
    FARM_SITE_SIZE = 3,
    FARM_MEADOW_RADIUS = 1,
    RAW_MATERIAL_SITE_SIZE = 2,
    RAW_MATERIAL_RADIUS = 1,
    CLAY_PIT_WATER_RADIUS = 3,
    FORT_SIZE = 3,
    TRIUMPHAL_ARCH_SIZE = 3,
    GATEHOUSE_MARK_SIZE = 2,
    SMALL_OBELISK_SIZE = 3,
    LARGE_OBELISK_SIZE = 5,
    BOOTH_SITE_SIZE = 2,
    BANDSTAND_SITE_SIZE = 3,
    PAVILION_SITE_SIZE = 4,
    FESTIVAL_SQUARE_SITE_SIZE = 5,
};

enum construction_wall_site {
    TERRAIN_BLOCKS_WALL = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_CANAL |
        TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_FLOODPLAIN | TERRAIN_MARSH | TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_DUNE |
        TERRAIN_DEEPWATER | TERRAIN_ROCK_NEAR_MONUMENT,
};

enum construction_temple_complex_state {
    TEMPLE_COMPLEX_UPGRADES_PLACED = 2,
};

extern int32_t construction_place_rules[CONSTRUCTION_PLACE_RULE_COUNT];

typedef struct {
    int32_t size;
    int32_t fire_proof;
    int32_t image_group;
    int32_t image_offset;
    int32_t labor_category;
} building_property;

static inline const building_model_record *building_model_get(int32_t type)
{
    return &building_model_table[type];
}

static inline const building_property *building_property_get(int32_t type)
{
    return &((const building_property *) building_properties)[type];
}

bool construction_allowed_on_floodplain(int32_t type);
void building_construction_update(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type);
void building_construction_place(int32_t player, int32_t orientation, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type);
void building_construction_clear_land_tool(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
void building_construction_place_houses(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
void building_construction_place_wall(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, uint32_t terrain);
void building_construction_place_plaza(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
void building_construction_place_garden(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end);
bool construction_area_has_entry_point(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
uint32_t construction_site_mask(int32_t type);

#endif
