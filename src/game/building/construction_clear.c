// Clearing land with the build tool: clears the terrain of a rectangle and marks its buildings with all their
// parts deleted (forts, monuments and temple complexes after a yes), cuts canals and removes bridges when
// confirmed, then updates tiles, venue plazas and routing. The four popup callbacks store the answer and clear
// again.

#include "building/construction_clear.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "building/ferry_landing.h"
#include "building/house_population.h"
#include "building/monument.h"
#include "building/monument_spots.h"
#include "city/message.h"
#include "figure/ships_player.h"
#include "game/undo_build.h"
#include "map/bridge.h"
#include "map/construction_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"
#include "sound/sound_play.h"
#include "widget/widget_data.h"
#include "window/popup_dialog.h"
#include <stdint.h>
#include <string.h>

static int32_t never_cleared(int32_t type)
{
    switch (type) {
        case BUILDING_BURNING_RUIN:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_NATIVE_CROPS:
        case BUILDING_MORTUARY_TEMPLE:
        case BUILDING_CAUSEWAY:
        case BUILDING_VALLEY_TEMPLE:
        case BUILDING_PYRAMID_ENTRY_TILE:
            return 1;
        default:
            return 0;
    }
}

static int32_t is_temple_complex_part(int32_t type)
{
    switch (type) {
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_MENU_TEMPLE_COMPLEXES:
        case BUILDING_TEMPLE_COMPLEX_GOD_5:
        case BUILDING_TEMPLE_COMPLEX_GOD_6:
        case BUILDING_TEMPLE_COMPLEX_GOD_7:
        case BUILDING_TEMPLE_COMPLEX_GOD_8:
        case BUILDING_TEMPLE_COMPLEX_GOD_9:
            return 1;
        default:
            return 0;
    }
}

static int32_t clear_building(int32_t measure_only, int32_t id)
{
    building *b = building_get(id);
    int32_t type = b->type;
    if (never_cleared(type) || b->state == BUILDING_STATE_DELETED) {
        return 0;
    }
    if ((type == BUILDING_FORT || type == BUILDING_FORT_GROUND) && measure_only == 0 && clear_land_confirmed.forts != CLEAR_LAND_YES) {
        return 0;
    }
    if (building_type_is_monument(type) && measure_only == 0 && clear_land_confirmed.monuments != CLEAR_LAND_YES) {
        return 0;
    }
    type = b->type;
    if (is_temple_complex_part(type) && measure_only == 0 && clear_land_confirmed.complexes != CLEAR_LAND_YES) {
        return 0;
    }
    if ((type == BUILDING_FORT || type == BUILDING_FORT_GROUND) && measure_only == 0 && clear_land_confirmed.forts == CLEAR_LAND_YES) {
        undo_available = measure_only;
    }
    if (type == BUILDING_PYRAMID) {
        pyramid_clear_complex_area(id);
        pyramid_remove_entry_tile(id, 1);
    }
    uint8_t storage = b->storage_id;
    if (storage) {
        storage_get(storage)->in_use = 0;
    }
    uint8_t dock = b->queue_slot;
    if (dock) {
        dock_queue_get(dock - 1)->queue.flag = 0;
    }
    uint8_t queue = b->queue_index;
    if (queue) {
        ferry_queue_get(queue)->queue.in_use = 0;
    }
    if (b->house_size && b->house_population && measure_only == 0) {
        figure_create_homeless(city_current_player, b->x, b->y, b->house_population);
        b->house_population = (int16_t) measure_only;
    }
    type = b->type;
    if (type == BUILDING_WOOD_CUTTER || type == BUILDING_REED_GATHERER || type == BUILDING_HUNTING_LODGE) {
        b->data.industry.worker_count = 0;
    }
    if (b->state != BUILDING_STATE_DELETED) {
        routed_building_items_placed++;
        game_undo_add_building(id);
    }
    b->state = BUILDING_STATE_DELETED;
    sound_effect_play(SOUND_EFFECT_CLEAR_LAND);
    b->is_deleted = 1;
    int32_t part = id;
    for (int32_t n = 0; n < CLEAR_LAND_MAX_PARTS; n++) {
        part = building_get(part)->prev_part_building_id;
        if (part <= 0) {
            break;
        }
        game_undo_add_building(part);
        building *linked = building_get(part);
        linked->state = BUILDING_STATE_DELETED;
        linked->is_deleted = 1;
    }
    part = id;
    for (int32_t n = 0; n < CLEAR_LAND_MAX_PARTS; n++) {
        part = building_get(part)->next_part_building_id;
        if (part <= 0) {
            break;
        }
        game_undo_add_building(part);
        building *linked = building_get(part);
        linked->state = BUILDING_STATE_DELETED;
        linked->is_deleted = 1;
    }
    return 1;
}

