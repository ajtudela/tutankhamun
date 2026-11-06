// The overlays' column heights (0..10) of a building or tile, used by the overlay footprints, the minimap and
// the tooltips: services, risks, farms, goods levels and the problems overlay, which also marks the building.
//
// The physician height skips the dentist building instead of the physician's own.

#include "widget/city_overlay_heights.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_services.h"
#include "city/culture.h"
#include "city/gods.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "game/game_internal.h"
#include "map/terrain.h"
#include "widget/city_building_tiles.h"
#include "widget/city_view_rotate.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum farm_top_edge {
    FARM_TOP_EDGE_NORTH = 0x50,
    FARM_TOP_EDGE_EAST = 0x40,
    FARM_TOP_EDGE_SOUTH = 0x42,
    FARM_TOP_EDGE_WEST = 0x52,
};

enum overlay_height {
    OVERLAY_HEIGHT_MAX = 10,
    OVERLAY_LEVEL_MAX = 9,
    TEMPLE_KINDS_PER_GOD = 3,
};

int32_t city_overlay_farm_hidden(int32_t building_id)
{
    if (!building_is_farm(building_id)) {
        return 0;
    }
    uint8_t edge = map_grids.edge[city_view.grid_offset];
    switch (map_info.orientation) {
        case VIEW_ORIENTATION_NORTH:
            return edge != FARM_TOP_EDGE_NORTH;
        case VIEW_ORIENTATION_EAST:
            return edge != FARM_TOP_EDGE_EAST;
        case VIEW_ORIENTATION_SOUTH:
            return edge != FARM_TOP_EDGE_SOUTH;
        case VIEW_ORIENTATION_WEST:
            return edge != FARM_TOP_EDGE_WEST;
        default:
            return 0;
    }
}

int32_t city_overlay_height_fertility(int32_t grid_offset)
{
    uint8_t v = map_grids.soil_fertility[grid_offset];
    if (v == 0) {
        return 0;
    }
    if (v < 10) {
        return 1;
    }
    if (v < 20) {
        return 2;
    }
    if (v < 30) {
        return 3;
    }
    if (v < 40) {
        return 4;
    }
    if (v < 50) {
        return 5;
    }
    if (v < 60) {
        return 6;
    }
    if (v < 70) {
        return 7;
    }
    if (v < 80) {
        return 8;
    }
    return 9;
}

int32_t city_overlay_height_desirability(int32_t grid_offset)
{
    int8_t v = map_grids.desirability[grid_offset];
    if ((map_grids.terrain[grid_offset] & TERRAIN_BUILDING) == 0 && v == 0) {
        return 0;
    }
    if (v < -10) {
        return 0;
    }
    if (v < -5) {
        return 1;
    }
    if (v < 0) {
        return 2;
    }
    if (v == 0) {
        return 3;
    }
    if (v < 5) {
        return 4;
    }
    if (v < 10) {
        return 5;
    }
    if (v < 15) {
        return 6;
    }
    if (v < 20) {
        return 7;
    }
    return (v > 24) ? 9 : 8;
}

int32_t city_overlay_resource_level(int32_t building_id, int32_t resource, int32_t slot)
{
    const building *b = building_get(building_id);
    double value;
    switch (b->type) {
        case BUILDING_BAZAAR:
        case BUILDING_BAZAAR_UP:
            value = (double) b->data.market.inventory[slot];
            break;
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            value = (double) b->data.granary.resource_stored[resource];
            break;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            value = (double) building_storageyard_get_amount(building_id, resource);
            break;
        case BUILDING_STORAGE_ROOM:
            value = (double) building_storageyard_get_amount(building_main(building_id), resource);
            break;
        default:
            if (b->house_size == 0) {
                return 0;
            }
            value = (double) b->data.house.inventory[slot] * (double) 4.0f;
            break;
    }
    value = value * (double) 0.0003125f;
    value = value * (double) -9.0f;
    int32_t level = 4 - (int32_t) value;
    return (level > OVERLAY_LEVEL_MAX) ? OVERLAY_LEVEL_MAX : level;
}

int32_t city_overlay_height_native(int32_t building_id)
{
    return 1;
}

int32_t city_overlay_height_fire(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_FIREHOUSE || b->type == BUILDING_BURNING_RUIN) {
        return 0;
    }
    if (city_overlay_farm_hidden(building_id)) {
        return 0;
    }
    return b->fire_risk / 100;
}

int32_t city_overlay_height_damage_terrain(int32_t grid_offset)
{
    return ((int16_t) map_grids.building_damage[grid_offset]) / 100;
}

int32_t city_overlay_height_damage(int32_t building_id)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->type == BUILDING_ARCHITECT_POST) {
        return 0;
    }
    if (city_overlay_farm_hidden(building_id)) {
        return 0;
    }
    return b->damage_risk / 100;
}

int32_t city_overlay_height_disease(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return 0;
    }
    return b->house_plague_risk / 100;
}

