// The build tool's entry points from the city view: choosing a build menu entry (a submenu, an editor tool with
// its pseudo type, or a building type with its placement flags and orientation) and the mouse down, move and up
// handlers; and the number of the scenario's allowed buildings, which the editor lists.

#include "building/construction_ui.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "building/ferry_landing.h"
#include "building/monument_data.h"
#include "building/sphinx.h"
#include "city/city_player.h"
#include "editor/editor_data.h"
#include "editor/editor_terrain_tool.h"
#include "game/undo_build.h"
#include "map/grid.h"
#include "map/routing_distance.h"
#include "map/routing_terrain.h"
#include "widget/widget_data.h"
#include "window/build_menu.h"
#include "window/building_menu.h"
#include "window/window_data.h"
#include "window/window_id.h"

uint8_t construction_dragged;

int32_t construction_rotation_saved;

static void reset_drag(void)
{
    construction_in_progress = 0;
    construction_end_x = 0;
    construction_start_x = 0;
    construction_end_y = 0;
    construction_start_y = 0;
    window_redraw_request = 1;
}

static void type_flags_generic(int32_t type)
{
    if (type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_GOD_9) {
        construction_orientation = 1;
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        construction_orientation = 1;
    } else if (type == BUILDING_MUD_TOWER || type == BUILDING_BRICK_TOWER || type == BUILDING_TOWER) {
        needs_wall = 1;
    } else if (type == BUILDING_ROADBLOCK) {
        needs_road = 1;
    } else if (type == BUILDING_WELL || type == BUILDING_WATER_SUPPLY || type == BUILDING_PERSONAL_MANSION || type == BUILDING_FAMILY_MANSION ||
        type == BUILDING_DYNASTY_MANSION || type == BUILDING_VILLAGE_PALACE_UP || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_CITY_PALACE) {
        needs_groundwater = 1;
    } else if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        construction_orientation = map_info.orientation;
    } else if (type >= BUILDING_SMALL_STATUE && type <= BUILDING_LARGE_STATUE) {
        construction_orientation = 0;
    }
}

static int32_t submenu_group(int32_t type)
{
    switch (type) {
        case BUILDING_MENU_FARMS:
            return BUILD_MENU_FARMS;
        case BUILDING_MENU_RAW_MATERIALS:
            return BUILD_MENU_RAW_MATERIALS;
        case BUILDING_MENU_CONSTRUCTION_GUILDS:
            return BUILD_MENU_GUILDS;
        case BUILDING_MENU_MONUMENTS:
            return BUILD_MENU_MONUMENTS;
        case BUILDING_MENU_WATER_CROSSINGS:
            return BUILD_MENU_WATER_CROSSINGS;
        case BUILDING_MENU_BEAUTIFICATION:
            return BUILD_MENU_BEAUTIFICATION;
        case BUILDING_MENU_MUD_STRUCTURES:
            return BUILD_MENU_MUD_STRUCTURES;
        case BUILDING_MENU_BRICK_STRUCTURES:
            return BUILD_MENU_BRICK_STRUCTURES;
        case BUILDING_MENU_DEFENSES:
            return BUILD_MENU_DEFENSES;
        case BUILDING_MENU_TEMPLES:
            return BUILD_MENU_TEMPLES;
        case BUILDING_MENU_TEMPLE_COMPLEXES:
            return (city_player_current()->has_temple_complex != 0) ? BUILD_MENU_TEMPLE_COMPLEX_UPGRADES : BUILD_MENU_TEMPLE_COMPLEXES;
        case BUILDING_MENU_SHRINES:
            return BUILD_MENU_SHRINES;
        case BUILDING_FORT:
            return BUILD_MENU_FORTS;
        default:
            return 0;
    }
}

