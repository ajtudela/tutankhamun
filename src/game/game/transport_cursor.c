// The cursor checks of a selected transport ship: embark when the ship is empty and a legion is on the mouse tile,
// land when it carries troops and the mouse tile has no water.

#include "game/transport_cursor.h"

#include "figure/figure.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "widget/city_selection.h"
#include "window/window_data.h"

#include <stdint.h>

static int32_t mouse_grid_offset(void)
{
    return map_grid_offset(map_info.cursor_tile_x, map_info.cursor_tile_y);
}

int32_t transport_cursor_can_embark(int32_t figure_id)
{
    if (selected_transport_active == 0 || city_figures.records[figure_id].has_troops_on_board != 0) {
        return 0;
    }
    return formation_legion_at_grid_offset(mouse_grid_offset()) != 0;
}

int32_t transport_cursor_can_land(int32_t figure_id)
{
    if (selected_transport_active == 0 || city_figures.records[figure_id].has_troops_on_board == 0) {
        return 0;
    }
    return (map_grids.terrain[mouse_grid_offset()] & TERRAIN_WATER) != TERRAIN_WATER;
}
