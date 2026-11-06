// Undo of the last construction: whether the undo is ready and lists a building, the per-tick countdown that disables
// it (time out, an earthquake, an undone building that changed), and the map restore after loading a game, which
// also cuts cycles out of the figure lists per tile.

#include "game/undo.h"

#include "building/building.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "widget/widget_data.h"
#include "window/window_data.h"

#include <stdint.h>
#include <string.h>

static void undo_clear_buildings(void)
{
    memset(undo_buildings, 0, UNDO_MAX_BUILDINGS * sizeof(building));
    memset(undo_building_ids, 0, UNDO_MAX_BUILDINGS * sizeof(undo_building_ids[0]));
}

static int32_t undo_mode_keeps_buildings(void)
{
    switch (undo_mode) {
        case BUILDING_ROAD:
        case BUILDING_MUD_WALL:
        case BUILDING_IRRIGATION_DITCH:
        case BUILDING_CLEAR_LAND:
        case BUILDING_PLAZA:
        case BUILDING_GARDENS:
        case BUILDING_LOW_BRIDGE:
        case BUILDING_SHIP_BRIDGE:
        case BUILDING_BRICK_WALL:
        case BUILDING_WALL:
            return 1;
    }
    return 0;
}

static int32_t undo_houses_moved_in(int32_t count)
{
    for (int32_t i = 0; i < count; i++) {
        int32_t id = undo_building_ids[i];
        if (id && building_get(id)->house_population) {
            return 1;
        }
    }
    return 0;
}

static int32_t undo_building_changed(int32_t index)
{
    const building *b = building_get(undo_building_ids[index]);
    const building *saved = &undo_buildings[index];
    return b->state == BUILDING_STATE_UNDO || b->state == BUILDING_STATE_RUBBLE || b->state == BUILDING_STATE_DELETED_BY_GAME ||
        b->grid_offset != saved->grid_offset || b->type != saved->type;
}

int32_t game_undo_contains_building(int32_t building_id)
{
    if (building_id <= 0 || undo_ready != 1 || undo_available != 1 || undo_num_buildings <= 0) {
        return 0;
    }
    for (int32_t i = 0; i < UNDO_MAX_BUILDINGS; i++) {
        if (undo_building_ids[i] == building_id) {
            return 1;
        }
    }
    return 0;
}

static void disable_undo(void)
{
    undo_available = 0;
    window_redraw_request = 1;
}

void game_undo_reduce_time_available(void)
{
    if (undo_available != 1 || undo_ready != 1) {
        return;
    }
    if (undo_timeout_ticks <= 0 || scenario_events.earthquake_active) {
        disable_undo();
        undo_clear_buildings();
        return;
    }
    undo_timeout_ticks--;
    if (undo_mode_keeps_buildings()) {
        return;
    }
    int32_t count = undo_num_buildings;
    if (count <= 0) {
        disable_undo();
        return;
    }
    if (undo_mode == BUILDING_HOUSE_CRUDE_HUT && undo_houses_moved_in(count)) {
        disable_undo();
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        if (undo_building_ids[i] && undo_building_changed(i)) {
            disable_undo();
            return;
        }
    }
}

void game_load_restore_buildings_and_figure_lists(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (!b->state) {
            continue;
        }
        int32_t size = building_property_get(b->type)->size;
        construction_size = size;
        building_construction_add_to_map(city_current_player, b->type, id, b->x, b->y, size, 0, 0);
    }
    for (int32_t id = 1; id < MAX_FIGURES; id++) {
        figure *f = figure_get(id);
        if (!f->state) {
            continue;
        }
        int32_t prev = id;
        int32_t next = f->next_figure_id_on_same_tile;
        while (next) {
            if (next == id) {
                figure_get(prev)->next_figure_id_on_same_tile = 0;
                break;
            }
            prev = next;
            next = figure_get(next)->next_figure_id_on_same_tile;
        }
    }
}
