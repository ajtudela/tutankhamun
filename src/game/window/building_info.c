// The building info window (window 9): opening it on the selected tile (the building there, the figures on the tile
// and its neighbours, or the kind of a tile without a building), the dispatch to the panel of the building's kind,
// the figure list and figure panel, the window's own buttons and input, and its place on the screen.
//
// Opening the window keeps some state from before:
// - a dune or fault tile that matches no test keeps the terrain kind of the last window;
// - the figure list holds at most seven figures;
// - the building's sound plays only on the window's first draw.

#include "window/building_info.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_state.h"
#include "building/building_stocks.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/house_evolve.h"
#include "building/house_evolve_checks.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "city/city_data.h"
#include "city/gods.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "city/resources.h"
#include "core/calc.h"
#include "empire/empire.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_phrase.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/formation_orders.h"
#include "figure/trade.h"
#include "game/game_data.h"
#include "graphics/arrow_buttons.h"
#include "graphics/generic_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/layout_buttons.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "input/input_data.h"
#include "io/io_data.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "scenario/scenario_climate.h"
#include "sound/city_channels.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "text/lang.h"
#include "widget/city_draw_view.h"
#include "widget/city_figure.h"
#include "widget/city_view.h"
#include "widget/city_view_rotate.h"
#include "widget/city_viewport.h"
#include "widget/screen_layout.h"
#include "widget/widget_data.h"
#include "window/building_info_advisors.h"
#include "window/building_info_education.h"
#include "window/building_info_entertainment.h"
#include "window/building_info_government.h"
#include "window/building_info_health.h"
#include "window/building_info_house.h"
#include "window/building_info_industry.h"
#include "window/building_info_military.h"
#include "window/building_info_monuments.h"
#include "window/building_info_municipal.h"
#include "window/building_info_natives.h"
#include "window/building_info_panels.h"
#include "window/building_info_religion.h"
#include "window/building_info_ships.h"
#include "window/building_info_storage.h"
#include "window/building_info_storage_orders.h"
#include "window/building_info_terrain.h"
#include "window/message_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t bi_figure_drawn;

building_info_tile bi_tile_block;

enum building_info_image_group {
    IMAGE_GROUP_BIG_PEOPLE_2 = 26,
    IMAGE_GROUP_BIG_PEOPLE_3 = 27,
    IMAGE_GROUP_BIG_PEOPLE_4 = 711,
};

enum big_people_2_image {
    BIG_OSTRICH_HUNTER = 0,
};

enum big_people_3_image {
    BIG_BIRD_HUNTER = 0,
    BIG_PRIESTESS_BAST = 1,
    BIG_PRIEST_PTAH = 2,
    BIG_PRIEST_RA = 3,
    BIG_PRIEST_SETH = 4,
};

enum big_people_4_image {
    BIG_ZOOKEEPER = 0,
    BIG_TOMB_ARTISAN = 1,
    BIG_SCORPION = 2,
    BIG_TOMB_ROBBER = 3,
    BIG_ASP = 4,
    BIG_LION = 5,
    BIG_FROG = 6,
    BIG_LOCUST = 7,
    BIG_MUMMY = 8,
};

enum building_info_window_layout {
    BLOCK_SIZE = 16,
    WINDOW_X = 8,
    WINDOW_Y = 32,
    WINDOW_WIDTH_BLOCKS = 29,
    SCREEN_LAYOUT_BOTTOM_PLACEMENT_HEIGHT = 600,
    SCREEN_LAYOUT_LOW_MOUSE_EXTRA = 12,
    SCREEN_LAYOUT_BOTTOM_MARGIN = 8,
    BOTTOM_BUTTONS_FROM_BOTTOM = 40,
    TITLE_Y = 8,
    HINT_FROM_BOTTOM = 22,
    PHRASE_BUTTON_X = 285,
    PHRASE_BUTTON_Y = 195,
    PHRASE_BUTTON_WIDTH = 150,
    PHRASE_BUTTON_HEIGHT = 25,
    PHRASE_BUTTON_TEXT_Y = 200,
    SPECIAL_ORDERS_X = 250,
    SPECIAL_ORDERS_Y = 78,
    ORDERS_BUTTON_X = 100,
    ORDERS_BUTTON_FROM_BOTTOM = 34,
    BACK_BUTTON_X = 80,
    BAZAAR_BACK_BUTTON_Y = 436,
    HOUSE_VENTURES_X = 112,
    HOUSE_VENTURES_FROM_BOTTOM_BLOCKS = 2,
    LEGION_RETURN_WIDTH_BLOCKS = 20,
    ORDERS_ROWS_PER_LINE = 3,
    ORDERS_LINES_PER_ROW = 4,
    ORDERS_TOP_BLOCKS = 7,
    ORDERS_HELP_FROM_BOTTOM = 8,
    GRANARY_BACK_FROM_BOTTOM = 16,
    STORAGE_BACK_FROM_BOTTOM = 24,
    TUTORIAL_BUTTON_X = 40,
    TUTORIAL_TOOLTIP_BASE = 40,
    HELP_CLOSE_TOOLTIP_BASE = 10,
    HELP_CLOSE_BUTTONS = 2,
};

enum terrain_panel_layout {
    TERRAIN_TEXT_NO_PEOPLE = 0,
    TERRAIN_TITLE_Y = 10,
    TERRAIN_TITLE_TEXT = 10,
    TERRAIN_DESCRIPTION_TEXT = 36,
    TERRAIN_DESCRIPTION_X = 36,
    TERRAIN_DESCRIPTION_FROM_BOTTOM = 113,
    TERRAIN_DESCRIPTION_MARGIN_BLOCKS = 4,
};

enum figure_list_layout {
    FIGURE_LIST_MAX = 7,
    FIGURE_LIST_TILES = 9,
    FIGURE_LIST_PANEL_X = 16,
    FIGURE_LIST_PANEL_Y = 40,
    FIGURE_LIST_PANEL_MARGIN_BLOCKS = 2,
    FIGURE_LIST_PANEL_HEIGHT_BLOCKS = 12,
    FIGURE_LIST_STEP = 60,
    FIGURE_LIST_BORDER_X = 25,
    FIGURE_LIST_BORDER_Y = 45,
    FIGURE_LIST_BORDER_SIZE = 52,
    FIGURE_LIST_PICTURE_X = 27,
    FIGURE_LIST_PICTURE_Y = 47,
    FIGURE_LIST_EMPTY_TEXT_Y = 120,
    FIGURE_PICTURE_BLIT_WIDTH = 12,
    FIGURE_PICTURE_BLIT_HEIGHT = 48,
    FIGURE_PICTURE_ROW_BYTES = 96,
    MENU_BAR_PANEL_IMAGE = 8,
    FIGURE_CAMERA_LEFT_TILES = 2,
    FIGURE_CAMERA_UP_TILES = 6,
};

enum figure_panel_layout {
    FIGURE_PANEL_X = 24,
    FIGURE_PANEL_Y = 102,
    FIGURE_PANEL_MARGIN_BLOCKS = 3,
    FIGURE_PANEL_HEIGHT = 122,
    PORTRAIT_X = 28,
    PORTRAIT_Y = 112,
    NAME_Y = 108,
    NAME_X = 90,
    KIND_X = 92,
    KIND_Y = 139,
    STATE_X = 90,
    GOODS_ICON_Y = 135,
    HUNTER_GOODS_X = 94,
    ENEMY_NAME_X = 28,
    ENEMY_KIND_Y = 149,
    PHRASE_X = 90,
    PHRASE_Y = 160,
    PHRASE_MARGIN_BLOCKS = 9,
    CARRIER_PHRASE_Y = 170,
    LOAD_ICON_Y = 150,
    LOAD_TEXT_X = 110,
    LOAD_TEXT_Y = 152,
    DESTINATION_X = 40,
    DESTINATION_Y = 200,
    TRADER_X = 40,
    TRADER_CITY_Y = 110,
    TRADER_STATE_Y = 150,
    TRADER_FIRST_ROW_Y = 170,
    TRADER_FIRST_ICON_Y = 167,
    TRADER_SECOND_ROW_Y = 200,
    TRADER_SECOND_ICON_Y = 197,
    TRADE_ICON_STEP = 25,
    CARRIED_GOODS_STEP = 32,
    CARRIED_GOODS_SLOTS = 4,
    UNITS_PER_LOAD = 100,
};

enum figure_info_text {
    FIGURE_INFO_TEXT_FROM = 0,
    FIGURE_INFO_TEXT_BUYS = 2,
    FIGURE_INFO_TEXT_SELLS = 3,
    FIGURE_INFO_TEXT_BOUGHT = 4,
    FIGURE_INFO_TEXT_SOLD = 5,
    FIGURE_INFO_TEXT_SHIP_ANCHORED = 6,
    FIGURE_INFO_TEXT_SHIP_DOCKED = 7,
    FIGURE_INFO_TEXT_RETURNING_HOME = 8,
    FIGURE_INFO_TEXT_SHIP_SAILING_TO_DOCKS = 9,
    FIGURE_INFO_TEXT_CARAVAN_TRADING = 10,
    FIGURE_INFO_TEXT_CARAVAN_TO_STORAGE_YARDS = 12,
    FIGURE_INFO_TEXT_NOTHING_TO_TRADE = 13,
    FIGURE_INFO_TEXT_CARRIER_FROM = 14,
    FIGURE_INFO_TEXT_GOING_TO = 15,
    FIGURE_INFO_TEXT_RETURNING_TO = 16,
    FIGURE_INFO_TEXT_COLLECTING = 17,
    FIGURE_INFO_TEXT_RETURNING_WITH = 18,
    FIGURE_INFO_TEXT_UNITS_OF = 20,
    FIGURE_INFO_TEXT_CARRYING = 21,
};

enum figure_kind_text {
    STORAGE_TEXT_BLOCK_OF = 23,
    STORAGE_TEXT_BLOCKS_OF = 24,
    RELIGION_TEXT_PRIESTESS = 38,
    ENEMY_TEXT_SOLDIER_OF = 14,
    RESOURCE_TEXT_PLURAL_OFFSET = 54,
};

enum figure_phrase_slots {
    FIGURE_PHRASE_SOUNDS = 8,
    FIGURE_PHRASE_LOADED_WEIGHT = 1,
};

enum building_open_sound {
    BUILDING_SOUND_PAN_CENTER = 64,
    BUILDING_SOUND_ONCE = 1,
    BUILDING_SOUND_NEW_ID = -1,
};

enum orders_toggle_value {
    ORDERS_TOGGLE_OFF = 1,
    ORDERS_TOGGLE_ON = 16,
};

int32_t building_info_window_heights[BUILDING_INFO_HEIGHT_CLASSES] = {
    [BUILDING_INFO_HEIGHT_LARGE] = 22,
    [BUILDING_INFO_HEIGHT_STANDARD] = 16,
    [BUILDING_INFO_HEIGHT_TALL] = 18,
    [BUILDING_INFO_HEIGHT_BANDSTAND] = 19,
    [BUILDING_INFO_HEIGHT_SHORT] = 14,
    [BUILDING_INFO_HEIGHT_FARM] = 24,
};

building_info_window_placement building_info_window_placements[BUILDING_INFO_SCREEN_SIZES] = {
    [BUILDING_INFO_SCREEN_640] = { .display_height = 480, .x = 8, .x_collapsed = 68, .low_mouse_y = 0, .bottom_y = 0 },
    [BUILDING_INFO_SCREEN_800] = { .display_height = 600, .x = 68, .x_collapsed = 128, .low_mouse_y = 312, .bottom_y = 592 },
    [BUILDING_INFO_SCREEN_1024] = { .display_height = 768, .x = 188, .x_collapsed = 248, .low_mouse_y = 396, .bottom_y = 760 },
};

const int32_t building_info_figure_tile_offsets[9] = { 0, -228, 228, 1, -1, -229, -227, 227, 229 };

