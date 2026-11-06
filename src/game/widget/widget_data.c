// Initial data of the widgets as C tables: the sidebar, the advisor bar, the top menu, the minimap, the
// overlays and the construction ghosts. Some are loaded from config/*.json by widget/widget_config.c.

#include "building/building_type.h"
#include "figure/action.h"
#include "widget/sidebar_buttons.h"
#include "widget/widget_data.h"
#include "window/advisors.h"
#include "window/intermezzo.h"
#include "window/message_list.h"

int8_t tbl_cc_ori0_x[FIGURE_CROSS_COUNTRY_OFFSETS] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
    -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28,
    -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26,
    -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24,
    -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22,
    -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20,
    -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18,
    -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16,
    -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14,
    -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12,
    -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10,
    -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8,
    -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6,
    -26, -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4,
    -28, -26, -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2,
    -30, -28, -26, -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0,
};

int8_t tbl_cc_ori0_y[FIGURE_CROSS_COUNTRY_OFFSETS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
};

int8_t tbl_cc_ori2_x[FIGURE_CROSS_COUNTRY_OFFSETS] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
    2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32,
    4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34,
    6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36,
    8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
    10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40,
    12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42,
    14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44,
    16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46,
    18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48,
    20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50,
    22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52,
    24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54,
    26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56,
    28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58,
    30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60,
};

int8_t tbl_cc_ori2_y[FIGURE_CROSS_COUNTRY_OFFSETS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
    -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
    -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7,
    -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
    -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5,
    -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4,
    -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3,
    -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2,
    -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1,
    -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0,
};

int8_t tbl_progress_x[FIGURE_PROGRESS_OFFSETS] = {
    2, 4, 6, 8, 10, 12, 14, 16, -12, -10, -8, -6, -4, -2, 0, 0,
    4, 8, 12, 16, 20, 24, 28, 32, -24, -20, -16, -12, -8, -4, 0, 0,
    2, 4, 6, 8, 10, 12, 14, 16, -12, -10, -8, -6, -4, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -2, -4, -6, -8, -10, -12, -14, -16, 12, 10, 8, 6, 4, 2, 0, 0,
    -4, -8, -12, -16, -20, -24, -28, -32, 24, 20, 16, 12, 8, 4, 0, 0,
    -2, -4, -6, -8, -10, -12, -14, -16, 12, 10, 8, 6, 4, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int8_t tbl_progress_y[FIGURE_PROGRESS_OFFSETS] = {
    -1, -2, -3, -4, -5, -6, -7, -8, 6, 5, 4, 3, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 4, 5, 6, 7, 8, -6, -5, -4, -3, -2, -1, 0, 0,
    2, 4, 6, 8, 10, 12, 14, 16, -12, -10, -8, -6, -4, -2, 0, 0,
    1, 2, 3, 4, 5, 6, 7, 8, -6, -5, -4, -3, -2, -1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -2, -3, -4, -5, -6, -7, -8, 6, 5, 4, 3, 2, 1, 0, 0,
    -2, -4, -6, -8, -10, -12, -14, -16, 12, 10, 8, 6, 4, 2, 0, 0,
};

int32_t building_type_overlay_tile[BUILDING_OVERLAY_TILE_TYPES] = { 0, 0, 0, 0, 0, 1, 13, 2, 2, 0, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 4, 4,
    4, 4, 4, 4, 4, 0, 11, 11, 10, 11, 11, 11, 10, 10, 7, 7, 0, 7, 0, 6, 0, 6, 10, 8, 0, 10, 10, 10, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 2, 3, 3, 3, 3, 2, 9, 9, 9,
    0, 8, 1, 1, 9, 9, 9, 9, 0, 0, 2, 0, 11, 0, 10, 10, 0, 0, 0, 0, 3, 3, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 10, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 8, 1, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 0, 0, 0, 0, 8, 13, 13, 13, 13, 10, 10, 0, 0, 0, 3, 3, 3, 7,
    10, 10, 12, 9, 10, 10, 9, 9, 9, 2, 2, 2, 2, 2, 3, 2, 0, 0, 2, 10, 10, 10, 3, 3, 10, 7, 12, 12, 5, 9, 5, 5, 12, 0, 12, 3, 3, 12, 12, 12, 3, 12, 0, 3, 12, 4,
    12, 12, 12, 12, 3, 3, 3, 12, 12 };

int32_t tile_count_for_size[FOOTPRINT_SIZES] = { 0, 1, 4, 9, 16, 25, 36 };

ghost_tile_pair construction_ghost_tile_offsets[GHOST_FOOTPRINT_TILES] = {
    { 0, 0 }, { -30, 15 }, { 30, 15 }, { 0, 30 }, { -60, 30 }, { 60, 30 }, { -30, 45 }, { 30, 45 }, { 0, 60 }, { -90, 45 }, { 90, 45 }, { -60, 60 }, { 60, 60 },
    { -30, 75 }, { 30, 75 }, { 0, 90 }, { -120, 60 }, { 120, 60 }, { -90, 75 }, { 90, 75 }, { -60, 90 }, { 60, 90 }, { -30, 105 }, { 30, 105 }, { 0, 120 },
    { -150, 75 }, { 150, 75 }, { -120, 90 }, { 120, 90 }, { -90, 105 }, { 90, 105 }, { -60, 120 }, { 60, 120 }, { -30, 135 }, { 30, 135 }, { 0, 150 },
};

building_part_offset building_part_offsets[BUILDING_PART_OFFSETS] = {
    { 0, -2 },
    { 4, 2 },
    { 2, 4 },
    { -2, 0 },
    { -2, 0 },
    { 0, -2 },
    { 4, 2 },
    { 2, 4 },
    { 2, 4 },
    { -2, 0 },
    { 0, -2 },
    { 4, 2 },
    { 4, 2 },
    { 2, 4 },
    { -2, 0 },
    { 0, -2 },
};

int32_t venue_booth_tiles[GHOST_VENUE_VIEWS][GHOST_BOOTH_TILES] = {
    { 0, 1, 2, 3 },
    { 2, 0, 3, 1 },
    { 3, 2, 1, 0 },
    { 1, 3, 0, 2 },
};

int32_t ghost_bandstand_tiles[GHOST_VENUE_VIEWS][GHOST_BANDSTAND_TILES] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8 },
    { 6, 3, 0, 7, 4, 1, 8, 5, 2 },
    { 8, 7, 6, 5, 4, 3, 2, 1, 0 },
    { 2, 5, 8, 1, 4, 7, 0, 3, 6 },
};