static int32_t type_flags(int32_t type)
{
    int32_t submenu = submenu_group(type);
    if (submenu) {
        build_menu_group = submenu;
        int32_t count = building_menu_count_items();
        build_menu_item_count = count;
        build_menu_y_offset = build_menu_types[BUILD_MENU_SUB_POSITIONS_FIRST + count * 2];
        construction_building_type = BUILDING_NONE;
        return 0;
    }
    switch (type) {
        case BUILDING_GRAIN_FARM:
        case BUILDING_BARLEY_FARM:
        case BUILDING_LETTUCE_FARM:
        case BUILDING_POMEGRANATES_FARM:
        case BUILDING_FLAX_FARM:
        case BUILDING_HENNA_FARM:
        case BUILDING_CHICKPEAS_FARM:
        case BUILDING_FIG_FARM:
            needs_farm_land = 1;
            break;
        case BUILDING_CATTLE_RANCH:
            needs_farm_land = 0;
            break;
        case BUILDING_STONE_QUARRY:
            construction_near_terrain_test = build_flags_has_bit1;
            break;
        case BUILDING_SANDSTONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_GRANITE_QUARRY:
        case BUILDING_GEMSTONE_MINE:
            construction_near_terrain_test = build_flags_is_kind_b;
            break;
        case BUILDING_GOLD_MINE:
        case BUILDING_COPPER_MINE:
            construction_near_terrain_test = build_flags_is_kind_a;
            break;
        case BUILDING_CLAY_PIT:
            needs_water_near = 1;
            break;
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_GATEHOUSE_UP:
            construction_orientation = 1;
            break;
        default:
            type_flags_generic(type);
            break;
    }
    return 1;
}

static void select_editor_tool(int32_t tool, int32_t type)
{
    editor_tool_kind = tool;
    construction_building_type = type;
}

static void select_water_tool(int32_t item)
{
    switch (item) {
        case 1:
            select_editor_tool(EDITOR_TOOL_WATER, BUILDING_EDITOR_WATER);
            break;
        case 2:
            select_editor_tool(EDITOR_TOOL_FLOODPLAIN, BUILDING_EDITOR_FLOODPLAIN);
            break;
        case 3:
            select_editor_tool(EDITOR_TOOL_MARSHLAND, BUILDING_MARSHLAND);
            break;
        case 4:
            select_editor_tool(EDITOR_TOOL_IRRIGATION, BUILDING_IRRIGATION_DITCH);
            break;
    }
}

static void select_special_point_tool(int32_t item)
{
    if (item <= SPECIAL_POINT_MENU_LAST_FISHING) {
        select_editor_tool(EDITOR_TOOL_FISHING_WATERS, BUILDING_EDITOR_FISHING_WATERS);
        editor_fishing_point_number = item;
    } else if (item <= SPECIAL_POINT_MENU_LAST_KILLER) {
        select_editor_tool(EDITOR_TOOL_KILLER_POINT, BUILDING_EDITOR_KILLER_POINT);
        editor_herd_point_number = item - SPECIAL_POINT_MENU_LAST_FISHING;
    } else if (item <= SPECIAL_POINT_MENU_LAST_PREY) {
        select_editor_tool(EDITOR_TOOL_PREY, BUILDING_EDITOR_PREY);
        editor_herd_point_number = item - SPECIAL_POINT_MENU_LAST_KILLER;
    } else {
        select_editor_tool(EDITOR_TOOL_DISEMBARK_POINT, BUILDING_DISEMBARK_POINT);
        editor_landing_point_number = item - SPECIAL_POINT_MENU_LAST_PREY;
    }
}