const int32_t building_info_figure_picture_offsets[7] = { 0, 5000, 10000, 15000, 20000, 25000, 30000 };

static int32_t is_temple_complex_of_god(int32_t type)
{
    return type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST;
}

static int32_t is_temple_complex(int32_t type)
{
    return is_temple_complex_of_god(type) || (type >= BUILDING_TEMPLE_COMPLEX_GOD_5 && type <= BUILDING_TEMPLE_COMPLEX_GOD_9);
}

static int32_t is_shrine(int32_t type)
{
    return type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_GOD_9;
}

static int32_t is_granary(int32_t type)
{
    return type == BUILDING_GRANARY || type == BUILDING_GRANARY_UP;
}

static int32_t is_storage_yard(int32_t type)
{
    return type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP;
}

static int32_t is_bazaar(int32_t type)
{
    return type == BUILDING_BAZAAR || type == BUILDING_BAZAAR_UP;
}

static int32_t terrain_height_class(int32_t kind)
{
    switch (kind) {
        case TERRAIN_INFO_GARDEN:
        case TERRAIN_INFO_MUD_WALL:
        case TERRAIN_INFO_BRICK_WALL:
        case TERRAIN_INFO_STRONG_WALL:
        case TERRAIN_INFO_RUBBLE:
            return BUILDING_INFO_HEIGHT_STANDARD;
        case TERRAIN_INFO_CANAL:
            return BUILDING_INFO_HEIGHT_SHORT;
        default:
            return BUILDING_INFO_HEIGHT_LARGE;
    }
}

static int32_t building_height_class(int32_t type)
{
    switch (type) {
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_BULLFIGHT_SCHOOL:
        case BUILDING_FESTIVAL_SQUARE:
        case BUILDING_APOTHECARY:
        case BUILDING_MORTUARY:
        case BUILDING_PHYSICIAN:
        case BUILDING_DENTIST:
        case BUILDING_ORACLE:
        case BUILDING_BURNING_RUIN:
        case BUILDING_WATER_LIFT:
        case BUILDING_WATER_SUPPLY:
        case BUILDING_NATIVE_HUT:
        case BUILDING_NATIVE_MEETING:
        case BUILDING_NATIVE_CROPS:
        case BUILDING_MISSION_POST:
        case BUILDING_POLICE_STATION:
        case BUILDING_ARCHITECT_POST:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_GATEHOUSE_UP:
        case BUILDING_MUD_TOWER:
        case BUILDING_BRICK_TOWER:
        case BUILDING_FORT:
        case BUILDING_MILITARY_ACADEMY:
        case BUILDING_MILITARY_ACADEMY_2:
        case BUILDING_MILITARY_ACADEMY_3:
        case BUILDING_RECRUITER:
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_DOCK:
        case BUILDING_DOCK_UP:
        case BUILDING_FISHING_WHARF:
        case BUILDING_TRANSPORT_WHARF:
        case BUILDING_WARSHIP_WHARF:
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_TAX_COLLECTOR_UP:
        case BUILDING_BREWERY:
        case BUILDING_WEAVER:
        case BUILDING_WEAPONSMITH:
        case BUILDING_JEWELER:
        case BUILDING_POTTER:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_STONEMASONS_GUILD:
        case BUILDING_ARTISANS_GUILD:
            return BUILDING_INFO_HEIGHT_STANDARD;
        case BUILDING_WELL:
            return BUILDING_INFO_HEIGHT_SHORT;
        case BUILDING_BANDSTAND:
            return BUILDING_INFO_HEIGHT_BANDSTAND;
        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_ZOO:
        case BUILDING_PAVILION:
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
        case BUILDING_VILLAGE_PALACE:
        case BUILDING_TOWN_PALACE:
            return BUILDING_INFO_HEIGHT_TALL;
        default:
            return BUILDING_INFO_HEIGHT_LARGE;
    }
}

int32_t building_info_height_class(void)
{
    if (building_info_mode == BUILDING_INFO_MODE_NONE) {
        return BUILDING_INFO_HEIGHT_LARGE;
    }
    if (building_info_mode == BUILDING_INFO_MODE_TERRAIN) {
        return terrain_height_class(info_terrain_kind);
    }
    if (building_info_mode != BUILDING_INFO_MODE_BUILDING) {
        return BUILDING_INFO_HEIGHT_LARGE;
    }
    int32_t type = building_info_building()->type;
    if (is_shrine(type) || type == BUILDING_TEMPLE_OSIRIS) {
        return BUILDING_INFO_HEIGHT_STANDARD;
    }
    if (building_is_farm(building_info_building_id)) {
        return BUILDING_INFO_HEIGHT_FARM;
    }
    return building_height_class(type);
}

static const building_info_window_placement *window_placement(int32_t display_height)
{
    for (int32_t size = 0; size < BUILDING_INFO_SCREEN_SIZES; size++) {
        if (building_info_window_placements[size].display_height == display_height) {
            return &building_info_window_placements[size];
        }
    }
    return 0;
}

static int32_t plaza_or_fault_kind(uint32_t terrain)
{
    if (terrain & TERRAIN_DUNE) {
        return TERRAIN_INFO_DUNE;
    }
    if (terrain & TERRAIN_ROCK) {
        return TERRAIN_INFO_FAULT;
    }
    if (terrain & TERRAIN_ROAD) {
        return TERRAIN_INFO_PLAZA;
    }
    return TERRAIN_INFO_UNCHANGED;
}

static int32_t rock_kind(int32_t grid_offset, uint32_t terrain)
{
    if (grid_offset == city_entry_flag_offset) {
        return TERRAIN_INFO_ENTRY_FLAG;
    }
    if (grid_offset == city_exit_flag_offset) {
        return TERRAIN_INFO_EXIT_FLAG;
    }
    uint32_t rocks = terrain & TERRAIN_ROCKS;
    if (rocks == (TERRAIN_ROCK | TERRAIN_ORE)) {
        return TERRAIN_INFO_ORE_ROCK;
    }
    if (rocks == TERRAIN_ROCK) {
        return TERRAIN_INFO_NORMAL_ROCK;
    }
    if (rocks == (TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT)) {
        return TERRAIN_INFO_CLIFF;
    }
    return TERRAIN_INFO_ROCK;
}

static int32_t open_land_kind(uint32_t terrain)
{
    uint32_t walls = terrain & TERRAIN_WALL_KINDS;
    int32_t has_building = (terrain & TERRAIN_BUILDING) != 0;
    if ((terrain & TERRAIN_WATER) && !(terrain & (TERRAIN_BUILDING | TERRAIN_FLOODPLAIN))) {
        return TERRAIN_INFO_WATER;
    }
    if (terrain & TERRAIN_SHRUB) {
        return TERRAIN_INFO_SHRUB;
    }
    if (terrain & TERRAIN_GARDEN) {
        return TERRAIN_INFO_GARDEN;
    }
    if ((terrain & TERRAIN_ROAD) && !has_building) {
        return TERRAIN_INFO_ROAD;
    }
    if (terrain & TERRAIN_CANAL) {
        return TERRAIN_INFO_CANAL;
    }
    if (terrain & TERRAIN_RUBBLE) {
        return TERRAIN_INFO_RUBBLE;
    }
    if (terrain & TERRAIN_MARSH) {
        return TERRAIN_INFO_MARSH;
    }
    if (terrain & TERRAIN_DUNE) {
        return TERRAIN_INFO_DUNE;
    }
    if (walls == TERRAIN_WALL) {
        return TERRAIN_INFO_MUD_WALL;
    }
    if (walls == (TERRAIN_WALL | TERRAIN_BRICK_WALL)) {
        return TERRAIN_INFO_BRICK_WALL;
    }
    if (walls == (TERRAIN_WALL | TERRAIN_STRONG_WALL)) {
        return TERRAIN_INFO_STRONG_WALL;
    }
    if (((terrain & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) == TERRAIN_FLOODPLAIN) && !has_building) {
        return TERRAIN_INFO_FLOODPLAIN;
    }
    if ((terrain & TERRAIN_MEADOW) && !has_building) {
        return TERRAIN_INFO_MEADOW;
    }
    if ((terrain & TERRAIN_FLOODPLAIN) && !has_building) {
        return TERRAIN_INFO_SUBMERGED_FLOODPLAIN;
    }
    if (building_info_building_id == 0) {
        return TERRAIN_INFO_EMPTY;
    }
    return TERRAIN_INFO_BUILDING;
}

int32_t building_info_tile_terrain_kind(int32_t grid_offset)
{
    uint32_t terrain = map_grids.terrain[grid_offset];
    if (map_grids.building_ids[grid_offset] == 0 && map_grids.sprite[grid_offset] != 0) {
        return (terrain & TERRAIN_WATER) ? TERRAIN_INFO_BRIDGE : TERRAIN_INFO_EMPTY;
    }
    if (map_grids.bitfields[grid_offset] & MAP_BITFIELD_PLAZA_OR_FAULT) {
        return plaza_or_fault_kind(terrain);
    }
    if (terrain & TERRAIN_TREE) {
        return TERRAIN_INFO_TREE;
    }
    if ((terrain & TERRAIN_ROCK) && building_info_building()->type != BUILDING_LIGHTHOUSE) {
        return rock_kind(grid_offset, terrain);
    }
    return open_land_kind(terrain);
}

static void use_main_building(int32_t building_id)
{
    const building *b = building_get(building_id);
    building_info_building_id = building_id;
    bi_tile_block.grid_offset = b->grid_offset;
    bi_tile_block.x = b->x;
    bi_tile_block.y = b->y;
}

static int32_t shows_main_building(int32_t type)
{
    switch (type) {
        case BUILDING_STORAGE_ROOM:
        case BUILDING_SENET_HOUSE:
        case BUILDING_ZOO:
        case BUILDING_SPHINX:
        case BUILDING_BOOTH:
        case BUILDING_BANDSTAND:
        case BUILDING_PAVILION:
            return 1;
        default:
            return 0;
    }
}

static void choose_shown_building(void)
{
    int32_t building_id = building_info_building_id;
    const building *b = building_get(building_id);
    int32_t type = b->type;
    if (type == BUILDING_FORT_GROUND) {
        int32_t fort_id = b->prev_part_building_id;
        use_main_building(fort_id);
        building_info_formation_id = building_get(fort_id)->formation_id;
    } else if (type == BUILDING_FORT) {
        building_info_formation_id = b->formation_id;
    } else if (shows_main_building(type)) {
        use_main_building(building_main(building_id));
    } else if (b->house_size) {
        window_building_house_find_worst_desirability();
        house_update_evolve_text(building_info_building_id);
    } else if (type == BUILDING_RECRUITER) {
        info_has_academy = formation_any_legion_with_soldiers() + building_barracks_tower_sentry_request;
    } else if (is_temple_complex(type)) {
        use_main_building(building_main(building_id));
    }
}

static int32_t shown_building_road_access(const building *b)
{
    int32_t type = b->type;
    if (is_granary(type)) {
        return map_has_road_access_granary(b->x, b->y);
    }
    if (is_shrine(type)) {
        return map_has_road_access_nearby(b->x, b->y);
    }
    if (is_temple_complex_of_god(type)) {
        return map_has_road_access_temple_complex(b->x, b->y, b->data.temple_complex.orientation);
    }
    return map_has_road_access(b->x, b->y, b->size);
}

static void init_building(void)
{
    building_info_mode = BUILDING_INFO_MODE_BUILDING;
    const building *selected = building_info_building();
    bi_road_access_block.worker_percentage = calc_percentage(selected->num_workers, building_model_table[selected->type].laborers);
    choose_shown_building();
    building *b = building_info_building();
    bi_road_access_block.has_road_access = 0;
    if (is_storage_yard(b->type)) {
        if (map_has_road_access(b->x, b->y, STORAGE_YARD_SIZE)) {
            bi_road_access_block.has_road_access = 1;
        }
        bi_storage_state = building_storageyard_get_space_info(building_info_building_id);
        if (storage_orders_rows() >= city_resources.count) {
            b->info_orders_scroll = 1;
        }
        return;
    }
    if (shown_building_road_access(b)) {
        bi_road_access_block.has_road_access = 1;
    }
}