int32_t venue_pavilion_tiles[GHOST_VENUE_VIEWS][GHOST_PAVILION_TILES] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
    { 12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3 },
    { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
    { 3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12 },
};

int32_t ghost_festival_tiles[GHOST_VENUE_VIEWS][GHOST_FESTIVAL_TILES] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
    { 20, 15, 10, 5, 0, 21, 16, 11, 6, 1, 22, 17, 12, 7, 2, 23, 18, 13, 8, 3, 24, 19, 14, 9, 4 },
    { 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
    { 4, 9, 14, 19, 24, 3, 8, 13, 18, 23, 2, 7, 12, 17, 22, 1, 6, 11, 16, 21, 0, 5, 10, 15, 20 },
};

int32_t overlay_fire_buildings[] = { BUILDING_FIREHOUSE, 0 };

int32_t overlay_damage_buildings[] = { BUILDING_ARCHITECT_POST, 0 };

int32_t overlay_crime_buildings[] = { BUILDING_POLICE_STATION, BUILDING_COURTHOUSE, BUILDING_FORT_CHARIOTEERS, BUILDING_FORT_ARCHERS, BUILDING_FORT_INFANTRY,
    BUILDING_FORT, 0 };

int32_t overlay_disease_buildings[] = { BUILDING_PHYSICIAN, 0 };

int32_t overlay_malaria_buildings[] = { BUILDING_APOTHECARY, 0 };

int32_t overlay_entertainment_buildings[] = { BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, BUILDING_BANDSTAND, BUILDING_CONSERVATORY, BUILDING_PAVILION,
    BUILDING_DANCE_SCHOOL, BUILDING_SENET_HOUSE, BUILDING_FESTIVAL_SQUARE, BUILDING_ZOO, 0 };

int32_t overlay_juggler_buildings[] = { BUILDING_BOOTH, BUILDING_JUGGLER_SCHOOL, 0 };

