// Creating a building record: a free run of records no undo step holds, or a requested id, filled with what
// placement and the first ticks read (creation sequence, house size and level, orientation, production, map
// position, ferry landing link, cleared shows). No free record shows the data limit warning.
//
// A free run is not bounded by the end of the array, so near a full city a large monument's parts can run past the last
// record.

#include "building/building_create.h"
#include "building/building.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "building/ferry_landing.h"
#include "building/house_evolve.h"
#include "city/city_player.h"
#include "city/health.h"
#include "city/message.h"
#include "figure/ships_player.h"
#include "game/undo.h"
#include "map/grid.h"
#include "map/water.h"
#include <stdint.h>

enum building_create_value {
    HOUSE_START_SENTIMENT = 50,
    TILE_SCATTER_MASK = 0x7F,
    ROAM_DIRECTION_MASK = 6,
    STATUE_VARIANTS = 4,
    STATUE_ROTATIONS = 4,
};

static void set_production(building *b, int32_t type, int32_t building_id)
{
    const building_production *production = &building_productions[type];
    b->output_resource = (uint8_t) production->output;
    if (production->input != INDUSTRY_INPUT_NONE) {
        b->subtype = (int16_t) production->input;
    }
    b->second_input_resource = (int16_t) production->second_input;
    if (type == BUILDING_SHIPWRIGHT) {
        building_repair_queue_add(building_id);
    }
}

static void set_statue_orientation(building *b)
{
    int32_t variant = construction_orientation % STATUE_VARIANTS;
    int32_t rotation = (construction_orientation + STATUE_ROTATIONS) / STATUE_ROTATIONS;
    if (rotation >= STATUE_ROTATIONS) {
        rotation = 0;
    }
    int32_t view_rotation = (map_info.orientation / 2) + rotation + 1;
    if (view_rotation >= STATUE_ROTATIONS) {
        view_rotation -= STATUE_ROTATIONS;
    }
    b->data.oriented.orientation = (uint8_t) (((uint8_t) variant << 4) + (uint8_t) view_rotation);
}

static int32_t run_is_free(int32_t first_id, int32_t count)
{
    int32_t end = first_id + count;
    for (int32_t id = first_id + 1; id < end; id++) {
        if (city_buildings.records[id].state != BUILDING_STATE_UNUSED) {
            return 0;
        }
    }
    for (int32_t id = first_id + 1; id < end; id++) {
        if (game_undo_contains_building(id) != 0) {
            return 0;
        }
    }
    return 1;
}

static int32_t find_free_run(int32_t count)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        if (city_buildings.records[id].state == BUILDING_STATE_UNUSED && game_undo_contains_building(id) == 0 && (count <= 1 || run_is_free(id, count))) {
            return id;
        }
    }
    return 0;
}

static int32_t house_size_of(int32_t type)
{
    if (type >= BUILDING_HOUSE_CRUDE_HUT && type <= BUILDING_HOUSE_SPACIOUS_APARTMENT) {
        return 1;
    }
    if (type >= BUILDING_HOUSE_COMMON_RESIDENCE && type <= BUILDING_HOUSE_FANCY_RESIDENCE) {
        return 2;
    }
    if (type >= BUILDING_HOUSE_COMMON_MANOR && type <= BUILDING_HOUSE_STATELY_MANOR) {
        return 3;
    }
    if (type >= BUILDING_HOUSE_MODEST_ESTATE && type <= BUILDING_HOUSE_PALATIAL_ESTATE) {
        return 4;
    }
    return 0;
}

static int32_t is_venue(int32_t type)
{
    return type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILION;
}

static int32_t counts_workers(int32_t type, int32_t building_id)
{
    switch (type) {
        case BUILDING_HUNTING_LODGE:
        case BUILDING_WOOD_CUTTER:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_STONEMASONS_GUILD:
        case BUILDING_ARTISANS_GUILD:
        case BUILDING_REED_GATHERER:
            return 1;
        default:
            return building_is_floodplain_farm(building_id) || type == BUILDING_WORK_CAMP;
    }
}

