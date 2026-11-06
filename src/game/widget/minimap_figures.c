// Which figures the minimap marks and in which colour (soldiers, enemies, predators, ships and the figures the
// overlay shows), and the overlay tile of a building type for the footprint overlay.
//
// The grand royal tomb is one past the overlay tile table and takes the word after it (tile 11).

#include "widget/minimap_figures.h"
#include "building/building_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include "window/window_data.h"
#include <stdint.h>

static int32_t overlay_tile_entry(int32_t type)
{
    if (type < BUILDING_OVERLAY_TILE_TYPES) {
        return building_type_overlay_tile[type];
    }
    return build_menu_types[0];
}

int32_t building_type_overlay_tile_index(int32_t type)
{
    if (type < BUILDING_NONE || type >= BUILDING_TYPE_COUNT) {
        return -1;
    }
    int32_t entry = overlay_tile_entry(type);
    if (entry > 0) {
        return entry - 1;
    }
    return -1;
}

static int32_t is_alert_figure(uint8_t type)
{
    return type == FIGURE_PROTESTER || type == FIGURE_CRIMINAL || type == FIGURE_TOMB_ROBBER || type == FIGURE_LOCUST || type == FIGURE_FROG ||
        type == FIGURE_MUMMY;
}

static int32_t is_enemy_figure(const figure *f)
{
    uint8_t type = f->type;
    if (type >= FIGURE_ENEMY_ARCHER && type <= FIGURE_KINGDOM_SOLDIER) {
        return 1;
    }
    if (type == FIGURE_ENEMY_EGYPTIAN_ARCHER || type == FIGURE_ENEMY_EGYPTIAN_INFANTRY || type == FIGURE_ENEMY_EGYPTIAN_CHARIOT || type == FIGURE_BEDOUIN) {
        return 1;
    }
    return type == FIGURE_NATIVE && f->action_state == FIGURE_ACTION_NATIVE_ATTACKING;
}

static int32_t is_own_ship(uint8_t type)
{
    return type == FIGURE_WARSHIP || type == FIGURE_TRANSPORT_SHIP || type == FIGURE_FISHING_BOAT || type == FIGURE_FERRY_BOAT || type == FIGURE_TRADE_SHIP;
}

static int32_t is_enemy_ship(uint8_t type)
{
    return type == FIGURE_ENEMY_WARSHIP || type == FIGURE_ENEMY_TRANSPORT_SHIP || type == FIGURE_ENEMY_EGYPTIAN_WARSHIP ||
        type == FIGURE_ENEMY_EGYPTIAN_TRANSPORT_SHIP;
}

int32_t minimap_figure_show(int32_t figure_id)
{
    while (figure_id != 0 && city_overlay != OVERLAY_HIDE_CLIFFS) {
        if (city_with_overlay_show_figure(figure_id) != 0) {
            return 1;
        }
        if (city_overlay == OVERLAY_NONE && is_alert_figure(city_figures.records[figure_id].type)) {
            return 1;
        }
        figure_id = city_figures.records[figure_id].next_figure_id_on_same_tile;
    }
    return 0;
}

int32_t minimap_figure_military_kind(int32_t figure_id)
{
    while (figure_id != 0) {
        const figure *f = &city_figures.records[figure_id];
        uint8_t type = f->type;
        if (type >= FIGURE_ARCHER && type <= FIGURE_CHARIOTEER) {
            return (formation_get(f->formation_id)->request_event_id < 0) ? MINIMAP_MILITARY_OWN_SOLDIER : MINIMAP_MILITARY_NONE;
        }
        if (is_enemy_figure(f)) {
            return MINIMAP_MILITARY_ENEMY;
        }
        if (figure_is_predator_animal(type) != 0) {
            return MINIMAP_MILITARY_PREDATOR;
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
    return MINIMAP_MILITARY_NONE;
}

int32_t minimap_figure_ship_kind(int32_t figure_id)
{
    while (figure_id != 0) {
        const figure *f = &city_figures.records[figure_id];
        if (is_own_ship(f->type)) {
            return MINIMAP_SHIP_OWN;
        }
        if (is_enemy_ship(f->type)) {
            return MINIMAP_SHIP_ENEMY;
        }
        figure_id = f->next_figure_id_on_same_tile;
    }
    return MINIMAP_SHIP_NONE;
}
