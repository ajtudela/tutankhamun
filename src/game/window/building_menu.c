// The enable flags of the build menu: which rows of each submenu the scenario allows (its buildings, gods and temple
// complexes, the empire's resources, the kingdom rank, the tutorial gates of the first missions), the number of rows a
// submenu shows and the sidebar's build buttons.

#include "window/building_menu.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/resource_type.h"
#include "editor/editor_allowed_buildings.h"
#include "empire/empire.h"
#include "empire/empire_helpers.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tutorial.h"
#include "platform/log.h"
#include "widget/widget_data.h"
#include "window/build_menu.h"
#include "window/window_data.h"
#include <stdbool.h>
#include <stdint.h>

enum building_menu_rules {
    BUILD_MENU_ENABLED_COUNT = BUILD_MENU_GROUPS * BUILD_MENU_ITEMS,
    BUILDING_ROAD_SYSTEM = 137,
    RANK_TOWN_FIRST = 6,
    RANK_CITY_FIRST = 8,
    RANK_CITY_END = 11,
    MISSION_FIRST = 0,
    MISSION_WITHOUT_JEWELER = 5,
    MISSION_BRICKWORKS_FIRST = 7,
    MISSION_LAST_TUTORIAL = 10,
    TUTORIAL_ENTRIES = 26,
    TEMPLE_COMPLEX_FIRST_UPGRADE = 1,
    TEMPLE_COMPLEX_SECOND_UPGRADE = 2,
    SIDEBAR_BUILD_BUTTONS = 12,
};

static bool allowed(int32_t building)
{
    return scenario_data.allowed_buildings[building] != 0;
}

static bool god_allowed(int32_t god)
{
    return scenario_data.god_status[god] != 0;
}

static bool temple_complex_allowed(int32_t god)
{
    return scenario_data.temple_complexes_allowed[god] != 0;
}

int32_t building_menu_tutorial_ok(void)
{
    if ((int32_t) settings_data.mission_id == tutorial_briefing_tables.tutorial.entries[TUTORIAL_FLAG_BRICKS_STORED].mission) {
        return tutorial_flags_a[TUTORIAL_FLAG_BRICKS_STORED] != 0;
    }
    return 1;
}

void building_menu_enable_all(void)
{
    for (int32_t i = 0; i < BUILD_MENU_ENABLED_COUNT; i++) {
        menu_enabled[i] = 1;
    }
}

static void enable_if(int32_t index, int32_t condition)
{
    if (condition) {
        menu_enabled[index] = 1;
    }
}

static void enable_if_allowed(int32_t index, int32_t building)
{
    enable_if(index, allowed(building));
}

static void enable_if_produce(int32_t index, int32_t resource)
{
    enable_if(index, empire_can_produce_resource(resource));
}

static void enable_if_produce_potentially(int32_t index, int32_t resource)
{
    enable_if(index, empire_can_produce_resource_potentially(resource));
}

static void enable_if_rank(int32_t index, int32_t building, int32_t first_rank, int32_t end_rank)
{
    if (allowed(building)) {
        int32_t rank = city_player_current()->player_rank;
        enable_if(index, rank >= first_rank && rank < end_rank);
    }
}

static bool has_monument_guild_demand(void)
{
    return guild_has_monument_demand(BUILDING_STONEMASONS_GUILD) || guild_has_monument_demand(BUILDING_BRICKLAYERS_GUILD) ||
        guild_has_monument_demand(BUILDING_CARPENTERS_GUILD);
}

static void apply_tutorial_menu_overrides(int32_t type, int32_t index, int32_t mission)
{
    for (int32_t entry = 0; entry < TUTORIAL_ENTRIES; entry++) {
        const tutorial_entry *tutorial = &tutorial_briefing_tables.tutorial.entries[entry];
        if (mission != tutorial->mission || tutorial->building_type_count <= 0) {
            continue;
        }
        for (int32_t i = 0; i < tutorial->building_type_count; i++) {
            if (type == tutorial->building_types[i]) {
                menu_enabled[index] = tutorial_flags_a[entry] != 0;
            }
        }
    }
}

