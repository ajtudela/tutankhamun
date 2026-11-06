// Warnings of the build tool: after a building passed the placement checks, hints that it will lack workers, road
// access, raw materials or a school for its entertainers; they never block the building. A missing raw material
// adds a hint to produce it, open a trade route or import it.
//
// Some hints are not the ones you would expect:
// - the zoo asks for the import of meat, not game meat;
// - a lamp maker lacking both oil and pottery swaps the trade route and import hints;
// - the papyrus users count only "import", not "import as needed", as imported.

#include "building/construction_warnings.h"
#include "building/building_figure_helpers.h"
#include "building/building_stocks.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "building/monument_tables.h"
#include "building/pyramid.h"
#include "city/city_player.h"
#include "city/food_supply.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "map/construction_terrain.h"
#include "map/grid.h"
#include "map/road_access.h"
#include "map/terrain.h"
#include "scenario/event_manager.h"
#include "scenario/scenario_internal.h"
#include "window/window_data.h"
#include <stdint.h>

int32_t construction_warning_rules[CONSTRUCTION_WARNING_RULE_COUNT] = {
    [CONSTRUCTION_WARNING_RULE_MIN_POPULATION] = 300,
    [CONSTRUCTION_WARNING_RULE_MIN_WORKERS_NEEDED] = 10,
    [CONSTRUCTION_WARNING_RULE_MAX_UNEMPLOYMENT_PERCENT] = 10,
    [CONSTRUCTION_WARNING_RULE_MIN_FOOD_PRODUCED_PERCENT] = 96,
    [CONSTRUCTION_WARNING_RULE_MAX_PROBLEM_MESSAGES] = 3,
};

static void warn(int32_t warning)
{
    city_warning_show(warning);
}

static int32_t imported(const city_player *player, int32_t resource)
{
    int16_t status = player->trade_status[resource];
    return status == TRADE_STATUS_IMPORT || status == TRADE_STATUS_IMPORT_AS_NEEDED;
}

static void hint_raw_material(const city_player *player, int32_t produce, int32_t import, int32_t w_produce, int32_t w_no_import, int32_t w_not_imported)
{
    if (empire_can_produce_resource(produce)) {
        warn(w_produce);
        return;
    }
    if (!empire_can_import_resource(import)) {
        warn(w_no_import);
        return;
    }
    if (!imported(player, import)) {
        warn(w_not_imported);
    }
}

static void hint_import(const city_player *player, int32_t resource, int32_t w_no_import, int32_t w_not_imported)
{
    if (!empire_can_import_resource(resource)) {
        warn(w_no_import);
    } else if (!imported(player, resource)) {
        warn(w_not_imported);
    }
}

static int32_t is_shrine(int32_t type)
{
    return type >= BUILDING_SHRINE_OSIRIS && type <= BUILDING_SHRINE_GOD_9;
}

void construction_warning_check_road_access(int32_t type, int32_t x, int32_t y, int32_t size)
{
    if (editor_active == 1) {
        return;
    }
    switch (type) {
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_LARGE_STATUE:
        case BUILDING_WELL:
        case BUILDING_FORT:
        case BUILDING_MUD_GATEHOUSE:
        case BUILDING_BRICK_GATEHOUSE:
        case BUILDING_GATEHOUSE:
        case BUILDING_MUD_GATEHOUSE_UP:
        case BUILDING_BRICK_GATEHOUSE_UP:
        case BUILDING_GATEHOUSE_UP:
        case BUILDING_TRIUMPHAL_ARCH:
        case BUILDING_HOUSE_CRUDE_HUT:
        case BUILDING_FORT_CHARIOTEERS:
        case BUILDING_FORT_ARCHERS:
        case BUILDING_FORT_INFANTRY:
        case BUILDING_TEMPLE_COMPLEX_ALTAR:
        case BUILDING_TEMPLE_COMPLEX_ORACLE:
        case BUILDING_PYRAMID:
        case BUILDING_SPHINX:
        case BUILDING_CAESAREUM:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_ABU_SIMBEL:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
            return;
        default:
            break;
    }
    if (map_has_road_access(x, y, size)) {
        return;
    }
    if ((type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP) && map_has_road_access(x, y, STORAGE_YARD_SIZE)) {
        return;
    }
    if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        return;
    }
    if (is_shrine(type) && map_has_road_access_nearby(x, y)) {
        return;
    }
    if (city_message_count_problem < construction_warning_rules[CONSTRUCTION_WARNING_RULE_MAX_PROBLEM_MESSAGES]) {
        if (is_shrine(type)) {
            warn(WARNING_SHRINES_NEAR_ROAD_REQUIRED);
            return;
        }
        warn(WARNING_NEEDS_ROAD_ACCESS);
    }
}