int32_t overlay_musician_buildings[] = { BUILDING_BANDSTAND, BUILDING_CONSERVATORY, 0 };

int32_t overlay_dancer_buildings[] = { BUILDING_PAVILION, BUILDING_DANCE_SCHOOL, 0 };

int32_t overlay_senet_buildings[] = { BUILDING_SENET_HOUSE, 0 };

int32_t overlay_zookeeper_buildings[] = { BUILDING_ZOO, 0 };

int32_t overlay_education_buildings[] = { BUILDING_SCRIBAL_SCHOOL, BUILDING_LIBRARY, 0 };

int32_t overlay_scribal_school_buildings[] = { BUILDING_SCRIBAL_SCHOOL, 0 };

int32_t overlay_library_buildings[] = { BUILDING_LIBRARY, 0 };

int32_t overlay_tax_income_buildings[] = { BUILDING_TAX_COLLECTOR, BUILDING_TAX_COLLECTOR_UP, BUILDING_VILLAGE_PALACE_UP, BUILDING_TOWN_PALACE_UP,
    BUILDING_CITY_PALACE, 0 };

int32_t overlay_bazaar_access_buildings[] = { BUILDING_BAZAAR, BUILDING_BAZAAR_UP, BUILDING_GRANARY, BUILDING_GRANARY_UP, BUILDING_STORAGE_YARD,
    BUILDING_STORAGE_YARD_UP, BUILDING_STORAGE_ROOM, 0 };

int32_t overlay_water_buildings[] = { BUILDING_WELL, BUILDING_WATER_SUPPLY, 0 };

int32_t overlay_dentist_buildings[] = { BUILDING_DENTIST, 0 };

int32_t overlay_apothecary_buildings[] = { BUILDING_APOTHECARY, 0 };

int32_t overlay_physician_buildings[] = { BUILDING_PHYSICIAN, 0 };

int32_t overlay_mortuary_buildings[] = { BUILDING_MORTUARY, 0 };

int32_t overlay_religion_buildings[] = { BUILDING_MENU_SHRINES, BUILDING_FESTIVAL_SQUARE, BUILDING_ORACLE, BUILDING_SHRINE_OSIRIS, BUILDING_SHRINE_RA,
    BUILDING_SHRINE_PTAH, BUILDING_SHRINE_SETH, BUILDING_SHRINE_BAST, BUILDING_TEMPLE_OSIRIS, BUILDING_TEMPLE_RA, BUILDING_TEMPLE_PTAH, BUILDING_TEMPLE_SETH,
    BUILDING_TEMPLE_BAST, BUILDING_TEMPLE_COMPLEX_OSIRIS, BUILDING_TEMPLE_COMPLEX_RA, BUILDING_TEMPLE_COMPLEX_PTAH, BUILDING_TEMPLE_COMPLEX_SETH,
    BUILDING_TEMPLE_COMPLEX_BAST, BUILDING_SPHINX, 0 };

int32_t overlay_city_defenses_buildings[] = { BUILDING_FORT_CHARIOTEERS, BUILDING_FORT_ARCHERS, BUILDING_FORT_INFANTRY, BUILDING_FORT_GROUND,
    BUILDING_POLICE_STATION, BUILDING_TRIUMPHAL_ARCH, BUILDING_FORT, BUILDING_MUD_GATEHOUSE, BUILDING_MUD_TOWER, BUILDING_BRICK_TOWER, BUILDING_TOWER,
    BUILDING_BRICK_WALL, BUILDING_WALL, BUILDING_BRICK_GATEHOUSE, BUILDING_GATEHOUSE, BUILDING_MUD_GATEHOUSE_UP, BUILDING_BRICK_GATEHOUSE_UP,
    BUILDING_GATEHOUSE_UP, BUILDING_MILITARY_ACADEMY, BUILDING_MILITARY_ACADEMY_2, BUILDING_MILITARY_ACADEMY_3, BUILDING_RECRUITER, BUILDING_TRANSPORT_WHARF,
    BUILDING_WARSHIP_WHARF, 0 };

int32_t overlay_magistrate_buildings[] = { BUILDING_COURTHOUSE, 0 };

