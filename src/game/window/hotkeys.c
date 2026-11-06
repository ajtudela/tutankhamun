// The hotkeys of the city window: the legion cycle, the map bookmarks F1-F4 (Ctrl sets them), the game speed keys,
// the overlays and the advisors; and two helpers, whether the mouse is inside a rectangle and the god of a temple type.

#include "window/hotkeys.h"
#include "building/building_type.h"
#include "building/royal_tomb.h"
#include "city/city_player.h"
#include "city/gods.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "game/tutorial.h"
#include "input/input_data.h"
#include "map/grid.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include "window/advisors.h"
#include "window/window_id.h"
#include <stdint.h>

static int32_t window_is_city_unit_selection(int32_t window)
{
    return window == WINDOW_CITY_LEGION_SELECTED || window == WINDOW_CITY_LEGION_ORDER || window == WINDOW_CITY_TRANSPORT_SELECTED ||
        window == WINDOW_CITY_WARSHIP_SELECTED;
}

static int32_t formation_is_cyclable_legion(const formation *m)
{
    return m->in_use == 1 && !m->herd_kind && m->is_legion;
}

void hotkey_cycle_legion(void)
{
    if (window_current_id != WINDOW_CITY) {
        return;
    }
    int32_t id = legion_cycle_last;
    int32_t found = 0;
    legion_cycle_last = 0;
    for (int32_t i = FORMATION_MAX_LEGIONS; i != 0; i--) {
        id++;
        if (id > FORMATION_MAX_LEGIONS) {
            id = 1;
        }
        if (formation_is_cyclable_legion(formation_get(id)) && found == 0) {
            found = id;
        }
    }
    legion_cycle_last = found;
    if (found > 0) {
        const formation *legion = formation_get(found);
        city_view_go_to_grid_offset(map_grid_offset(legion->x, legion->y));
        window_redraw_request = 1;
    }
}

void hotkey_go_to_bookmark(int32_t index)
{
    if (window_current_id == WINDOW_CITY_LEGION_SELECTED || window_current_id == WINDOW_CITY_LEGION_ORDER) {
        window_current_id = WINDOW_CITY;
    } else if (window_current_id != WINDOW_CITY) {
        return;
    }
    if (index < 0 || index > MAP_BOOKMARK_LAST_INDEX) {
        return;
    }
    int32_t offset = map_bookmarks[index].grid_offset;
    if (offset > MAP_BOOKMARK_NONE) {
        city_view_go_to_grid_offset(offset);
        window_redraw_request = 1;
    }
}

void hotkey_set_bookmark(int32_t index)
{
    if (window_is_city_unit_selection(window_current_id)) {
        window_current_id = WINDOW_CITY;
    } else if (window_current_id != WINDOW_CITY) {
        return;
    }
    if (index < 0 || index > MAP_BOOKMARK_LAST_INDEX) {
        return;
    }
    int32_t center_row = (city_view.height_tiles / 2) + map_info.camera_y;
    int32_t center_column = (city_view.width_tiles / 2) + map_info.camera_x;
    map_bookmarks[index].grid_offset = city_view.lookup[(center_row * VIEW_COLS) + center_column];
}

void map_bookmarks_reset(void)
{
    for (int32_t i = 0; i < MAP_BOOKMARKS; i++) {
        map_bookmarks[i].grid_offset = MAP_BOOKMARK_NONE;
        map_bookmarks[i].unused = MAP_BOOKMARK_NONE;
    }
}

void hotkey_change_game_speed(int32_t down)
{
    if (window_current_id == WINDOW_CITY) {
        button_parameter = down;
        settings_change_game_speed();
    }
}

int32_t mouse_inside_rect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    return x <= mouse_x && mouse_x < (x + width) && y <= mouse_y && mouse_y < (y + height);
}

int32_t building_type_to_god(int32_t type)
{
    switch (type) {
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
            return GOD_OSIRIS;
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_COMPLEX_RA:
            return GOD_RA;
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
            return GOD_PTAH;
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
            return GOD_SETH;
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_BAST:
            return GOD_BAST;
        default:
            return -1;
    }
}

void hotkey_toggle_overlay(int32_t overlay)
{
    if (window_is_city_unit_selection(window_current_id)) {
        window_current_id = WINDOW_CITY;
    } else if (window_current_id == WINDOW_TOOLTIP_HOVER || window_current_id == WINDOW_TOOLTIP) {
        if (tooltip_window_id != WINDOW_CITY) {
            return;
        }
    } else if (window_current_id != WINDOW_CITY) {
        return;
    }
    if (city_overlay != overlay) {
        city_overlay_previous = 0;
        city_overlay = overlay;
    } else {
        city_overlay_previous = overlay;
        city_overlay = 0;
    }
    window_redraw_request = 1;
    royal_tombs_update_all_images();
}

void hotkey_show_advisor(int32_t advisor)
{
    if (window_current_id == WINDOW_ADVISORS) {
        if (advisor_shown == advisor) {
            window_current_id = WINDOW_CITY;
            window_redraw_request = 1;
            city_player_current()->trade_prices_mode = 0;
            return;
        }
    } else if (window_is_city_unit_selection(window_current_id)) {
        window_current_id = WINDOW_CITY;
    } else if (window_current_id != WINDOW_CITY) {
        return;
    }
    button_parameter = advisor;
    window_advisors_show_checked();
}