void construction_warning_check_workers(int32_t type)
{
    if (editor_active != 1 && type != BUILDING_WELL && city_warning_shown == 0 && building_model_get(type)->laborers > 0 &&
        city_player_current()->workers_needed >= construction_warning_rules[CONSTRUCTION_WARNING_RULE_MIN_WORKERS_NEEDED]) {
        warn(WARNING_CITY_NEEDS_MORE_WORKERS);
    }
}

void construction_warning_check_unemployment(int32_t type)
{
    (void) type;
    const city_player *player = city_player_current();
    if (editor_active != 1 && (settings_data.is_custom_scenario != 0 || settings_data.mission_id != 0) &&
        player->population >= construction_warning_rules[CONSTRUCTION_WARNING_RULE_MIN_POPULATION] &&
        player->unemployment_percentage > construction_warning_rules[CONSTRUCTION_WARNING_RULE_MAX_UNEMPLOYMENT_PERCENT]) {
        warn(WARNING_TOO_FEW_JOBS_FOR_POPULATION);
    }
}

void construction_warning_check_labor_need(int32_t type)
{
    (void) type;
    const city_player *player = city_player_current();
    if (editor_active != 1 && player->population >= construction_warning_rules[CONSTRUCTION_WARNING_RULE_MIN_POPULATION] &&
        player->food_stored_total <= (player->food_needed_per_month / 2)) {
        warn(WARNING_LOW_FOOD_LEVELS);
    }
}

static int32_t needs_defense(int32_t type)
{
    if ((type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) || (type >= BUILDING_TEMPLE_OSIRIS && type <= BUILDING_TEMPLE_BAST) ||
        is_shrine(type)) {
        return 1;
    }
    switch (type) {
        case BUILDING_BANDSTAND:
        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_PAVILION:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_POLICE_STATION:
        case BUILDING_PERSONAL_MANSION:
        case BUILDING_FAMILY_MANSION:
        case BUILDING_DYNASTY_MANSION:
        case BUILDING_ARCHITECT_POST:
        case BUILDING_TAX_COLLECTOR:
        case BUILDING_ROADBLOCK:
        case BUILDING_FIREHOUSE:
        case BUILDING_COURTHOUSE:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_ZOO:
            return 1;
        default:
            return 0;
    }
}

void construction_warning_check_military(int32_t type)
{
    if (editor_active == 1 || !needs_defense(type)) {
        return;
    }
    scenario_event *event = scenario_invasion_next(&scenario_events, 0, 0);
    if (!event) {
        return;
    }
    while (!scenario_event_is_running(event)) {
        event = scenario_invasion_next(&scenario_events, event, 0);
        if (!event) {
            return;
        }
    }
    if (formation_num_legions == 0 && city_player_current()->total_warships == 0) {
        warn(WARNING_NO_TROOPS_FOR_DEFENSE);
    }
}

void construction_warning_check_granary(int32_t type)
{
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_GRANARY && building_count_active[BUILDING_BAZAAR] < 1) {
        warn(WARNING_BUILD_BAZAARS_TO_DISTRIBUTE_FOOD);
    }
}

void construction_warning_check_booth(int32_t type)
{
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_BOOTH && building_count_active[BUILDING_JUGGLER_SCHOOL] < 1) {
        warn(WARNING_BUILD_JUGGLING_SCHOOL);
    }
}

void construction_warning_check_bandstand(int32_t type)
{
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_BANDSTAND && building_count_active[BUILDING_CONSERVATORY] < 1) {
        warn(WARNING_BUILD_CONSERVATORY);
    }
}