static int32_t figure_is_listed(const figure *f)
{
    switch (f->type) {
        case FIGURE_NONE:
        case FIGURE_EXPLOSION:
        case FIGURE_MAP_FLAG:
        case FIGURE_FLOTSAM:
        case FIGURE_ARROW:
        case FIGURE_JAVELIN:
        case FIGURE_BOLT:
        case FIGURE_BALLISTA:
        case FIGURE_CREATURE:
        case FIGURE_SEAGULLS:
        case FIGURE_SPEAR:
        case FIGURE_CHARIOT_RACER:
        case FIGURE_HUNTER_SPEAR:
            return 0;
        case FIGURE_ARCHER:
        case FIGURE_INFANTRY:
        case FIGURE_CHARIOTEER:
        case FIGURE_STANDARD_BEARER:
            return formation_get(f->formation_id)->request_event_id < 0;
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
            return f->request_event_id < 0 && f->action_state != FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_REQUEST;
        default:
            return 1;
    }
}

static void collect_figures(int32_t grid_offset)
{
    info_figure_index = 0;
    info_figure_count = 0;
    for (int32_t i = 0; i < FIGURE_LIST_MAX; i++) {
        info_figure_ids[i] = 0;
    }
    int32_t count = 0;
    for (int32_t t = 0; t < FIGURE_LIST_TILES && count < FIGURE_LIST_MAX; t++) {
        int32_t figure_id = map_grids.figure_ids[grid_offset + building_info_figure_tile_offsets[t]];
        while (figure_id > 0) {
            const figure *f = figure_get(figure_id);
            if (f->state != FIGURE_STATE_DEAD && f->action_state != FIGURE_ACTION_CORPSE && figure_is_listed(f)) {
                info_figure_ids[count] = figure_id;
                info_figure_count = count + 1;
                figure_phrase_determine(figure_id);
                count = info_figure_count;
            }
            figure_id = f->next_figure_id_on_same_tile;
            if (count >= FIGURE_LIST_MAX) {
                break;
            }
        }
    }
}

static void choose_figure_mode(void)
{
    for (int32_t i = 0; i < FIGURE_LIST_MAX; i++) {
        int32_t figure_id = info_figure_ids[i];
        if (figure_id <= 0) {
            continue;
        }
        const figure *f = figure_get(figure_id);
        uint8_t type = f->type;
        if (type == FIGURE_STANDARD_BEARER || type == FIGURE_ARCHER || type == FIGURE_INFANTRY || type == FIGURE_CHARIOTEER) {
            building_info_mode = BUILDING_INFO_MODE_LEGION;
            building_info_formation_id = f->formation_id;
            return;
        }
        if (type == FIGURE_TRANSPORT_SHIP) {
            building_info_mode = BUILDING_INFO_MODE_TRANSPORT_SHIP;
            building_info_transport_id = figure_id;
            return;
        }
        if (type == FIGURE_WARSHIP) {
            building_info_mode = BUILDING_INFO_MODE_WARSHIP;
            building_info_warship_id = figure_id;
            return;
        }
    }
}

static void place_window_of_screen_layout(int32_t height)
{
    int32_t window_width = WINDOW_WIDTH_BLOCKS * BLOCK_SIZE;
    int32_t view_width = (g_sidebar_collapsed == 1) ? city_view_width_collapsed : city_view_width_expanded;
    building_info_x = ((view_width - window_width) / 2) + 1;
    int32_t opens_at_bottom = (g_screen_height >= SCREEN_LAYOUT_BOTTOM_PLACEMENT_HEIGHT) && (mouse_y <= (g_screen_height / 2) + SCREEN_LAYOUT_LOW_MOUSE_EXTRA);
    building_info_y = opens_at_bottom ? ((g_screen_height - SCREEN_LAYOUT_BOTTOM_MARGIN) - (height * BLOCK_SIZE)) : WINDOW_Y;
}

static void place_window(void)
{
    building_info_x = WINDOW_X;
    building_info_y = WINDOW_Y;
    info_w_blocks = WINDOW_WIDTH_BLOCKS;
    int32_t height = building_info_window_heights[building_info_height_class()];
    building_info_h = height;
    if (!screen_layout_is_standard_size()) {
        place_window_of_screen_layout(height);
        return;
    }
    const building_info_window_placement *placement = window_placement(g_screen_height);
    if (!placement) {
        return;
    }
    int32_t opens_at_bottom = (placement->bottom_y != 0) && (mouse_y <= placement->low_mouse_y);
    building_info_y = opens_at_bottom ? (placement->bottom_y - (height * BLOCK_SIZE)) : WINDOW_Y;
    building_info_x = (g_sidebar_collapsed == 1) ? placement->x_collapsed : placement->x;
}

static void read_monument_progress(int32_t building_id)
{
    switch (building_get(building_id)->type) {
        case BUILDING_PYRAMID:
            pyramid_get_progress_info(building_id, &bi_progress_pyramid);
            break;
        case BUILDING_SUN_TEMPLE:
            sun_temple_get_progress(building_id, bi_progress_sun_temple);
            break;
        case BUILDING_ALEXANDRIA_LIBRARY:
            library_get_progress(&library_object, building_id, &bi_library_report);
            break;
        case BUILDING_ABU_SIMBEL:
            abu_simbel_get_progress(&abu_simbel_object, building_id, &bi_progress_abu_simbel);
            break;
        case BUILDING_CAESAREUM:
            caesareum_get_progress(&caesareum_object, building_id, &bi_progress_caesareum);
            break;
        case BUILDING_LIGHTHOUSE:
            lighthouse_get_progress(&lighthouse_object, building_id, &bi_progress_lighthouse);
            break;
        case BUILDING_MAUSOLEUM:
            mausoleum_get_progress(building_id, &bi_progress_mausoleum);
            break;
        case BUILDING_SMALL_ROYAL_TOMB:
            royal_tomb_progress(BUILDING_SMALL_ROYAL_TOMB, building_id, bi_progress_tomb);
            break;
        case BUILDING_MEDIUM_ROYAL_TOMB:
            royal_tomb_progress(BUILDING_MEDIUM_ROYAL_TOMB, building_id, bi_progress_tomb);
            break;
        case BUILDING_LARGE_ROYAL_TOMB:
            royal_tomb_progress(BUILDING_LARGE_ROYAL_TOMB, building_id, bi_progress_tomb);
            break;
        case BUILDING_GRAND_ROYAL_TOMB:
            royal_tomb_progress(BUILDING_GRAND_ROYAL_TOMB, building_id, bi_progress_tomb);
            break;
        default:
            break;
    }
}

static int32_t selected_building_id(int32_t grid_offset)
{
    int32_t building_id = map_grids.building_ids[grid_offset];
    int32_t type = building_get(building_id)->type;
    if (type == BUILDING_MORTUARY_TEMPLE || type == BUILDING_CAUSEWAY || type == BUILDING_VALLEY_TEMPLE) {
        building_id = pyramid_find_by_complex_building(building_id);
    }
    if (building_get(building_id)->type == BUILDING_PYRAMID_ENTRY_TILE) {
        building_id = building_get_subtype(building_id);
    }
    return building_id;
}

int32_t window_building_info_init(void)
{
    if (editor_active == 1) {
        return 0;
    }
    if (city_warning_active_count > 0) {
        city_warning_clear_all();
        return 0;
    }
    bi_tile_block.y = map_info.cursor_tile_y;
    bi_tile_block.x = map_info.cursor_tile_x;
    focus_button_id_secondary = 0;
    focus_button_id = 0;
    image_button_focus_id = 0;
    info_first_draw = 1;
    building_info_orders_shown = 0;
    window_current_id = WINDOW_BUILDING_INFO;
    window_redraw_request = WINDOW_REDRAW_ALL;
    int32_t grid_offset = map_grid_offset(map_info.cursor_tile_x, map_info.cursor_tile_y);
    bi_tile_block.grid_offset = grid_offset;
    bi_road_access_block.rubble_building_type = map_grids.rubble_building_type[grid_offset];
    building_info_building_id = selected_building_id(grid_offset);
    bi_road_access_block.has_groundwater = map_grids.terrain[grid_offset] & TERRAIN_GROUNDWATER;
    bi_road_access_block.canal = map_grids.canals[grid_offset];
    if (building_info_building_id != 0 && building_info_building()->state == BUILDING_STATE_UNUSED) {
        window_current_id = WINDOW_CITY;
        return 0;
    }
    city_resource_determine_available();
    read_monument_progress(building_info_building_id);
    building_info_mode = BUILDING_INFO_MODE_TERRAIN;
    bi_figure_drawn = 0;
    int32_t kind = building_info_tile_terrain_kind(grid_offset);
    if (kind == TERRAIN_INFO_BUILDING) {
        init_building();
    } else if (kind != TERRAIN_INFO_UNCHANGED) {
        info_terrain_kind = kind;
    }
    collect_figures(grid_offset);
    choose_figure_mode();
    place_window();
    if (building_info_mode == BUILDING_INFO_MODE_BUILDING) {
        window_building_info_advisors(building_info_advisor_buttons);
    }
    return 1;
}

