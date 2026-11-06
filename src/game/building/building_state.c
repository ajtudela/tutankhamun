// Building counters and state: the highest building ids, the counts per type and per industry (with wharves,
// docks and the recruiter), venue shows counted down, the removal of buildings marked for deletion, the money
// value of buildings, and the clearing of the building array and counters at the start of a game.
//
// The venue show countdown also counts down the same bytes of the floodplain farms.

#include "building/building_state.h"
#include "building/building.h"
#include "building/building_delete.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/construction_clear.h"
#include "building/construction_venue.h"
#include "building/destroy_paths.h"
#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include "city/finance.h"
#include "city/population.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "editor/editor_data.h"
#include "figure/figure.h"
#include "game/game_internal.h"
#include "map/building_tiles.h"
#include "map/moisture.h"
#include "map/road_access.h"
#include "map/routing_terrain.h"
#include "map/tiles.h"
#include "widget/widget_data.h"
#include <stdint.h>
#include <string.h>

enum building_state_limit {
    WORKING_DOCK_SLOTS = 10,
    VENUE_SHOW_KINDS = 3,
};

void building_update_highest_id(void)
{
    building_highest_id = 0;
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        if (building_get(id)->state != BUILDING_STATE_UNUSED) {
            building_highest_id = id;
        }
    }
    if (building_highest_id > building_highest_id_ever) {
        building_highest_id_ever = building_highest_id;
    }
    building_storage_highest_id = 0;
    for (int32_t id = 1; id < STORAGE_RECORDS; id++) {
        if (storage_get(id)->in_use) {
            building_storage_highest_id = id;
        }
    }
}

static int32_t counted_on_main_part(int32_t type)
{
    return (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) ||
        (type >= BUILDING_TEMPLE_COMPLEX_GOD_5 && type <= BUILDING_TEMPLE_COMPLEX_GOD_9);
}

void building_count_update(void)
{
    for (int32_t i = 0; i < RESOURCE_MAX; i++) {
        building_count_industry_active[i] = 0;
    }
    for (int32_t i = 0; i < RESOURCE_MAX; i++) {
        building_count_industry_total[i] = 0;
    }
    city_player *player = city_player_current();
    player->working_wharfs = 0;
    player->shipyard_boats_requested = 0;
    for (int32_t i = 0; i < WORKING_DOCK_SLOTS; i++) {
        player->working_dock_ids[i] = 0;
    }
    player->mortuary_workers = 0;
    for (int32_t i = 0; i < BUILDING_TYPE_COUNT; i++) {
        building_count_total[i] = 0;
    }
    for (int32_t i = 0; i < BUILDING_TYPE_COUNT; i++) {
        building_count_active[i] = 0;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_CREATED) {
            continue;
        }
        int32_t type = b->type;
        if (b->house_size) {
            building_count_total[type]++;
            if (b->house_population != 0) {
                building_count_active[type]++;
            }
            continue;
        }
        int32_t venue = 0;
        int32_t resource = building_productions[type].counted_as;
        if (type == BUILDING_WATER_LIFT || type == BUILDING_WELL || type == BUILDING_WATER_SUPPLY) {
            building_count_total[type]++;
            if (b->has_water_access && (type == BUILDING_WELL || b->num_workers > 0)) {
                building_count_active[type]++;
            }
        } else if (resource) {
            building_count_industry_total[resource]++;
            if (b->num_workers > 0) {
                building_count_industry_active[resource]++;
            }
        } else if (type == BUILDING_FISHING_WHARF) {
            building_count_industry_total[RESOURCE_FISH]++;
            if (b->num_workers > 0) {
                player->working_wharfs++;
                if (b->data.industry.worker_figure_id == 0) {
                    player->shipyard_boats_requested++;
                }
                building_count_industry_active[RESOURCE_FISH]++;
            }
        } else if (type == BUILDING_DOCK || type == BUILDING_DOCK_UP) {
            int32_t slot = building_count_total[BUILDING_DOCK];
            if (slot < WORKING_DOCK_SLOTS) {
                if (b->num_workers > 0 && b->has_water_access) {
                    player->working_dock_ids[slot] = (int16_t) id;
                }
                building_count_total[BUILDING_DOCK]++;
            }
        } else if (counted_on_main_part(type)) {
            if (building_main(id) == id) {
                building_count_total[b->type]++;
                if (b->num_workers > 0) {
                    building_count_active[b->type]++;
                }
            }
        } else if (type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_GOD_9) {
            building_count_total[type]++;
            if (map_has_road_access_nearby(b->x, b->y)) {
                building_count_active[b->type]++;
            }
        } else {
            building_count_total[type]++;
            if (type == BUILDING_BOOTH) {
                if (building_main(id) != id) {
                    continue;
                }
                if (b->num_workers > 0) {
                    building_count_active[BUILDING_BOOTH]++;
                }
                venue = 1;
            } else if (type == BUILDING_BANDSTAND) {
                if (building_main(id) != id) {
                    continue;
                }
                if (b->num_workers > 0) {
                    building_count_active[BUILDING_BOOTH]++;
                    building_count_active[BUILDING_BANDSTAND]++;
                }
                venue = 1;
            } else if (type == BUILDING_PAVILION) {
                if (b->num_workers > 0) {
                    building_count_active[BUILDING_BOOTH]++;
                    building_count_active[BUILDING_BANDSTAND]++;
                    building_count_active[BUILDING_PAVILION]++;
                }
                venue = 1;
            } else if (type == BUILDING_SENET_HOUSE) {
                if (b->num_workers > 0) {
                    building_count_active[BUILDING_SENET_HOUSE]++;
                }
            } else if (type == BUILDING_ZOO) {
                if (b->num_workers > 0) {
                    building_count_active[BUILDING_ZOO]++;
                }
            } else {
                if (type == BUILDING_RECRUITER) {
                    player->recruiter_building_id = id;
                }
                if (b->num_workers > 0) {
                    building_count_active[type]++;
                }
            }
        }
        int32_t immigrant_id = b->immigrant_figure_id;
        if (immigrant_id != 0) {
            const figure *immigrant = figure_get(immigrant_id);
            if (immigrant->state != FIGURE_STATE_ALIVE || immigrant->destination_building_id != id) {
                b->immigrant_figure_id = 0;
            }
        }
        if (venue || building_is_floodplain_farm(id)) {
            uint8_t running = 0;
            for (int32_t n = 0; n < VENUE_SHOW_KINDS; n++) {
                if (b->data.entertainment.show_days_left[n]) {
                    running++;
                    b->data.entertainment.show_days_left[n]--;
                }
            }
            b->data.entertainment.num_shows = running;
        }
    }
}