int32_t city_overlay_height_malaria(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return 0;
    }
    return b->house_malaria_risk / 100;
}

int32_t city_overlay_height_crime(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_POLICE_STATION || b->type == BUILDING_COURTHOUSE || b->type == BUILDING_BURNING_RUIN || b->house_size == 0) {
        return 0;
    }
    return b->crime_risk / 100;
}

static int32_t service_height(uint8_t access)
{
    return OVERLAY_HEIGHT_MAX - (access / 10);
}

static int32_t entertainment_venues(void)
{
    return building_count_active[BUILDING_JUGGLER_SCHOOL] + building_count_active[BUILDING_BULLFIGHT_SCHOOL] + building_count_active[BUILDING_DANCE_SCHOOL] +
        building_count_active[BUILDING_CONSERVATORY] + building_count_active[BUILDING_PAVILION] + building_count_active[BUILDING_ZOO] +
        building_count_active[BUILDING_SENET_HOUSE] + building_count_active[BUILDING_BOOTH] + building_count_active[BUILDING_BANDSTAND];
}

int32_t city_overlay_height_entertainment(int32_t building_id)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    switch (b->type) {
        case BUILDING_BANDSTAND:
        case BUILDING_BOOTH:
        case BUILDING_SENET_HOUSE:
        case BUILDING_PAVILION:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_BULLFIGHT_SCHOOL:
        case BUILDING_ZOO:
            return 0;
        default:
            break;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if (entertainment_venues() == 0) {
        return 0;
    }
    return OVERLAY_HEIGHT_MAX - (b->data.house.entertainment / 10);
}

int32_t city_overlay_height_education(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type != BUILDING_SCRIBAL_SCHOOL && b->type != BUILDING_LIBRARY && b->house_size != 0 &&
        (building_count_active[BUILDING_LIBRARY] + building_count_active[BUILDING_SCRIBAL_SCHOOL]) != 0) {
        switch (b->data.house.education) {
            case HOUSE_EDUCATION_SCHOOL_OR_LIBRARY:
                return 8;
            case HOUSE_EDUCATION_SCHOOL_AND_LIBRARY:
                return 5;
            case HOUSE_EDUCATION_HIGHER:
                return 2;
            default:
                break;
        }
    }
    return 0;
}

int32_t city_overlay_height_booth(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_BOOTH || b->type == BUILDING_JUGGLER_SCHOOL) {
        return 0;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if ((building_count_active[BUILDING_JUGGLER_SCHOOL] + building_count_active[BUILDING_BOOTH]) == 0) {
        return 0;
    }
    return service_height(b->data.house.juggler);
}
int32_t city_overlay_height_bandstand(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_BANDSTAND || b->type == BUILDING_CONSERVATORY || b->type == BUILDING_JUGGLER_SCHOOL) {
        return 0;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if ((building_count_active[BUILDING_JUGGLER_SCHOOL] + building_count_active[BUILDING_CONSERVATORY] + building_count_active[BUILDING_BANDSTAND]) == 0) {
        return 0;
    }
    return service_height(b->data.house.musician);
}
int32_t city_overlay_height_pavilion(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_PAVILION || b->type == BUILDING_CONSERVATORY || b->type == BUILDING_DANCE_SCHOOL) {
        return 0;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if ((building_count_active[BUILDING_DANCE_SCHOOL] + building_count_active[BUILDING_CONSERVATORY] + building_count_active[BUILDING_PAVILION]) == 0) {
        return 0;
    }
    return service_height(b->data.house.dancer);
}
int32_t city_overlay_height_senet(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_SENET_HOUSE || b->type == BUILDING_BULLFIGHT_SCHOOL) {
        return 0;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if ((building_count_active[BUILDING_BULLFIGHT_SCHOOL] + building_count_active[BUILDING_SENET_HOUSE]) == 0) {
        return 0;
    }
    return service_height(b->data.house.senet);
}

int32_t city_overlay_height_bazaar_access(int32_t building_id)
{
    const building *b = building_get(building_id);
    int16_t type = b->type;
    if (type == BUILDING_BAZAAR || type == BUILDING_BAZAAR_UP || type == BUILDING_FISHING_WHARF || type == BUILDING_GRANARY || type == BUILDING_GRANARY_UP ||
        b->house_size == 0 || building_count_active[BUILDING_BAZAAR] == 0) {
        return 0;
    }
    int32_t needed = 0;
    if (house_models[b->subtype].food_types != 0) {
        needed = b->house_population;
    }
    int32_t sum = 0;
    for (int32_t i = 0; i < (RESOURCE_MAX_FOOD - RESOURCE_MIN_FOOD); i++) {
        sum += b->data.house.inventory[i];
    }
    if (needed == 0) {
        return 0;
    }
    int32_t percent = calc_percentage(sum, needed);
    if (percent > 200) {
        return 0;
    }
    if (percent >= 100) {
        return 1;
    }
    return (percent <= 0) ? OVERLAY_HEIGHT_MAX : 5;
}