int32_t *overlay_show_building_lists[OVERLAY_COUNT] = {
    [OVERLAY_RELIGION] = overlay_religion_buildings,
    [OVERLAY_FIRE] = overlay_fire_buildings,
    [OVERLAY_DAMAGE] = overlay_damage_buildings,
    [OVERLAY_CRIME] = overlay_crime_buildings,
    [OVERLAY_ENTERTAINMENT] = overlay_entertainment_buildings,
    [OVERLAY_JUGGLER] = overlay_juggler_buildings,
    [OVERLAY_MUSICIAN] = overlay_musician_buildings,
    [OVERLAY_DANCER] = overlay_dancer_buildings,
    [OVERLAY_SENET] = overlay_senet_buildings,
    [OVERLAY_ZOOKEEPER] = overlay_zookeeper_buildings,
    [OVERLAY_EDUCATION] = overlay_education_buildings,
    [OVERLAY_SCRIBAL_SCHOOL] = overlay_scribal_school_buildings,
    [OVERLAY_LIBRARY] = overlay_library_buildings,
    [OVERLAY_DENTIST] = overlay_dentist_buildings,
    [OVERLAY_PHYSICIAN] = overlay_physician_buildings,
    [OVERLAY_APOTHECARY] = overlay_apothecary_buildings,
    [OVERLAY_MORTUARY] = overlay_mortuary_buildings,
    [OVERLAY_TAX_INCOME] = overlay_tax_income_buildings,
    [OVERLAY_BAZAAR_ACCESS] = overlay_bazaar_access_buildings,
    [OVERLAY_DISEASE] = overlay_disease_buildings,
    [OVERLAY_WATER] = overlay_water_buildings,
    [OVERLAY_MALARIA] = overlay_malaria_buildings,
    [OVERLAY_CITY_DEFENSES] = overlay_city_defenses_buildings,
    [OVERLAY_MAGISTRATE] = overlay_magistrate_buildings,
};

int32_t overlay_tiles_size2[4] = { 0, 1, 2, 3 };

int32_t overlay_tiles_size3[9] = { 0, 1, 2, 1, 3, 2, 3, 3, 3 };

int32_t overlay_tiles_size4[16] = { 0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 3, 3, 3, 3, 3, 3 };