void building_update_money_values(void)
{
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE) {
            b->tax_income_or_storage = 0;
            if (!b->house_size) {
                b->tax_income_or_storage = city_finance_building_money_value(id);
            }
        }
    }
}

static int32_t is_wall_type(int32_t type)
{
    return type == BUILDING_MUD_GATEHOUSE || type == BUILDING_MUD_TOWER || type == BUILDING_BRICK_GATEHOUSE || type == BUILDING_BRICK_TOWER ||
        type == BUILDING_TOWER || type == BUILDING_MUD_GATEHOUSE_UP || type == BUILDING_BRICK_GATEHOUSE_UP;
}

static void remove_tiles(building *b, int32_t id)
{
    switch (b->type) {
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
        default:
            map_building_tiles_remove(id, b->x, b->y);
            break;
    }
}

void building_update_state(void)
{
    int32_t walls_changed = 0;
    int32_t routing_changed = 0;
    int32_t pyramid_changed = 0;
    if (building_highest_id <= 0) {
        return;
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_CREATED) {
            b->state = BUILDING_STATE_IN_USE;
        }
        int32_t state = b->state;
        if (state == BUILDING_STATE_IN_USE) {
            continue;
        }
        if (state == BUILDING_STATE_UNDO || state == BUILDING_STATE_DELETED) {
            int32_t type = b->type;
            if (type == BUILDING_GATEHOUSE || type == BUILDING_GATEHOUSE_UP) {
                building_gatehouse_remove_tiles(id);
                building_delete(id);
                walls_changed = 1;
                routing_changed = 1;
                continue;
            }
            if (is_wall_type(type)) {
                walls_changed = 1;
            }
            if (type == BUILDING_FESTIVAL_SQUARE || type == BUILDING_BANDSTAND || type == BUILDING_BOOTH || type == BUILDING_PAVILION) {
                if (building_main(id) == id) {
                    venue_remove_tiles(id, b->type);
                    routing_changed = 1;
                }
            } else {
                if (type == BUILDING_PYRAMID) {
                    pyramid_changed = 1;
                }
                remove_tiles(b, id);
                routing_changed = 1;
                type = b->type;
                if ((undo_mode == BUILDING_CLEAR_LAND && type == BUILDING_ABU_SIMBEL) || type == BUILDING_CAESAREUM || type == BUILDING_ALEXANDRIA_LIBRARY ||
                    type == BUILDING_LIGHTHOUSE || type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB ||
                    type == BUILDING_LARGE_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB) {
                    undo_available = 0;
                }
            }
            building_delete(id);
        } else if (state == BUILDING_STATE_RUBBLE) {
            if (b->house_size) {
                city_population_remove_home_removed(city_current_player, b->house_population);
            }
            building_delete(id);
        } else if (state == BUILDING_STATE_DELETED_BY_GAME) {
            building_delete(id);
        }
    }
    if (pyramid_changed) {
        map_pyramid_redraw_pending = 1;
        map_pyramid_redraw();
    }
    if (walls_changed) {
        map_tiles_update_all_walls();
    }
    if (routing_changed) {
        map_routing_update_land_citizen();
        map_routing_update_land_noncitizen();
    }
}

void building_reset_tick47_counters(void)
{
    building_counters.unread_reset_b[1] = 0;
    building_counters.unread_reset_b[0] = 0;
    building_counters.unread_reset_a[1] = 0;
    building_counters.unread_reset_a[0] = 0;
}

int32_t building_get_subtype(int32_t building_id)
{
    return city_buildings.records[building_id].subtype;
}

void building_clear_all(void)
{
    memset(city_buildings.records, 0, MAX_BUILDINGS * sizeof(building));
    building_highest_id_ever = 0;
    building_extra_sequence = 0;
}

void building_counters_clear(void)
{
    memset(&building_counters, 0, sizeof(building_counters));
}

int32_t building_count_warship_wharves(void)
{
    int32_t count = 0;
    int32_t last = building_highest_id;
    for (int32_t id = 1; id <= last; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_WARSHIP_WHARF) {
            count++;
        }
    }
    return count;
}