void construction_warning_check_pavilion(int32_t type)
{
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_PAVILION && building_count_active[BUILDING_DANCE_SCHOOL] < 1) {
        warn(WARNING_BUILD_DANCE_SCHOOL);
    }
}

void construction_warning_check_fort(int32_t type)
{
    if (editor_active != 1 && city_warning_shown == 0 &&
        (type == BUILDING_FORT_CHARIOTEERS || type == BUILDING_FORT_ARCHERS || type == BUILDING_FORT_INFANTRY) &&
        building_count_active[BUILDING_RECRUITER] < 1) {
        warn(WARNING_NEEDS_RECRUITER_TO_CONSCRIPT);
    }
}

void construction_warning_check_recruiter(int32_t type)
{
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_RECRUITER && city_player_current()->storage_stored[RESOURCE_WEAPONS] < 1) {
        warn(WARNING_SOLDIERS_NEED_SUPPLIES_OF_WEAPONS);
    }
}

void construction_warning_check_tower(int32_t type, int32_t x, int32_t y, int32_t size)
{
    if (editor_active != 1 && city_warning_shown == 0 && (type == BUILDING_MUD_TOWER || type == BUILDING_BRICK_TOWER || type == BUILDING_TOWER) &&
        !map_terrain_ring_has_wall(x, y, size)) {
        warn(WARNING_MUST_BE_NEXT_TO_WALL_FOR_PATROL);
    }
}

void construction_warning_check_well(int32_t type, int32_t x, int32_t y)
{
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_WELL && (map_grids.terrain[map_grid_offset(x, y)] & TERRAIN_GROUNDWATER) == 0) {
        warn(WARNING_NEEDS_ACCESS_TO_WATER_LIFT);
    }
}

void construction_warning_check_house_food(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active != 1 && city_warning_shown == 0 && type == BUILDING_HOUSE_CRUDE_HUT &&
        player->population >= construction_warning_rules[CONSTRUCTION_WARNING_RULE_MIN_POPULATION] && scenario_data.kingdom_supplies_grain == 0) {
        int32_t total = player->food_consumed_last_month;
        if (calc_percentage(city_resource_food_supply_percentage(), total) < construction_warning_rules[CONSTRUCTION_WARNING_RULE_MIN_FOOD_PRODUCED_PERCENT]) {
            warn(WARNING_PEOPLE_EATING_MORE_THAN_PRODUCED);
        }
    }
}

static void check_workshop(int32_t type, int32_t workshop_type, int32_t product, int32_t input, int32_t warning, int32_t w_produce)
{
    const city_player *player = city_player_current();
    if (editor_active != 1 && type == workshop_type && building_count_industry_active[input] < 1 && player->storage_stored[product] < 1 &&
        player->storage_stored[input] < 1) {
        warn(warning);
        hint_raw_material(player, input, input, w_produce, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
    }
}

void construction_warning_check_brewery(int32_t type)
{
    check_workshop(type, BUILDING_BREWERY, RESOURCE_BEER, RESOURCE_BARLEY, WARNING_NEEDS_BARLEY, WARNING_BUILD_BARLEY_FARM);
}

void construction_warning_check_weaver(int32_t type)
{
    check_workshop(type, BUILDING_WEAVER, RESOURCE_LINEN, RESOURCE_FLAX, WARNING_BUILDING_NEEDS_FLAX, WARNING_BUILD_FLAX_FARM);
}

void construction_warning_check_jeweler(int32_t type)
{
    check_workshop(type, BUILDING_JEWELER, RESOURCE_LUXURY_GOODS, RESOURCE_GEMS, WARNING_NEEDS_GEMS, WARNING_BUILD_GEM_MINE);
}

void construction_warning_check_potter(int32_t type)
{
    check_workshop(type, BUILDING_POTTER, RESOURCE_POTTERY, RESOURCE_CLAY, WARNING_BUILDING_NEEDS_CLAY, WARNING_BUILD_CLAY_PIT);
}

void construction_warning_check_papyrus(int32_t type)
{
    check_workshop(type, BUILDING_PAPYRUS_MAKER, RESOURCE_PAPYRUS, RESOURCE_REEDS, WARNING_BUILDING_NEEDS_REEDS, WARNING_BUILD_REED_GATHERER);
}

void construction_warning_check_paint(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active != 1 && type == BUILDING_PAINT_MAKER && building_count_industry_active[RESOURCE_HENNA] < 1 &&
        player->storage_stored[RESOURCE_PAINT] < 1 && player->storage_stored[RESOURCE_HENNA] < 1) {
        warn(WARNING_BUILDING_NEEDS_HENNA);
        hint_raw_material(player, RESOURCE_HENNA, RESOURCE_HENNA, WARNING_BUILD_HENNA_FARM, WARNING_IMPORT_HENNA_TRADE_ROUTE, WARNING_IMPORT_HENNA_OVERSEER);
    }
}