static const building_info_draw_fn building_draw_functions[BUILDING_TYPE_COUNT] = {
    [BUILDING_MENU_WATER_LIFT] = window_building_draw_water_lift,
    [BUILDING_WATER_LIFT] = window_building_draw_water_lift,
    [BUILDING_HOUSE_CRUDE_HUT] = window_building_draw_house,
    [BUILDING_HOUSE_STURDY_HUT] = window_building_draw_house,
    [BUILDING_HOUSE_MEAGER_SHANTY] = window_building_draw_house,
    [BUILDING_HOUSE_COMMON_SHANTY] = window_building_draw_house,
    [BUILDING_HOUSE_ROUGH_COTTAGE] = window_building_draw_house,
    [BUILDING_HOUSE_ORDINARY_COTTAGE] = window_building_draw_house,
    [BUILDING_HOUSE_MODEST_HOMESTEAD] = window_building_draw_house,
    [BUILDING_HOUSE_SPACIOUS_HOMESTEAD] = window_building_draw_house,
    [BUILDING_HOUSE_MODEST_APARTMENT] = window_building_draw_house,
    [BUILDING_HOUSE_SPACIOUS_APARTMENT] = window_building_draw_house,
    [BUILDING_HOUSE_COMMON_RESIDENCE] = window_building_draw_house,
    [BUILDING_HOUSE_SPACIOUS_RESIDENCE] = window_building_draw_house,
    [BUILDING_HOUSE_ELEGANT_RESIDENCE] = window_building_draw_house,
    [BUILDING_HOUSE_FANCY_RESIDENCE] = window_building_draw_house,
    [BUILDING_HOUSE_COMMON_MANOR] = window_building_draw_house,
    [BUILDING_HOUSE_SPACIOUS_MANOR] = window_building_draw_house,
    [BUILDING_HOUSE_ELEGANT_MANOR] = window_building_draw_house,
    [BUILDING_HOUSE_STATELY_MANOR] = window_building_draw_house,
    [BUILDING_HOUSE_MODEST_ESTATE] = window_building_draw_house,
    [BUILDING_HOUSE_PALATIAL_ESTATE] = window_building_draw_house,
    [BUILDING_BANDSTAND] = window_building_draw_bandstand,
    [BUILDING_BOOTH] = window_building_draw_booth,
    [BUILDING_SENET_HOUSE] = window_building_draw_senet_house,
    [BUILDING_PAVILION] = window_building_draw_pavilion,
    [BUILDING_CONSERVATORY] = window_building_draw_conservatory,
    [BUILDING_DANCE_SCHOOL] = window_building_draw_dance_school,
    [BUILDING_JUGGLER_SCHOOL] = window_building_draw_juggler_school,
    [BUILDING_BULLFIGHT_SCHOOL] = window_building_draw_bullfight_school,
    [BUILDING_SMALL_STATUE] = window_building_draw_statue,
    [BUILDING_MEDIUM_STATUE] = window_building_draw_statue,
    [BUILDING_LARGE_STATUE] = window_building_draw_statue,
    [BUILDING_APOTHECARY] = window_building_draw_apothecary,
    [BUILDING_MORTUARY] = window_building_draw_mortuary,
    [BUILDING_DENTIST] = window_building_draw_dentist,
    [BUILDING_SCRIBAL_SCHOOL] = window_building_draw_scribal_school,
    [BUILDING_LIBRARY] = window_building_draw_library,
    [BUILDING_POLICE_STATION] = window_building_draw_police_station,
    [BUILDING_TRIUMPHAL_ARCH] = window_building_draw_triumphal_arch,
    [BUILDING_FORT] = window_building_draw_fort,
    [BUILDING_MUD_GATEHOUSE] = window_building_draw_mud_gatehouse,
    [BUILDING_MUD_GATEHOUSE_UP] = window_building_draw_mud_gatehouse,
    [BUILDING_MUD_TOWER] = window_building_draw_mud_tower,
    [BUILDING_SHIPWRIGHT] = window_building_draw_shipwright,
    [BUILDING_DOCK] = window_building_draw_dock,
    [BUILDING_DOCK_UP] = window_building_draw_dock,
    [BUILDING_FISHING_WHARF] = window_building_draw_fishing_wharf,
    [BUILDING_PERSONAL_MANSION] = window_building_draw_mansion,
    [BUILDING_FAMILY_MANSION] = window_building_draw_mansion,
    [BUILDING_DYNASTY_MANSION] = window_building_draw_mansion,
    [BUILDING_MISSION_POST] = window_building_draw_mission_post,
    [BUILDING_ARCHITECT_POST] = window_building_draw_architect_post,
    [BUILDING_VILLAGE_PALACE] = window_building_draw_palace,
    [BUILDING_TOWN_PALACE] = window_building_draw_palace,
    [BUILDING_VILLAGE_PALACE_UP] = window_building_draw_palace,
    [BUILDING_TOWN_PALACE_UP] = window_building_draw_palace,
    [BUILDING_CITY_PALACE] = window_building_draw_palace,
    [BUILDING_TAX_COLLECTOR] = window_building_draw_tax_collector,
    [BUILDING_TAX_COLLECTOR_UP] = window_building_draw_tax_collector,
    [BUILDING_NATIVE_HUT] = window_building_draw_native_hut,
    [BUILDING_NATIVE_MEETING] = window_building_draw_native_meeting,
    [BUILDING_WELL] = window_building_draw_well,
    [BUILDING_NATIVE_CROPS] = window_building_draw_native_crops,
    [BUILDING_MILITARY_ACADEMY] = window_building_draw_military_academy,
    [BUILDING_MILITARY_ACADEMY_2] = window_building_draw_military_academy,
    [BUILDING_MILITARY_ACADEMY_3] = window_building_draw_military_academy,
    [BUILDING_RECRUITER] = window_building_draw_recruiter,
    [BUILDING_ORACLE] = window_building_draw_oracle,
    [BUILDING_BURNING_RUIN] = window_building_draw_burning_ruin,
    [BUILDING_BARLEY_FARM] = window_building_draw_barley_farm,
    [BUILDING_FLAX_FARM] = window_building_draw_flax_farm,
    [BUILDING_GRAIN_FARM] = window_building_draw_grain_farm,
    [BUILDING_LETTUCE_FARM] = window_building_draw_lettuce_farm,
    [BUILDING_POMEGRANATES_FARM] = window_building_draw_pomegranate_farm,
    [BUILDING_CHICKPEAS_FARM] = window_building_draw_chickpea_farm,
    [BUILDING_STONE_QUARRY] = window_building_draw_stone_quarry,
    [BUILDING_WOOD_CUTTER] = window_building_draw_wood_cutter,
    [BUILDING_CLAY_PIT] = window_building_draw_clay_pit,
    [BUILDING_BREWERY] = window_building_draw_brewery,
    [BUILDING_WEAVER] = window_building_draw_weaver,
    [BUILDING_WEAPONSMITH] = window_building_draw_weaponsmith,
    [BUILDING_JEWELER] = window_building_draw_jeweler,
    [BUILDING_POTTER] = window_building_draw_potter,
    [BUILDING_HUNTING_LODGE] = window_building_draw_hunting_lodge,
    [BUILDING_FERRY_LANDING] = window_building_draw_ferry,
    [BUILDING_ROADBLOCK] = window_building_draw_roadblock,
    [BUILDING_GOLD_MINE] = window_building_draw_gold_mine,
    [BUILDING_GEMSTONE_MINE] = window_building_draw_gemstone_mine,
    [BUILDING_FIREHOUSE] = window_building_draw_firehouse,
    [BUILDING_BRICK_GATEHOUSE] = window_building_draw_brick_gatehouse,
    [BUILDING_BRICK_GATEHOUSE_UP] = window_building_draw_brick_gatehouse,
    [BUILDING_GATEHOUSE] = window_building_draw_clay_gatehouse,
    [BUILDING_GATEHOUSE_UP] = window_building_draw_clay_gatehouse,
    [BUILDING_BRICK_TOWER] = window_building_draw_brick_tower,
    [BUILDING_TOWER] = window_building_draw_clay_tower,
    [BUILDING_CARPENTERS_GUILD] = window_building_draw_carpenters_guild,
    [BUILDING_BRICKLAYERS_GUILD] = window_building_draw_bricklayers_guild,
    [BUILDING_STONEMASONS_GUILD] = window_building_draw_stonemasons_guild,
    [BUILDING_WATER_SUPPLY] = window_building_draw_water_supply,
    [BUILDING_TRANSPORT_WHARF] = window_building_draw_transport_wharf,
    [BUILDING_WARSHIP_WHARF] = window_building_draw_warship_wharf,
    [BUILDING_PYRAMID] = window_building_draw_pyramid,
    [BUILDING_COURTHOUSE] = window_building_draw_courthouse,
    [BUILDING_CATTLE_RANCH] = window_building_draw_cattle_ranch,
    [BUILDING_REED_GATHERER] = window_building_draw_reed_gatherer,
    [BUILDING_FIG_FARM] = window_building_draw_fig_farm,
    [BUILDING_WORK_CAMP] = window_building_draw_work_camp,
    [BUILDING_PAPYRUS_MAKER] = window_building_draw_papyrus_maker,
    [BUILDING_BRICKWORKS] = window_building_draw_brickworks,
    [BUILDING_CHARIOT_MAKER] = window_building_draw_chariot_maker,
    [BUILDING_PHYSICIAN] = window_building_draw_physician,
    [BUILDING_SMALL_OBELISK] = window_building_draw_obelisk,
    [BUILDING_LARGE_OBELISK] = window_building_draw_obelisk,
    [BUILDING_FESTIVAL_SQUARE] = window_building_draw_festival_square,
    [BUILDING_SPHINX] = window_building_draw_sphinx,
    [BUILDING_SUN_TEMPLE] = window_building_draw_sun_temple,
    [BUILDING_COPPER_MINE] = window_building_draw_copper_mine,
    [BUILDING_MAUSOLEUM] = window_building_draw_mausoleum,
    [BUILDING_HENNA_FARM] = window_building_draw_henna_farm,
    [BUILDING_ALEXANDRIA_LIBRARY] = window_building_draw_great_library,
    [BUILDING_ZOO] = window_building_draw_zoo,
    [BUILDING_CAESAREUM] = window_building_draw_caesareum,
    [BUILDING_LIGHTHOUSE] = window_building_draw_lighthouse,
    [BUILDING_SMALL_ROYAL_TOMB] = window_building_draw_royal_tomb,
    [BUILDING_MEDIUM_ROYAL_TOMB] = window_building_draw_royal_tomb,
    [BUILDING_LARGE_ROYAL_TOMB] = window_building_draw_royal_tomb,
    [BUILDING_GRAND_ROYAL_TOMB] = window_building_draw_royal_tomb,
    [BUILDING_ABU_SIMBEL] = window_building_draw_abu_simbel,
    [BUILDING_ARTISANS_GUILD] = window_building_draw_artisans_guild,
    [BUILDING_LAMP_MAKER] = window_building_draw_lamp_maker,
    [BUILDING_PAINT_MAKER] = window_building_draw_paint_maker,
};

building_info_draw_fn building_info_draw_function(int32_t type)
{
    if (type < 0 || type >= BUILDING_TYPE_COUNT) {
        return 0;
    }
    return building_draw_functions[type];
}

static int32_t building_sound_type(const building *b)
{
    if (b->type != BUILDING_FORT) {
        return b->type;
    }
    switch (b->subtype) {
        case FIGURE_CHARIOTEER:
            return BUILDING_FORT_CHARIOTEERS;
        case FIGURE_ARCHER:
            return BUILDING_FORT_ARCHERS;
        case FIGURE_INFANTRY:
            return BUILDING_FORT_INFANTRY;
        default:
            return BUILDING_FORT;
    }
}

static void play_building_sound(void)
{
    int32_t type = building_sound_type(building_info_building());
    int32_t channel = city_channel_base.types[type].roam;
    if (channel <= -1 || info_first_draw == 0 || settings_data.sound_effects_enabled == 0 || building_info_mode != BUILDING_INFO_MODE_BUILDING) {
        return;
    }
    sound_request *request = &sound_manager.request;
    request->volume = settings_data.sound_effects_volume;
    request->pan = BUILDING_SOUND_PAN_CENTER;
    request->name_index = channel;
    request->id = BUILDING_SOUND_NEW_ID;
    request->loop_count = BUILDING_SOUND_ONCE;
    sound_lib_play_sample(&sound_manager, request);
}

static void draw_building(void)
{
    int32_t type = building_info_building()->type;
    int32_t orders = building_info_orders_shown != 0;
    switch (type) {
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_SHRINE_OSIRIS:
            window_building_draw_temple(GOD_OSIRIS);
            return;
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_SHRINE_RA:
            window_building_draw_temple(GOD_RA);
            return;
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_SHRINE_PTAH:
            window_building_draw_temple(GOD_PTAH);
            return;
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_SHRINE_SETH:
            window_building_draw_temple(GOD_SETH);
            return;
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_SHRINE_BAST:
            window_building_draw_temple(GOD_BAST);
            return;
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
            (orders ? window_building_draw_bazaar_orders : window_building_draw_bazaar)();
            return;
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            (orders ? window_building_draw_granary_orders : window_building_draw_granary)();
            return;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            (orders ? window_building_draw_storage_yard_orders : window_building_draw_storage_yard)();
            return;
        case BUILDING_LIMESTONE_QUARRY:
            window_building_draw_quarry(RESOURCE_LIMESTONE);
            return;
        case BUILDING_GRANITE_QUARRY:
            window_building_draw_quarry(RESOURCE_GRANITE);
            return;
        case BUILDING_SANDSTONE_QUARRY:
            window_building_draw_quarry(RESOURCE_SANDSTONE);
            return;
        default: {
            building_info_draw_fn draw = building_info_draw_function(type);
            if (draw) {
                draw();
            }
            return;
        }
    }
}

