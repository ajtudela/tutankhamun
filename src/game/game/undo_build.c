// Undo of the last construction, the build side: backing up the grids when a drag starts, listing every building a
// placement created or cleared with a copy of its record, and the undo itself, which refunds the cost and puts the
// cleared buildings back, restores the grids of the drag tools or removes the created buildings.

#include "game/undo_build.h"

#include "building/building.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "building/construction_venue.h"
#include "building/destroy_paths.h"
#include "building/ferry_landing.h"
#include "building/house_size.h"
#include "building/monument.h"
#include "building/monument_instant.h"
#include "building/shipwright.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "figure/boats.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/ships_player.h"
#include "game/game_data.h"
#include "game/mission_records.h"
#include "game/undo.h"
#include "map/building_tiles.h"
#include "map/construction_terrain.h"
#include "map/farm.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/tiles.h"
#include "widget/widget_data.h"

#include <stdint.h>
#include <string.h>

enum undo_build_rule {
    UNDO_BUILD_READY_TICKS = 500,
    UNDO_BUILD_FORMATIONS = 50,
    UNDO_BUILD_TEMPLE_COMPLEX_UPGRADES = 2,
    UNDO_BUILD_BITFIELDS_KEEP_MASK = 0xAF,
    UNDO_BUILD_FERRY_PARTNER_NONE = -1,
};

static const int32_t undo_grid_bytes = GRID_CELLS * (int32_t) sizeof(uint32_t);

static void undo_clear_list(void)
{
    memset(undo_buildings, 0, UNDO_MAX_BUILDINGS * sizeof(building));
    memset(undo_building_ids, 0, UNDO_MAX_BUILDINGS * sizeof(undo_building_ids[0]));
}

int32_t game_undo_backup_map(void)
{
    for (int32_t i = 0; i < RESOURCE_MAX; i++) {
        undo_resources[i] = 0;
    }
    undo_ready = 0;
    undo_available = 1;
    undo_timeout_ticks = 0;
    undo_cost = 0;
    undo_num_buildings = 0;
    undo_mode = construction_building_type;
    undo_clear_list();
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        uint8_t state = building_get(id)->state;
        if (state == BUILDING_STATE_UNDO) {
            undo_available = 0;
            return 0;
        }
        if (state == BUILDING_STATE_DELETED) {
            undo_available = 0;
        }
    }
    memcpy(map_grids.images_backup, map_grids.images, undo_grid_bytes);
    memcpy(map_terrain_backup, map_grids.terrain, undo_grid_bytes);
    memcpy(map_canals_backup, map_grids.canals, GRID_CELLS);
    memcpy(map_bitfields_backup, map_grids.bitfields, GRID_CELLS);
    memcpy(map_edge_backup, map_grids.edge, GRID_CELLS);
    memcpy(map_sprite_backup, map_grids.sprite, GRID_CELLS);
    return 1;
}

void game_undo_start_build(void)
{
    for (int16_t *ids = undo_building_ids; undo_num_buildings > 0; ids++, undo_num_buildings--) {
        if (*ids != 0) {
            building *b = building_get(*ids);
            if (b->state == BUILDING_STATE_DELETED) {
                b->state = BUILDING_STATE_IN_USE;
            }
            b->is_deleted = 0;
        }
    }
    undo_num_buildings = 0;
    undo_clear_list();
}

int32_t game_undo_add_building(int32_t building_id)
{
    if (building_id <= 0) {
        return 0;
    }
    int32_t found = 0;
    undo_num_buildings = 0;
    for (int32_t i = 0; i < UNDO_MAX_BUILDINGS; i++) {
        if (undo_building_ids[i] != 0) {
            undo_num_buildings++;
        }
        if (undo_building_ids[i] == building_id) {
            found = 1;
        }
    }
    if (found) {
        return 0;
    }
    for (int32_t i = 0; i < UNDO_MAX_BUILDINGS; i++) {
        if (undo_building_ids[i] == 0) {
            undo_num_buildings++;
            memcpy(&undo_buildings[i], building_get(building_id), sizeof(building));
            undo_building_ids[i] = (int16_t) building_id;
            return 1;
        }
    }
    undo_available = 0;
    return 0;
}

