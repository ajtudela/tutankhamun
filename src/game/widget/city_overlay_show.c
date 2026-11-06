// Which buildings and figures an overlay draws fully: a building by the overlay's type list (or while it
// burns under the fire overlay), a figure by the overlay's service or risk, plus the special cases of the crime,
// defenses, bazaar access and problems overlays.
//
// The education overlay's figures also show under the water crossings overlay, and the apothecary's under the malaria
// overlay.

#include "widget/city_overlay_show.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "widget/city_building_tiles.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include <stdint.h>

bool city_with_overlay_show_building(int32_t building_id)
{
    int32_t overlay = city_overlay;
    if (overlay == OVERLAY_HIDE_CLIFFS) {
        return 1;
    }
    if (overlay == OVERLAY_FIRE && city_buildings.records[building_id].fire_duration != 0) {
        return 1;
    }
    int32_t *list = overlay_show_building_lists[overlay];
    if (!list) {
        return 0;
    }
    int16_t type = building_get(building_id)->type;
    for (; *list != 0; list++) {
        if (*list == type) {
            return 1;
        }
    }
    return 0;
}

static int32_t is_law_figure(uint8_t type)
{
    return type == FIGURE_CONSTABLE || type == FIGURE_MAGISTRATE || type == FIGURE_PROTESTER || type == FIGURE_CRIMINAL || type == FIGURE_TOMB_ROBBER ||
        type == FIGURE_ARCHER || type == FIGURE_INFANTRY || type == FIGURE_CHARIOTEER || type == FIGURE_STANDARD_BEARER;
}

static int32_t crime_shows_priest(const figure *f)
{
    const city_player *player = city_player_current();
    if (player->has_temple_complex == 0) {
        return 0;
    }
    int32_t complex_god = building_temple_god(player->temple_complex_building_id);
    if (building_temple_god(f->building_id) != complex_god) {
        return 0;
    }
    if (complex_god == GOD_RA) {
        return building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ALTAR) != 0;
    }
    if (complex_god != GOD_SETH) {
        return 0;
    }
    return building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ORACLE) != 0;
}

static int32_t is_military_figure(uint8_t type)
{
    switch (type) {
        case FIGURE_ARCHER:
        case FIGURE_INFANTRY:
        case FIGURE_CHARIOTEER:
        case FIGURE_STANDARD_BEARER:
        case FIGURE_NATIVE:
        case FIGURE_TOWER_SENTRY:
        case FIGURE_ENEMY_ARCHER:
        case FIGURE_ENEMY_INFANTRY:
        case FIGURE_ENEMY_CHARIOT:
        case FIGURE_ENEMY_CAMEL:
        case FIGURE_ENEMY_ELEPHANT:
        case FIGURE_ENEMY_BARBARIAN_CHARIOT:
        case FIGURE_ENEMY_BARBARIAN_LIGHT:
        case FIGURE_ENEMY_BARBARIAN_HEAVY:
        case FIGURE_ENEMY_BARBARIAN_ARCHER:
        case FIGURE_ENEMY_BARBARIAN_MOUNTED:
        case FIGURE_ENEMY_BARBARIAN_HEAVY_2:
        case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
        case FIGURE_ENEMY_EGYPTIAN_ARCHER:
        case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
        case FIGURE_KINGDOM_SOLDIER:
        case FIGURE_NATIVE_TRADER:
        case FIGURE_ARROW:
        case FIGURE_JAVELIN:
        case FIGURE_BOLT:
        case FIGURE_BALLISTA:
        case FIGURE_TRANSPORT_SHIP:
        case FIGURE_WARSHIP:
        case FIGURE_CONSTABLE:
        case FIGURE_ENEMY_TRANSPORT_SHIP:
        case FIGURE_ENEMY_WARSHIP:
        case FIGURE_BEDOUIN:
        case FIGURE_ENEMY_EGYPTIAN_WARSHIP:
        case FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP:
            return 1;
        default:
            return 0;
    }
}

static int32_t defenses_show_priest(const figure *f)
{
    int16_t temple_type = building_get(f->building_id)->type;
    if (temple_type != BUILDING_TEMPLE_SETH && temple_type != BUILDING_TEMPLE_COMPLEX_SETH) {
        return 0;
    }
    const city_player *player = city_player_current();
    if (player->has_temple_complex == 0) {
        return 0;
    }
    return building_temple_complex_has_upgrade(player->temple_complex_building_id, TEMPLE_COMPLEX_ORACLE) != 0;
}