void construction_warning_check_cattle(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active != 1 && type == BUILDING_CATTLE_RANCH && building_count_industry_active[RESOURCE_GRAIN] < 1 &&
        player->storage_stored[RESOURCE_STRAW] < 1) {
        warn(WARNING_BUILDING_NEEDS_STRAW);
        hint_raw_material(player, RESOURCE_GRAIN, RESOURCE_STRAW, WARNING_BUILD_GRAIN_FARM, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT,
            WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
    }
}

static void hint_straw(const city_player *player, int32_t w_no_import, int32_t w_not_imported)
{
    hint_raw_material(player, RESOURCE_GRAIN, RESOURCE_STRAW, WARNING_BUILD_GRAIN_FARM, w_no_import, w_not_imported);
}

static void hint_clay(const city_player *player)
{
    if (empire_can_produce_resource(RESOURCE_CLAY)) {
        warn(WARNING_BUILD_CLAY_PIT);
    } else {
        hint_import(player, RESOURCE_CLAY, WARNING_IMPORT_CLAY_TRADE_ROUTE, WARNING_IMPORT_CLAY_OVERSEER);
    }
}

void construction_warning_check_bricks(int32_t type)
{
    if (editor_active == 1 || type != BUILDING_BRICKWORKS) {
        return;
    }
    int32_t clay = building_count_industry_active[RESOURCE_CLAY];
    int32_t straw = building_count_industry_active[RESOURCE_GRAIN];
    if (clay > 0 && straw > 0) {
        return;
    }
    const city_player *player = city_player_current();
    if (player->storage_stored[RESOURCE_BRICKS] > 0) {
        return;
    }
    int32_t clay_stock = player->storage_stored[RESOURCE_CLAY];
    if (clay_stock > 0 && player->storage_stored[RESOURCE_STRAW] > 0) {
        return;
    }
    if (clay <= 0 && clay_stock <= 0) {
        if (straw <= 0 && player->storage_stored[RESOURCE_STRAW] <= 0) {
            warn(WARNING_BUILDING_NEEDS_CLAY_AND_STRAW);
            hint_clay(player);
            hint_straw(player, WARNING_IMPORT_STRAW_TRADE_ROUTE, WARNING_IMPORT_STRAW_OVERSEER);
            return;
        }
        warn(WARNING_BUILDING_NEEDS_CLAY);
        hint_raw_material(player, RESOURCE_CLAY, RESOURCE_CLAY, WARNING_BUILD_CLAY_PIT, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
        return;
    }
    if (straw > 0 || player->storage_stored[RESOURCE_STRAW] > 0) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_STRAW);
    hint_straw(player, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
}

void construction_warning_check_lamps(int32_t type)
{
    if (editor_active == 1 || type != BUILDING_LAMP_MAKER) {
        return;
    }
    int32_t oil = building_count_industry_active[RESOURCE_OIL];
    int32_t pottery = building_count_industry_active[RESOURCE_POTTERY];
    if (oil > 0 && pottery > 0) {
        return;
    }
    const city_player *player = city_player_current();
    if (player->storage_stored[RESOURCE_LAMPS] > 0) {
        return;
    }
    int32_t oil_stock = player->storage_stored[RESOURCE_OIL];
    if (oil_stock > 0 && player->storage_stored[RESOURCE_POTTERY] > 0) {
        return;
    }
    if (oil <= 0 && oil_stock <= 0) {
        if (pottery <= 0 && player->storage_stored[RESOURCE_POTTERY] <= 0) {
            warn(WARNING_BUILDING_NEEDS_OIL_AND_POTTERY);
            hint_import(player, RESOURCE_OIL, WARNING_IMPORT_OIL_OVERSEER, WARNING_IMPORT_OIL_TRADE_ROUTE);
            hint_raw_material(player, RESOURCE_POTTERY, RESOURCE_POTTERY, WARNING_BUILD_POTTER, WARNING_IMPORT_POTTERY_OVERSEER, WARNING_IMPORT_POTTERY_TRADE_ROUTE);
            return;
        }
        warn(WARNING_BUILDING_NEEDS_OIL);
        hint_import(player, RESOURCE_OIL, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
        return;
    }
    if (pottery > 0 || player->storage_stored[RESOURCE_POTTERY] > 0) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_POTTERY);
    hint_raw_material(
        player, RESOURCE_POTTERY, RESOURCE_POTTERY, WARNING_BUILD_POTTER, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
}

static void hint_product_import(const city_player *player, int32_t product)
{
    if (empire_can_import_resource_potentially(product)) {
        hint_import(player, product, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
    }
}

void construction_warning_check_senet(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active == 1 || type != BUILDING_SENET_HOUSE || player->storage_stored[RESOURCE_BEER] > 0) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_BEER);
    if (building_count_industry_total[RESOURCE_BARLEY] > 0) {
        return;
    }
    if ((empire_our_city_sells_resource(RESOURCE_BARLEY) || empire_can_import_resource_potentially(RESOURCE_BARLEY)) &&
        building_count_industry_total[RESOURCE_BEER] == 0) {
        if (!empire_can_import_resource_potentially(RESOURCE_BEER)) {
            warn(WARNING_BUILD_BREWERY);
            return;
        }
        hint_import(player, RESOURCE_BEER, WARNING_TAX_COLLECTOR_ROBBED, WARNING_BUILD_WEAVER_OR_IMPORT_LINEN);
        return;
    }
    if (empire_can_import_resource_potentially(RESOURCE_BARLEY) && building_count_industry_total[RESOURCE_BEER] != 0) {
        hint_import(player, RESOURCE_BARLEY, WARNING_IMPORT_BARLEY_TRADE_ROUTE, WARNING_IMPORT_BARLEY_OVERSEER);
        return;
    }
    hint_product_import(player, RESOURCE_BEER);
}

void construction_warning_check_zoo(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active == 1 || type != BUILDING_ZOO || player->storage_stored[RESOURCE_GAMEMEAT] > 0) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_GAME_MEAT);
    if (building_count_industry_total[RESOURCE_GAMEMEAT] > 0) {
        return;
    }
    if ((empire_our_city_sells_resource(RESOURCE_GAMEMEAT) || empire_can_import_resource_potentially(RESOURCE_GAMEMEAT)) &&
        building_count_industry_total[RESOURCE_GAMEMEAT] == 0) {
        if (!empire_can_import_resource_potentially(RESOURCE_GAMEMEAT)) {
            warn(WARNING_BUILD_HUNTING_LODGE);
        } else if (!empire_can_import_resource(RESOURCE_GAMEMEAT)) {
            warn(WARNING_GAME_MEAT_IMPORT_TRADE_ROUTE);
        } else if (!imported(player, RESOURCE_MEAT)) {
            warn(WARNING_BUILD_HUNTING_LODGE_OR_IMPORT);
        }
    } else {
        hint_product_import(player, RESOURCE_GAMEMEAT);
    }
    if (building_count_industry_active[RESOURCE_GRAIN] > 0 || player->storage_stored[RESOURCE_STRAW] > 0) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_STRAW);
    hint_straw(player, WARNING_IMPORT_STRAW_TRADE_ROUTE, WARNING_IMPORT_STRAW_OVERSEER);
}