void window_building_info_draw_background(void)
{
    widget_city_draw_current_view();
    play_building_sound();
    switch (building_info_mode) {
        case BUILDING_INFO_MODE_NONE:
            window_building_draw_no_info();
            return;
        case BUILDING_INFO_MODE_TERRAIN:
            window_building_draw_terrain();
            return;
        case BUILDING_INFO_MODE_LEGION:
            window_building_draw_legion(building_info_formation_id);
            return;
        case BUILDING_INFO_MODE_TRANSPORT_SHIP:
            window_building_draw_boat();
            return;
        case BUILDING_INFO_MODE_WARSHIP:
            window_building_draw_ship();
            return;
        case BUILDING_INFO_MODE_BUILDING:
            draw_building();
            return;
        default:
            return;
    }
}

static int32_t figure_speaks(int32_t type)
{
    switch (type) {
        case FIGURE_LABOR_SEEKER:
        case FIGURE_TAX_COLLECTOR:
        case FIGURE_FIREMAN:
        case FIGURE_JUGGLER:
        case FIGURE_MUSICIAN:
        case FIGURE_DANCER:
        case FIGURE_SENET_PLAYER:
        case FIGURE_PRIEST:
        case FIGURE_TEACHER:
        case FIGURE_LIBRARIAN:
        case FIGURE_DENTIST:
        case FIGURE_PHYSICIAN:
        case FIGURE_EMBALMER:
        case FIGURE_WORKER:
        case FIGURE_MARKET_BUYER:
        case FIGURE_PATRICIAN:
        case FIGURE_WATER_CARRIER:
        case FIGURE_CONSTABLE:
        case FIGURE_MAGISTRATE:
        case FIGURE_ZOOKEEPER:
            return 1;
        default:
            return 0;
    }
}

static int32_t other_phrase_is_loaded(void)
{
    for (int32_t i = 0; i < FIGURE_PHRASE_SOUNDS; i++) {
        if (i != figure_phrase_id && phrase_weight_block.phrase_weights[i] > FIGURE_PHRASE_LOADED_WEIGHT) {
            return 1;
        }
    }
    return 0;
}

static void draw_phrase_button(void)
{
    if (bi_figure_drawn == 0 || !figure_speaks(figure_get(info_figure_ids[info_figure_index])->type) || !other_phrase_is_loaded()) {
        return;
    }
    int32_t x = building_info_x + PHRASE_BUTTON_X;
    button_border_draw_ui(
        x, building_info_y + PHRASE_BUTTON_Y, PHRASE_BUTTON_WIDTH, PHRASE_BUTTON_HEIGHT, focus_button_id_secondary == 1, building_info_x, building_info_y);
    lang_text_draw_centered_ui(
        LANG_GROUP_YES_NO, LANG_TELL_ME_MORE, x, building_info_y + PHRASE_BUTTON_TEXT_Y, PHRASE_BUTTON_WIDTH, image_group(IMAGE_GROUP_FONT_GREEN), 0, building_info_x, building_info_y);
}

static void draw_building_foreground(void)
{
    int32_t type = building_info_building()->type;
    int32_t orders = building_info_orders_shown != 0;
    if (type >= BUILDING_HOUSE_CRUDE_HUT && type <= BUILDING_HOUSE_PALATIAL_ESTATE) {
        return;
    }
    if (is_granary(type)) {
        (orders ? window_building_draw_granary_orders_foreground : window_building_draw_granary_foreground)();
    } else if (is_storage_yard(type)) {
        (orders ? window_building_draw_storage_yard_orders_foreground : window_building_draw_storage_yard_foreground)();
    } else if (is_bazaar(type)) {
        (orders ? window_building_draw_bazaar_orders_foreground : window_building_draw_bazaar_foreground)();
    } else if (type == BUILDING_VILLAGE_PALACE_UP || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_CITY_PALACE || type == BUILDING_TAX_COLLECTOR ||
        type == BUILDING_TAX_COLLECTOR_UP) {
        window_building_draw_palace_foreground();
    } else if (type == BUILDING_PERSONAL_MANSION || type == BUILDING_FAMILY_MANSION) {
        window_building_draw_mansion_foreground();
    }
}

void window_building_info_draw_foreground(void)
{
    switch (building_info_mode) {
        case BUILDING_INFO_MODE_TERRAIN:
            draw_phrase_button();
            return;
        case BUILDING_INFO_MODE_LEGION:
            window_building_draw_legion_foreground(building_info_formation_id);
            return;
        case BUILDING_INFO_MODE_BUILDING:
            draw_building_foreground();
            return;
        default:
            return;
    }
}

static int32_t generic_buttons_row(int32_t x, int32_t y, void *buttons, int32_t count, int32_t *focus)
{
    int32_t focused = generic_buttons_get_focus(x, y, buttons, count);
    *focus = ui_layout_buttons(x, y, buttons, count, focused);
    return generic_buttons_handle_mouse(x, y, buttons, count);
}

static int32_t help_close_buttons(int32_t y)
{
    image_buttons_set_tooltip(building_info_x, y, building_info_help_close_buttons.buttons, HELP_CLOSE_BUTTONS, HELP_CLOSE_TOOLTIP_BASE);
    return image_buttons_handle_mouse(building_info_x, y, building_info_help_close_buttons.buttons, HELP_CLOSE_BUTTONS);
}

static int32_t bottom_buttons_y(void)
{
    return (building_info_h * BLOCK_SIZE) - BOTTOM_BUTTONS_FROM_BOTTOM + building_info_y;
}

static int32_t orders_bottom(int32_t rows)
{
    return (calc_divide_round_up(rows * ORDERS_LINES_PER_ROW, ORDERS_ROWS_PER_LINE) + ORDERS_TOP_BLOCKS) * BLOCK_SIZE;
}

static int32_t legion_order_of_layout(int32_t layout)
{
    switch (layout) {
        case FORMATION_DOUBLE_LINE_1:
        case FORMATION_DOUBLE_LINE_2:
            return LEGION_ORDER_HOLD_TIGHT;
        case FORMATION_SINGLE_LINE_1:
        case FORMATION_SINGLE_LINE_2:
            return LEGION_ORDER_HOLD_LOOSE;
        case FORMATION_CHARGE:
            return LEGION_ORDER_ENGAGE;
        default:
            return -1;
    }
}

static int32_t handle_legion_right_click(void)
{
    if (building_info_mode != BUILDING_INFO_MODE_LEGION) {
        return 0;
    }
    const formation *legion = formation_get(building_info_formation_id);
    if (legion->num_figures == 0) {
        return 0;
    }
    if (legion->figure_type == FIGURE_CHARIOTEER && legion->layout == FORMATION_COLUMN) {
        window_current_id = WINDOW_CITY_LEGION_ORDER;
        return 1;
    }
    if (legion->layout == FORMATION_MOP_UP) {
        return 0;
    }
    int32_t order = legion_order_of_layout(legion->layout);
    if (order >= 0) {
        button_parameter = order;
    }
    legion_order_button();
    window_current_id = WINDOW_CITY_LEGION_SELECTED;
    return 1;
}

static void handle_right_click(void)
{
    if (!handle_legion_right_click()) {
        window_current_id = WINDOW_CITY;
    }
    window_redraw_request = WINDOW_REDRAW_ALL;
}

static int32_t storage_orders_shown_rows(void)
{
    int32_t rows = city_resources.count;
    if (storage_orders_rows() < city_resources.count) {
        rows = storage_orders_rows();
    }
    return rows;
}

static int32_t special_orders_rows(int32_t type)
{
    if (is_storage_yard(type)) {
        return storage_orders_shown_rows();
    }
    if (is_bazaar(type)) {
        int32_t rows = 0;
        for (int32_t i = 0; i < city_resources.count; i++) {
            if (resource_is_bazaar_good(city_resources.list[i])) {
                rows++;
            }
        }
        return rows;
    }
    return city_foods.count;
}

static int32_t handle_window_buttons(void)
{
    if (building_info_orders_shown == 0) {
        if (building_info_mode == BUILDING_INFO_MODE_BUILDING) {
            window_building_handle_advisor_buttons();
        }
        return help_close_buttons(bottom_buttons_y());
    }
    int32_t rows = special_orders_rows(building_info_building()->type);
    if (generic_buttons_row(building_info_x + SPECIAL_ORDERS_X, SPECIAL_ORDERS_Y, buttons_special_orders.resources, rows, &focus_button_id_secondary)) {
        return 1;
    }
    return help_close_buttons(orders_bottom(rows) - ORDERS_HELP_FROM_BOTTOM);
}

static void handle_figure_list(void)
{
    if (generic_buttons_row(building_info_x, building_info_y, buttons_figure_list.buttons, info_figure_count, &focus_button_id)) {
        return;
    }
    if (generic_buttons_row(building_info_x + PHRASE_BUTTON_X, building_info_y + PHRASE_BUTTON_Y, buttons_figure_phrase.phrase, 1, &focus_button_id_secondary)) {
        return;
    }
    int32_t button = figure_get(info_figure_ids[info_figure_index])->phrase_group;
    if (tutorial_flags_b[button]) {
        int32_t x = building_info_x + TUTORIAL_BUTTON_X;
        int32_t y = bottom_buttons_y();
        image_buttons_set_tooltip(x, y, tutorial_buttons[button], 1, button + TUTORIAL_TOOLTIP_BASE);
        image_buttons_handle_mouse(x, y, tutorial_buttons[button], 1);
    }
}

static int32_t orders_button_y(void)
{
    return (building_info_h * BLOCK_SIZE) - ORDERS_BUTTON_FROM_BOTTOM + building_info_y;
}

static int32_t handle_granary_buttons(void)
{
    int32_t x = building_info_x;
    if (building_info_orders_shown == 0) {
        return generic_buttons_row(x + ORDERS_BUTTON_X, orders_button_y(), buttons_granary_orders.buttons, 1, &focus_button_id);
    }
    return generic_buttons_row(x + BACK_BUTTON_X, orders_bottom(city_foods.count) - GRANARY_BACK_FROM_BOTTOM, buttons_back_orders.buttons, 1, &focus_button_id);
}

static void handle_building_buttons(void)
{
    int32_t x = building_info_x;
    int32_t type = building_info_building()->type;
    if (type > BUILDING_CLEAR_LAND && type < BUILDING_BANDSTAND) {
        int32_t y = ((building_info_h - HOUSE_VENTURES_FROM_BOTTOM_BLOCKS) * BLOCK_SIZE) + building_info_y;
        if (generic_buttons_row(x + HOUSE_VENTURES_X, y, buttons_house_ventures.buttons, 1, &focus_button_id_secondary)) {
            return;
        }
    }
    if (is_granary(building_info_building()->type) && handle_granary_buttons()) {
        return;
    }
    type = building_info_building()->type;
    if (is_bazaar(type)) {
        if (building_info_orders_shown != 0) {
            generic_buttons_row(x + BACK_BUTTON_X, BAZAAR_BACK_BUTTON_Y, buttons_back_orders.buttons, 1, &focus_button_id);
            return;
        }
        generic_buttons_row(x + ORDERS_BUTTON_X, orders_button_y(), buttons_bazaar_orders.buttons, 1, &focus_button_id);
    } else if (is_storage_yard(type)) {
        if (building_info_orders_shown == 0) {
            generic_buttons_row(x + ORDERS_BUTTON_X, orders_button_y(), buttons_storage_orders.buttons, 1, &focus_button_id);
            return;
        }
        int32_t rows = storage_orders_shown_rows();
        if (storage_orders_scrollbar() == 0) {
            generic_buttons_row(x + BACK_BUTTON_X, orders_bottom(rows) - STORAGE_BACK_FROM_BOTTOM, buttons_storage_back.buttons, 1, &focus_button_id);
        }
    } else if (type == BUILDING_FESTIVAL_SQUARE) {
        if (generic_buttons_row(x, building_info_y, festival_square_button.festival_square, 1, &focus_button_id) == 0) {
            window_building_festival_button();
        }
    } else if (type == BUILDING_PERSONAL_MANSION || type == BUILDING_FAMILY_MANSION || type == BUILDING_DYNASTY_MANSION) {
        generic_buttons_row(x, building_info_y, mansion_salary_button.mansion_salary, 1, &focus_button_id);
    }
}