int32_t overlay_tiles_size5[25] = { 0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 1, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

int32_t overlay_tiles_size6[36] = { 0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 1, 3, 3, 3, 2, 1, 3, 3, 3, 3, 2, 1, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

ghost_tile_pair ghost_fort_ground_tiles[GHOST_FORT_VIEWS] = { { 2, 1 }, { 1, -3 }, { -2, -2 }, { -2, 1 } };

ghost_tile_pair ghost_fort_ground_half_tiles[GHOST_FORT_VIEWS] = { { 0, 0 }, { 1, 1 }, { 0, 0 }, { 1, 1 } };

int32_t ghost_fort_ground_pixels[GHOST_FORT_VIEWS] = { 4, 3, -4, -3 };

int32_t ghost_fort_ground_grid_offsets[GHOST_FORT_VIEWS] = { -225, -224, 4, 3 };

int32_t ghost_sphinx_part_grid_offsets[GHOST_SPHINX_PART_SLOTS] = { 6, 6, 6, 6, 12, 12, 12, 12 };

ghost_tile_pair ghost_sphinx_part_tiles[GHOST_SPHINX_PART_SLOTS] = { { 3, 3 }, { 3, -3 }, { -3, -3 }, { -3, 3 }, { 6, 6 }, { 6, -6 }, { -6, -6 }, { -6, 6 } };

int32_t ghost_sphinx_part_pixels[GHOST_SPHINX_PART_SLOTS] = { 6, 6, -6, -5, 12, 12, -12, -10 };

overlay_tooltip_rule overlay_tooltip_rules[OVERLAY_COUNT] = {
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 0, 0, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 0, 1, 0 },
    { 0, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 0, 0, 0 },
    { 0, 0, 1 },
    { 2, 1, 0 },
    { 1, 1, 0 },
    { 0, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 1, 1, 0 },
    { 0, 0, 0 },
    { 1, 1, 0 },
    { 0, 0, 0 },
    { 1, 1, 0 },
    { 0, 1, 0 },
    { 1, 1, 0 },
    { 0, 0, 0 },
};

char format_numbered[8] = "%d. ";

char text_line_break[4] = "@L";

uint8_t str_comma[8] = { 44 };

int32_t sidebar_slide_offsets[SIDEBAR_SLIDE_OFFSETS] = { 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 21, 24, 27, 30, 33, 37, 41, 45, 49, 54, 59, 64, 70,
    76, 83, 91, 99, 106, 113, 119, 125, 130, 135, 139, 143, 146, 149, 152, 154, 156, 158, 160, 162, 165, 0 };

const char hud_level_label[8] = "Level";

uint8_t str_event[8] = { 69, 118, 101, 110, 116 };

uint8_t str_minor_bad[12] = { 77, 105, 110, 111, 114, 32, 66, 97, 100 };

uint8_t str_major_bad[12] = { 77, 97, 106, 111, 114, 32, 66, 97, 100 };

uint8_t str_minor_good[12] = { 77, 105, 110, 111, 114, 32, 71, 111, 111, 100 };

uint8_t str_major_good[12] = { 77, 97, 106, 111, 114, 32, 71, 111, 111, 100 };

const char hud_max_prosperity_label[16] = " Max prosperity";

const char hud_happiness_label[12] = " Happiness";

uint8_t str_allow_all[12] = { 65, 108, 108, 111, 119, 32, 65, 108, 108, 32 };

float debug_key_value = 60.0;

int32_t debug_key_toggle = 1;

int32_t debug_keys_enabled = 1;

image_button advisor_bar_buttons[ADVISOR_BAR_BUTTONS] = {
    { 12, 9, 33, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 0, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_LABOR, 0 },
    { 52, 9, 39, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 4, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_MILITARY, 0 },
    { 96, 9, 34, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 8, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_KINGDOM, 0 },
    { 135, 9, 38, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 12, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_RATINGS, 0 },
    { 178, 9, 46, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 16, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_TRADE, 0 },
    { 229, 9, 48, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 20, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_POPULATION, 0 },
    { 282, 9, 35, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 24, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_HEALTH, 0 },
    { 321, 9, 38, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 28, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_EDUCATION, 0 },
    { 363, 9, 39, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 32, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_ENTERTAINMENT, 0 },
    { 406, 9, 35, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 36, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_RELIGION, 0 },
    { 445, 9, 40, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 40, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_FINANCIAL, 0 },
    { 490, 9, 46, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 44, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_CHIEF, 0 },
    { 542, 9, 40, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 48, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_MONUMENTS, 0 },
    { 588, 9, 42, 32, 4, IMAGE_GROUP_ADVISOR_BAR, 52, 0, button_advisor_bar, button_none, 1, 0, 0, 0,
        ADVISOR_NONE, 0 },
};

uint8_t military_empire_button_shown = 1;


uint8_t construction_placing_royal_tomb;

int32_t editor_wide_bounds;

int32_t rotate_button_hovered_part;

int32_t top_menu_focus_info_id;

int32_t minimap_hovered;

int32_t top_menu_focus_menu_id;

int32_t menu_bar_focus_item;

city_view_state city_view;

uint8_t g_fog_disabled;

int32_t scenario_max_year;

int32_t debug_figure_info_enabled;

int32_t debug_building_info_enabled;

int32_t debug_evolve_info_enabled;

int32_t debug_evolve_error;

int32_t undo_ready;

int32_t undo_available;

int32_t bridge_direction;

char tooltip_composed[500];

int32_t tooltip_type;

int32_t tooltip_last_time;

int32_t tooltip_box_height;

int32_t tooltip_box_width;

int32_t tooltip_box_y;

int32_t tooltip_box_x;

int32_t tooltip_text_id;

int32_t g_last_image_id;

int32_t scroll_in_progress;

int32_t time_current_millis;

int32_t g_map_visible_y1;

int32_t g_map_visible_y0;

int32_t g_map_visible_x1;

int32_t g_map_visible_x0;

int32_t city_view_orientation;

int32_t city_overlay;

int32_t city_overlay_previous;

void *debug_sprite_a;

uint8_t *debug_sprite_b;

int32_t debug_city_info_enabled;

int32_t city_view_width_collapsed;

int32_t city_view_width_expanded;

int32_t debug_show_happiness;

int32_t g_top_menu_force;

int32_t g_sidebar_collapsed;

int32_t g_sidebar_slide_step;

int32_t ambient_scores[12];