void construction_warning_check_papyrus_users(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active == 1 || (type != BUILDING_SCRIBAL_SCHOOL && type != BUILDING_LIBRARY) || player->storage_stored[RESOURCE_PAPYRUS] > 0) {
        return;
    }
    warn(WARNING_NEEDS_PAPYRUS);
    if (building_count_industry_total[RESOURCE_REEDS] >= 1) {
        return;
    }
    if ((empire_our_city_sells_resource(RESOURCE_REEDS) || empire_can_import_resource_potentially(RESOURCE_REEDS)) &&
        building_count_industry_total[RESOURCE_PAPYRUS] == 0) {
        if (!empire_can_import_resource_potentially(RESOURCE_PAPYRUS)) {
            warn(WARNING_BUILD_PAPYRUS_MAKER);
            return;
        }
        if (!empire_can_import_resource(RESOURCE_PAPYRUS)) {
            warn(WARNING_MANSION_DESTROYED);
            return;
        }
        if (player->trade_status[RESOURCE_PAPYRUS] != TRADE_STATUS_IMPORT) {
            warn(WARNING_COURTHOUSE_ROBBED);
        }
        return;
    }
    if (empire_can_import_resource_potentially(RESOURCE_REEDS) && building_count_industry_total[RESOURCE_PAPYRUS] != 0) {
        hint_import(player, RESOURCE_REEDS, WARNING_IMPORT_REEDS_TRADE_ROUTE, WARNING_IMPORT_REEDS_OVERSEER);
        return;
    }
    hint_product_import(player, RESOURCE_PAPYRUS);
}