void window_building_info_handle_input(void)
{
    focus_button_id = 0;
    if (mouse_right_went_up) {
        handle_right_click();
        return;
    }
    if (handle_window_buttons() || building_info_mode == BUILDING_INFO_MODE_NONE) {
        return;
    }
    int32_t x = building_info_x;
    int32_t y = building_info_y;
    switch (building_info_mode) {
        case BUILDING_INFO_MODE_LEGION: {
            if (generic_buttons_row(x, y, buttons_legion.buttons, LEGION_ORDER_BUTTONS, &focus_button_id)) {
                return;
            }
            int32_t return_x = (((info_w_blocks - LEGION_RETURN_WIDTH_BLOCKS) * BLOCK_SIZE) / 2) + x;
            generic_buttons_row(return_x, bottom_buttons_y(), buttons_legion_return.buttons, 1, &focus_button_id_secondary);
            return;
        }
        case BUILDING_INFO_MODE_TRANSPORT_SHIP:
            generic_buttons_row(x, y, buttons_boat.buttons, SHIP_ORDER_BUTTONS, &focus_button_id);
            return;
        case BUILDING_INFO_MODE_WARSHIP:
            generic_buttons_row(x, y, buttons_ship.orders, SHIP_ORDER_BUTTONS, &focus_button_id);
            return;
        default:
            break;
    }
    if (bi_figure_drawn != 0) {
        handle_figure_list();
        return;
    }
    if (building_info_mode == BUILDING_INFO_MODE_BUILDING) {
        handle_building_buttons();
    }
}

void window_building_draw_figure_in_city(int32_t grid_offset, int32_t figure_id)
{
    int32_t camera_x = map_info.camera_x;
    int32_t camera_y = map_info.camera_y;
    city_view_grid_offset_to_xy_view(grid_offset);
    map_info.camera_x = map_result_x - FIGURE_CAMERA_LEFT_TILES;
    map_info.camera_y = map_result_y - FIGURE_CAMERA_UP_TILES;
    city_view_check_camera_boundaries();
    city_view_set_editor_viewport();
    city_draw_figure_in_view(map_info.camera_x, map_info.camera_y, map_info.grid_start, figure_id);
    map_info.camera_y = camera_y;
    map_info.camera_x = camera_x;
    if (g_sidebar_collapsed == 1) {
        city_view_set_viewport_without_sidebar();
    } else {
        city_view_set_viewport_with_sidebar();
    }
}

static uint32_t *figure_picture(int32_t index)
{
    return (uint32_t *) (spare_screen + building_info_figure_picture_offsets[index]);
}

void window_building_prepare_figure_list(void)
{
    if (info_figure_count <= 0) {
        return;
    }
    int32_t image = image_group(IMAGE_GROUP_PANEL) + MENU_BAR_PANEL_IMAGE;
    if (image_collection.image_count <= image) {
        image = 0;
    }
    int32_t min_y = image_record_height(image);
    for (int32_t i = 0; i < info_figure_count; i++) {
        int32_t figure_id = info_figure_ids[i];
        window_building_draw_figure_in_city(figure_get(figure_id)->grid_offset, figure_id);
        int32_t x = city_view.selected_figure_pixel_x;
        int32_t y = city_view.selected_figure_pixel_y;
        g_draw_x = x;
        g_draw_y = y;
        g_img_width = FIGURE_PICTURE_BLIT_WIDTH;
        g_img_height = FIGURE_PICTURE_BLIT_HEIGHT;
        if (x < 0) {
            g_draw_x = 0;
        }
        if (y < min_y) {
            g_draw_y = min_y;
        }
        graphics_save_to_buffer(figure_picture(i), g_screen_pitch - FIGURE_PICTURE_ROW_BYTES, 0);
    }
    widget_city_draw_current_view();
}

void window_building_draw_no_info(void)
{
    int32_t x = building_info_x;
    int32_t y = building_info_y;
    int32_t width = info_w_blocks * BLOCK_SIZE;
    outer_panel_draw(x, y, info_w_blocks, building_info_h);
    lang_text_draw_centered_ui(LANG_GROUP_TERRAIN_INFO, TERRAIN_TEXT_NO_PEOPLE, x, y + TITLE_Y, width, image_group(IMAGE_GROUP_FONT_LARGE), 0, x, y);
    int32_t hint_y = (building_info_h * BLOCK_SIZE) - HINT_FROM_BOTTOM + y;
    lang_text_draw_centered_ui(LANG_GROUP_WINDOW_HINTS, WINDOW_HINT_RIGHT_CLICK_TO_CONTINUE, x, hint_y, width, image_group(IMAGE_GROUP_FONT_GREEN), 0, x, y);
}

static int32_t terrain_help_message(int32_t kind)
{
    switch (kind) {
        case TERRAIN_INFO_ROAD:
            return building_info_help_messages[BUILDING_INFO_HELP_ROAD];
        case TERRAIN_INFO_CANAL:
            return building_info_help_messages[BUILDING_INFO_HELP_IRRIGATION_DITCH];
        case TERRAIN_INFO_MUD_WALL:
        case TERRAIN_INFO_BRICK_WALL:
        case TERRAIN_INFO_STRONG_WALL:
            return building_info_help_messages[BUILDING_INFO_HELP_WALLS];
        case TERRAIN_INFO_BRIDGE:
            return building_info_help_messages[BUILDING_INFO_HELP_BRIDGE];
        default:
            return 0;
    }
}

static building_info_draw_fn terrain_panel(int32_t kind)
{
    switch (kind) {
        case TERRAIN_INFO_GARDEN:
            return window_building_draw_gardens;
        case TERRAIN_INFO_PLAZA:
            return (info_figure_count <= 0) ? window_building_draw_plaza : 0;
        case TERRAIN_INFO_MUD_WALL:
            return window_building_draw_mud_wall;
        case TERRAIN_INFO_BRICK_WALL:
            return window_building_draw_brick_wall;
        case TERRAIN_INFO_STRONG_WALL:
            return window_building_draw_strong_wall;
        case TERRAIN_INFO_CANAL:
            return window_building_draw_canal;
        case TERRAIN_INFO_RUBBLE:
            return window_building_draw_ruins;
        default:
            return 0;
    }
}

void window_building_draw_terrain(void)
{
    int32_t kind = info_terrain_kind;
    info_help_id = terrain_help_message(kind);
    building_info_draw_fn panel = terrain_panel(kind);
    if (panel) {
        panel();
        return;
    }
    if (info_first_draw != 0) {
        info_first_draw = 0;
        if (info_figure_count > 0) {
            figure_phrase_play();
        }
    }
    if (info_figure_count > 0 && info_figure_ids[info_figure_index] != 0) {
        int32_t from_shipwreck_on = figure_get(info_figure_ids[info_figure_index])->type >= FIGURE_SHIPWRECK;
        info_help_id = building_info_help_messages[from_shipwreck_on ? BUILDING_INFO_HELP_FIGURE_FROM_SHIPWRECK_ON : BUILDING_INFO_HELP_FIGURE];
    }
    window_building_prepare_figure_list();
    int32_t x = building_info_x;
    int32_t y = building_info_y;
    outer_panel_draw(x, y, info_w_blocks, building_info_h);
    if (info_figure_count == 0) {
        lang_text_draw_centered_ui(
            LANG_GROUP_TERRAIN_INFO, info_terrain_kind + TERRAIN_TITLE_TEXT, x, y + TERRAIN_TITLE_Y, info_w_blocks * BLOCK_SIZE, image_group(IMAGE_GROUP_FONT_LARGE), 0, x, y);
    }
    kind = info_terrain_kind;
    if (kind != TERRAIN_INFO_ROAD && kind != TERRAIN_INFO_PLAZA) {
        int32_t text_y = (building_info_h * BLOCK_SIZE) - TERRAIN_DESCRIPTION_FROM_BOTTOM + y;
        int32_t width = (info_w_blocks - TERRAIN_DESCRIPTION_MARGIN_BLOCKS) * BLOCK_SIZE;
        lang_text_draw_multiline_ui(LANG_GROUP_TERRAIN_INFO, kind + TERRAIN_DESCRIPTION_TEXT, x + TERRAIN_DESCRIPTION_X, text_y, width, image_group(IMAGE_GROUP_FONT_BLACK), 0, x, y);
    }
    window_building_draw_figure_list();
}

void window_building_draw_figure_list(void)
{
    int32_t x = building_info_x;
    int32_t y = building_info_y;
    inner_panel_draw_ui(
        x + FIGURE_LIST_PANEL_X, y + FIGURE_LIST_PANEL_Y, info_w_blocks - FIGURE_LIST_PANEL_MARGIN_BLOCKS, FIGURE_LIST_PANEL_HEIGHT_BLOCKS, x, y);
    if (info_figure_count <= 0) {
        lang_text_draw_centered_ui(
            LANG_GROUP_TERRAIN_INFO, TERRAIN_TEXT_NO_PEOPLE, x, y + FIGURE_LIST_EMPTY_TEXT_Y, info_w_blocks * BLOCK_SIZE, image_group(IMAGE_GROUP_FONT_GREEN), 0, x, y);
        bi_figure_drawn = 1;
        return;
    }
    for (int32_t i = 0, dx = 0; i < info_figure_count; i++, dx += FIGURE_LIST_STEP) {
        button_border_draw_ui(
            dx + FIGURE_LIST_BORDER_X + x, y + FIGURE_LIST_BORDER_Y, FIGURE_LIST_BORDER_SIZE, FIGURE_LIST_BORDER_SIZE, i == info_figure_index, x, y);
        g_draw_y = y + FIGURE_LIST_PICTURE_Y;
        g_draw_x = dx + FIGURE_LIST_PICTURE_X + x;
        g_img_width = FIGURE_PICTURE_BLIT_WIDTH;
        g_img_height = FIGURE_PICTURE_BLIT_HEIGHT;
        graphics_draw_from_buffer(figure_picture(i), g_screen_pitch - FIGURE_PICTURE_ROW_BYTES, 0);
    }
    window_building_draw_figure_info(info_figure_ids[info_figure_index]);
    bi_figure_drawn = 1;
}

static int32_t god_portrait(int32_t building_id, int32_t fallback_type)
{
    int32_t priests = image_group(IMAGE_GROUP_BIG_PEOPLE_3);
    switch (building_temple_god(building_id)) {
        case GOD_RA:
            return priests + BIG_PRIEST_RA;
        case GOD_PTAH:
            return priests + BIG_PRIEST_PTAH;
        case GOD_SETH:
            return priests + BIG_PRIEST_SETH;
        case GOD_BAST:
            return priests + BIG_PRIESTESS_BAST;
        default:
            return fallback_type + image_group(IMAGE_GROUP_BIG_PEOPLE);
    }
}

static int32_t hunter_portrait(void)
{
    int32_t prey = climate_prey(scenario_data.climate_type)->figure_type;
    switch (prey) {
        case FIGURE_BIRDS:
            return image_group(IMAGE_GROUP_BIG_PEOPLE_3) + BIG_BIRD_HUNTER;
        case FIGURE_OSTRICH:
            return image_group(IMAGE_GROUP_BIG_PEOPLE_2) + BIG_OSTRICH_HUNTER;
        case FIGURE_ANTELOPE:
            return image_group(IMAGE_GROUP_BIG_PEOPLE) + FIGURE_HUNTER;
        default:
            return image_group(IMAGE_GROUP_BIG_PEOPLE);
    }
}

