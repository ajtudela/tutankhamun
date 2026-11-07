// Placing buildings with the build tool: the update while dragging (cost and tile marks) and the placement on
// release (site checks, materials, creating the building and adding it to the map), with the drag tools for
// houses, walls, plazas and gardens and the clear land tool.
//
// The site checks keep a few mix-ups of messages and materials:
// - the mausoleum's missing sandstone shows the papyrus maker warning;
// - the sun temple and the mausoleum count their sandstone as granite;
// - a second Abu Simbel is refused with the Pharos text;
// - the temple complex check falls through to the later checks when the site fits.

#include "building/construction_place.h"
#include "building/abu_simbel.h"
#include "building/building_create.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_storage.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_bridge.h"
#include "building/construction_clear.h"
#include "building/construction_load.h"
#include "building/construction_routed.h"
#include "building/construction_venue.h"
#include "building/construction_warnings.h"
#include "building/ferry_landing.h"
#include "building/gatehouse_place.h"
#include "building/house_size.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_checks.h"
#include "building/monument_object.h"
#include "building/monument_place.h"
#include "building/monument_spots.h"
#include "building/monument_tables.h"
#include "building/obelisk.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/finance_estimates.h"
#include "city/resource_type.h"
#include "figure/animals_wild.h"
#include "figure/formation.h"
#include "figure/herds.h"
#include "figure/military_internal.h"
#include "game/game_data.h"
#include "game/undo_build.h"
#include "graphics/image_collection.h"
#include "map/construction_terrain.h"
#include "map/construction_tiles.h"
#include "map/ferry.h"
#include "map/map_data.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"
#include "map/water.h"
#include "widget/city_view_rotate.h"
#include "window/popup_dialog.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

uint8_t construction_ferry_hover_ok;

int32_t construction_place_rules[CONSTRUCTION_PLACE_RULE_COUNT] = {
    [CONSTRUCTION_PLACE_RULE_OUT_OF_CREDIT_TREASURY] = -5000,
    [CONSTRUCTION_PLACE_RULE_MAX_FORTS] = 6,
    [CONSTRUCTION_PLACE_RULE_MAX_WARSHIP_WHARVES] = 6,
    [CONSTRUCTION_PLACE_RULE_MAX_DOCKS] = 10,
    [CONSTRUCTION_PLACE_RULE_LIBRARY_PAPYRUS] = 500,
    [CONSTRUCTION_PLACE_RULE_SMALL_OBELISK_GRANITE] = 10000,
    [CONSTRUCTION_PLACE_RULE_LARGE_OBELISK_GRANITE] = 20000,
    [CONSTRUCTION_PLACE_RULE_SUN_TEMPLE_SANDSTONE] = 22000,
    [CONSTRUCTION_PLACE_RULE_MAUSOLEUM_SANDSTONE] = 24000,
};

enum place_check_result {
    PLACE_RETURN,
    PLACE_CHECKED,
    PLACE_CONTINUE,
};

enum construction_update_result {
    UPDATE_ABORTED,
    UPDATE_ROUTED,
    UPDATE_DONE,
    UPDATE_NOT_HANDLED,
};

static void restore_terrain_and_canals_from_backup(void)
{
    memcpy(map_grids.terrain, map_terrain_backup, GRID_CELLS * sizeof(uint32_t));
    memcpy(map_grids.canals, map_canals_backup, GRID_CELLS);
}

static void restore_plaza_grids_from_backup(void)
{
    restore_terrain_and_canals_from_backup();
    memcpy(map_grids.bitfields, map_bitfields_backup, GRID_CELLS);
    memcpy(map_grids.edge, map_edge_backup, GRID_CELLS);
}

static int32_t over_credit_limit(void)
{
    return city_player_current()->treasury <= construction_place_rules[CONSTRUCTION_PLACE_RULE_OUT_OF_CREDIT_TREASURY];
}

static int32_t forts_full(void)
{
    return formation_num_legions >= construction_place_rules[CONSTRUCTION_PLACE_RULE_MAX_FORTS];
}

static int32_t warship_wharves_full(void)
{
    return building_warship_wharf_count >= construction_place_rules[CONSTRUCTION_PLACE_RULE_MAX_WARSHIP_WHARVES];
}

static int32_t is_fort(int32_t type)
{
    return type == BUILDING_FORT_CHARIOTEERS || type == BUILDING_FORT_ARCHERS || type == BUILDING_FORT_INFANTRY;
}

static uint32_t wall_terrain(int32_t type)
{
    switch (type) {
        case BUILDING_MUD_WALL:
            return TERRAIN_WALL;
        case BUILDING_BRICK_WALL:
            return TERRAIN_WALL | TERRAIN_BRICK_WALL;
        default:
            return TERRAIN_WALL | TERRAIN_STRONG_WALL;
    }
}

bool construction_allowed_on_floodplain(int32_t type)
{
    switch (type) {
        case BUILDING_ROAD:
        case BUILDING_IRRIGATION_DITCH:
        case BUILDING_CLEAR_LAND:
        case BUILDING_WATER_LIFT:
        case BUILDING_BARLEY_FARM:
        case BUILDING_FLAX_FARM:
        case BUILDING_GRAIN_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_CHICKPEAS_FARM:
        case BUILDING_FIG_FARM:
        case BUILDING_HENNA_FARM:
            return 1;
        default:
            return 0;
    }
}

static int32_t point_in_area(int32_t x, int32_t y, int32_t x_min, int32_t y_min, int32_t x_max, int32_t y_max)
{
    return x >= x_min && x <= x_max && y >= y_min && y <= y_max;
}

bool construction_area_has_entry_point(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t x_min = (x0 < x1) ? x0 : x1;
    int32_t x_max = (x0 > x1) ? x0 : x1;
    int32_t y_min = (y0 < y1) ? y0 : y1;
    int32_t y_max = (y0 > y1) ? y0 : y1;
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        if (point_in_area(scenario_data.predator_herd_x[i], scenario_data.predator_herd_y[i], x_min, y_min, x_max, y_max)) {
            return 1;
        }
    }
    for (int32_t i = 0; i < SCENARIO_HERD_POINTS; i++) {
        if (point_in_area(scenario_data.prey_herd_x[i], scenario_data.prey_herd_y[i], x_min, y_min, x_max, y_max)) {
            return 1;
        }
    }
    return 0;
}

void building_construction_place_houses(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    int32_t no_road_nearby = 0;
    int32_t x_min = (x_start <= x_end) ? x_start : x_end;
    int32_t x_max = (x_start <= x_end) ? x_end : x_start;
    int32_t y_min = (y_end < y_start) ? y_end : y_start;
    int32_t y_max = (y_end < y_start) ? y_start : y_end;
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
    routed_building_items_placed = 0;
    game_undo_start_build();
    map_tile_cursor_offset = map_grid_offset(x_min, y_min);
    for (map_tile_cursor_y = y_min; map_tile_cursor_y <= y_max; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x_min; map_tile_cursor_x <= x_max; map_tile_cursor_x++) {
            int32_t offset = map_tile_cursor_offset;
            if ((map_grids.terrain[offset] & TERRAIN_FLOODPLAIN) == 0 && floodplain_node_dry(offset) && (map_grids.terrain[offset] & TERRAIN_NOT_CLEAR) == 0 &&
                map_grids.figure_ids[offset] == 0) {
                if (measure_only == 0) {
                    int32_t id = building_create(city_current_player, BUILDING_HOUSE_CRUDE_HUT, map_tile_cursor_x, map_tile_cursor_y, 0, 0);
                    game_undo_add_building(id);
                    if (id > 0) {
                        routed_building_items_placed++;
                        map_building_tiles_add(id, map_tile_cursor_x, map_tile_cursor_y, 1, 1, image_group(IMAGE_GROUP_VACANT_LOT), TERRAIN_BUILDING);
                        if (!map_terrain_exists_tile_in_radius_with_type(map_tile_cursor_x, map_tile_cursor_y, 1, HOUSE_ROAD_RADIUS, TERRAIN_ROAD)) {
                            no_road_nearby = 1;
                        }
                    }
                } else {
                    routed_building_items_placed++;
                    map_grids.bitfields[offset] |= MAP_BITFIELD_OVERLAY;
                }
            }
            map_tile_cursor_offset++;
        }
        map_tile_cursor_offset += GRID_SIZE - (x_max - x_min + 1);
    }
    if (measure_only == 0) {
        construction_warning_check_house_food(BUILDING_HOUSE_CRUDE_HUT);
        construction_warning_check_unemployment(BUILDING_HOUSE_CRUDE_HUT);
        construction_warning_check_labor_need(BUILDING_HOUSE_CRUDE_HUT);
        city_migration_post_message(MIGRATION_MESSAGE_AFTER_NEW_HOUSES);
        if (no_road_nearby) {
            city_warning_show(WARNING_PLOTS_TOO_FAR_FROM_ROAD);
        }
        map_routing_update_land_citizen_rect(x_start, y_start, x_end, y_end, 1);
        map_routing_update_land_noncitizen_rect(x_start, y_start, x_end, y_end);
        window_redraw_request = 1;
    }
}