void construction_warning_check_mortuary(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active == 1 || type != BUILDING_MORTUARY || player->storage_stored[RESOURCE_LINEN] > 0) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_LINEN);
    if (building_count_industry_total[RESOURCE_FLAX] > 0) {
        return;
    }
    if ((empire_our_city_sells_resource(RESOURCE_FLAX) || empire_can_import_resource_potentially(RESOURCE_FLAX)) &&
        building_count_industry_total[RESOURCE_LINEN] == 0) {
        if (!empire_can_import_resource_potentially(RESOURCE_LINEN)) {
            warn(WARNING_BUILD_WEAVER);
            return;
        }
        hint_import(player, RESOURCE_LINEN, WARNING_LINEN_IMPORT_TRADE_ROUTE, WARNING_LINEN_IMPORT_OVERSEER);
        return;
    }
    if (empire_can_import_resource_potentially(RESOURCE_FLAX) && building_count_industry_total[RESOURCE_LINEN] != 0) {
        hint_import(player, RESOURCE_FLAX, WARNING_IMPORT_FLAX_TRADE_ROUTE, WARNING_IMPORT_FLAX_OVERSEER);
        return;
    }
    hint_product_import(player, RESOURCE_LINEN);
}

void construction_warning_check_carpenters(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active != 1 && type == BUILDING_CARPENTERS_GUILD && building_count_industry_active[RESOURCE_TIMBER] < 1 &&
        player->storage_stored[RESOURCE_TIMBER] < 1) {
        warn(WARNING_BUILDING_NEEDS_WOOD);
        hint_raw_material(
            player, RESOURCE_TIMBER, RESOURCE_TIMBER, WARNING_BUILD_WOOD_CUTTER, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
    }
}

static int32_t has_material(const city_player *player, int32_t resource)
{
    return building_count_industry_active[resource] > 0 || player->storage_stored[resource] > 0;
}