static void link_ferry_landing(building *b, int32_t building_id)
{
    if (construction_ferry_state == FERRY_PLACEMENT_NONE) {
        construction_ferry_first = building_id;
    } else if (construction_ferry_state == FERRY_PLACEMENT_FIRST_PLACED || construction_ferry_state == FERRY_PLACEMENT_FIRST_EXISTING) {
        int32_t first_id = construction_ferry_first;
        b->linked_building_id = first_id;
        building_get(first_id)->linked_building_id = building_id;
    }
}

static void clear_shows(building *b)
{
    b->data.entertainment.juggler_id = 0;
    b->data.entertainment.musician_id = 0;
    b->data.entertainment.dancer_id = 0;
    b->data.entertainment.reserved_2 = 0;
    b->data.entertainment.show_days_left[0] = 0;
    b->data.entertainment.show_days_left[1] = 0;
    b->data.entertainment.show_days_left[2] = 0;
}

int32_t building_create(int32_t player, int32_t type, int32_t x, int32_t y, int32_t count, int32_t reuse_id)
{
    int32_t id;
    if (reuse_id != 0) {
        if (building_get(reuse_id)->state != BUILDING_STATE_UNUSED || game_undo_contains_building(reuse_id) != 0) {
            city_warning_show(WARNING_DATA_LIMIT_REACHED);
            return 0;
        }
        id = reuse_id;
    } else {
        id = find_free_run(count);
        if (id == 0) {
            city_warning_show(WARNING_DATA_LIMIT_REACHED);
            return 0;
        }
    }
    building *b = building_get(id);
    const building_property *property = building_property_get(type);
    b->state = BUILDING_STATE_CREATED;
    b->player = (uint8_t) player;
    b->reserved_id = city_player_get(player)->building_reserved_id;
    b->size = (uint8_t) property->size;
    b->type = (int16_t) type;
    b->created_sequence = (uint16_t) building_extra_sequence;
    b->sentiment.house = HOUSE_START_SENTIMENT;
    b->distance_from_entry = 0;
    building_extra_sequence++;
    int16_t stored = (int16_t) type;
    b->house_size = (uint8_t) house_size_of(stored);
    if (stored >= BUILDING_HOUSE_CRUDE_HUT && stored <= BUILDING_HOUSE_PALATIAL_ESTATE) {
        b->subtype = (int16_t) (type - BUILDING_HOUSE_CRUDE_HUT);
    } else {
        b->subtype = 0;
    }
    if (b->house_size != 0) {
        b->crime_risk = house_models[b->subtype].crime_base;
        b->house_malaria_risk = 0;
        b->house_plague_risk = 0;
        city_health_update_house(id, 1);
    } else {
        b->crime_risk = 0;
    }
    if ((stored >= BUILDING_SHRINE_OSIRIS && stored <= BUILDING_SHRINE_GOD_9) || stored == BUILDING_GATEHOUSE_UP) {
        b->data.oriented.orientation = (uint8_t) construction_orientation;
    } else if (stored >= BUILDING_SMALL_STATUE && stored <= BUILDING_LARGE_STATUE) {
        set_statue_orientation(b);
    }
    set_production(b, stored, id);
    if (stored == BUILDING_GRANARY || stored == BUILDING_GRANARY_UP) {
        b->data.granary.resource_stored[RESOURCE_NONE] = GRANARY_CAPACITY;
    }
    b->x = (int16_t) x;
    b->y = (int16_t) y;
    b->grid_offset = map_grid_offset(x, y);
    uint8_t scatter = map_grids.random[b->grid_offset] & TILE_SCATTER_MASK;
    b->house_figure_generation_delay = scatter;
    b->figure_roam_direction = (uint8_t) (scatter & ROAM_DIRECTION_MASK);
    b->fire_proof = (uint8_t) property->fire_proof;
    b->is_adjacent_to_water = (uint8_t) map_terrain_is_adjacent_to_water(x, y, b->size);
    if (stored == BUILDING_FERRY_LANDING) {
        link_ferry_landing(b, id);
    }
    if (is_venue(stored)) {
        clear_shows(b);
    }
    if (counts_workers(stored, id)) {
        b->data.industry.worker_count = 0;
    }
    if (building_highest_id <= id) {
        building_highest_id = id;
    }
    return id;
}