int32_t window_building_figure_big_image(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    int32_t type = f->type;
    int32_t portraits = image_group(IMAGE_GROUP_BIG_PEOPLE_4);
    switch (type) {
        case FIGURE_TOMB_ROBBER:
            return portraits + BIG_TOMB_ROBBER;
        case FIGURE_PRIEST:
            return god_portrait(f->building_id, f->type);
        case FIGURE_LIBRARIAN:
            return image_group(IMAGE_GROUP_BIG_PEOPLE) + FIGURE_TEACHER;
        case FIGURE_HUNTER:
            return hunter_portrait();
        case FIGURE_FESTIVAL_PRIEST:
            if (f->resource_id != FIGURE_PRIEST) {
                return f->resource_id + image_group(IMAGE_GROUP_BIG_PEOPLE);
            }
            return god_portrait(f->building_id, f->resource_id);
        case FIGURE_ASP:
            return portraits + BIG_ASP;
        case FIGURE_LION:
            return portraits + BIG_LION;
        case FIGURE_SCORPION:
            return portraits + BIG_SCORPION;
        case FIGURE_ZOOKEEPER:
            return portraits + BIG_ZOOKEEPER;
        case FIGURE_FROG:
            return portraits + BIG_FROG;
        case FIGURE_LOCUST:
            return portraits + BIG_LOCUST;
        case FIGURE_TOMB_ARTISAN:
            return portraits + BIG_TOMB_ARTISAN;
        case FIGURE_MUMMY:
            return portraits + BIG_MUMMY;
        default:
            return image_group(IMAGE_GROUP_BIG_PEOPLE) + type;
    }
}

void window_building_draw_resource_icon(int32_t resource_index, int32_t x, int32_t y)
{
    int32_t text_width = g_text_width;
    image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + resource_index, x, y);
    g_text_width = text_width;
}

static int32_t inventory_resource(int32_t slot)
{
    switch (slot) {
        case INVENTORY_GRAIN:
            return RESOURCE_GRAIN;
        case INVENTORY_MEAT:
            return RESOURCE_MEAT;
        case INVENTORY_LETTUCE:
            return RESOURCE_LETTUCE;
        case INVENTORY_CHICKPEAS:
            return RESOURCE_CHICKPEAS;
        case INVENTORY_POMEGRANATES:
            return RESOURCE_POMEGRANATES;
        case INVENTORY_FIGS:
            return RESOURCE_FIGS;
        case INVENTORY_FISH:
            return RESOURCE_FISH;
        case INVENTORY_GAMEMEAT:
            return RESOURCE_GAMEMEAT;
        case INVENTORY_BEER:
            return RESOURCE_BEER;
        case INVENTORY_POTTERY:
            return RESOURCE_POTTERY;
        case INVENTORY_LINEN:
            return RESOURCE_LINEN;
        case INVENTORY_JEWELRY:
            return RESOURCE_LUXURY_GOODS;
        default:
            return -1;
    }
}

void window_building_draw_carried_goods(int32_t figure_id, int32_t x, int32_t y)
{
    int32_t text_width = g_text_width;
    const figure *f = figure_get(figure_id);
    for (int32_t i = 0; i < CARRIED_GOODS_SLOTS; i++) {
        int32_t slot = f->buyer_resources[i];
        if (slot != INVENTORY_NONE) {
            image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + inventory_resource(slot), x, y);
            x += CARRIED_GOODS_STEP;
            g_text_width = text_width;
        }
    }
}

static void draw_text(int32_t group, int32_t id, int32_t x, int32_t y, int32_t font)
{
    lang_text_draw_ui(group, id, x, y, font, 0, building_info_x, building_info_y);
}

static int32_t resource_is_stone(int32_t resource)
{
    switch (resource) {
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_MARBLE:
            return 1;
        default:
            return 0;
    }
}

static int32_t resource_is_counted_in_units(int32_t resource)
{
    return resource_is_stone(resource) || resource == RESOURCE_CHARIOTS || resource == RESOURCE_WEAPONS || resource == RESOURCE_DEBEN;
}

static int32_t trade_amount(int32_t resource, int32_t amount)
{
    return resource_is_counted_in_units(resource) ? amount : (amount * UNITS_PER_LOAD);
}

static int32_t trade_ship_text(int32_t action)
{
    switch (action) {
        case FIGURE_ACTION_TRADE_SHIP_ANCHORED:
            return FIGURE_INFO_TEXT_SHIP_ANCHORED;
        case FIGURE_ACTION_TRADE_SHIP_DOCKED:
            return FIGURE_INFO_TEXT_SHIP_DOCKED;
        case FIGURE_ACTION_TRADE_SHIP_LEAVING:
            return FIGURE_INFO_TEXT_RETURNING_HOME;
        default:
            return FIGURE_INFO_TEXT_SHIP_SAILING_TO_DOCKS;
    }
}

static int32_t caravan_text(int32_t action, int32_t has_traded)
{
    switch (action) {
        case FIGURE_ACTION_TRADE_CARAVAN_TRADING:
            return FIGURE_INFO_TEXT_CARAVAN_TRADING;
        case FIGURE_ACTION_TRADE_CARAVAN_LEAVING:
            return has_traded ? FIGURE_INFO_TEXT_RETURNING_HOME : FIGURE_INFO_TEXT_NOTHING_TO_TRADE;
        case FIGURE_ACTION_TRADE_CARAVAN_ARRIVING:
            return FIGURE_INFO_TEXT_CARAVAN_TO_STORAGE_YARDS;
        default:
            return FIGURE_INFO_TEXT_RETURNING_HOME;
    }
}

static void draw_city_trade_row(const uint8_t *trades, int32_t city, int32_t text, int32_t text_y, int32_t icon_y, int32_t font)
{
    int32_t x = building_info_x + TRADER_X;
    g_text_width = 0;
    draw_text(LANG_GROUP_FIGURE_INFO, text, x, building_info_y + text_y, font);
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (trades[r] && empire_city_has_trade_limit(city, r)) {
            image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + r, g_text_width + x, building_info_y + icon_y);
            g_text_width += TRADE_ICON_STEP;
        }
    }
}

static void draw_trader_row(const uint8_t *amounts, int32_t text, int32_t text_y, int32_t icon_y, int32_t font)
{
    int32_t x = building_info_x + TRADER_X;
    g_text_width = 0;
    draw_text(LANG_GROUP_FIGURE_INFO, text, x, building_info_y + text_y, font);
    for (int32_t r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        int32_t amount = amounts[r];
        if (amount) {
            text_draw_number_ui(trade_amount(r, amount), NUMBER_PREFIX_NONE, postfix_none, g_text_width + x, building_info_y + text_y, font, 0, building_info_x,
                building_info_y);
            image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + r, g_text_width + x, building_info_y + icon_y);
            g_text_width += TRADE_ICON_STEP;
        }
    }
}

static const figure *trader_leader(const figure *f)
{
    if (f->type == FIGURE_TRADE_CARAVAN_DONKEY) {
        f = figure_get(f->leading_figure_id);
    }
    if (f->type == FIGURE_TRADE_CARAVAN_DONKEY) {
        f = figure_get(f->leading_figure_id);
    }
    return f;
}

static const figure *draw_trader_info(const figure *f, int32_t type, int32_t font)
{
    f = trader_leader(f);
    int32_t city = f->empire_city_id;
    int32_t x = building_info_x + TRADER_X;
    int32_t y = building_info_y;
    g_text_width = 0;
    draw_text(LANG_GROUP_FIGURE_TYPES, type, x, y + TRADER_CITY_Y, font);
    int32_t names = settings_data.classical_names ? LANG_GROUP_CITY_NAMES_CLASSICAL : LANG_GROUP_CITY_NAMES;
    draw_text(names, empire_city_get(city)->name_id, g_text_width + x, y + TRADER_CITY_Y, font);
    const empire_trader *trader = &empire_map_routes.traders[f->trader_id];
    int32_t has_traded = trader->sold_amount != 0 || trader->bought_amount != 0;
    int32_t text = (type == FIGURE_TRADE_SHIP) ? trade_ship_text(f->action_state) : caravan_text(f->action_state, has_traded);
    draw_text(LANG_GROUP_FIGURE_INFO, text, x, y + TRADER_STATE_Y, font);
    if (!has_traded) {
        const empire_city *trade_city = empire_city_get(city);
        draw_city_trade_row(trade_city->buys, city, FIGURE_INFO_TEXT_BUYS, TRADER_FIRST_ROW_Y, TRADER_FIRST_ICON_Y, font);
        draw_city_trade_row(trade_city->sells, city, FIGURE_INFO_TEXT_SELLS, TRADER_SECOND_ROW_Y, TRADER_SECOND_ICON_Y, font);
    } else {
        draw_trader_row(trader->bought_resources, FIGURE_INFO_TEXT_BOUGHT, TRADER_FIRST_ROW_Y, TRADER_FIRST_ICON_Y, font);
        draw_trader_row(trader->sold_resources, FIGURE_INFO_TEXT_SOLD, TRADER_SECOND_ROW_Y, TRADER_SECOND_ICON_Y, font);
    }
    return f;
}

static void draw_name(const figure *f, int32_t x)
{
    if (f->name_group != -1) {
        draw_text(f->name_group, f->name_id, x, building_info_y + NAME_Y, image_group(IMAGE_GROUP_FONT_LARGE_B));
    }
}

static void draw_phrase(const figure *f, int32_t phrase_group, int32_t y, int32_t font)
{
    int8_t phrase = (int8_t) f->phrase_id;
    if (phrase_group != -1 && phrase >= 0) {
        int32_t width = (info_w_blocks - PHRASE_MARGIN_BLOCKS) * BLOCK_SIZE;
        lang_text_draw_multiline_ui(phrase_group, phrase, building_info_x + PHRASE_X, building_info_y + y, width, font, 0, building_info_x, building_info_y);
    }
}

static void draw_portrait(int32_t big_image)
{
    if (big_image > -1) {
        image_draw(big_image, building_info_x + PORTRAIT_X, building_info_y + PORTRAIT_Y);
    }
}

static int32_t carrier_is_returning(const figure *f)
{
    int32_t action = f->action_state;
    switch (f->type) {
        case FIGURE_CART_PUSHER:
            return action == FIGURE_ACTION_CART_RETURNING;
        case FIGURE_STORAGE_YARD_CART:
            return action == FIGURE_ACTION_STORAGE_CART_RETURNING_EMPTY || action == FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_FOOD ||
                action == FIGURE_ACTION_STORAGE_CART_RETURNING_WITH_GOODS;
        case FIGURE_DOCKER:
            return action == FIGURE_ACTION_DOCKER_EXPORT_QUEUE || action == FIGURE_ACTION_DOCKER_EXPORT_RETURNING || action == FIGURE_ACTION_DOCKER_RETURNING;
        default:
            return 0;
    }
}

static int32_t docker_is_idle(const figure *f)
{
    return f->type == FIGURE_DOCKER && f->action_state == FIGURE_ACTION_DOCKER_IDLE;
}

static void draw_carrier_load_name(const figure *f, int32_t amount, int32_t font)
{
    int32_t resource = (int8_t) f->resource_id;
    int32_t x = g_text_width + LOAD_TEXT_X + building_info_x;
    int32_t y = building_info_y + LOAD_TEXT_Y;
    if (amount == 0) {
        return;
    }
    if (resource_is_stone(resource)) {
        draw_text(LANG_GROUP_STORAGE_YARD, (amount == 1) ? STORAGE_TEXT_BLOCK_OF : STORAGE_TEXT_BLOCKS_OF, x, y, font);
    } else if (resource != RESOURCE_CHARIOTS && resource != RESOURCE_WEAPONS && resource != RESOURCE_NONE) {
        draw_text(LANG_GROUP_FIGURE_INFO, FIGURE_INFO_TEXT_UNITS_OF, x, y, font);
    }
    int32_t name = f->resource_id;
    if ((name == RESOURCE_CHARIOTS || name == RESOURCE_WEAPONS) && amount != 1) {
        name += RESOURCE_TEXT_PLURAL_OFFSET;
    }
    draw_text(LANG_GROUP_RESOURCE_NAMES, name, g_text_width + LOAD_TEXT_X + building_info_x, y, font);
}