static int32_t clamp_to_map(int32_t value, int32_t max)
{
    int32_t clamped = (value < max) ? value : max;
    return (clamped < 0) ? 0 : clamped;
}

void building_construction_place_wall(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, uint32_t terrain)
{
    restore_terrain_and_canals_from_backup();
    map_images_restore_free_tiles();
    int32_t x0 = clamp_to_map(x_start, map_info.grid_width - 1);
    int32_t y0 = clamp_to_map(y_start, map_info.grid_height - 1);
    int32_t x1 = clamp_to_map(x_end, map_info.grid_width - 1);
    int32_t y1 = clamp_to_map(y_end, map_info.grid_height - 1);
    int32_t dx = ((x1 - x0) < 0) ? (x0 - x1) : (x1 - x0);
    int32_t dy = ((y1 - y0) < 0) ? (y0 - y1) : (y1 - y0);
    if (dx > dy) {
        if (y1 > y0) {
            y1 = y0 + 1;
        } else if (y1 < y0) {
            y1 = y0 - 1;
        }
    } else if (x1 > x0) {
        x1 = x0 + 1;
    } else if (x1 < x0) {
        x1 = x0 - 1;
    }
    if (x1 < x0) {
        int32_t t = x0;
        x0 = x1;
        x1 = t;
    }
    if (y1 < y0) {
        int32_t t = y0;
        y0 = y1;
        y1 = t;
    }
    routed_building_items_placed = 0;
    int32_t row_step = map_info.grid_width - x1 + x0 - 1;
    int32_t offset = map_grid_offset(x0, y0);
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            if ((map_grids.terrain[offset] & TERRAIN_BLOCKS_WALL) == 0 && floodplain_node_dry(offset) &&
                map_grid_is_inside(map_info.diamond_x_min + x, map_info.diamond_y_min + y) && !construction_area_has_entry_point(x, y, x, y) &&
                map_grids.figure_ids[offset] == 0) {
                map_tile_cursor_offset = offset;
                map_tiles_set_wall(x, y, 1, terrain);
            }
        }
        offset += map_info.grid_border_size + row_step;
    }
    if (measure_only == 0) {
        map_routing_update_land_citizen_rect(x0, y0, x1, y1, 1);
        map_routing_update_land_noncitizen_rect(x0, y0, x1, y1);
        map_routing_update_walls();
        window_redraw_request = 1;
    }
}

void building_construction_place_plaza(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    if (x_end < x_start) {
        int32_t t = x_start;
        x_start = x_end;
        x_end = t;
    }
    int32_t y_max = y_end;
    if (y_end < y_start) {
        y_max = y_start;
        y_start = y_end;
    }
    restore_plaza_grids_from_backup();
    map_images_restore_free_tiles();
    if (x_start < 0) {
        x_start = 0;
    }
    if (y_start < 0) {
        y_start = 0;
    }
    if (x_end >= map_info.grid_width) {
        x_end = map_info.grid_width - 1;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    routed_building_items_placed = 0;
    int32_t row_step = map_info.grid_width - x_end;
    int32_t offset = map_grid_offset(x_start, y_start);
    for (int32_t y = y_start; y <= y_max; y++) {
        for (int32_t x = x_start; x <= x_end; x++, offset++) {
            uint32_t terrain = map_grids.terrain[offset];
            if ((terrain & TERRAIN_ROAD) && (terrain & (TERRAIN_WATER | TERRAIN_BUILDING)) == 0 && map_grids.building_ids[offset] == 0 &&
                map_tiles_road_desirability_paved(offset)) {
                uint8_t bits = map_grids.bitfields[offset];
                if ((bits & MAP_BITFIELD_PLAZA_OR_FAULT) == 0) {
                    routed_building_items_placed++;
                }
                map_grids.images[offset] = 0;
                map_grids.bitfields[offset] = (bits & ~MAP_BITFIELD_SIZE) | MAP_BITFIELD_PLAZA_OR_FAULT;
                map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
            }
        }
        offset += map_info.grid_border_size + row_step - 1 + x_start;
    }
    map_tiles_update_all_plazas(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 1);
}

void building_construction_place_garden(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    if (x_end < x_start) {
        int32_t t = x_start;
        x_start = x_end;
        x_end = t;
    }
    if (y_end < y_start) {
        int32_t t = y_start;
        y_start = y_end;
        y_end = t;
    }
    restore_plaza_grids_from_backup();
    map_images_restore_free_tiles();
    if (x_start < 0) {
        x_start = 0;
    }
    if (y_start < 0) {
        y_start = 0;
    }
    if (x_end >= map_info.grid_width) {
        x_end = map_info.grid_width - 1;
    }
    if (y_end >= map_info.grid_height) {
        y_end = map_info.grid_height - 1;
    }
    int32_t row_step = map_info.grid_width - x_end;
    routed_building_items_placed = 0;
    int32_t offset = map_grid_offset(x_start, y_start);
    for (int32_t y = y_start; y <= y_end; y++) {
        for (int32_t x = x_start; x <= x_end; x++, offset++) {
            if ((map_grids.terrain[offset] & TERRAIN_NOT_EMPTY) == 0 && floodplain_node_dry(offset) && !construction_area_has_entry_point(x, y, x, y)) {
                routed_building_items_placed++;
                map_grids.terrain[offset] |= TERRAIN_GARDEN;
            }
        }
        offset += map_info.grid_border_size + row_step - 1 + x_start;
    }
    map_tiles_update_all_gardens();
}

static int32_t is_any_temple_complex(int32_t type)
{
    return building_type_is_temple_complex(type) || type == BUILDING_MENU_TEMPLE_COMPLEXES || (type >= BUILDING_TEMPLE_COMPLEX_GOD_5 && type <= BUILDING_TEMPLE_COMPLEX_GOD_9);
}

void building_construction_clear_land_tool(int32_t measure_only, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    clear_land_confirmed.forts = 0;
    clear_land_confirmed.bridges = 0;
    clear_land_confirmed.monuments = 0;
    clear_land_confirmed.complexes = 0;
    if (measure_only != 0) {
        building_construction_clear_land(measure_only, x_start, y_start, x_end, y_end);
        return;
    }
    int32_t x_min = (x_start <= x_end) ? x_start : x_end;
    int32_t x_max = (x_start <= x_end) ? x_end : x_start;
    int32_t y_max = (y_end < y_start) ? y_start : y_end;
    map_tile_cursor_y = (y_end < y_start) ? y_end : y_start;
    if (x_min < 0) {
        x_min = 0;
    }
    if (x_max >= map_info.grid_width) {
        x_max = map_info.grid_width - 1;
    }
    if (map_tile_cursor_y < 0) {
        map_tile_cursor_y = 0;
    }
    if (y_max >= map_info.grid_height) {
        y_max = map_info.grid_height - 1;
    }
    int32_t fort = 0;
    int32_t bridge = 0;
    int32_t monument = 0;
    int32_t complex = 0;
    map_tile_cursor_offset = map_grid_offset(x_min, map_tile_cursor_y);
    for (; map_tile_cursor_y <= y_max; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x_min; map_tile_cursor_x <= x_max; map_tile_cursor_x++) {
            int32_t offset = map_tile_cursor_offset;
            int32_t id = map_grids.building_ids[offset];
            int32_t type = building_get(id)->type;
            if (id != 0 && (type == BUILDING_FORT || type == BUILDING_FORT_GROUND)) {
                fort = 1;
            }
            if (map_grids.sprite[offset] != 0 && (map_grids.terrain[offset] & TERRAIN_WATER) && id == 0) {
                bridge = 1;
            }
            if (building_type_is_monument(type)) {
                monument = 1;
            }
            if (is_any_temple_complex(type)) {
                complex = 1;
            }
            map_tile_cursor_offset++;
        }
        map_tile_cursor_offset += GRID_SIZE - (x_max - x_min + 1);
    }
    clear_land_rect.x_start = x_start;
    clear_land_rect.x_end = x_end;
    clear_land_rect.y_end = y_end;
    clear_land_rect.y_start = y_start;
    if (monument) {
        window_popup_dialog_show(POPUP_CLEAR_MONUMENT, clear_land_confirm_monuments, POPUP_BUTTONS_OK_CANCEL);
        return;
    }
    if (complex) {
        window_popup_dialog_show(POPUP_CLEAR_TEMPLE_COMPLEX, clear_land_confirm_complexes, POPUP_BUTTONS_OK_CANCEL);
        return;
    }
    if (fort) {
        window_popup_dialog_show(POPUP_CLEAR_FORT, clear_land_confirm_forts, POPUP_BUTTONS_OK_CANCEL);
        return;
    }
    if (bridge) {
        window_popup_dialog_show(POPUP_CLEAR_BRIDGE, clear_land_confirm_bridges, POPUP_BUTTONS_OK_CANCEL);
        return;
    }
    building_construction_clear_land(0, x_start, y_start, x_end, y_end);
}