static int32_t editor_menu_select(int32_t group, int32_t item)
{
    switch (group) {
        case BUILD_MENU_EDITOR_BRUSH_SIZE:
            editor_brush_size = item - 1;
            return 1;
        case BUILD_MENU_EDITOR_ANIMALS:
            scenario_data.predator_variant = (int16_t) (item - 1);
            return 1;
        case BUILD_MENU_EDITOR_WATER:
            select_water_tool(item);
            return 1;
        case BUILD_MENU_EDITOR_ENTRY_EXIT_POINTS:
            editor_tool_variant = item;
            if (item == 1) {
                select_editor_tool(EDITOR_TOOL_ENTRY_POINT, BUILDING_EDITOR_ENTRY_POINT);
            } else if (item == 2) {
                select_editor_tool(EDITOR_TOOL_EXIT_POINT, BUILDING_EDITOR_EXIT_POINT);
            }
            return 1;
        case BUILD_MENU_EDITOR_RIVER_POINTS:
            if (item == 1) {
                select_editor_tool(EDITOR_TOOL_RIVER_IN, BUILDING_EDITOR_RIVER_IN);
            } else if (item == 2) {
                select_editor_tool(EDITOR_TOOL_RIVER_OUT, BUILDING_EDITOR_RIVER_OUT);
            }
            return 1;
        case BUILD_MENU_EDITOR_NATIVES:
            if (item == 1) {
                select_editor_tool(EDITOR_TOOL_NATIVE_HUT, BUILDING_NATIVE_HUT);
            } else if (item == 2) {
                select_editor_tool(EDITOR_TOOL_NATIVE_CENTER, BUILDING_NATIVE_MEETING);
            } else if (item == 3) {
                select_editor_tool(EDITOR_TOOL_NATIVE_FIELD, BUILDING_NATIVE_CROPS);
            }
            return 1;
        case BUILD_MENU_EDITOR_SPECIAL_POINTS:
            select_special_point_tool(item);
            return 1;
        case BUILD_MENU_EDITOR_INVASION_POINTS:
            select_editor_tool(EDITOR_TOOL_INVASION_POINT, BUILDING_EDITOR_INVASION_POINT);
            editor_invasion_point_number = item;
            return 1;
        case BUILD_MENU_EDITOR_ELEVATION:
            if (item == 1) {
                select_editor_tool(EDITOR_TOOL_RAISE_LAND, BUILDING_EDITOR_RAISE_LAND);
            } else if (item == 2) {
                select_editor_tool(EDITOR_TOOL_LOWER_LAND, BUILDING_EDITOR_LOWER_LAND);
            } else if (item == 3) {
                select_editor_tool(EDITOR_TOOL_ACCESS_RAMP, BUILDING_EDITOR_ACCESS_RAMP);
            }
            return 1;
        case BUILD_MENU_EDITOR_ROCKS:
            construction_building_type = build_menu_types[item];
            if (construction_building_type == BUILDING_SAND_DUNES) {
                editor_tool_kind = EDITOR_TOOL_SAND_DUNES;
            } else {
                editor_tool_kind = (construction_building_type != BUILDING_CLIFF) ? EDITOR_TOOL_ROCKS : EDITOR_TOOL_CLIFF;
            }
            return 1;
        default:
            return 0;
    }
}

static int32_t monument_with_generic_flags(int32_t subkind)
{
    switch (subkind) {
        case MONUMENT_DEF_SUN_TEMPLE:
            return BUILDING_SUN_TEMPLE;
        case MONUMENT_DEF_LIBRARY:
            return BUILDING_ALEXANDRIA_LIBRARY;
        case MONUMENT_DEF_ABU_SIMBEL:
            return BUILDING_ABU_SIMBEL;
        case MONUMENT_DEF_CAESAREUM:
            return BUILDING_CAESAREUM;
        case MONUMENT_DEF_SMALL_ROYAL_TOMB:
            return BUILDING_SMALL_ROYAL_TOMB;
        case MONUMENT_DEF_MEDIUM_ROYAL_TOMB:
            return BUILDING_MEDIUM_ROYAL_TOMB;
        case MONUMENT_DEF_LARGE_ROYAL_TOMB:
            return BUILDING_LARGE_ROYAL_TOMB;
        case MONUMENT_DEF_GRAND_ROYAL_TOMB:
            return BUILDING_GRAND_ROYAL_TOMB;
        case MONUMENT_DEF_LIGHTHOUSE:
            return BUILDING_LIGHTHOUSE;
        case MONUMENT_DEF_MAUSOLEUM:
            return BUILDING_MAUSOLEUM;
        default:
            return BUILDING_NONE;
    }
}

static int32_t monument_menu_select(int32_t item)
{
    const monument_def *def = &monument_defs[scenario_monument_menu_def(item)];
    if (def->kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY) {
        construction_building_type = BUILDING_PYRAMID;
        pyramid_layout.size_being_placed = def->size;
        type_flags_generic(BUILDING_PYRAMID);
        return 1;
    }
    int32_t generic = monument_with_generic_flags(def->subkind);
    if (generic != BUILDING_NONE) {
        construction_building_type = generic;
        type_flags_generic(generic);
        return 1;
    }
    if (def->subkind == MONUMENT_DEF_SPHINX) {
        city_player *player = city_player_current();
        int16_t stage = player->sphinx_stage;
        construction_building_type = BUILDING_SPHINX;
        player->sphinx_stage = SPHINX_MENU_STAGE;
        sphinx_load_stage_scripts();
        player->sphinx_stage = stage;
    } else if (def->subkind == MONUMENT_DEF_OBELISK) {
        construction_building_type = BUILDING_SMALL_OBELISK + def->size;
        trade_goods_list.list.beauty_variant = item;
    }
    return 0;
}