static void draw_carrier_route(const figure *f, int32_t font)
{
    int32_t home_id = f->building_id;
    int32_t destination_id = f->destination_building_id;
    if (home_id == 0 || destination_id == 0) {
        return;
    }
    g_text_width = 0;
    int32_t returning = carrier_is_returning(f);
    if (docker_is_idle(f)) {
        return;
    }
    int32_t x = building_info_x + DESTINATION_X;
    int32_t y = building_info_y + DESTINATION_Y;
    int32_t target_id = returning ? home_id : destination_id;
    int32_t source_id = returning ? destination_id : home_id;
    draw_text(LANG_GROUP_FIGURE_INFO, returning ? FIGURE_INFO_TEXT_RETURNING_TO : FIGURE_INFO_TEXT_GOING_TO, x, y, font);
    draw_text(LANG_GROUP_BUILDING_TYPES, building_get(target_id)->type, g_text_width + x, y, font);
    draw_text(LANG_GROUP_FIGURE_INFO, FIGURE_INFO_TEXT_CARRIER_FROM, g_text_width + x, y, font);
    draw_text(LANG_GROUP_BUILDING_TYPES, building_get(source_id)->type, g_text_width + x, y, font);
}

static void draw_carrier_info(const figure *f, int32_t big_image, int32_t phrase_group, int32_t font)
{
    int32_t x = building_info_x;
    int32_t y = building_info_y;
    draw_portrait(big_image);
    g_text_width = 0;
    draw_name(f, x + NAME_X);
    g_text_width = 0;
    draw_text(LANG_GROUP_BUILDING_TYPES, building_get(f->building_id)->type, x + KIND_X, y + KIND_Y, font);
    draw_text(LANG_GROUP_FIGURE_TYPES, FIGURE_CART_PUSHER, g_text_width + KIND_X + x, y + KIND_Y, font);
    g_text_width = 0;
    if (!docker_is_idle(f) && f->resource_amount != 0 && f->resource_id != RESOURCE_NONE) {
        image_draw(f->resource_id + image_group(IMAGE_GROUP_RESOURCE_ICONS), x + KIND_X, y + LOAD_ICON_Y);
    }
    int32_t amount = f->resource_amount;
    if (resource_is_counted_in_units((int8_t) f->resource_id)) {
        amount /= UNITS_PER_LOAD;
    }
    if (amount != 0) {
        text_draw_number_ui(amount, NUMBER_PREFIX_NONE, postfix_none, g_text_width + LOAD_TEXT_X + x, y + LOAD_TEXT_Y, font, 0, x, y);
    }
    draw_carrier_load_name(f, amount, font);
    draw_phrase(f, phrase_group, CARRIER_PHRASE_Y, font);
    draw_carrier_route(f, font);
}

static void draw_kind_with_name(const figure *f, int32_t kind_text, int32_t font)
{
    g_text_width = 0;
    draw_name(f, building_info_x + NAME_X);
    g_text_width = 0;
    draw_text(LANG_GROUP_FIGURE_TYPES, kind_text, building_info_x + KIND_X, building_info_y + KIND_Y, font);
}

static void draw_market_buyer_info(int32_t figure_id, int32_t big_image, int32_t phrase_group, int32_t font)
{
    const figure *f = figure_get(figure_id);
    draw_portrait(big_image);
    draw_kind_with_name(f, FIGURE_MARKET_BUYER, font);
    int32_t y = building_info_y;
    if (f->action_state == FIGURE_ACTION_MARKET_BUYER_TO_STORAGE) {
        draw_text(LANG_GROUP_FIGURE_INFO, FIGURE_INFO_TEXT_COLLECTING, g_text_width + STATE_X + building_info_x, y + KIND_Y, font);
        window_building_draw_carried_goods(figure_id, g_text_width + STATE_X + building_info_x, y + GOODS_ICON_Y);
    } else if (f->action_state == FIGURE_ACTION_MARKET_BUYER_RETURNING) {
        draw_text(LANG_GROUP_FIGURE_INFO, FIGURE_INFO_TEXT_RETURNING_WITH, g_text_width + STATE_X + building_info_x, y + KIND_Y, font);
        window_building_draw_figure_goods(figure_id, g_text_width + STATE_X + building_info_x, y + GOODS_ICON_Y);
    }
    draw_phrase(f, phrase_group, PHRASE_Y, font);
}

static void draw_delivery_boy_info(const figure *f, int32_t big_image, int32_t phrase_group, int32_t font)
{
    draw_portrait(big_image);
    int32_t icon = inventory_resource(f->collecting_item_id);
    draw_kind_with_name(f, FIGURE_DELIVERY_BOY, font);
    draw_text(LANG_GROUP_FIGURE_INFO, FIGURE_INFO_TEXT_CARRYING, g_text_width + STATE_X + building_info_x, building_info_y + KIND_Y, font);
    window_building_draw_resource_icon(icon, g_text_width + STATE_X + building_info_x, building_info_y + GOODS_ICON_Y);
    draw_phrase(f, phrase_group, PHRASE_Y, font);
}

static void draw_hunter_info(const figure *f, int32_t big_image, int32_t font)
{
    draw_kind_with_name(f, FIGURE_HUNTER, font);
    draw_portrait(big_image);
    if (f->action_state == FIGURE_ACTION_HUNTER_RETURN_WITH_PREY) {
        int32_t x = g_text_width + HUNTER_GOODS_X + building_info_x;
        draw_text(LANG_GROUP_FIGURE_INFO, FIGURE_INFO_TEXT_RETURNING_WITH, x, building_info_y + KIND_Y, font);
        image_draw(image_group(IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_GAMEMEAT, g_text_width + HUNTER_GOODS_X + building_info_x, building_info_y + GOODS_ICON_Y);
    }
}

static void draw_walker_info(const figure *f, int32_t type, int32_t big_image, int32_t font)
{
    if (f->type == FIGURE_SLED) {
        big_image = -1;
    }
    if (f->type == FIGURE_LABORER) {
        type = (f->leading_figure_id != 0) ? FIGURE_SLED_PULLER : FIGURE_LABORER;
    }
    draw_portrait(big_image);
    g_text_width = 0;
    if (f->name_group > -1) {
        draw_text(f->name_group, f->name_id, building_info_x + NAME_X, building_info_y + NAME_Y, image_group(IMAGE_GROUP_FONT_LARGE_B));
    }
    g_text_width = 0;
    int32_t x = g_text_width + KIND_X + building_info_x;
    if (type == FIGURE_PRIEST && building_temple_god(f->building_id) == GOD_BAST) {
        draw_text(LANG_GROUP_RELIGION_ADVISOR, RELIGION_TEXT_PRIESTESS, x, building_info_y + KIND_Y, font);
    } else {
        draw_text(LANG_GROUP_FIGURE_TYPES, type, x, building_info_y + KIND_Y, font);
    }
}

static int32_t is_enemy_soldier(int32_t type)
{
    return (type > FIGURE_TOWER_SENTRY && type < FIGURE_ENEMY_EGYPTIAN_INFANTRY) || type == FIGURE_BEDOUIN;
}

static int32_t is_enemy_ship(int32_t type)
{
    return type == FIGURE_ENEMY_TRANSPORT_SHIP || type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP ||
        type == FIGURE_ENEMY_EGYPTIAN_WARSHIP;
}

static int32_t is_boat_or_wildlife(int32_t type)
{
    return type == FIGURE_FISHING_BOAT || type == FIGURE_SHIPWRECK || type == FIGURE_BIRDS || type == FIGURE_OSTRICH || type == FIGURE_ANTELOPE;
}

static int32_t is_carrier(int32_t type)
{
    return type == FIGURE_CART_PUSHER || type == FIGURE_STORAGE_YARD_CART || type == FIGURE_DOCKER;
}

static int32_t is_trader(int32_t type)
{
    return type == FIGURE_TRADE_CARAVAN || type == FIGURE_TRADE_CARAVAN_DONKEY || type == FIGURE_TRADE_SHIP;
}

static void draw_tutorial_button(const figure *f)
{
    int32_t button = f->phrase_group;
    if (button != 0 && tutorial_flags_b[button]) {
        image_buttons_draw(building_info_x + TUTORIAL_BUTTON_X, bottom_buttons_y(), tutorial_buttons[button], 1);
    }
}

void window_building_draw_figure_info(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    int32_t type = f->type;
    int32_t phrase_group = figure_phrase_sound_id(figure_id);
    int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
    if (type == FIGURE_FESTIVAL_PRIEST) {
        type = f->resource_id;
    }
    info_help_id = building_info_help_messages[BUILDING_INFO_HELP_FIGURE];
    int32_t big_image = window_building_figure_big_image(figure_id);
    int32_t panel_width = (info_w_blocks - FIGURE_PANEL_MARGIN_BLOCKS) * BLOCK_SIZE;
    button_border_draw_ui(
        building_info_x + FIGURE_PANEL_X, building_info_y + FIGURE_PANEL_Y, panel_width, FIGURE_PANEL_HEIGHT, 0, building_info_x, building_info_y);
    int32_t own_type = f->type;
    if (is_trader(type)) {
        f = draw_trader_info(f, type, font);
    } else if (is_enemy_soldier(type)) {
        g_text_width = 0;
        draw_name(f, building_info_x + ENEMY_NAME_X);
        g_text_width = 0;
        int32_t enemy = formation_get(f->formation_id)->enemy_type;
        draw_text(LANG_GROUP_ENEMY_NAMES, enemy + ENEMY_TEXT_SOLDIER_OF, building_info_x + ENEMY_NAME_X, building_info_y + ENEMY_KIND_Y, font);
    } else if (is_enemy_ship(own_type)) {
        g_text_width = 0;
        draw_text(LANG_GROUP_FIGURE_TYPES, own_type, building_info_x + PORTRAIT_X, building_info_y + NAME_Y, image_group(IMAGE_GROUP_FONT_LARGE_B));
    } else if (is_boat_or_wildlife(own_type)) {
        draw_portrait(big_image);
        g_text_width = 0;
        draw_text(LANG_GROUP_FIGURE_TYPES, type, building_info_x + KIND_X, building_info_y + KIND_Y, font);
    } else if (is_carrier(type)) {
        draw_carrier_info(f, big_image, phrase_group, font);
    } else if (type == FIGURE_MARKET_BUYER) {
        draw_market_buyer_info(figure_id, big_image, phrase_group, font);
    } else if (type == FIGURE_DELIVERY_BOY) {
        draw_delivery_boy_info(f, big_image, phrase_group, font);
    } else if (type == FIGURE_HUNTER) {
        draw_hunter_info(f, big_image, font);
        draw_phrase(f, phrase_group, PHRASE_Y, font);
    } else {
        draw_walker_info(f, type, big_image, font);
        draw_phrase(f, phrase_group, PHRASE_Y, font);
    }
    draw_tutorial_button(f);
}

void building_info_toggle_details(void)
{
    building_info_details_shown = building_info_details_shown != 1;
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void building_info_toggle_dc(void)
{
    building *b = building_info_building();
    if (b->info_orders_toggle == ORDERS_TOGGLE_OFF) {
        b->info_orders_toggle = ORDERS_TOGGLE_ON;
    } else if (b->info_orders_toggle == ORDERS_TOGGLE_ON) {
        b->info_orders_toggle = ORDERS_TOGGLE_OFF;
    }
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void window_building_info_help(void)
{
    window_message_dialog_show((info_help_id < 1) ? building_info_help_messages[BUILDING_INFO_HELP_WINDOW] : info_help_id, 0, 0);
    window_redraw_request = WINDOW_REDRAW_ALL;
}

void window_building_info_select_figure(void)
{
    info_figure_index = button_parameter;
    figure_phrase_play();
    window_redraw_request = 1;
}