static int32_t update_drag_tool(int32_t type, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    switch (type) {
        case BUILDING_ROAD:
            building_construction_place_road(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_MUD_WALL:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
            building_construction_place_wall(1, x_start, y_start, x_end, y_end, wall_terrain(type));
            return UPDATE_ROUTED;
        case BUILDING_MENU_WATER_LIFT:
            building_construction_place_canal_for_connector(1, x_start, y_start, x_end, y_end);
            construction_canals_changed = 1;
            map_tiles_update_region_canals(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0);
            construction_place_ok = 0;
            return UPDATE_DONE;
        case BUILDING_IRRIGATION_DITCH:
            building_construction_place_canal(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_CLEAR_LAND:
            building_construction_clear_land_tool(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_HOUSE_CRUDE_HUT:
            building_construction_place_houses(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_BANDSTAND:
            construction_place_bandstand(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_BOOTH:
            construction_place_booth(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_PAVILION:
            construction_place_pavilion(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_FESTIVAL_SQUARE:
            construction_place_festival_square(1, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_PLAZA:
            building_construction_place_plaza(x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_GARDENS:
            building_construction_place_garden(x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        default:
            return UPDATE_NOT_HANDLED;
    }
}

int32_t building_construction_is_drag_tool(int32_t type)
{
    switch (type) {
        case BUILDING_ROAD:
        case BUILDING_MUD_WALL:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
        case BUILDING_MENU_WATER_LIFT:
        case BUILDING_IRRIGATION_DITCH:
        case BUILDING_CLEAR_LAND:
        case BUILDING_HOUSE_CRUDE_HUT:
        case BUILDING_BANDSTAND:
        case BUILDING_BOOTH:
        case BUILDING_PAVILION:
        case BUILDING_FESTIVAL_SQUARE:
        case BUILDING_PLAZA:
        case BUILDING_GARDENS:
            return 1;
        default:
            return 0;
    }
}

static void link_ferry_landings(int32_t x, int32_t y, int32_t offset)
{
    construction_ferry_hover_ok = 0;
    int32_t id = map_grids.building_ids[offset];
    building *second = building_get(id);
    if (second->linked_building_id >= 0) {
        return;
    }
    const building *first = building_get(construction_ferry_first);
    int32_t route = 0;
    if (map_ferry_route_exists(x, y, first->x, first->y, first->size, second->data.oriented.orientation << 1,
            ((first->data.oriented.orientation * 2) + 4) & 7, NULL, &route)) {
        id = map_grids.building_ids[offset];
        construction_ferry_state = FERRY_PLACEMENT_PAIR_LINKED;
        construction_ferry_hover_ok = 1;
        building_get(id)->linked_building_id = construction_ferry_first;
        building_get(construction_ferry_first)->linked_building_id = id;
        construction_ferry_landing_placed(id);
    }
}

static void update_ferry_landing(int32_t x, int32_t y)
{
    if (map_water_determine_orientation_size2(x, y, 1, 0) == 0) {
        construction_place_ok = 1;
    }
    int32_t offset = map_grid_offset(x, y);
    if (!map_is_ferry_landing(offset)) {
        return;
    }
    if (construction_ferry_state == FERRY_PLACEMENT_NONE) {
        construction_ferry_hover_ok = 0;
        if (building_get(map_grids.building_ids[offset])->linked_building_id < 0) {
            construction_ferry_state = FERRY_PLACEMENT_FIRST_EXISTING;
            construction_ferry_hover_ok = 1;
            construction_ferry_first = map_grids.building_ids[offset];
        }
    } else if (construction_ferry_state == FERRY_PLACEMENT_FIRST_PLACED) {
        link_ferry_landings(x, y, offset);
    }
}

static int32_t update_shore_site(int32_t x, int32_t y)
{
    if (map_water_determine_orientation_size3(x, y, 1) == 0) {
        construction_place_ok = 1;
    }
    return UPDATE_DONE;
}

static int32_t mark_site(int32_t size, int32_t x, int32_t y, uint32_t mask)
{
    map_building_tiles_mark_construction(size, x, y, mask, 0, 0);
    return UPDATE_DONE;
}

static int32_t mark_site_unless(int32_t placed, int32_t size, int32_t x, int32_t y)
{
    if (placed) {
        return UPDATE_DONE;
    }
    return mark_site(size, x, y, UINT32_MAX);
}

static int32_t update_building_site(int32_t type, int32_t x, int32_t y)
{
    const city_player *player = city_player_current();
    int32_t size = construction_size;
    switch (type) {
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_FORT_ARCHERS:
        case BUILDING_FORT_INFANTRY:
            if (forts_full()) {
                return UPDATE_ABORTED;
            }
            map_building_tiles_area_free(FORT_SIZE, x, y, UINT32_MAX, 1);
            return UPDATE_DONE;
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_TEMPLE_COMPLEX_GOD_5:
        case BUILDING_TEMPLE_COMPLEX_GOD_6:
        case BUILDING_TEMPLE_COMPLEX_GOD_7:
        case BUILDING_TEMPLE_COMPLEX_GOD_8:
        case BUILDING_TEMPLE_COMPLEX_GOD_9:
            map_building_tiles_area_free(TEMPLE_COMPLEX_PART_SIZE, x, y, UINT32_MAX, 1);
            return UPDATE_DONE;
        case BUILDING_SPHINX:
            map_building_tiles_area_free(SPHINX_PART_DX, x, y, UINT32_MAX, 1);
            return UPDATE_DONE;
        case BUILDING_SUN_TEMPLE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_CAESAREUM:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_ABU_SIMBEL:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
            return UPDATE_DONE;
        case BUILDING_DISTRIBUTION_CENTER:
            return mark_site_unless(player->distribution_center_placed != 0, size, x, y);
        case BUILDING_TRIUMPHAL_ARCH:
            return mark_site(TRIUMPHAL_ARCH_SIZE, x, y, ~(uint32_t) TERRAIN_ROAD);
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
            return mark_site(GATEHOUSE_MARK_SIZE, x, y, ~(uint32_t) TERRAIN_ROAD);
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            return mark_site(STORAGE_YARD_SIZE, x, y, UINT32_MAX);
        case BUILDING_SHIPWRIGHT:
        case BUILDING_DOCK:
            return update_shore_site(x, y);
        case BUILDING_WARSHIP_WHARF:
            if (map_water_determine_orientation_size3(x, y, 1) == 0) {
                construction_place_ok = 1;
            }
            if (warship_wharves_full()) {
                return UPDATE_ABORTED;
            }
            return update_shore_site(x, y);
        case BUILDING_FISHING_WHARF:
        case BUILDING_TRANSPORT_WHARF:
        case BUILDING_WATER_LIFT:
            if (map_water_determine_orientation_size2(x, y, 1, 0) == 0) {
                construction_place_ok = 1;
            }
            return UPDATE_DONE;
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
            if (player->mansion_placed != 0) {
                return UPDATE_DONE;
            }
            if (needs_groundwater != 0 && !map_terrain_count_tiles_in_radius_reach(x, y, construction_size, 0, TERRAIN_GROUNDWATER, needs_groundwater)) {
                return UPDATE_ABORTED;
            }
            return mark_site(size, x, y, UINT32_MAX);
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            if (construction_bridge_length > 1) {
                construction_cost = construction_bridge_length * construction_cost;
            }
            return UPDATE_DONE;
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
            return mark_site_unless(player->palace_placed != 0, size, x, y);
        case BUILDING_MILITARY_ACADEMY:
            return mark_site_unless(player->military_academy_placed != 0, size, x, y);
        case BUILDING_RECRUITER:
            return mark_site_unless(building_count_total[BUILDING_RECRUITER] > 0, size, x, y);
        case BUILDING_FERRY_LANDING:
            update_ferry_landing(x, y);
            return UPDATE_DONE;
        case BUILDING_GATEHOUSE:
        case BUILDING_GATEHOUSE_UP:
            building_gatehouse_mark_construction(x, y, construction_orientation);
            return UPDATE_DONE;
        case BUILDING_SMALL_OBELISK:
            return mark_site(SMALL_OBELISK_SIZE, x, y, ~(uint32_t) TERRAIN_ROAD);
        case BUILDING_LARGE_OBELISK:
            return mark_site(LARGE_OBELISK_SIZE, x, y, ~(uint32_t) TERRAIN_ROAD);
        default:
            return UPDATE_NOT_HANDLED;
    }
}

static int32_t update_terrain_needs(int32_t type, int32_t x, int32_t y)
{
    uint8_t floodplain_ok = construction_allowed_on_floodplain(type);
    if (!floodplain_ok && map_terrain_all_tiles_in_radius_are(x, y, construction_size, 0, TERRAIN_FLOODPLAIN)) {
        return UPDATE_ABORTED;
    }
    if (needs_farm_land != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, FARM_SITE_SIZE, FARM_MEADOW_RADIUS, (TERRAIN_MEADOW | TERRAIN_FLOODPLAIN)) ? UPDATE_DONE
                                                                                                                                         : UPDATE_ABORTED;
    }
    if (construction_near_terrain_test != 0) {
        return map_terrain_any_tile_in_radius(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, construction_near_terrain_test) ? UPDATE_DONE
                                                                                                                                      : UPDATE_ABORTED;
    }
    if (needs_trees != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, TERRAIN_TREE) ? UPDATE_DONE : UPDATE_ABORTED;
    }
    if (needs_water_near != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, RAW_MATERIAL_SITE_SIZE, CLAY_PIT_WATER_RADIUS, TERRAIN_WATER) ? UPDATE_DONE : UPDATE_ABORTED;
    }
    if (needs_wall != 0) {
        return map_terrain_all_tiles_in_radius_are(x, y, TOWER_SIZE, 0, TERRAIN_WALL) ? UPDATE_DONE : UPDATE_ABORTED;
    }
    if (needs_road != 0) {
        if (!map_terrain_all_tiles_in_radius_are(x, y, 1, 0, TERRAIN_ROAD)) {
            return UPDATE_ABORTED;
        }
        map_building_tiles_mark_construction(construction_size, x, y, ~(uint32_t) TERRAIN_ROAD, 0, 0);
        return UPDATE_DONE;
    }
    if (needs_groundwater != 0) {
        return map_terrain_count_tiles_in_radius_reach(x, y, construction_size, 0, TERRAIN_GROUNDWATER, needs_groundwater) ? UPDATE_DONE : UPDATE_ABORTED;
    }
    if (needs_marsh != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, TERRAIN_MARSH) ? UPDATE_DONE : UPDATE_ABORTED;
    }
    if (type != BUILDING_PYRAMID) {
        map_building_tiles_mark_construction(construction_size, x, y, UINT32_MAX, 0, floodplain_ok);
    }
    return UPDATE_DONE;
}

static void clear_marks(void)
{
    memand_grid(map_grids.bitfields, GRID_CELLS, (uint8_t) ~(MAP_BITFIELD_OVERLAY | MAP_BITFIELD_DELETED));
}

void building_construction_update(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type)
{
    construction_cost = 0;
    construction_cost_refund = 0;
    if (type == BUILDING_NONE || over_credit_limit()) {
        return;
    }
    clear_marks();
    construction_cost = building_model_get(type)->cost;
    construction_size = building_property_get(type)->size;
    int32_t result = update_drag_tool(type, x_start, y_start, x_end, y_end);
    if (result == UPDATE_NOT_HANDLED) {
        result = update_building_site(type, x_end, y_end);
    }
    if (result == UPDATE_NOT_HANDLED) {
        result = update_terrain_needs(type, x_end, y_end);
    }
    if (result == UPDATE_ABORTED) {
        return;
    }
    if (result == UPDATE_ROUTED && routed_building_items_placed >= 0) {
        construction_cost = routed_building_items_placed * construction_cost;
    }
    construction_cost -= construction_cost_refund;
}

static void run_warnings(int32_t type, int32_t x, int32_t y, int32_t size)
{
    construction_warning_check_house_food(type);
    construction_warning_check_workers(type);
    construction_warning_check_granary(type);
    construction_warning_check_booth(type);
    construction_warning_check_bandstand(type);
    construction_warning_check_pavilion(type);
    construction_warning_check_fort(type);
    construction_warning_check_military(type);
    construction_warning_check_recruiter(type);
    construction_warning_check_tower(type, x, y, size);
    construction_warning_check_well(type, x, y);
    construction_warning_check_brewery(type);
    construction_warning_check_weaver(type);
    construction_warning_check_jeweler(type);
    construction_warning_check_potter(type);
    construction_warning_check_papyrus(type);
    construction_warning_check_bricks(type);
    construction_warning_check_lamps(type);
    construction_warning_check_paint(type);
    construction_warning_check_cattle(type);
    construction_warning_check_senet(type);
    construction_warning_check_zoo(type);
    construction_warning_check_papyrus_users(type);
    construction_warning_check_mortuary(type);
    construction_warning_check_carpenters(type);
    construction_warning_check_artisans(type);
    construction_warning_check_pyramid(type);
    construction_warning_check_granite(type);
    construction_warning_check_ptolemaic(type);
    construction_warning_check_venue_schools(type);
    construction_warning_check_road_access(type, x, y, size);
}

static void fail(int32_t warning)
{
    city_warning_show(warning);
    clear_marks();
}

static int32_t refuse(int32_t warning)
{
    city_warning_show(warning);
    return PLACE_RETURN;
}

static int32_t refuse_and_clear(int32_t warning)
{
    fail(warning);
    return PLACE_RETURN;
}

static int32_t place_water_lift(int32_t player, int32_t *type_io, int32_t x, int32_t y, int32_t *building_id)
{
    if (map_water_determine_orientation_size2(x, y, 0, 1)) {
        return refuse_and_clear(WARNING_SCOUT_ALONG_RIVER_FOR_SITE);
    }
    *type_io = BUILDING_WATER_LIFT;
    *building_id = building_create(player, BUILDING_WATER_LIFT, x, y, 0, 0);
    construction_warning_check_workers(BUILDING_WATER_LIFT);
    construction_warning_check_well(BUILDING_WATER_LIFT, x, y);
    game_undo_add_building(*building_id);
    map_grids.canals[map_grid_offset(x, y)] = 0;
    map_tiles_update_region_canals(0, 0, map_info.grid_width - 1, map_info.grid_height - 1, 0);
    map_routing_update_land_citizen_rect(x - construction_size, y - construction_size, construction_size + x, construction_size + y, 1);
    map_routing_update_land_noncitizen_rect(x - construction_size, y - construction_size, construction_size + x, construction_size + y);
    return PLACE_CHECKED;
}

static int32_t check_open_water_site(int32_t x, int32_t y)
{
    if (map_water_determine_orientation_size3(x, y, 0)) {
        return refuse(WARNING_SCOUT_ALONG_RIVER_FOR_SITE);
    }
    if (!building_dock_is_connected_to_open_water(x, y)) {
        return refuse(WARNING_INLAND_LAKE_HAS_NO_SEA_ACCESS);
    }
    return PLACE_CHECKED;
}

static int32_t check_water_site(int32_t player, int32_t *type_io, int32_t x, int32_t y, int32_t *building_id)
{
    int32_t type = *type_io;
    switch (type) {
        case BUILDING_FISHING_WHARF:
        case BUILDING_TRANSPORT_WHARF:
            if (map_water_determine_orientation_size2(x, y, 0, 0)) {
                return refuse(WARNING_SCOUT_ALONG_RIVER_FOR_SITE);
            }
            return PLACE_CHECKED;
        case BUILDING_MENU_WATER_LIFT:
            return place_water_lift(player, type_io, x, y, building_id);
        case BUILDING_GATEHOUSE:
        case BUILDING_GATEHOUSE_UP:
            if (!building_gatehouse_site_fits(x, y, construction_orientation)) {
                clear_marks();
                return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
            }
            return PLACE_CHECKED;
        case BUILDING_WARSHIP_WHARF:
            if (check_open_water_site(x, y) != PLACE_CHECKED) {
                return PLACE_RETURN;
            }
            if (warship_wharves_full()) {
                return refuse(WARNING_FULL_COMPLEMENT_OF_WARSHIP_WHARVES);
            }
            return PLACE_CHECKED;
        case BUILDING_SHIPWRIGHT:
            return check_open_water_site(x, y);
        case BUILDING_FERRY_LANDING:
            if (construction_ferry_site_ok == 0) {
                if (construction_ferry_hover_ok != 0) {
                    return PLACE_RETURN;
                }
                return refuse(WARNING_NO_VALID_SPOT_FOR_FERRY_LANDING);
            }
            if (map_water_determine_orientation_size2(x, y, 0, 0)) {
                return refuse(WARNING_SCOUT_ALONG_RIVER_FOR_SITE);
            }
            return PLACE_CHECKED;
        case BUILDING_DOCK:
        case BUILDING_DOCK_UP:
            if (building_count_total[BUILDING_DOCK] >= construction_place_rules[CONSTRUCTION_PLACE_RULE_MAX_DOCKS]) {
                return refuse(WARNING_MAX_DOCKS_REACHED);
            }
            return check_open_water_site(x, y);
        default:
            return PLACE_CONTINUE;
    }
}

static int32_t check_monument_object_site(int32_t can_place, int32_t incomplete, int32_t fail_warning, int32_t incomplete_warning)
{
    if (!can_place) {
        city_warning_show(fail_warning);
        return refuse(WARNING_ENTRANCE_ON_CLEAR_LAND);
    }
    if (incomplete) {
        return refuse(incomplete_warning);
    }
    return PLACE_CHECKED;
}

static int32_t check_royal_tomb_site(tomb_object *tomb, bool (*can_place)(tomb_object *, int32_t),
    bool (*has_incomplete)(tomb_object *))
{
    int32_t fits = can_place(tomb, map_info.hover_offset);
    int32_t incomplete = fits ? has_incomplete(tomb) : 0;
    return check_monument_object_site(fits, incomplete, WARNING_MUST_BE_OVER_CLIFFS, WARNING_ONE_CAESAREUM_ONLY);
}

static int32_t check_cliff_monument_site(int32_t type)
{
    switch (type) {
        case BUILDING_SMALL_ROYAL_TOMB:
            return check_royal_tomb_site(&small_royal_tomb_object.base, small_royal_tomb_can_place, small_royal_tomb_has_incomplete);
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return check_royal_tomb_site(&medium_royal_tomb_object.base, medium_royal_tomb_can_place, medium_royal_tomb_has_incomplete);
        case BUILDING_LARGE_ROYAL_TOMB:
            return check_royal_tomb_site(&large_royal_tomb_object.base, large_royal_tomb_can_place, large_royal_tomb_has_incomplete);
        case BUILDING_GRAND_ROYAL_TOMB:
            return check_royal_tomb_site(&grand_royal_tomb_object.base, grand_royal_tomb_can_place, grand_royal_tomb_has_incomplete);
        case BUILDING_LIGHTHOUSE: {
            int32_t fits = lighthouse_can_place(&lighthouse_object.base, map_info.hover_offset);
            int32_t incomplete = fits ? lighthouse_has_incomplete(&lighthouse_object.base) : 0;
            return check_monument_object_site(fits, incomplete, WARNING_MUST_BE_OVER_ROCKS, WARNING_ONE_PHAROS_ONLY);
        }
        case BUILDING_ABU_SIMBEL: {
            int32_t fits = abu_simbel_can_place(&abu_simbel_object.base, map_info.hover_offset);
            int32_t incomplete = fits ? abu_simbel_has_incomplete(&abu_simbel_object.base) : 0;
            return check_monument_object_site(fits, incomplete, WARNING_MUST_BE_OVER_CLIFFS, WARNING_ONE_PHAROS_ONLY);
        }
        default:
            return PLACE_CONTINUE;
    }
}

static int32_t take_material(int32_t resource, int32_t amount, int32_t warning, int32_t *materials_used)
{
    if (city_player_current()->storage_stored[resource] < amount) {
        return refuse_and_clear(warning);
    }
    building_storageyards_remove_resource(city_current_player, resource, amount);
    *materials_used += amount;
    return PLACE_CHECKED;
}

static int32_t check_materials(int32_t type)
{
    if (type == BUILDING_LIBRARY) {
        if (cheat_allow_all == 1) {
            return PLACE_CHECKED;
        }
        return take_material(RESOURCE_PAPYRUS, construction_place_rules[CONSTRUCTION_PLACE_RULE_LIBRARY_PAPYRUS], WARNING_NEED_500_PAPYRUS_FOR_LIBRARY,
            &undo_resources[RESOURCE_PAPYRUS]);
    }
    if (type == BUILDING_SMALL_OBELISK) {
        if (cheat_allow_all == 1) {
            return PLACE_CHECKED;
        }
        return take_material(RESOURCE_GRANITE, construction_place_rules[CONSTRUCTION_PLACE_RULE_SMALL_OBELISK_GRANITE],
            WARNING_NEED_100_BLOCKS_OF_GRANITE_FOR_OBELISK, &undo_resources[RESOURCE_GRANITE]);
    }
    if (type == BUILDING_LARGE_OBELISK) {
        if (cheat_allow_all == 1) {
            return PLACE_CHECKED;
        }
        return take_material(RESOURCE_GRANITE, construction_place_rules[CONSTRUCTION_PLACE_RULE_LARGE_OBELISK_GRANITE],
            WARNING_NEED_200_BLOCKS_OF_GRANITE_FOR_LARGE_OBELISK, &undo_resources[RESOURCE_GRANITE]);
    }
    return PLACE_CONTINUE;
}

static int32_t check_terrain_needs(int32_t type, int32_t x, int32_t y)
{
    if (type == BUILDING_GOLD_MINE) {
        if (construction_near_terrain_test != 0 &&
            !map_terrain_any_tile_in_radius(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, construction_near_terrain_test)) {
            return refuse(WARNING_BUILD_NEXT_TO_ROCKY_AREAS);
        }
        if (!city_buildings_has_palace()) {
            city_warning_show(WARNING_PALACE_REQUIRED_FOR_GOLD_CONVERSION);
        }
        return PLACE_CHECKED;
    }
    if (needs_farm_land != 0) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, FARM_SITE_SIZE, FARM_MEADOW_RADIUS, (TERRAIN_MEADOW | TERRAIN_FLOODPLAIN))) {
            return refuse(WARNING_BUILD_FARMS_ON_MEADOW);
        }
    } else if (construction_near_terrain_test != 0) {
        if (!map_terrain_any_tile_in_radius(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, construction_near_terrain_test)) {
            return refuse(WARNING_BUILD_NEXT_TO_ROCKY_AREAS);
        }
    } else if (needs_trees != 0) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, TERRAIN_TREE)) {
            return refuse(WARNING_BUILD_WOOD_CUTTERS_NEXT_TO_TREES);
        }
    } else if (needs_marsh != 0) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, RAW_MATERIAL_SITE_SIZE, RAW_MATERIAL_RADIUS, TERRAIN_MARSH)) {
            return refuse(WARNING_BUILD_REED_GATHERERS_NEAR_MARSH);
        }
    } else if (needs_water_near != 0) {
        if (!map_terrain_exists_tile_in_radius_with_type(x, y, RAW_MATERIAL_SITE_SIZE, CLAY_PIT_WATER_RADIUS, (TERRAIN_WATER | TERRAIN_FLOODPLAIN))) {
            return refuse(WARNING_BUILD_CLAY_PITS_CLOSE_TO_WATER);
        }
    } else if (needs_wall != 0) {
        if (!map_terrain_all_tiles_in_radius_are(x, y, TOWER_SIZE, 0, TERRAIN_WALL)) {
            return refuse(WARNING_BUILD_TOWERS_ON_THICK_WALLS);
        }
    } else if (needs_road != 0) {
        if (!map_terrain_all_tiles_in_radius_are(x, y, 1, 0, TERRAIN_ROAD)) {
            return refuse(WARNING_ONLY_BUILD_ROADBLOCKS_ON_ROADS);
        }
        if (map_grids.sprite[map_grid_offset(x, y)] != 0) {
            return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
        }
    } else if (needs_groundwater != 0) {
        if (!map_terrain_count_tiles_in_radius_reach(x, y, building_property_get(type)->size, 0, TERRAIN_GROUNDWATER, needs_groundwater)) {
            return refuse(WARNING_NEEDS_GROUNDWATER);
        }
    }
    return PLACE_CONTINUE;
}