void building_construction_clear_land(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    int32_t radius = 0;
    routed_building_items_placed = 0;
    game_undo_start_build();
    memcpy(map_grids.terrain, map_terrain_backup, GRID_CELLS * sizeof(map_grids.terrain[0]));
    memcpy(map_grids.canals, map_canals_backup, GRID_CELLS * sizeof(map_grids.canals[0]));
    map_images_restore_free_tiles();
    int32_t x_min, x_max, y_min, y_max;
    if (x_start <= x_end) {
        x_min = x_start;
        x_max = x_end;
    } else {
        x_min = x_end;
        x_max = x_start;
    }
    if (y_start <= y_end) {
        y_min = y_start;
        y_max = y_end;
    } else {
        y_min = y_end;
        y_max = y_start;
    }
    if (x_min < 0) {
        x_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (y_min < 0) {
        y_min = 0;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    int32_t width = x_max - x_min + 1;
    int32_t height = y_max - y_min + 1;
    map_tile_cursor_offset = map_grid_offset(x_min, y_min);
    for (map_tile_cursor_y = y_min; map_tile_cursor_y <= y_max; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x_min; map_tile_cursor_x <= x_max; map_tile_cursor_x++, map_tile_cursor_offset++) {
            int32_t offset = map_tile_cursor_offset;
            uint32_t raw = map_grids.terrain[offset];
            uint32_t terrain = raw & TERRAIN_NOT_CLEAR;
            if (terrain & (TERRAIN_ROCK | TERRAIN_ELEVATION | TERRAIN_MARSH | TERRAIN_DUNE)) {
                continue;
            }
            int32_t id = map_grids.building_ids[offset];
            int32_t blocked = (terrain & TERRAIN_BUILDING) || (terrain & TERRAIN_WALKABLE_BUILDING);
            if (!blocked && id != 0 && (raw & TERRAIN_MONUMENT)) {
                int32_t type = building_get(id)->type;
                blocked = (type == BUILDING_ALEXANDRIA_LIBRARY) || (type == BUILDING_SUN_TEMPLE);
            }
            if (blocked) {
                if (id == 0) {
                } else if (!clear_building(measure_only, id)) {
                    continue;
                }
            } else if (terrain & TERRAIN_CANAL) {
                map_grids.terrain[offset] = raw & TERRAIN_KEPT_BY_BUILDING;
                map_grids.canals[offset] = 0;
                routed_building_items_placed++;
                if (map_grids.canals[offset - GRID_SIZE] == CANAL_NEIGHBOUR_STRAIGHT_ALONG_Y) {
                    map_grids.canals[offset - GRID_SIZE] = CANAL_NEIGHBOUR_END_ABOVE_GAP;
                }
                if (map_grids.canals[offset + 1] == CANAL_NEIGHBOUR_STRAIGHT_ALONG_X) {
                    map_grids.canals[offset + 1] = CANAL_NEIGHBOUR_END_RIGHT_OF_GAP;
                }
                if (map_grids.canals[offset + GRID_SIZE] == CANAL_NEIGHBOUR_STRAIGHT_ALONG_Y) {
                    map_grids.canals[offset + GRID_SIZE] = CANAL_NEIGHBOUR_END_BELOW_GAP;
                }
                if (map_grids.canals[offset - 1] == CANAL_NEIGHBOUR_STRAIGHT_ALONG_X) {
                    map_grids.canals[offset - 1] = CANAL_NEIGHBOUR_END_LEFT_OF_GAP;
                }
            } else if (terrain & TERRAIN_WATER) {
                if ((measure_only == 0) && (map_bridge_count_figures(offset) >= 1)) {
                    city_warning_show(WARNING_CANNOT_DEMOLISH_BRIDGE_WITH_PEOPLE);
                    continue;
                }
                if (clear_land_confirmed.bridges == CLEAR_LAND_YES) {
                    map_bridge_remove(map_tile_cursor_offset, measure_only);
                    routed_building_items_placed++;
                }
            } else if (terrain != 0) {
                uint8_t edge = map_grids.edge[offset];
                map_grids.terrain[offset] = raw & TERRAIN_KEPT_BY_BUILDING;
                uint8_t bits = map_grids.bitfields[offset] & (uint8_t) ~MAP_BITFIELD_SIZE;
                map_grids.bitfields[offset] = bits;
                map_grids.edge[offset] = edge | MAP_EDGE_DRAW_TILE;
                if (measure_only == 0) {
                    map_grids.bitfields[offset] = bits & (uint8_t) ~MAP_BITFIELD_PLAZA_OR_FAULT;
                    map_grids.edge[offset] = edge | MAP_EDGE_DRAW_TILE;
                }
                routed_building_items_placed++;
            }
            if ((measure_only == 0) && (terrain & (TERRAIN_BUILDING | TERRAIN_ROAD | TERRAIN_CANAL))) {
                radius = CLEAR_LAND_ROAD_UPDATE_RADIUS;
            }
        }
        map_tile_cursor_offset += GRID_SIZE - width;
    }
    int32_t size = ((width <= height) ? height : width) + 2;
    map_tiles_update_region_empty_land(x_min, y_min, x_max, y_max, 0, 1);
    if (radius) {
        map_tiles_update_region_empty_land(x_min - radius, y_min - radius, x_max + radius, y_max + radius, 1, 0);
    }
    map_tiles_update_region_meadow(x_min, y_min, x_max, y_max);
    map_tiles_update_region_marshland(x_min, y_min, x_max, y_max);
    map_tiles_update_region_floodplain(x_min, y_min, x_max, y_max);
    map_tiles_update_region_canals(x_min, y_min, x_max, y_max, 0);
    map_tiles_update_roads_around(x_min, y_min, size);
    map_tiles_update_region_rubble(x_min, y_min, x_max, y_max);
    map_tiles_update_all_gardens();
    map_tiles_update_all_plazas(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 1);
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_UNUSED || b->state == BUILDING_STATE_DELETED) {
            continue;
        }
        int32_t type = b->type;
        if (type == BUILDING_BOOTH || type == BUILDING_PAVILION || type == BUILDING_FESTIVAL_SQUARE) {
            if ((building_main(id) == id) && (b->venue_has_plaza != 0)) {
                map_add_venue_plaza_tiles(b->venue_plaza_grid_offset, type, id);
            }
        } else if (type == BUILDING_BANDSTAND) {
            if (building_get(b->next_part_building_id)->type == BUILDING_BANDSTAND) {
                venue_place_adjacent_tiles(id, b->x, b->y);
            }
            if ((building_main(id) == id) && (b->venue_has_plaza != 0)) {
                map_add_venue_plaza_tiles(b->venue_plaza_grid_offset, BUILDING_BANDSTAND, id);
            }
        }
    }
    map_tiles_update_region_walls(x_min, y_min, size);
    map_tiles_update_region_water(x_min, y_min, x_max, y_max, 0);
    if (measure_only == 0) {
        map_tiles_update_region_canal_images(x_min - 1, y_min - 1, x_max + 1, y_max + 1);
        map_tiles_update_region_roads(x_min - 1, y_min - 1, x_max + 1, y_max + 1);
        map_routing_update_water();
        map_routing_update_land_citizen();
        map_routing_update_land_noncitizen();
        map_routing_update_walls();
    }
}

static int32_t answer(void)
{
    return (popup_answer == 1) ? CLEAR_LAND_YES : CLEAR_LAND_NO;
}

static void confirm_rect(void)
{
    building_construction_clear_land(0, clear_land_rect.x_start, clear_land_rect.y_start, clear_land_rect.x_end, clear_land_rect.y_end);
}

void clear_land_confirm_monuments(void)
{
    clear_land_confirmed.monuments = answer();
    confirm_rect();
}

void clear_land_confirm_complexes(void)
{
    clear_land_confirmed.complexes = answer();
    confirm_rect();
}

void clear_land_confirm_forts(void)
{
    clear_land_confirmed.forts = answer();
    confirm_rect();
}

void clear_land_confirm_bridges(void)
{
    clear_land_confirmed.bridges = answer();
    confirm_rect();
}
