// Deleting a building: frees what it owns (storage, ferry queue and repair slot records), clears the city's links
// to it (distribution center, palace, recruiter, fort legion, ferry pair, monument slots, obelisks, temple
// complex), zeroes the record and closes the undo step of a building being placed.

#include "building/building_delete.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/destroy_paths.h"
#include "building/ferry_landing.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "figure/boats.h"
#include "figure/ships_player.h"
#include "game/undo_build.h"
#include "map/tiles.h"
#include <stdint.h>
#include <string.h>

void building_delete(int32_t building_id)
{
    building_clear_related_data(building_id);
    memset(building_get(building_id), 0, sizeof(building));
    if (undo_mode == BUILDING_CLEAR_LAND) {
        game_undo_finish_build();
    }
}

static void clear_storage_and_slots(int32_t building_id)
{
    building *b = building_get(building_id);
    uint8_t storage = b->storage_id;
    if (storage != 0) {
        b->storage_id = 0;
        storage_get(storage)->in_use = 0;
    }
    uint8_t queue_index = b->queue_index;
    if (queue_index != 0) {
        b->queue_index = 0;
        ferry_queue_get(queue_index)->queue.in_use = 0;
    }
    if (b->queue_slot != 0) {
        building_repair_queue_remove(building_id);
        b->queue_slot = 0;
    }
}

static void clear_monument_part(int32_t building_id, int32_t subtype_equals)
{
    monument_unregister(building_id);
    if (building_get(building_id)->subtype == subtype_equals) {
        map_tiles_update_all();
        window_redraw_request = 1;
    }
}

static void clear_by_type(int32_t building_id)
{
    building *b = building_get(building_id);
    city_player *city = city_player_current();
    int32_t type = b->type;
    switch (type) {
        case BUILDING_DISTRIBUTION_CENTER:
            if (b->grid_offset == city->distribution_center_grid_offset) {
                city->distribution_center_grid_offset = 0;
                city->distribution_center_x = 0;
                city->distribution_center_y = 0;
                city->distribution_center_placed = 0;
            }
            return;
        case BUILDING_FORT:
            formation_legion_delete_for_fort(b->formation_id);
            return;
        case BUILDING_SHIPWRIGHT:
            building_repair_queue_remove(building_id);
            return;
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
            city->mansion_placed = 0;
            return;
        case BUILDING_MILITARY_ACADEMY:
            city->military_academy_placed = 0;
            return;
        case BUILDING_RECRUITER:
            if (b->grid_offset == city->recruiter_grid_offset) {
                city->recruiter_grid_offset = 0;
                city->recruiter_x = 0;
                city->recruiter_y = 0;
                city->recruiter_placed = 0;
            }
            return;
        case BUILDING_FERRY_LANDING:
            if (b->linked_building_id > 0) {
                building_get(b->linked_building_id)->linked_building_id = -1;
                ferry_queues_release(building_id);
            }
            return;
        case BUILDING_ROADBLOCK:
        case BUILDING_PYRAMID_ENTRY_TILE:
            map_tiles_update_region_roads(b->x, b->y, b->x, b->y);
            map_tiles_update_region_canal_images(b->x, b->y, b->x, b->y);
            return;
        case BUILDING_WARSHIP_WHARF:
            building_warship_wharf_count -= 1;
            return;
        case BUILDING_PYRAMID:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_CAESAREUM:
            monument_unregister(building_id);
            return;
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            if (b->grid_offset == city->palace_grid_offset) {
                city->palace_grid_offset = 0;
                city->palace_x = 0;
                city->palace_y = 0;
                city->palace_placed = 0;
            }
            return;
        case BUILDING_FESTIVAL_SQUARE:
            city->festival_square_x = -1;
            city->festival_square_y = -1;
            city->festival_square_placed = 0;
            return;
        case BUILDING_SPHINX:
            city->sphinx_stage_before_delete = city->sphinx_stage;
            city->sphinx_stage = 0;
            monument_unregister(building_id);
            return;
        case BUILDING_LIGHTHOUSE:
            clear_monument_part(building_id, LIGHTHOUSE_PARTS - 1);
            return;
        case BUILDING_SMALL_ROYAL_TOMB:
            clear_monument_part(building_id, SMALL_TOMB_PARTS - 1);
            return;
        case BUILDING_ABU_SIMBEL:
            clear_monument_part(building_id, ABU_SIMBEL_PARTS - 1);
            return;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            clear_monument_part(building_id, MEDIUM_TOMB_PARTS - 1);
            return;
        case BUILDING_LARGE_ROYAL_TOMB:
            clear_monument_part(building_id, LARGE_TOMB_PARTS - 1);
            return;
        case BUILDING_GRAND_ROYAL_TOMB:
            clear_monument_part(building_id, GRAND_TOMB_PARTS - 1);
            return;
        default:
            break;
    }
    if (type == BUILDING_SMALL_OBELISK || type == BUILDING_LARGE_OBELISK) {
        obelisk_on_delete(building_id);
        return;
    }
    if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        city->has_temple_complex = 0;
        city->temple_complex_upgrades = 0;
        building_temple_complex_remove_tiles(building_main(building_id));
        city->temple_complex_building_id = 0;
    }
}

void building_clear_related_data(int32_t building_id)
{
    clear_storage_and_slots(building_id);
    clear_by_type(building_id);
}
