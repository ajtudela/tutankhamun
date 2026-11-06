// The city window's click on military units: the legion of the first soldier on the tile (opens the legion
// window), a transport ship on the tile or next to it (the transport window), or a warship (the warship window).
//
// The warship on the tile is taken only with a request event, otherwise the first neighbour without one.

#include "widget/city_selection.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <stdint.h>

enum city_selection {
    NO_REQUEST_EVENT_BELOW = 0,
};

int32_t formation_legion_at_grid_offset(int32_t grid_offset)
{
    for (int32_t id = map_grids.figure_ids[grid_offset]; id != 0;) {
        const figure *f = figure_get(id);
        if (f->type >= FIGURE_ARCHER && f->type <= FIGURE_STANDARD_BEARER) {
            return f->formation_id;
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

static int32_t figure_of_type_at(int32_t grid_offset, uint8_t type)
{
    for (int32_t id = map_grids.figure_ids[grid_offset]; id != 0;) {
        const figure *f = figure_get(id);
        if (f->type == type) {
            return id;
        }
        id = f->next_figure_id_on_same_tile;
    }
    return 0;
}

int32_t figure_transport_ship_at_grid_offset(int32_t grid_offset)
{
    return figure_of_type_at(grid_offset, FIGURE_TRANSPORT_SHIP);
}

int32_t figure_warship_at_grid_offset(int32_t grid_offset)
{
    return figure_of_type_at(grid_offset, FIGURE_WARSHIP);
}

static int32_t no_click_selection(void)
{
    return (window_current_id == WINDOW_CITY && construction_building_type != BUILDING_NONE) || map_info.selected_grid == 0;
}

static int32_t clicked_grid_offset(void)
{
    return map_info.grid_start + map_info.cursor_tile_x + (map_info.cursor_tile_y * GRID_SIZE);
}

static int32_t has_request_event(int32_t figure_id)
{
    return figure_get(figure_id)->request_event_id >= NO_REQUEST_EVENT_BELOW;
}

int32_t city_select_legion(void)
{
    if (no_click_selection()) {
        return 0;
    }
    int32_t legion = formation_legion_at_grid_offset(clicked_grid_offset());
    if (legion <= 0) {
        return 0;
    }
    if (formation_get(legion)->request_event_id >= NO_REQUEST_EVENT_BELOW) {
        return 0;
    }
    if (formation_legion_is_on_ship(legion) != 0) {
        return 0;
    }
    formation_selected = legion;
    window_current_id = WINDOW_CITY_LEGION_SELECTED;
    window_redraw_request = WINDOW_REDRAW_ALL;
    return 1;
}

int32_t city_select_transport(void)
{
    if (no_click_selection()) {
        return 0;
    }
    int32_t grid = clicked_grid_offset();
    int32_t id = figure_transport_ship_at_grid_offset(grid);
    for (int32_t d = 0; id <= 0 && d < DIRECTION_COUNT; d++) {
        id = figure_transport_ship_at_grid_offset(grid + map_grid_direction_delta[d]);
    }
    if (id <= 0) {
        return 0;
    }
    selected_transport_id = id;
    selected_transport_active = 1;
    window_redraw_request = WINDOW_REDRAW_ALL;
    window_current_id = WINDOW_CITY_TRANSPORT_SELECTED;
    return 1;
}

static int32_t neighbour_warship_without_request(int32_t grid)
{
    for (int32_t d = 0; d < DIRECTION_COUNT; d++) {
        int32_t other = figure_warship_at_grid_offset(grid + map_grid_direction_delta[d]);
        if (other > 0 && !has_request_event(other)) {
            return other;
        }
    }
    return 0;
}

int32_t city_select_warship(void)
{
    if (no_click_selection()) {
        return 0;
    }
    int32_t grid = clicked_grid_offset();
    int32_t id = figure_warship_at_grid_offset(grid);
    if (id <= 0 || !has_request_event(id)) {
        id = neighbour_warship_without_request(grid);
        if (id == 0) {
            return 0;
        }
    }
    selected_warship_id = id;
    selected_warship_active = 1;
    window_redraw_request = WINDOW_REDRAW_ALL;
    window_current_id = WINDOW_CITY_WARSHIP_SELECTED;
    return 1;
}