void construction_warning_check_artisans(int32_t type)
{
    if (editor_active == 1 || type != BUILDING_ARTISANS_GUILD) {
        return;
    }
    const city_player *player = city_player_current();
    if (has_material(player, RESOURCE_CLAY) && has_material(player, RESOURCE_PAINT)) {
        return;
    }
    warn(WARNING_BUILDING_NEEDS_CLAY_AND_PAINT);
    if (building_count_industry_active[RESOURCE_CLAY] == 0 && player->storage_stored[RESOURCE_CLAY] == 0) {
        hint_clay(player);
    }
    if (building_count_industry_active[RESOURCE_PAINT] != 0 || player->storage_stored[RESOURCE_PAINT] != 0) {
        return;
    }
    if ((empire_our_city_sells_resource(RESOURCE_HENNA) || empire_can_import_resource_potentially(RESOURCE_HENNA)) &&
        building_count_industry_total[RESOURCE_PAINT] == 0) {
        if (!empire_can_import_resource_potentially(RESOURCE_PAINT)) {
            warn(WARNING_BUILD_PAINT_MAKER);
            return;
        }
        hint_import(player, RESOURCE_PAINT, WARNING_BUILD_PAINT_MAKER_OR_IMPORT, WARNING_PAINT_IMPORT_OVERSEER);
        return;
    }
    if (empire_can_import_resource_potentially(RESOURCE_HENNA) && building_count_industry_total[RESOURCE_PAINT] != 0) {
        hint_import(player, RESOURCE_HENNA, WARNING_IMPORT_HENNA_TRADE_ROUTE, WARNING_IMPORT_HENNA_OVERSEER);
        return;
    }
    hint_product_import(player, RESOURCE_PAINT);
}