void building_menu_update(void)
{
    for (int32_t group = 0; group < BUILD_MENU_GROUPS; group++) {
        for (int32_t item = 0; item < BUILD_MENU_ITEMS; item++) {
            int32_t index = item + group * BUILD_MENU_ITEMS;

            if (cheat_allow_all == 1 || editor_active == 1) {
                menu_enabled[index] = 1;
                continue;
            }

            int32_t type = build_menu_types[BUILD_MENU_TYPE_FIRST + index];
            menu_enabled[index] = 0;

            enable_if(index, type >= BUILDING_HOUSE_CRUDE_HUT && type <= BUILDING_HOUSE_PALATIAL_ESTATE);
            if (type == BUILDING_CLEAR_LAND || type == BUILDING_WELL || type == BUILDING_ROAD_SYSTEM || type == BUILDING_ROAD ||
                type == BUILDING_ARCHITECT_POST || type == BUILDING_FIREHOUSE || type == BUILDING_POLICE_STATION) {
                menu_enabled[index] = 1;
            } else if (type == BUILDING_WATER_SUPPLY) {
                enable_if_allowed(index, ALLOWED_WATER_SUPPLY);
            } else if (type == BUILDING_HUNTING_LODGE) {
                enable_if_produce(index, RESOURCE_GAMEMEAT);
            } else if (type == BUILDING_GRANARY) {
                enable_if_allowed(index, ALLOWED_GRANARY);
            } else if (type == BUILDING_SCRIBAL_SCHOOL) {
                enable_if_allowed(index, ALLOWED_SCRIBAL_SCHOOL);
            } else if (type == BUILDING_LIBRARY) {
                enable_if_allowed(index, ALLOWED_LIBRARY);
            } else if (type == BUILDING_PHYSICIAN) {
                enable_if_allowed(index, ALLOWED_PHYSICIAN);
            } else if (type == BUILDING_APOTHECARY) {
                enable_if_allowed(index, ALLOWED_APOTHECARY);
            } else if (type == BUILDING_MORTUARY) {
                enable_if_allowed(index, ALLOWED_MORTUARY);
            } else if (type == BUILDING_COURTHOUSE) {
                enable_if_allowed(index, ALLOWED_COURTHOUSE);
            } else if (type == BUILDING_FERRY_LANDING) {
                enable_if_allowed(index, ALLOWED_FERRY);
            } else if (type == BUILDING_GOLD_MINE) {
                enable_if_allowed(index, ALLOWED_GOLD_MINE);
            } else if (type == BUILDING_BOOTH || type == BUILDING_JUGGLER_SCHOOL) {
                enable_if_allowed(index, ALLOWED_BOOTH);
            } else if (type == BUILDING_FESTIVAL_SQUARE) {
                enable_if_allowed(index, ALLOWED_FESTIVAL_SQUARE);
            } else if (type == BUILDING_FIG_FARM) {
                enable_if_produce(index, RESOURCE_FIGS);
            } else if (type == BUILDING_FLAX_FARM) {
                enable_if_produce(index, RESOURCE_FLAX);
            } else if (type == BUILDING_HENNA_FARM) {
                enable_if_produce(index, RESOURCE_HENNA);
            } else if (type == BUILDING_BARLEY_FARM) {
                enable_if_produce(index, RESOURCE_BARLEY);
            } else if (type == BUILDING_CLAY_PIT) {
                enable_if_produce(index, RESOURCE_CLAY);
            } else if (type == BUILDING_POTTER) {
                enable_if_produce_potentially(index, RESOURCE_CLAY);
            } else if (type == BUILDING_GARDENS) {
                enable_if_allowed(index, ALLOWED_GARDENS);
            } else if (type == BUILDING_BREWERY) {
                enable_if_produce_potentially(index, RESOURCE_BEER);
            } else if (type == BUILDING_ROADBLOCK) {
                enable_if_allowed(index, ALLOWED_ROADBLOCK);
            } else if (type == BUILDING_LOW_BRIDGE) {
                enable_if_allowed(index, ALLOWED_BRIDGE);
            } else if (type == BUILDING_TAX_COLLECTOR) {
                enable_if_allowed(index, ALLOWED_TAX_COLLECTOR);
            } else if (type == BUILDING_PAPYRUS_MAKER) {
                enable_if_produce_potentially(index, RESOURCE_REEDS);
            } else if (type == BUILDING_LAMP_MAKER) {
                if (empire_can_produce_resource_potentially(RESOURCE_OIL)) {
                    enable_if_produce_potentially(index, RESOURCE_POTTERY);
                }
            } else if (type == BUILDING_PAINT_MAKER) {
                enable_if_produce_potentially(index, RESOURCE_HENNA);
            } else if (type == BUILDING_REED_GATHERER) {
                enable_if_produce(index, RESOURCE_REEDS);
            } else if (type == BUILDING_BAZAAR) {
                enable_if_allowed(index, ALLOWED_BAZAAR);
            } else if (type == BUILDING_STORAGE_YARD) {
                enable_if_allowed(index, ALLOWED_STORAGE_YARD);
            } else if (type == BUILDING_WORK_CAMP) {
                enable_if_allowed(index, ALLOWED_WORK_CAMP);
            } else if (type == BUILDING_WEAVER) {
                enable_if_produce_potentially(index, RESOURCE_FLAX);
            } else if (type == BUILDING_TEMPLE_OSIRIS || type == BUILDING_SHRINE_OSIRIS) {
                enable_if(index, god_allowed(GOD_OSIRIS));
            } else if (type == BUILDING_TEMPLE_BAST || type == BUILDING_SHRINE_BAST) {
                enable_if(index, god_allowed(GOD_BAST));
            } else if (type == BUILDING_CARPENTERS_GUILD) {
                enable_if_produce_potentially(index, RESOURCE_TIMBER);
            } else if (type == BUILDING_BRICKLAYERS_GUILD) {
                if (empire_can_produce_resource(RESOURCE_BRICKS)) {
                    menu_enabled[index] = 1;
                } else {
                    enable_if_produce_potentially(index, RESOURCE_BRICKS);
                }
            } else if (type == BUILDING_SHIPWRIGHT) {
                enable_if(index, allowed(ALLOWED_FISHING_WHARF) || allowed(ALLOWED_TRANSPORT_WHARF) || allowed(ALLOWED_WARSHIP_WHARF));
            } else if (type == BUILDING_DENTIST) {
                enable_if_allowed(index, ALLOWED_DENTIST);
            } else if (type == BUILDING_VILLAGE_PALACE_UP) {
                enable_if_rank(index, ALLOWED_PALACES, 0, RANK_TOWN_FIRST);
            }

            if (building_menu_tutorial_ok() && type == BUILDING_MENU_CONSTRUCTION_GUILDS) {
                enable_if(index, has_monument_guild_demand());
            } else if (type == BUILDING_MENU_RAW_MATERIALS) {
                enable_if(index,
                    empire_can_produce_resource(RESOURCE_REEDS) || empire_can_produce_resource(RESOURCE_CLAY) || empire_can_produce_resource(RESOURCE_TIMBER) ||
                        empire_can_produce_resource(RESOURCE_STONE) || empire_can_produce_resource(RESOURCE_LIMESTONE) ||
                        empire_can_produce_resource(RESOURCE_SANDSTONE) || empire_can_produce_resource(RESOURCE_COPPER) ||
                        empire_can_produce_resource(RESOURCE_GEMS) || allowed(ALLOWED_GOLD_MINE));
            } else if (type == BUILDING_MENU_FARMS) {
                enable_if(index,
                    empire_can_produce_resource(RESOURCE_BARLEY) || empire_can_produce_resource(RESOURCE_GRAIN) ||
                        empire_can_produce_resource(RESOURCE_LETTUCE) || empire_can_produce_resource(RESOURCE_CHICKPEAS) ||
                        empire_can_produce_resource(RESOURCE_FLAX) || empire_can_produce_resource(RESOURCE_POMEGRANATES) ||
                        empire_can_produce_resource(RESOURCE_FIGS) || empire_can_produce_resource(RESOURCE_HENNA));
            } else if (type == BUILDING_TEMPLE_COMPLEX_OSIRIS) {
                enable_if(index, temple_complex_allowed(GOD_OSIRIS));
            } else if (type == BUILDING_TEMPLE_COMPLEX_PTAH) {
                enable_if(index, temple_complex_allowed(GOD_PTAH));
            } else if (type == BUILDING_TEMPLE_COMPLEX_SETH) {
                enable_if(index, temple_complex_allowed(GOD_SETH));
            } else if (type == BUILDING_TEMPLE_COMPLEX_RA) {
                enable_if(index, temple_complex_allowed(GOD_RA));
            } else if (type == BUILDING_TEMPLE_COMPLEX_BAST) {
                enable_if(index, temple_complex_allowed(GOD_BAST));
            } else if (type == BUILDING_MENU_TEMPLE_COMPLEXES) {
                enable_if(index,
                    temple_complex_allowed(GOD_OSIRIS) || temple_complex_allowed(GOD_PTAH) || temple_complex_allowed(GOD_SETH) ||
                        temple_complex_allowed(GOD_RA) || temple_complex_allowed(GOD_BAST));
            } else if (type == BUILDING_TEMPLE_RA) {
                enable_if(index, god_allowed(GOD_RA));
            } else if (type == BUILDING_TEMPLE_PTAH) {
                enable_if(index, god_allowed(GOD_PTAH));
            } else if (type == BUILDING_TEMPLE_SETH) {
                enable_if(index, god_allowed(GOD_SETH));
            } else if (type == BUILDING_MENU_TEMPLES || type == BUILDING_MENU_SHRINES) {
                enable_if(index, god_allowed(GOD_OSIRIS) || god_allowed(GOD_PTAH) || god_allowed(GOD_SETH) || god_allowed(GOD_RA) || god_allowed(GOD_BAST));
            } else if (type == BUILDING_SHRINE_RA) {
                enable_if(index, god_allowed(GOD_RA));
            } else if (type == BUILDING_SHRINE_PTAH) {
                enable_if(index, god_allowed(GOD_PTAH));
            } else if (type == BUILDING_SHRINE_SETH) {
                enable_if(index, god_allowed(GOD_SETH));
            } else if (type == BUILDING_SMALL_STATUE || type == BUILDING_MEDIUM_STATUE || type == BUILDING_LARGE_STATUE) {
                enable_if_allowed(index, ALLOWED_STATUES);
            } else if (type == BUILDING_MENU_BEAUTIFICATION) {
                enable_if(index, allowed(ALLOWED_STATUES) || allowed(ALLOWED_PLAZA) || allowed(ALLOWED_GARDENS));
            } else if (type == BUILDING_PLAZA) {
                enable_if_allowed(index, ALLOWED_PLAZA);
            } else if (type == BUILDING_MENU_WATER_CROSSINGS) {
                enable_if(index, allowed(ALLOWED_FERRY) || allowed(ALLOWED_BRIDGE));
            }

            int32_t custom = settings_data.is_custom_scenario;
            int8_t mission = settings_data.mission_id;

            if (custom == 0 && mission == MISSION_FIRST && type == BUILDING_POLICE_STATION) {
                menu_enabled[index] = 0;
                apply_tutorial_menu_overrides(type, index, mission);
                continue;
            }

            if (type == BUILDING_GRAIN_FARM) {
                enable_if_produce(index, RESOURCE_GRAIN);
            } else if (type == BUILDING_CATTLE_RANCH) {
                enable_if_produce(index, RESOURCE_MEAT);
            } else if (type == BUILDING_IRRIGATION_DITCH) {
                enable_if_allowed(index, ALLOWED_IRRIGATION_DITCH);
            } else if (type == BUILDING_TOWN_PALACE_UP) {
                enable_if_rank(index, ALLOWED_PALACES, RANK_TOWN_FIRST, RANK_CITY_FIRST);
            } else if (type == BUILDING_CITY_PALACE) {
                enable_if_rank(index, ALLOWED_PALACES, RANK_CITY_FIRST, RANK_CITY_END);
            } else if (type == BUILDING_PERSONAL_MANSION) {
                enable_if_rank(index, ALLOWED_MANSIONS, 0, RANK_TOWN_FIRST);
            } else if (type == BUILDING_FAMILY_MANSION) {
                enable_if_rank(index, ALLOWED_MANSIONS, RANK_TOWN_FIRST, RANK_CITY_FIRST);
            } else if (type == BUILDING_DYNASTY_MANSION) {
                enable_if_rank(index, ALLOWED_MANSIONS, RANK_CITY_FIRST, RANK_CITY_END);
            } else if (type == BUILDING_MENU_WATER_LIFT) {
                enable_if_allowed(index, ALLOWED_WATER_LIFT);
            } else if (type == BUILDING_BANDSTAND || type == BUILDING_CONSERVATORY) {
                enable_if_allowed(index, ALLOWED_BANDSTAND);
            } else if (type == BUILDING_PAVILION || type == BUILDING_DANCE_SCHOOL) {
                enable_if_allowed(index, ALLOWED_PAVILION);
            } else if (type == BUILDING_SENET_HOUSE) {
                enable_if_allowed(index, ALLOWED_SENET_HOUSE);
            } else if (type == BUILDING_ZOO) {
                enable_if_allowed(index, ALLOWED_ZOO);
            } else if (type == BUILDING_DOCK || type == BUILDING_DOCK_UP) {
                enable_if_allowed(index, ALLOWED_DOCK);
            } else if (type == BUILDING_FISHING_WHARF) {
                if (empire_can_produce_resource(RESOURCE_FISH)) {
                    enable_if_allowed(index, ALLOWED_FISHING_WHARF);
                }
            } else if (type == BUILDING_TRANSPORT_WHARF) {
                enable_if_allowed(index, ALLOWED_TRANSPORT_WHARF);
            } else if (type == BUILDING_WARSHIP_WHARF) {
                enable_if_allowed(index, ALLOWED_WARSHIP_WHARF);
            } else if (type == BUILDING_WALL) {
                enable_if_allowed(index, ALLOWED_WALL);
            } else if (type == BUILDING_TOWER) {
                enable_if_allowed(index, ALLOWED_TOWER);
            } else if (type == BUILDING_GATEHOUSE_UP) {
                enable_if_allowed(index, ALLOWED_GATEHOUSE);
            } else if (type == BUILDING_FORT) {
                enable_if(index, allowed(ALLOWED_FORT_INFANTRY) || allowed(ALLOWED_FORT_ARCHERS) || allowed(ALLOWED_FORT_CHARIOTEERS));
            } else if (type == BUILDING_MILITARY_ACADEMY) {
                enable_if_allowed(index, ALLOWED_ACADEMY);
            } else if (type == BUILDING_FORT_CHARIOTEERS) {
                enable_if_allowed(index, ALLOWED_FORT_CHARIOTEERS);
            } else if (type == BUILDING_FORT_ARCHERS) {
                enable_if_allowed(index, ALLOWED_FORT_ARCHERS);
            } else if (type == BUILDING_FORT_INFANTRY) {
                enable_if_allowed(index, ALLOWED_FORT_INFANTRY);
            } else if (type == BUILDING_RECRUITER) {
                enable_if_allowed(index, ALLOWED_RECRUITER);
            } else if (type == BUILDING_BAZAAR_UP) {
                enable_if_allowed(index, ALLOWED_BAZAAR);
            } else if (type == BUILDING_GRANARY_UP) {
                enable_if_allowed(index, ALLOWED_GRANARY);
            } else if (type == BUILDING_STORAGE_YARD_UP) {
                enable_if_allowed(index, ALLOWED_STORAGE_YARD);
            } else if (type == BUILDING_MENU_DEFENSES) {
                enable_if(index, allowed(ALLOWED_WALL) || allowed(ALLOWED_TOWER) || allowed(ALLOWED_GATEHOUSE));
            } else if (type == BUILDING_LETTUCE_FARM) {
                enable_if_produce(index, RESOURCE_LETTUCE);
            } else if (type == BUILDING_CHICKPEAS_FARM) {
                enable_if_produce(index, RESOURCE_CHICKPEAS);
            } else if (type == BUILDING_POMEGRANATES_FARM) {
                enable_if_produce(index, RESOURCE_POMEGRANATES);
            } else if (type == BUILDING_WEAPONSMITH) {
                enable_if_allowed(index, ALLOWED_WEAPONSMITH);
            } else if (type == BUILDING_JEWELER) {
                if (empire_can_produce_resource_potentially(RESOURCE_GEMS)) {
                    enable_if(index, !(mission == MISSION_WITHOUT_JEWELER && custom == 0));
                }
            } else if (type == BUILDING_STONE_QUARRY || type == BUILDING_STONEMASONS_GUILD) {
                enable_if_produce(index, RESOURCE_STONE);
            } else if (type == BUILDING_GEMSTONE_MINE) {
                enable_if_produce(index, RESOURCE_GEMS);
            } else if (type == BUILDING_COPPER_MINE) {
                enable_if_produce(index, RESOURCE_COPPER);
            } else if (type == BUILDING_WOOD_CUTTER) {
                enable_if_produce(index, RESOURCE_TIMBER);
            } else if (type == BUILDING_SANDSTONE_QUARRY) {
                enable_if_produce(index, RESOURCE_SANDSTONE);
            } else if (type == BUILDING_LIMESTONE_QUARRY) {
                enable_if_produce(index, RESOURCE_LIMESTONE);
            } else if (type == BUILDING_GRANITE_QUARRY) {
                enable_if_produce(index, RESOURCE_GRANITE);
            } else if (type == BUILDING_CHARIOT_MAKER) {
                enable_if_allowed(index, ALLOWED_CHARIOT_MAKER);
            } else if (type == BUILDING_BRICKWORKS) {
                if (custom != 0 || mission >= MISSION_BRICKWORKS_FIRST) {
                    int32_t produce_b = empire_can_produce_resource(RESOURCE_CLAY) || empire_can_produce_resource_potentially(RESOURCE_CLAY);
                    int32_t produce_1 = empire_can_produce_resource(RESOURCE_GRAIN);
                    enable_if(index,
                        (produce_b && produce_1) || (produce_b && empire_can_import_resource_potentially(RESOURCE_STRAW)) ||
                            (produce_1 && empire_can_import_resource_potentially(RESOURCE_CLAY)) ||
                            (empire_can_import_resource_potentially(RESOURCE_STRAW) && empire_can_import_resource_potentially(RESOURCE_CLAY)));
                }
            } else {
                if (type == BUILDING_TEMPLE_COMPLEX_GOD_5 || type == BUILDING_TEMPLE_COMPLEX_GOD_6 || type == BUILDING_TEMPLE_COMPLEX_GOD_7 ||
                    type == BUILDING_TEMPLE_COMPLEX_GOD_8 || type == BUILDING_TEMPLE_COMPLEX_GOD_9) {
                    menu_enabled[index] = 0;
                }
                if (type == BUILDING_TEMPLE_GOD_5 || type == BUILDING_TEMPLE_GOD_6 || type == BUILDING_TEMPLE_GOD_7 || type == BUILDING_TEMPLE_GOD_8 ||
                    type == BUILDING_TEMPLE_GOD_9) {
                    menu_enabled[index] = 0;
                }
                if (type == BUILDING_SHRINE_GOD_5 || type == BUILDING_SHRINE_GOD_6 || type == BUILDING_SHRINE_GOD_7 || type == BUILDING_SHRINE_GOD_8 ||
                    type == BUILDING_SHRINE_GOD_9) {
                    menu_enabled[index] = 0;
                }
                enable_if(index, type == BUILDING_TEMPLE_COMPLEX_ALTAR || type == BUILDING_TEMPLE_COMPLEX_ORACLE);
            }

            if (mission <= MISSION_LAST_TUTORIAL && custom == 0) {
                apply_tutorial_menu_overrides(type, index, mission);
            }
        }
    }
}