static int32_t problems_show_figure(const figure *f)
{
    if (f->type == FIGURE_CART_PUSHER) {
        return f->action_state == FIGURE_ACTION_CART_INITIAL && f->min_max_seen != 0;
    }
    if (f->type == FIGURE_LABOR_SEEKER) {
        return (building_get(f->building_id)->problem_flags & BUILDING_PROBLEM_NO_LABOR_ACCESS) != 0;
    }
    return f->type == FIGURE_FROG || f->type == FIGURE_MUMMY || f->type == FIGURE_LOCUST;
}

static int32_t bazaar_access_shows_figure(const figure *f)
{
    uint8_t type = f->type;
    if (type == FIGURE_MARKET_TRADER || type == FIGURE_MARKET_BUYER || type == FIGURE_DELIVERY_BOY || type == FIGURE_FISHING_BOAT) {
        return 1;
    }
    if (type != FIGURE_CART_PUSHER) {
        return 0;
    }
    return f->resource_id >= RESOURCE_MIN_FOOD && f->resource_id < RESOURCE_MAX_FOOD;
}

int32_t city_with_overlay_show_figure(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    uint8_t type = f->type;
    switch (city_overlay) {
        case OVERLAY_RELIGION:
            return type == FIGURE_PRIEST;
        case OVERLAY_FIRE:
            return type == FIGURE_FIREMAN;
        case OVERLAY_DAMAGE:
            return type == FIGURE_ARCHITECT;
        case OVERLAY_CRIME:
            if (is_law_figure(type)) {
                return 1;
            }
            return type == FIGURE_PRIEST && crime_shows_priest(f);
        case OVERLAY_ENTERTAINMENT:
            return type == FIGURE_JUGGLER || type == FIGURE_MUSICIAN || type == FIGURE_DANCER || type == FIGURE_SENET_PLAYER || type == FIGURE_ZOOKEEPER;
        case OVERLAY_JUGGLER:
            return type == FIGURE_JUGGLER;
        case OVERLAY_MUSICIAN:
            return type == FIGURE_MUSICIAN;
        case OVERLAY_DANCER:
            return type == FIGURE_DANCER;
        case OVERLAY_SENET:
            return type == FIGURE_SENET_PLAYER;
        case OVERLAY_ZOOKEEPER:
            return type == FIGURE_ZOOKEEPER;
        case OVERLAY_EDUCATION:
            return type == FIGURE_LIBRARIAN || type == FIGURE_TEACHER;
        case OVERLAY_SCRIBAL_SCHOOL:
        case OVERLAY_WATER_CROSSINGS:
            return type == FIGURE_TEACHER;
        case OVERLAY_LIBRARY:
            return type == FIGURE_LIBRARIAN;
        case OVERLAY_DENTIST:
            return type == FIGURE_DENTIST;
        case OVERLAY_PHYSICIAN:
            return type == FIGURE_PHYSICIAN;
        case OVERLAY_APOTHECARY:
        case OVERLAY_MALARIA:
            return type == FIGURE_HERBALIST;
        case OVERLAY_MORTUARY:
            return type == FIGURE_EMBALMER;
        case OVERLAY_TAX_INCOME:
            return type == FIGURE_TAX_COLLECTOR;
        case OVERLAY_BAZAAR_ACCESS:
            return bazaar_access_shows_figure(f);
        case OVERLAY_LABOR:
            return type == FIGURE_LABOR_SEEKER;
        case OVERLAY_NATIVE:
            return type == FIGURE_NATIVE || type == FIGURE_MISSIONARY;
        case OVERLAY_PROBLEMS:
            return problems_show_figure(f);
        case OVERLAY_DISEASE:
            return type == FIGURE_PHYSICIAN || type == FIGURE_PLAGUED_CITIZEN;
        case OVERLAY_WATER:
            return type == FIGURE_WATER_CARRIER;
        case OVERLAY_EMPTY_HOUSING:
            return type == FIGURE_IMMIGRANT || type == FIGURE_EMIGRANT;
        case OVERLAY_CITY_DEFENSES:
            if (is_military_figure(type)) {
                return 1;
            }
            return type == FIGURE_PRIEST && defenses_show_priest(f);
        case OVERLAY_MAGISTRATE:
            return type == FIGURE_MAGISTRATE;
        case OVERLAY_HIDE_CLIFFS:
            return 1;
        default:
            return 0;
    }
}