void build_menu_set_type(void)
{
    if (construction_ferry_state == FERRY_PLACEMENT_FIRST_PLACED) {
        construction_ferry_cancel(construction_ferry_first);
    }
    if (construction_ferry_state == FERRY_PLACEMENT_FIRST_EXISTING) {
        construction_ferry_state = FERRY_PLACEMENT_NONE;
    }
    int32_t group = build_menu_group;
    int32_t item = button_parameter;
    needs_groundwater = 0;
    needs_road = 0;
    needs_wall = 0;
    needs_water_near = 0;
    needs_trees = 0;
    needs_farm_land = 0;
    needs_marsh = 0;
    construction_near_terrain_test = 0;
    construction_rotation_saved = time_current_millis;
    construction_orientation = 0;
    map_info.hover_offset = 0;
    map_info.selected_grid = 0;
    construction_hover_locked = 0;
    if (editor_menu_select(group, item)) {
        window_current_id = WINDOW_CITY;
        reset_drag();
        return;
    }
    int32_t flags_set;
    if (group == BUILD_MENU_MONUMENTS) {
        flags_set = monument_menu_select(item);
    } else {
        construction_building_type = build_menu_types[(group * BUILD_MENU_ITEMS) + item];
        flags_set = 0;
    }
    if (flags_set || type_flags(construction_building_type)) {
        window_current_id = WINDOW_CITY;
    }
    reset_drag();
}

void building_construction_start(void)
{
    if (map_info.hover_offset == 0 || settings_data.paused) {
        return;
    }
    construction_start_y = map_info.hover_y;
    construction_end_y = map_info.hover_y;
    construction_start_x = map_info.hover_x;
    construction_end_x = map_info.hover_x;
    construction_dragged = 0;
    construction_start_grid_offset = map_grid_offset(map_info.hover_x, map_info.hover_y);
    construction_start_elevation = map_grids.elevation[construction_start_grid_offset];
    if ((construction_ferry_state != FERRY_PLACEMENT_NONE || construction_building_type != BUILDING_FERRY_LANDING) && game_undo_backup_map() == 0) {
        return;
    }
    construction_in_progress = 1;
    switch (construction_building_type) {
        case BUILDING_ROAD:
            map_routing_calculate_distances_for_building(construction_start_x, construction_start_y, ROUTED_MODE_ROAD);
            break;
        case BUILDING_MUD_WALL:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
            map_routing_calculate_distances_for_wall(construction_start_x, construction_start_y);
            break;
        case BUILDING_IRRIGATION_DITCH:
        case BUILDING_MENU_WATER_LIFT:
            map_routing_calculate_distances_for_building(construction_start_x, construction_start_y, ROUTED_MODE_CANAL);
            break;
    }
}

void widget_city_build_move(void)
{
    if (construction_in_progress == 0 || map_info.hover_offset == 0) {
        return;
    }
    construction_dragged = 1;
    if (!construction_hover_locked) {
        map_info.last_hover = map_info.hover_offset;
        construction_end_y = map_info.hover_y;
        construction_end_x = map_info.hover_x;
    }
    int32_t type = construction_building_type;
    if (editor_active == 0 || type == BUILDING_ROAD || type == BUILDING_NATIVE_HUT || type == BUILDING_NATIVE_MEETING || type == BUILDING_NATIVE_CROPS) {
        building_construction_update(construction_start_x, construction_start_y, construction_end_x, construction_end_y, type);
        return;
    }
    editor_tool_update_use(construction_end_x, construction_end_y);
}

void widget_city_build_end(void)
{
    if (editor_active == 1) {
        map_routing_update_water();
    }
    int32_t type = construction_building_type;
    if (type == BUILDING_EDITOR_RAISE_LAND || type == BUILDING_EDITOR_LOWER_LAND || type == BUILDING_EDITOR_ACCESS_RAMP) {
        editor_elevation_smooth_all();
        construction_in_progress = 0;
        return;
    }
    if (construction_in_progress == 0) {
        return;
    }
    construction_in_progress = 0;
    if (map_info.selected_grid == 0) {
        if (!construction_dragged) {
            return;
        }
        map_info.selected_grid = map_info.last_hover;
    }
    building_construction_place(city_current_player, map_info.orientation, construction_start_x, construction_start_y, construction_end_x,
        construction_end_y, construction_building_type);
}