static int32_t material_missing_hint(const city_player *player, int32_t resource, int32_t w_produce)
{
    if (empire_can_produce_resource(resource)) {
        warn(w_produce);
        return 1;
    }
    if (empire_can_import_resource(resource)) {
        if (!imported(player, resource)) {
            warn(WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
        }
        return 1;
    }
    return 0;
}

static void warn_missing_material(const city_player *player, int32_t resource, int32_t warning, int32_t w_produce)
{
    warn(warning);
    if (!material_missing_hint(player, resource, w_produce)) {
        warn(WARNING_SETUP_TRADE_ROUTE_TO_IMPORT);
    }
}

static void check_material_and_limestone(int32_t a, int32_t w_both, int32_t w_a_no_import, int32_t w_a_not_imported, int32_t w_a_produce, int32_t w_a)
{
    const city_player *player = city_player_current();
    int32_t b = RESOURCE_LIMESTONE;
    int32_t a_count = building_count_industry_active[a];
    int32_t b_count = building_count_industry_active[b];
    if (a_count > 0 && b_count > 0) {
        return;
    }
    int32_t a_stock = player->storage_stored[a];
    if (a_stock > 0 && player->storage_stored[b] > 0) {
        return;
    }
    if (a_count < 1 && a_stock < 1) {
        if (b_count < 1 && player->storage_stored[b] < 1) {
            warn(w_both);
            if (empire_can_produce_resource(a)) {
                warn(w_a_produce);
            } else {
                hint_import(player, a, w_a_no_import, w_a_not_imported);
            }
            if (empire_can_produce_resource(b)) {
                warn(WARNING_BUILD_LIMESTONE_QUARRY);
                return;
            }
            if (empire_can_import_resource(b)) {
                if (!imported(player, b)) {
                    warn(WARNING_IMPORT_LIMESTONE_OVERSEER);
                }
                return;
            }
            warn(WARNING_IMPORT_LIMESTONE_TRADE_ROUTE);
            return;
        }
        if (a_stock < 1) {
            warn_missing_material(player, a, w_a, w_a_produce);
            return;
        }
    }
    if (b_count > 0 || player->storage_stored[b] > 0) {
        return;
    }
    warn_missing_material(player, b, WARNING_MONUMENT_NEEDS_LIMESTONE, WARNING_BUILD_LIMESTONE_QUARRY);
}

static void check_single_material(int32_t resource, int32_t warning, int32_t w_produce)
{
    const city_player *player = city_player_current();
    if (building_count_industry_active[resource] > 0 || player->storage_stored[resource] > 0) {
        return;
    }
    warn_missing_material(player, resource, warning, w_produce);
}

void construction_warning_check_pyramid(int32_t type)
{
    if (editor_active == 1 || type != BUILDING_PYRAMID) {
        return;
    }
    switch (scenario_get_pyramid_style()) {
        case PYRAMID_STYLE_BENT:
        case PYRAMID_STYLE_PYRAMID:
            check_material_and_limestone(RESOURCE_STONE, WARNING_MONUMENT_NEEDS_STONE_AND_LIMESTONE, WARNING_IMPORT_STONE_TRADE_ROUTE, WARNING_IMPORT_STONE_OVERSEER,
                WARNING_BUILD_STONE_QUARRY, WARNING_MONUMENT_NEEDS_PLAIN_STONE);
            break;
        case PYRAMID_STYLE_MUDBRICK:
            check_material_and_limestone(RESOURCE_BRICKS, WARNING_MONUMENT_NEEDS_BRICKS_AND_LIMESTONE, WARNING_IMPORT_BRICKS_TRADE_ROUTE,
                WARNING_IMPORT_BRICKS_OVERSEER, WARNING_BUILD_BRICKWORKS, WARNING_MONUMENT_NEEDS_BRICKS);
            break;
        case PYRAMID_STYLE_STEPPED:
            check_single_material(RESOURCE_STONE, WARNING_MONUMENT_NEEDS_PLAIN_STONE, WARNING_BUILD_STONE_QUARRY);
            break;
        case PYRAMID_STYLE_MASTABA:
            check_single_material(RESOURCE_BRICKS, WARNING_MONUMENT_NEEDS_BRICKS, WARNING_BUILD_BRICKWORKS);
            break;
        default:
            break;
    }
}

void construction_warning_check_granite(int32_t type)
{
    const city_player *player = city_player_current();
    if (editor_active != 1 && (type == BUILDING_SUN_TEMPLE || type == BUILDING_MAUSOLEUM) && building_count_industry_active[RESOURCE_SANDSTONE] < 1 &&
        player->storage_stored[RESOURCE_SANDSTONE] < 1) {
        warn(WARNING_MONUMENT_NEEDS_SANDSTONE);
        hint_raw_material(player, RESOURCE_SANDSTONE, RESOURCE_SANDSTONE, WARNING_BUILD_SANDSTONE_QUARRY, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT,
            WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
    }
}

static void check_imported_material(const city_player *player, int32_t resource, int32_t warning)
{
    if (building_count_industry_active[resource] != 0 || player->storage_stored[resource] != 0) {
        return;
    }
    warn(warning);
    if (empire_can_import_resource(resource)) {
        if (!imported(player, resource)) {
            warn(WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
        }
        return;
    }
    warn(WARNING_SETUP_TRADE_ROUTE_TO_IMPORT);
}

void construction_warning_check_ptolemaic(int32_t type)
{
    if (editor_active == 1 || (type != BUILDING_LIGHTHOUSE && type != BUILDING_CAESAREUM && type != BUILDING_ALEXANDRIA_LIBRARY)) {
        return;
    }
    const city_player *player = city_player_current();
    if (player->storage_stored[RESOURCE_MARBLE] == 0) {
        warn(WARNING_MONUMENT_NEEDS_MARBLE);
        hint_import(player, RESOURCE_MARBLE, WARNING_SETUP_TRADE_ROUTE_TO_IMPORT, WARNING_OVERSEER_OF_COMMERCE_TO_IMPORT);
    }
    if (type == BUILDING_ALEXANDRIA_LIBRARY) {
        check_imported_material(player, RESOURCE_COPPER, WARNING_MONUMENT_NEEDS_COPPER);
    } else if (type == BUILDING_CAESAREUM) {
        check_imported_material(player, RESOURCE_GRANITE, WARNING_MONUMENT_NEEDS_GRANITE);
    }
}

void construction_warning_check_venue_schools(int32_t type)
{
    if (editor_active != 1 && (type == BUILDING_BOOTH || type == BUILDING_PAVILION || type == BUILDING_BANDSTAND)) {
        if (building_count_total[BUILDING_JUGGLER_SCHOOL] < 1) {
            warn(WARNING_BUILD_JUGGLERS_SCHOOL);
        }
        if (building_count_total[BUILDING_CONSERVATORY] < 1 && type != BUILDING_BOOTH) {
            warn(WARNING_VENUE_BUILD_CONSERVATORY);
        }
        if (building_count_total[BUILDING_DANCE_SCHOOL] < 1 && type == BUILDING_PAVILION) {
            warn(WARNING_VENUE_BUILD_DANCE_SCHOOL);
        }
    }
}