void game_undo_finish_build(void)
{
    undo_ready = 1;
    undo_timeout_ticks = UNDO_BUILD_READY_TICKS;
    undo_cost = construction_place_cost;
    window_redraw_request = 1;
}

void map_images_restore_free_tiles(void)
{
    int32_t height = map_info.grid_height;
    for (int32_t y = 0; y < height; y++) {
        int32_t x = map_row_x_start[y];
        int32_t end = map_row_x_end[y];
        if (x >= end) {
            continue;
        }
        int32_t offset = map_row_start_offset[y];
        for (int32_t n = end - x; n != 0; n--) {
            if (map_grids.building_ids[offset] == 0) {
                map_grids.images[offset] = map_grids.images_backup[offset];
            }
            offset++;
        }
    }
}


static void restore_legion_record(int32_t formation_id, int32_t kind)
{
    const mission_legion_record *finished = &mission_finished_records()->legions[kind];
    if (formation_get(formation_id)->legion_id == finished->legion_id) {
        mission_started_records()->legions[kind] = *finished;
    }
}

static void undo_fort(int32_t building_id, const building *b)
{
    for (int32_t i = 1; i < UNDO_BUILD_FORMATIONS; i++) {
        const formation *m = formation_get(i);
        if (m->in_use == 0 || m->is_legion != 1 || m->building_id != building_id) {
            continue;
        }
        switch (b->subtype) {
            case FIGURE_CHARIOTEER:
                restore_legion_record(i, MISSION_LEGION_CHARIOTEERS);
                break;
            case FIGURE_INFANTRY:
                restore_legion_record(i, MISSION_LEGION_INFANTRY);
                break;
            case FIGURE_ARCHER:
                restore_legion_record(i, MISSION_LEGION_ARCHERS);
                break;
        }
        return;
    }
}

static void restore_storage(int32_t id, building *b)
{
    int32_t storage = b->storage_id;
    if (storage == 0 || storage_get(storage)->in_use != 0) {
        b->storage_id = (uint8_t) building_storage_create(id);
        return;
    }
    storage_get(storage)->in_use = 1;
    if (building_storage_highest_id <= storage) {
        building_storage_highest_id = storage;
    }
}

static void restore_ferry_queue(int32_t id, building *b)
{
    int32_t queue = b->queue_index;
    if (queue == 0 || ferry_queue_get(queue)->queue.in_use != 0) {
        b->queue_index = (uint8_t) building_ferry_record_create(id);
    } else {
        ferry_queue_get(queue)->queue.in_use = 1;
    }
}

static void restore_repair_slot(int32_t id, const building *b)
{
    int32_t slot = b->queue_slot;
    if (slot == 0 || dock_queue_get(slot - 1)->queue.flag != 0) {
        building_repair_queue_add(id);
    } else {
        dock_queue_get(slot - 1)->queue.flag = 1;
    }
}

static void restore_on_map(int32_t id, const building *b)
{
    int32_t size = building_property_get(b->type)->size;
    if (building_is_farm(id)) {
        map_building_place_farm(id, 0);
    } else if ((b->type < BUILDING_TEMPLE_COMPLEX_OSIRIS || b->type > BUILDING_TEMPLE_COMPLEX_BAST) && b->type != BUILDING_GATEHOUSE_UP) {
        map_building_tiles_add(id, b->x, b->y, size, size, 0, 0);
    } else {
        building_construction_add_to_map(city_current_player, b->type, id, b->x, b->y, size, 0, 0);
    }
}