static int32_t check_fort_site(int32_t x, int32_t y, uint32_t mask, uint8_t floodplain_ok)
{
    if (!map_building_tiles_shrub_fits(x + LEGION_STANDARD_FORT_DX, y + LEGION_STANDARD_FORT_DY, FORT_GROUND_SIZE, FORT_GROUND_SIZE, mask, 0, floodplain_ok)) {
        return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
    }
    if (forts_full()) {
        return refuse(WARNING_ARMY_HAS_FULL_COMPLEMENT_OF_FORTS);
    }
    return PLACE_CONTINUE;
}

static int32_t check_sphinx_site(int32_t x, int32_t y, int32_t y_start, uint32_t mask, uint8_t floodplain_ok)
{
    int32_t middle_x = x + SPHINX_PART_DX;
    int32_t right_x = x + (2 * SPHINX_PART_DX);
    if (city_player_current()->sphinx_stage != 0) {
        return refuse(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
    }
    if (!map_building_tiles_shrub_fits(middle_x, y, SPHINX_PART_DX, SPHINX_PART_DX, mask, 0, floodplain_ok) ||
        !map_building_tiles_shrub_fits(right_x, y, SPHINX_PART_DX, SPHINX_PART_DX, mask, 0, floodplain_ok)) {
        return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
    }
    if (construction_area_has_entry_point(middle_x, y, right_x - 1, y_start + SPHINX_PART_DX - 1) ||
        construction_area_has_entry_point(right_x, y, right_x + SPHINX_PART_DX - 1, y_start + SPHINX_PART_DX - 1)) {
        return refuse_and_clear(WARNING_CANNOT_BUILD_OVER_ANIMAL_BREEDING_GROUNDS);
    }
    return PLACE_CHECKED;
}

static int32_t check_sandstone_monument(int32_t resource_rule, int32_t warning)
{
    if (cheat_allow_all != 0) {
        return PLACE_CHECKED;
    }
    int32_t amount = construction_place_rules[resource_rule];
    if (city_player_current()->storage_stored[RESOURCE_SANDSTONE] < amount) {
        return refuse(warning);
    }
    building_storageyards_remove_resource(city_current_player, RESOURCE_SANDSTONE, amount);
    undo_resources[RESOURCE_SANDSTONE] += amount;
    return PLACE_CHECKED;
}

static int32_t check_great_monument_site(int32_t type, int32_t x_start, int32_t y_start)
{
    switch (type) {
        case BUILDING_SUN_TEMPLE:
            if (!sun_temple_tile_is_free(map_info.hover_offset)) {
                return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
            }
            if (sun_temple_any_completed()) {
                return refuse(WARNING_ONLY_ONE_SUN_TEMPLE_AT_A_TIME);
            }
            return check_sandstone_monument(CONSTRUCTION_PLACE_RULE_SUN_TEMPLE_SANDSTONE, WARNING_NEED_220_BLOCKS_OF_SANDSTONE_FOR_SUN_TEMPLE);
        case BUILDING_ALEXANDRIA_LIBRARY:
            if (!library_can_place(&library_object.base, map_info.hover_offset)) {
                return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
            }
            if (library_has_incomplete(&library_object.base)) {
                return refuse(WARNING_ONE_LIBRARY_ONLY);
            }
            return PLACE_CHECKED;
        case BUILDING_CAESAREUM:
            if (!caesareum_can_place(&caesareum_object.base, map_info.hover_offset)) {
                return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
            }
            if (caesareum_has_incomplete(&caesareum_object.base)) {
                return refuse(WARNING_ONE_CAESAREUM_ONLY);
            }
            return PLACE_CHECKED;
        case BUILDING_MAUSOLEUM:
            if (!mausoleum_can_place(map_grid_offset(x_start, y_start))) {
                return refuse(WARNING_MUST_BUILD_ON_CLEARED_LAND);
            }
            return check_sandstone_monument(CONSTRUCTION_PLACE_RULE_MAUSOLEUM_SANDSTONE, WARNING_BUILD_PAPYRUS_MAKER_OR_IMPORT);
        default:
            return PLACE_CONTINUE;
    }
}

static int32_t check_temple_complex_site(int32_t x, int32_t y)
{
    if (city_player_current()->has_temple_complex != 0) {
        return refuse_and_clear(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
    }
    int32_t site = construction_temple_complex_can_place(x, y);
    if (site == TEMPLE_COMPLEX_SITE_BLOCKED) {
        return refuse_and_clear(WARNING_MUST_BUILD_ON_CLEARED_LAND);
    }
    if (site == TEMPLE_COMPLEX_SITE_ENTRY_POINT) {
        return refuse_and_clear(WARNING_CANNOT_BUILD_OVER_ANIMAL_BREEDING_GROUNDS);
    }
    return PLACE_CONTINUE;
}

static int32_t check_temple_complex_annex(int32_t type)
{
    const city_player *player = city_player_current();
    int32_t main_id = building_main(map_grids.building_ids[map_info.hover_offset]);
    if (player->has_temple_complex == 0) {
        return refuse_and_clear(WARNING_MUST_BUILD_TEMPLE_COMPLEX_FIRST);
    }
    if (!building_type_is_temple_complex(building_get(main_id)->type)) {
        return refuse_and_clear(WARNING_ORACLE_ALTAR_PLACEMENT);
    }
    if (player->temple_complex_upgrades < TEMPLE_COMPLEX_UPGRADES_PLACED) {
        return refuse_and_clear(WARNING_MUST_HAVE_COMPLETED_TEMPLE_FIRST);
    }
    int32_t upgrade = (type == BUILDING_TEMPLE_COMPLEX_ALTAR) ? TEMPLE_COMPLEX_ALTAR : TEMPLE_COMPLEX_ORACLE;
    if (building_temple_complex_has_upgrade(player->temple_complex_building_id, upgrade)) {
        return refuse_and_clear(WARNING_TEMPLE_COMPLEX_LIMITS);
    }
    return PLACE_CHECKED;
}

static int32_t is_palace_upgrade(int32_t type)
{
    return type == BUILDING_CITY_PALACE || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_VILLAGE_PALACE_UP;
}

static int32_t is_mansion(int32_t type)
{
    return type == BUILDING_DYNASTY_MANSION || type == BUILDING_FAMILY_MANSION || type == BUILDING_PERSONAL_MANSION;
}

static int32_t check_unique_building(int32_t type, int32_t x, int32_t y)
{
    const city_player *player = city_player_current();
    if (is_palace_upgrade(type) && player->palace_placed != 0) {
        return refuse_and_clear(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
    }
    if (is_mansion(type) && player->mansion_placed != 0) {
        return refuse_and_clear(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
    }
    switch (type) {
        case BUILDING_TAX_COLLECTOR:
            if (player->palace_placed == 0) {
                return refuse_and_clear(WARNING_NO_TAXES_WITHOUT_PALACE);
            }
            return PLACE_CHECKED;
        case BUILDING_RECRUITER:
            if (building_count_total[BUILDING_RECRUITER] > 0) {
                return refuse_and_clear(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
            }
            building_count_total[BUILDING_RECRUITER] = 1;
            return PLACE_CHECKED;
        case BUILDING_MILITARY_ACADEMY:
            if (player->military_academy_placed != 0) {
                return refuse_and_clear(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
            }
            return PLACE_CHECKED;
        case BUILDING_PYRAMID: {
            int32_t reason = 0;
            if (!pyramid_can_place(map_grid_offset(x, y), pyramid_layout.size_being_placed, &reason)) {
                return refuse((reason == PYRAMID_PLACE_COMPLEX_TEMPLE_BLOCKED) ? WARNING_CAUSEWAY_NEEDS_WATER : WARNING_MUST_BE_ON_CLEAR_LAND);
            }
            return PLACE_CHECKED;
        }
        default:
            return PLACE_CHECKED;
    }
}

static int32_t check_special_sites(int32_t type, int32_t x, int32_t y, int32_t x_start, int32_t y_start, uint32_t mask, uint8_t floodplain_ok)
{
    if (is_fort(type) && check_fort_site(x, y, mask, floodplain_ok) == PLACE_RETURN) {
        return PLACE_RETURN;
    }
    if (type == BUILDING_SPHINX) {
        return check_sphinx_site(x, y, y_start, mask, floodplain_ok);
    }
    int32_t result = check_great_monument_site(type, x_start, y_start);
    if (result != PLACE_CONTINUE) {
        return result;
    }
    if (building_type_is_temple_complex(type) && check_temple_complex_site(x, y) == PLACE_RETURN) {
        return PLACE_RETURN;
    }
    if (type == BUILDING_TEMPLE_COMPLEX_ALTAR || type == BUILDING_TEMPLE_COMPLEX_ORACLE) {
        return check_temple_complex_annex(type);
    }
    return check_unique_building(type, x, y);
}

static int32_t check_building(int32_t player, int32_t *type_io, int32_t x, int32_t y, int32_t x_start, int32_t y_start, int32_t size, uint32_t mask,
    uint8_t floodplain_ok, int32_t *building_id)
{
    int32_t result = check_water_site(player, type_io, x, y, building_id);
    if (result != PLACE_CONTINUE) {
        return result;
    }
    int32_t type = *type_io;
    result = check_cliff_monument_site(type);
    if (result != PLACE_CONTINUE) {
        return result;
    }
    if (!map_building_tiles_shrub_fits(x, y, size, size, mask, 0, floodplain_ok)) {
        return refuse_and_clear(WARNING_MUST_BUILD_ON_CLEARED_LAND);
    }
    result = check_materials(type);
    if (result != PLACE_CONTINUE) {
        return result;
    }
    result = check_terrain_needs(type, x, y);
    if (result != PLACE_CONTINUE) {
        return result;
    }
    return check_special_sites(type, x, y, x_start, y_start, mask, floodplain_ok);
}

static int32_t refuse_over_breeding_grounds(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    if (construction_area_has_entry_point(x0, y0, x1, y1)) {
        fail(WARNING_CANNOT_BUILD_OVER_ANIMAL_BREEDING_GROUNDS);
        return 1;
    }
    return 0;
}

static int32_t place_venue(int32_t type, int32_t site_size, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end)
{
    if (refuse_over_breeding_grounds(x_start, y_start, x_start + site_size - 1, y_start + site_size - 1)) {
        return 0;
    }
    int32_t placed;
    switch (type) {
        case BUILDING_BOOTH:
            placed = construction_place_booth(0, x_start, y_start, x_end, y_end);
            break;
        case BUILDING_BANDSTAND:
            placed = construction_place_bandstand(0, x_start, y_start, x_end, y_end);
            break;
        default:
            placed = construction_place_pavilion(0, x_start, y_start, x_end, y_end);
            break;
    }
    if (!placed) {
        if (type == BUILDING_BOOTH) {
            clear_marks();
        }
        city_warning_show(WARNING_ENTERTAINMENT_VENUE_AT_INTERSECTION);
        return 0;
    }
    construction_warning_check_venue_schools(type);
    return 1;
}

uint32_t construction_site_mask(int32_t type)
{
    switch (type) {
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_GATEHOUSE_UP:
        case BUILDING_TRIUMPHAL_ARCH:
            return ~(uint32_t) TERRAIN_ROAD;
        case BUILDING_MUD_TOWER:
        case BUILDING_BRICK_TOWER:
        case BUILDING_TOWER:
            return ~(uint32_t) TERRAIN_WALL_KINDS;
        case BUILDING_ROADBLOCK:
            return TERRAIN_BUILDING;
        default:
            return UINT32_MAX;
    }
}

static void turn_origin_to_view(int32_t orientation, int32_t size, int32_t *x, int32_t *y)
{
    switch (orientation) {
        case VIEW_ORIENTATION_WEST:
            *y += 1 - size;
            break;
        case VIEW_ORIENTATION_SOUTH:
            *x += 1 - size;
            *y += 1 - size;
            break;
        case VIEW_ORIENTATION_EAST:
            *x += 1 - size;
            break;
        default:
            break;
    }
}

static int32_t monument_record_count(int32_t type)
{
    switch (type) {
        case BUILDING_SUN_TEMPLE:
            return SUN_TEMPLE_PARTS;
        case BUILDING_ABU_SIMBEL:
            return ABU_SIMBEL_PARTS;
        case BUILDING_ALEXANDRIA_LIBRARY:
            return LIBRARY_PARTS;
        case BUILDING_CAESAREUM:
            return CAESAREUM_PARTS;
        case BUILDING_SMALL_ROYAL_TOMB:
            return SMALL_TOMB_PARTS;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return MEDIUM_TOMB_PARTS;
        case BUILDING_LARGE_ROYAL_TOMB:
            return LARGE_TOMB_PARTS;
        case BUILDING_GRAND_ROYAL_TOMB:
            return GRAND_TOMB_PARTS;
        case BUILDING_LIGHTHOUSE:
            return LIGHTHOUSE_PARTS;
        case BUILDING_MAUSOLEUM:
            return MAUSOLEUM_PARTS;
        default:
            return 0;
    }
}

static int32_t create_checked_building(int32_t player, int32_t type, int32_t x, int32_t y, int32_t building_id)
{
    if (is_fort(type)) {
        return building_create(player, BUILDING_FORT, x, y, 0, 0);
    }
    if (type == BUILDING_PYRAMID) {
        return building_create(player, BUILDING_PYRAMID, x, y, PYRAMID_MAX_PARTS, 0);
    }
    if (type == BUILDING_WATER_LIFT) {
        return building_id;
    }
    return building_create(player, type, x, y, monument_record_count(type), 0);
}

static int32_t place_site_building(int32_t player, int32_t orientation, int32_t x_start, int32_t y_start, int32_t *type_io, int32_t *x, int32_t *y)
{
    int32_t type = *type_io;
    int32_t building_id = 0;
    uint8_t floodplain_ok = construction_allowed_on_floodplain(type);
    uint32_t mask = construction_site_mask(type);
    construction_size = building_property_get(type)->size;
    int32_t size = (type == BUILDING_STORAGE_YARD) ? STORAGE_YARD_SIZE : construction_size;
    if (refuse_over_breeding_grounds(x_start, y_start, size - 1 + x_start, size - 1 + y_start)) {
        return 0;
    }
    if (type != BUILDING_GATEHOUSE && type != BUILDING_GATEHOUSE_UP) {
        turn_origin_to_view(orientation, size, x, y);
    }
    if (check_building(player, &type, *x, *y, x_start, y_start, size, mask, floodplain_ok, &building_id) != PLACE_CHECKED) {
        return 0;
    }
    *type_io = type;
    run_warnings(type, *x, *y, size);
    construction_hover_locked = 0;
    building_id = create_checked_building(player, type, *x, *y, building_id);
    if (building_id == 0) {
        return 0;
    }
    if (monument_record_count(type) == 0) {
        game_undo_add_building(building_id);
    }
    if (building_id < 1) {
        return 0;
    }
    building_construction_add_to_map(player, type, building_id, *x, *y, construction_size, 1, 0);
    if (type == BUILDING_FERRY_LANDING) {
        construction_ferry_landing_placed(building_id);
    }
    return 1;
}

static void undo_preview_near_enemy(int32_t type)
{
    switch (type) {
        case BUILDING_MUD_WALL:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
        case BUILDING_ROAD:
        case BUILDING_IRRIGATION_DITCH:
            restore_terrain_and_canals_from_backup();
            map_images_restore_free_tiles();
            break;
        case BUILDING_PLAZA:
        case BUILDING_GARDENS:
            restore_plaza_grids_from_backup();
            map_images_restore_free_tiles();
            break;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            construction_bridge_length = 0;
            break;
        default:
            clear_marks();
            break;
    }
}

static int32_t place_tool(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type)
{
    switch (type) {
        case BUILDING_CLEAR_LAND:
            building_construction_clear_land_tool(0, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_MUD_WALL:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
            if (refuse_over_breeding_grounds(x_start, y_start, x_end, y_end)) {
                return UPDATE_ABORTED;
            }
            building_construction_place_wall(0, x_start, y_start, x_end, y_end, wall_terrain(type));
            return UPDATE_ROUTED;
        case BUILDING_ROAD:
            if (refuse_over_breeding_grounds(x_start, y_start, x_end, y_end)) {
                return UPDATE_ABORTED;
            }
            building_construction_place_road(0, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_PLAZA:
            building_construction_place_plaza(x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_BOOTH:
            return place_venue(type, BOOTH_SITE_SIZE, x_start, y_start, x_end, y_end) ? UPDATE_DONE : UPDATE_ABORTED;
        case BUILDING_BANDSTAND:
            return place_venue(type, BANDSTAND_SITE_SIZE, x_start, y_start, x_end, y_end) ? UPDATE_DONE : UPDATE_ABORTED;
        case BUILDING_PAVILION:
            return place_venue(type, PAVILION_SITE_SIZE, x_start, y_start, x_end, y_end) ? UPDATE_DONE : UPDATE_ABORTED;
        case BUILDING_FESTIVAL_SQUARE:
            if (city_player_current()->festival_square_placed != 0) {
                city_warning_show(WARNING_ONLY_ONE_BUILDING_OF_THIS_TYPE);
                return UPDATE_ABORTED;
            }
            if (refuse_over_breeding_grounds(x_start, y_start, x_start + FESTIVAL_SQUARE_SITE_SIZE - 1, y_start + FESTIVAL_SQUARE_SITE_SIZE - 1)) {
                return UPDATE_ABORTED;
            }
            if (!construction_place_festival_square(0, x_start, y_start, x_end, y_end)) {
                city_warning_show(WARNING_FESTIVAL_SQUARE_ON_INTERSECTION);
                return UPDATE_ABORTED;
            }
            return UPDATE_DONE;
        case BUILDING_GARDENS:
            if (refuse_over_breeding_grounds(x_start, y_start, x_end, y_end)) {
                return UPDATE_ABORTED;
            }
            building_construction_place_garden(x_start, y_start, x_end, y_end);
            construction_place_cost = routed_building_items_placed * construction_place_cost;
            map_routing_update_land_citizen_rect(x_end - construction_size, y_end - construction_size, construction_size + x_end, construction_size + y_end, 1);
            map_routing_update_land_noncitizen_rect(x_end - construction_size, y_end - construction_size, construction_size + x_end, construction_size + y_end);
            return UPDATE_DONE;
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
            map_bridge_add(x_end, y_end, type == BUILDING_SHIP_BRIDGE);
            if (construction_bridge_length < 2) {
                city_warning_show(WARNING_SCOUT_ALONG_RIVER_FOR_SITE);
                return UPDATE_ABORTED;
            }
            construction_place_cost = construction_bridge_length * construction_place_cost;
            return UPDATE_DONE;
        case BUILDING_IRRIGATION_DITCH:
            building_construction_place_canal(0, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        case BUILDING_HOUSE_CRUDE_HUT:
            if (refuse_over_breeding_grounds(x_start, y_start, x_end, y_end)) {
                return UPDATE_ABORTED;
            }
            building_construction_place_houses(0, x_start, y_start, x_end, y_end);
            return UPDATE_ROUTED;
        default:
            return UPDATE_NOT_HANDLED;
    }
}

void building_construction_place(int32_t player, int32_t orientation, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, int32_t type)
{
    city_warning_shown = 0;
    construction_place_cost = 0;
    construction_cost_refund = 0;
    if (type == BUILDING_NONE) {
        return;
    }
    if (over_credit_limit()) {
        clear_marks();
        city_warning_show(WARNING_OUT_OF_CREDIT);
        return;
    }
    int32_t x = x_end;
    int32_t y = y_end;
    if (type != BUILDING_CLEAR_LAND && figure_enemy_near_building_site(x_start, y_start, x_end, y_end)) {
        undo_preview_near_enemy(type);
        city_warning_show(WARNING_TOO_CLOSE_TO_ENEMY_TROOPS);
        return;
    }
    if ((type == BUILDING_SMALL_OBELISK || type == BUILDING_LARGE_OBELISK) && obelisk_find_incomplete() > 0) {
        fail(WARNING_ONLY_ONE_OBELISK_AT_A_TIME);
        return;
    }
    construction_place_cost = building_model_get(type)->cost;
    int32_t result = place_tool(x_start, y_start, x_end, y_end, type);
    if (result == UPDATE_ABORTED) {
        return;
    }
    if (result == UPDATE_NOT_HANDLED) {
        if (!place_site_building(player, orientation, x_start, y_start, &type, &x, &y)) {
            return;
        }
    }
    if (result == UPDATE_ROUTED) {
        construction_place_cost = routed_building_items_placed * construction_place_cost;
    }
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
    map_routing_update_walls();
    formation_herds_scared_by_building(x, y);
    int32_t cost = construction_place_cost;
    if (type == BUILDING_FERRY_LANDING && construction_ferry_state == FERRY_PLACEMENT_NONE) {
        cost = 0;
    }
    construction_place_cost = cost - construction_cost_refund;
    city_finance_process_construction(player, construction_place_cost);
    if (type != BUILDING_TRIUMPHAL_ARCH && type != BUILDING_CLEAR_LAND) {
        game_undo_finish_build();
    }
    city_rebuild_monument_slots();
}