int32_t city_overlay_height_water(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_WATER_SUPPLY) {
        return 0;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if (building_count_active[BUILDING_WATER_SUPPLY] == 0) {
        return 0;
    }
    return service_height(b->data.house.water_supply);
}

int32_t city_overlay_height_religion(int32_t building_id)
{
    const building *b = building_get(building_id);
    switch (b->type) {
        case BUILDING_ORACLE:
        case BUILDING_MENU_SHRINES:
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_SPHINX:
            return 0;
        default:
            break;
    }
    if (b->house_size == 0) {
        return 0;
    }
    int32_t count = 0;
    for (int32_t god = 0; god < GOD_COUNT; god++) {
        for (int32_t kind = 0; kind < TEMPLE_KINDS_PER_GOD; kind++) {
            count += building_count_active[building_type_for_god(god, kind)];
        }
    }
    if ((building_count_total[BUILDING_ORACLE] + count) == 0) {
        return 0;
    }
    return (b->data.house.num_gods * 17) / 10;
}

static int32_t height_service(int32_t building_id, int32_t type, int32_t count, uint8_t access)
{
    const building *b = building_get(building_id);
    if (b->type == type) {
        return 0;
    }
    if (b->house_size == 0) {
        return 0;
    }
    if (count == 0) {
        return 0;
    }
    return service_height(access);
}
int32_t city_overlay_height_school(int32_t building_id)
{
    return height_service(building_id, BUILDING_SCRIBAL_SCHOOL, building_count_active[BUILDING_SCRIBAL_SCHOOL], building_get(building_id)->data.house.school);
}
int32_t city_overlay_height_library(int32_t building_id)
{
    return height_service(building_id, BUILDING_LIBRARY, building_count_active[BUILDING_LIBRARY], building_get(building_id)->data.house.library);
}
int32_t city_overlay_height_dentist(int32_t building_id)
{
    return height_service(building_id, BUILDING_DENTIST, building_count_active[BUILDING_DENTIST], building_get(building_id)->data.house.dentist);
}
int32_t city_overlay_height_physician(int32_t building_id)
{
    return height_service(building_id, BUILDING_DENTIST, building_count_active[BUILDING_PHYSICIAN], building_get(building_id)->data.house.physician);
}
int32_t city_overlay_height_apothecary(int32_t building_id)
{
    return height_service(building_id, BUILDING_APOTHECARY, building_count_active[BUILDING_APOTHECARY], building_get(building_id)->data.house.apothecary);
}
int32_t city_overlay_height_mortuary(int32_t building_id)
{
    return height_service(building_id, BUILDING_MORTUARY, building_count_active[BUILDING_MORTUARY], building_get(building_id)->data.house.mortuary);
}
int32_t city_overlay_height_courthouse(int32_t building_id)
{
    return height_service(building_id, BUILDING_COURTHOUSE, building_count_active[BUILDING_COURTHOUSE], building_get(building_id)->data.house.courthouse);
}

int32_t city_overlay_height_tax_income(int32_t building_id)
{
    if (building_get(building_id)->house_size == 0) {
        return 0;
    }
    if ((building_count_active[BUILDING_TOWN_PALACE_UP] + building_count_active[BUILDING_CITY_PALACE] + building_count_active[BUILDING_VILLAGE_PALACE_UP] +
            building_count_active[BUILDING_TAX_COLLECTOR_UP] + building_count_active[BUILDING_TAX_COLLECTOR]) == 0) {
        return 0;
    }
    return building_house_tax_income(building_id) / 25;
}

static int32_t needs_problem_mark(int32_t building_id, const building *b)
{
    int16_t type = b->type;
    if (type == BUILDING_WELL || type == BUILDING_WATER_SUPPLY) {
        return b->has_water_access == 0;
    }
    int32_t figure_waiting = city_building_cart_pusher_waiting(b);
    if (building_is_farm(building_id) || (type >= BUILDING_STONE_QUARRY && type <= BUILDING_CLAY_PIT)) {
        return figure_waiting;
    }
    if (city_building_needs_stock(type)) {
        return figure_waiting || b->loads_stored <= 0;
    }
    return 0;
}

int32_t city_overlay_height_problems(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->house_size != 0) {
        return 0;
    }
    if (needs_problem_mark(building_id, b)) {
        b->show_on_problem_overlay = 1;
    }
    return b->show_on_problem_overlay != 0;
}

int32_t city_overlay_height_labor(int32_t building_id)
{
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        return 0;
    }
    return calc_percentage(b->houses_covered, 300) / 10;
}

int32_t city_overlay_height_city_defenses(int32_t building_id)
{
    for (const int32_t *t = overlay_city_defenses_buildings; *t != 0; t++) {
        if (building_get(building_id)->type == *t) {
            return 1;
        }
    }
    return 0;
}