static int32_t restore_city_reference(int32_t id, building *b)
{
    city_player *player = city_player_current();
    switch (b->type) {
        case BUILDING_DISTRIBUTION_CENTER:
            b->state = BUILDING_STATE_IN_USE;
            player->distribution_center_grid_offset = b->grid_offset;
            player->distribution_center_x = b->x;
            player->distribution_center_y = b->y;
            player->distribution_center_placed = 1;
            return 1;
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
            b->state = BUILDING_STATE_IN_USE;
            player->mansion_placed = 1;
            return 1;
        case BUILDING_MILITARY_ACADEMY:
            b->state = BUILDING_STATE_IN_USE;
            player->military_academy_placed = 1;
            return 1;
        case BUILDING_RECRUITER:
            b->state = BUILDING_STATE_IN_USE;
            player->recruiter_grid_offset = b->grid_offset;
            player->recruiter_x = b->x;
            player->recruiter_y = b->y;
            player->recruiter_placed = 1;
            return 1;
        case BUILDING_FERRY_LANDING:
            if (b->linked_building_id > 0) {
                building_get(b->linked_building_id)->linked_building_id = id;
            }
            b->state = BUILDING_STATE_IN_USE;
            return 1;
        case BUILDING_WARSHIP_WHARF:
            b->state = BUILDING_STATE_IN_USE;
            building_warship_wharf_count++;
            return 1;
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            b->state = BUILDING_STATE_IN_USE;
            player->palace_grid_offset = b->grid_offset;
            player->palace_x = b->x;
            player->palace_y = b->y;
            player->palace_placed = 1;
            return 1;
        case BUILDING_FESTIVAL_SQUARE:
            b->state = BUILDING_STATE_IN_USE;
            player->festival_square_x = b->x;
            player->festival_square_y = b->y;
            player->festival_square_placed = 1;
            return 1;
        case BUILDING_SPHINX:
            player->sphinx_stage = (int16_t) player->sphinx_stage_before_delete;
            monument_register(id);
            b->state = BUILDING_STATE_IN_USE;
            return 1;
        case BUILDING_PYRAMID:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
            monument_register(id);
            b->state = BUILDING_STATE_IN_USE;
            return 1;
        default:
            break;
    }
    if (b->type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && b->type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        player->has_temple_complex = 1;
        player->temple_complex_upgrades = UNDO_BUILD_TEMPLE_COMPLEX_UPGRADES;
        player->temple_complex_building_id = id;
    }
    return 0;
}

void game_undo_restore_building(int32_t id)
{
    if (id <= 0) {
        return;
    }
    building *b = building_get(id);
    restore_on_map(id, b);
    if (b->type == BUILDING_FISHING_WHARF || b->type == BUILDING_TRANSPORT_WHARF || b->type == BUILDING_WARSHIP_WHARF) {
        b->data.dock.ship_id = 0;
    }
    if (b->house_size) {
        b->house_population = 0;
    }
    if (building_type_has_storage(b->type)) {
        restore_storage(id, b);
    }
    if (b->type == BUILDING_FERRY_LANDING) {
        restore_ferry_queue(id, b);
    }
    if (b->type == BUILDING_SHIPWRIGHT) {
        restore_repair_slot(id, b);
    }
    if (!restore_city_reference(id, b)) {
        b->state = BUILDING_STATE_IN_USE;
    }
}

void game_undo_restore_houses(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_UNUSED && b->type >= BUILDING_HOUSE_CRUDE_HUT && b->type <= BUILDING_HOUSE_PALATIAL_ESTATE &&
            b->house_population == 0) {
            building_house_change_to_vacant_lot(id);
        }
    }
}

static void undo_clear_land(void)
{
    int32_t mausoleum = 0;
    for (int32_t i = 0; i < undo_num_buildings; i++) {
        int32_t id = undo_building_ids[i];
        if (id == 0) {
            continue;
        }
        building *b = building_get(id);
        memcpy(b, &undo_buildings[i], sizeof(building));
        game_undo_restore_building(id);
        if (b->type == BUILDING_MAUSOLEUM) {
            mausoleum = id;
        }
    }
    memcpy(map_grids.terrain, map_terrain_backup, undo_grid_bytes);
    memcpy(map_grids.canals, map_canals_backup, GRID_CELLS);
    memcpy(map_grids.sprite, map_sprite_backup, GRID_CELLS);
    memcpy(map_grids.images, map_grids.images_backup, undo_grid_bytes);
    memcpy(map_grids.bitfields, map_bitfields_backup, GRID_CELLS);
    memcpy(map_grids.edge, map_edge_backup, GRID_CELLS);
    memand_grid(map_grids.bitfields, GRID_CELLS, UNDO_BUILD_BITFIELDS_KEEP_MASK);
    game_undo_restore_houses();
    if (mausoleum) {
        mausoleum_update_all_images(mausoleum);
    }
}