int32_t building_menu_count_items(void)
{
    int32_t count = 0;
    for (int32_t item = 0; item < BUILD_MENU_ITEMS; item++) {
        int32_t index = item + build_menu_group * BUILD_MENU_ITEMS;
        int32_t type = build_menu_types[BUILD_MENU_TYPE_FIRST + index];
        if (type < 1) {
            return count;
        }
        int32_t counted;
        switch (build_menu_group) {
            case BUILD_MENU_RELIGION:
                if (type == BUILDING_MENU_MONUMENTS) {
                    counted = building_menu_tutorial_ok() && city_count_monuments() < scenario_count_monuments();
                } else if (type == BUILDING_MENU_TEMPLE_COMPLEXES && city_player_current()->has_temple_complex) {
                    counted = !building_temple_complex_has_upgrade(city_player_current()->temple_complex_building_id, TEMPLE_COMPLEX_FIRST_UPGRADE) ||
                        !building_temple_complex_has_upgrade(city_player_current()->temple_complex_building_id, TEMPLE_COMPLEX_SECOND_UPGRADE);
                } else {
                    counted = menu_enabled[index] != 0;
                }
                break;
            case BUILD_MENU_INDUSTRY:
                if (type == BUILDING_MENU_CONSTRUCTION_GUILDS) {
                    counted = building_menu_tutorial_ok() && has_monument_guild_demand();
                } else {
                    counted = menu_enabled[index] != 0;
                }
                break;
            case BUILD_MENU_GUILDS:
                counted = building_menu_tutorial_ok() && guild_has_monument_demand(type);
                break;
            case BUILD_MENU_MONUMENTS:
                counted = building_menu_tutorial_ok() && scenario_data.monument_def_ids[type - 1] != 0 && city_player_current()->monument_def_id[type - 1] == 0;
                break;
            case BUILD_MENU_TEMPLE_COMPLEX_UPGRADES:
                counted = !building_temple_complex_has_upgrade(city_player_current()->temple_complex_building_id, item + 1);
                break;
            default:
                counted = menu_enabled[index] != 0;
                break;
        }
        if (counted) {
            count++;
        }
    }
    return count;
}

static void update_sidebar_button(image_button *button)
{
    button->enabled = 1;
    build_menu_group = button->parameter1;
    if (building_menu_count_items() < 1) {
        button->enabled = 0;
    }
}

void sidebar_build_buttons_update(void)
{
    for (int32_t i = 0; i < SIDEBAR_BUILD_BUTTONS; i++) {
        update_sidebar_button(&buttons_expanded_build[i]);
        update_sidebar_button(&buttons_collapsed_build[i]);
    }
}