static void remove_building_tiles(int32_t id, building *b)
{
    switch (b->type) {
        case BUILDING_PYRAMID:
            if (b->subtype == 0) {
                pyramid_clear_complex_area(id);
            }
            break;
        case BUILDING_FERRY_LANDING:
            if (b->linked_building_id > 0) {
                building_get(b->linked_building_id)->linked_building_id = UNDO_BUILD_FERRY_PARTNER_NONE;
            }
            ferry_queues_release(id);
            break;
        case BUILDING_BOOTH:
        case BUILDING_BANDSTAND:
        case BUILDING_PAVILION:
            venue_remove_tiles(id, b->type);
            break;
        case BUILDING_GATEHOUSE:
        case BUILDING_GATEHOUSE_UP:
            building_gatehouse_remove_tiles(id);
            break;
        case BUILDING_ALEXANDRIA_LIBRARY:
            library_remove_tiles(&library_object, id);
            break;
        case BUILDING_ABU_SIMBEL:
            abu_simbel_remove_tiles(&abu_simbel_object, id);
            break;
        case BUILDING_CAESAREUM:
            caesareum_remove_tiles(&caesareum_object, id);
            break;
        case BUILDING_LIGHTHOUSE:
            lighthouse_remove_tiles(&lighthouse_object, id);
            break;
        case BUILDING_SMALL_ROYAL_TOMB:
            small_royal_tomb_clear_tiles(&small_royal_tomb_object.base, id);
            break;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            medium_royal_tomb_clear_tiles(&medium_royal_tomb_object.base, id);
            break;
        case BUILDING_LARGE_ROYAL_TOMB:
            large_royal_tomb_clear_tiles(&large_royal_tomb_object.base, id);
            break;
        case BUILDING_GRAND_ROYAL_TOMB:
            grand_royal_tomb_clear_tiles(&grand_royal_tomb_object.base, id);
            break;
        case BUILDING_FORT:
            undo_fort(id, b);
            break;
    }
}

static void undo_created_buildings(int32_t type)
{
    if (type == BUILDING_MENU_WATER_LIFT) {
        memcpy(map_grids.terrain, map_terrain_backup, undo_grid_bytes);
        memcpy(map_grids.canals, map_canals_backup, GRID_CELLS);
        map_images_restore_free_tiles();
    }
    for (int32_t i = 0; i < undo_num_buildings; i++) {
        int32_t id = undo_building_ids[i];
        if (id == 0) {
            continue;
        }
        building *b = building_get(id);
        remove_building_tiles(id, b);
        b->state = BUILDING_STATE_UNDO;
    }
}

void game_undo_perform(void)
{
    if (undo_ready != 1 || undo_available != 1) {
        return;
    }
    int32_t cost = undo_cost;
    city_player *player = city_player_current();
    player->treasury += cost;
    undo_available = 0;
    player->construction_this_year -= cost;
    for (int32_t r = 0; r < RESOURCE_MAX; r++) {
        if (undo_resources[r] != 0) {
            building_storage_store_anywhere(r, undo_resources[r], 0);
        }
    }
    int32_t type = undo_mode;
    if (type == BUILDING_CLEAR_LAND) {
        undo_clear_land();
    } else if (type == BUILDING_IRRIGATION_DITCH || type == BUILDING_ROAD || type == BUILDING_MUD_WALL || type == BUILDING_BRICK_WALL ||
        type == BUILDING_WALL) {
        memcpy(map_grids.terrain, map_terrain_backup, undo_grid_bytes);
        memcpy(map_grids.canals, map_canals_backup, GRID_CELLS);
        map_images_restore_free_tiles();
    } else if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
        memcpy(map_grids.terrain, map_terrain_backup, undo_grid_bytes);
        memcpy(map_grids.sprite, map_sprite_backup, GRID_CELLS);
        map_images_restore_free_tiles();
    } else if (type == BUILDING_PLAZA || type == BUILDING_GARDENS) {
        memcpy(map_grids.terrain, map_terrain_backup, undo_grid_bytes);
        memcpy(map_grids.canals, map_canals_backup, GRID_CELLS);
        memcpy(map_grids.bitfields, map_bitfields_backup, GRID_CELLS);
        memcpy(map_grids.edge, map_edge_backup, GRID_CELLS);
        map_images_restore_free_tiles();
    } else if (undo_num_buildings != 0) {
        undo_created_buildings(type);
    }
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
    undo_num_buildings = 0;
    building_storage_reset_building_ids();
    building_ferry_records_update();
    building_repair_slots_update();
    city_rebuild_monument_slots();
    map_tiles_update_all();
}
