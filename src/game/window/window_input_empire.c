// The input of the scenario editor's empire map: its buttons and the trade route editing (pick, drag, insert and
// delete way points; draw a new route point by point with a rubber-band last point). Every change rebuilds the
// route's path and its end objects and redraws the window.

#include "core/crt_compat.h"
#include "editor/editor_data.h"
#include "editor/editor_new_map_tiles.h"
#include "editor/empire_editor_objects.h"
#include "editor/empire_editor_routes.h"
#include "empire/empire.h"
#include "empire/empire_types.h"
#include "game/game_internal.h"
#include "graphics/arrow_buttons.h"
#include "graphics/generic_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/layout_buttons.h"
#include "input/input_data.h"
#include "widget/city_view.h"
#include "window/empire_editor_input.h"
#include "window/empire_scroll.h"
#include "window/window_data.h"
#include "window/window_input.h"
#include <stdint.h>

static int32_t generic_buttons(int32_t x, int32_t y, generic_button *buttons, int32_t count, int32_t *focus)
{
    int32_t hover = generic_buttons_get_focus(x, y, buttons, count);
    *focus = ui_layout_buttons(x, y, (uint8_t *) buttons, count, hover);
    return generic_buttons_handle_mouse(x, y, buttons, count);
}

static int32_t panel_buttons(const generic_button *buttons, int32_t count)
{
    return generic_buttons_handle_mouse(223, g_screen_height - 114, buttons, count);
}

static int32_t mouse_on_map(void)
{
    return mouse_x >= empire_border_x && mouse_x < display_width - (empire_border_x * 2) && mouse_y >= empire_border_y &&
        mouse_y < screen_height - empire_border_bottom - empire_border_y;
}

static int32_t route_insert_point(empire_route *route, int32_t pos, int32_t x, int32_t y)
{
    if (route->num_points >= EMPIRE_ROUTE_POINTS) {
        return 0;
    }
    for (int32_t i = route->num_points - 1; i >= pos; i--) {
        route->points[i + 1] = route->points[i];
    }
    route->points[pos].x = (int16_t) x;
    route->points[pos].y = (int16_t) y;
    route->num_points++;
    return 1;
}

static void route_append_point(empire_route *route, int32_t x, int32_t y)
{
    if (route_insert_point(route, route->num_points, x, y)) {
        empire_route_rebuild(route);
    }
}

static void route_move_point(empire_route *route, int32_t index, int32_t x, int32_t y)
{
    if (mouse_on_map() && index >= 0 && index < route->num_points) {
        route->points[index].x = (int16_t) x;
        route->points[index].y = (int16_t) y;
        empire_route_rebuild(route);
    }
}

static void route_select_other(int32_t wx, int32_t wy)
{
    for (int32_t r = 0; r < EMPIRE_ROUTES; r++) {
        uint8_t on_point;
        if (empire_route_point_at(&empire_map_routes.routes[r], wx, wy, &on_point) >= 0) {
            route_current = r;
            window_redraw_request = 1;
            return;
        }
    }
}

static int32_t route_edit_press(empire_route *route, int32_t wx, int32_t wy)
{
    uint8_t on_point = 0;
    int32_t index = empire_route_point_at(route, wx, wy, &on_point);
    route_point_selected = index;
    if (index < 0) {
        route_select_other(wx, wy);
        return 0;
    }
    if (!on_point) {
        int32_t pos = index + 1;
        if (route_insert_point(route, pos, wx, wy) && empire_route_rebuild(route)) {
            route_point_selected = pos;
        } else {
            route_point_selected = -1;
        }
        return 1;
    }
    route_move_point(route, index, wx, wy);
    return 1;
}

static int32_t route_edit_release(empire_route *route, int32_t wx, int32_t wy)
{
    int32_t index = route_point_selected;
    route_point_selected = -1;
    if (index < 0) {
        return 0;
    }
    route_move_point(route, index, wx, wy);
    return 1;
}

static int32_t route_edit_drag(empire_route *route, int32_t wx, int32_t wy)
{
    int32_t index = route_point_selected;
    if (index < 0) {
        return 0;
    }
    route_move_point(route, index, wx, wy);
    return 1;
}

static int32_t route_edit_delete(empire_route *route, int32_t wx, int32_t wy)
{
    uint8_t on_point = 0;
    int32_t index = empire_route_point_at(route, wx, wy, &on_point);
    if (index < 0 || on_point == 0) {
        return 0;
    }
    empire_route_delete_point(route, index);
    if (empire_route_last_point(route) > 0) {
        return 0;
    }
    empire_route_set_in_use(route, 0);
    empire_route_clear(route);
    return 1;
}

static int32_t route_edit_mouse(empire_route *route, int32_t wx, int32_t wy)
{
    if (mouse_left_went_down != 0) {
        return route_edit_press(route, wx, wy);
    }
    if (mouse_left_went_up != 0) {
        return route_edit_release(route, wx, wy);
    }
    if (mouse_left_down != 0) {
        return route_edit_drag(route, wx, wy);
    }
    if (mouse_right_went_up != 0) {
        return route_edit_delete(route, wx, wy);
    }
    return 0;
}

static void redraw_when_route_ends_changed(int32_t saved_start, int32_t saved_end)
{
    const empire_route *route = &empire_map_routes.routes[route_current];
    if (saved_start != route->first_object || saved_end != route->last_object) {
        window_redraw_request = 1;
    }
}

static void editor_routes_edit(int32_t wx, int32_t wy)
{
    if (panel_buttons(build_menu_buttons.empire_editor_routes, 4) != 0) {
        return;
    }
    empire_route *route = &empire_map_routes.routes[route_current];
    int32_t saved_end = route->last_object;
    int32_t saved_start = route->first_object;
    if (route_edit_mouse(route, wx, wy)) {
        empire_editor_route_dragging = 0;
    }
    redraw_when_route_ends_changed(saved_start, saved_end);
}

static void editor_routes_draw(int32_t wx, int32_t wy, int32_t saved_start, int32_t saved_end)
{
    empire_route *route = &empire_map_routes.routes[route_current];
    int32_t last = route->num_points - 1;
    if (mouse_left_went_up != 0) {
        if (last < 0) {
            if (!mouse_on_map()) {
                return;
            }
            route->in_use = 1;
            route_append_point(route, wx, wy);
        } else {
            route_move_point(route, last, wx, wy);
        }
        route_append_point(route, wx, wy);
        empire_editor_route_dragging = 0;
    } else if (mouse_right_went_up != 0) {
        empire_ui_mode = EMPIRE_EDITOR_MODE_EDIT_ROUTE;
    } else {
        route_move_point(route, last, wx, wy);
        empire_editor_route_dragging = 0;
    }
    redraw_when_route_ends_changed(saved_start, saved_end);
}

static void editor_select_object(int32_t object)
{
    g_empire_selected_object = object - 1;
    empire_editor_image = empire_map_objects[object - 1].image_id;
}

void window_input_empire_editor(void)
{
    int32_t wx = empire_scroll_x - empire_border_x + mouse_x;
    int32_t wy = empire_scroll_y - empire_border_y + mouse_y;
    int32_t saved_start = empire_map_routes.routes[route_current].first_object;
    int32_t saved_end = empire_map_routes.routes[route_current].last_object;
    int32_t object;

    empire_editor_count_objects();
    empire_object_set_city_images();
    empire_scroll_map(scroll_get_direction());
    if (generic_buttons(15, g_screen_height - 116, build_menu_buttons.empire_editor_modes, 8, &focus_button_id) != 0) {
        return;
    }
    if (focus_button_id == 0) {
        if (generic_buttons(223, g_screen_height - 114, build_menu_buttons.empire_editor_general, 1, &focus_button_id) != 0) {
            return;
        }
        if (focus_button_id == 1) {
            focus_button_id = 10;
        }
    }
    switch (empire_ui_mode) {
        case EMPIRE_EDITOR_MODE_ADD_ROUTE:
            editor_routes_draw(wx, wy, saved_start, saved_end);
            return;
        case EMPIRE_EDITOR_MODE_EDIT_ROUTE:
            editor_routes_edit(wx, wy);
            return;
        case EMPIRE_EDITOR_MODE_ADD_OBJECT:
            if (empire_editor_image_kind == 0 &&
                arrow_buttons_handle_mouse(223, g_screen_height - 114, empire_editor_arrow_buttons, 5) != 0) {
                return;
            }
            if (panel_buttons(build_menu_buttons.empire_editor_ornament_kinds, 3) != 0) {
                return;
            }
            empire_editor_place_object(empire_editor_object_count, 0);
            object = empire_editor_object_at_mouse();
            if (object == 0) {
                return;
            }
            empire_ui_mode = EMPIRE_EDITOR_MODE_EDIT_OBJECTS;
            editor_select_object(object);
            return;
        case EMPIRE_EDITOR_MODE_EDIT_OBJECTS:
            if (window_input_delay_frames > 0) {
                window_input_delay_frames--;
                return;
            }
            if (empire_map_objects[g_empire_selected_object].type == EMPIRE_OBJECT_ORNAMENT &&
                arrow_buttons_handle_mouse(223, g_screen_height - 114, empire_editor_arrow_buttons, 5) != 0) {
                return;
            }
            object = g_empire_selected_object;
            switch (empire_map_objects[object].type) {
                case EMPIRE_OBJECT_CITY:
                    if (panel_buttons(build_menu_buttons.empire_editor_city, 6) != 0) {
                        return;
                    }
                    if (empire_map_objects[object].city_type == EMPIRE_CITY_OURS) {
                        if (panel_buttons(build_menu_buttons.empire_editor_our_city_sold, EMPIRE_OBJECT_SOLD_RESOURCES) != 0) {
                            return;
                        }
                    } else if (panel_buttons(build_menu_buttons.empire_editor_trade_city_amounts, 2 * EMPIRE_OBJECT_BOUGHT_RESOURCES) != 0) {
                        return;
                    }
                    empire_editor_check_city(object);
                    empire_editor_objects_start();
                    empire_editor_update_city_kinds();
                    break;
                case EMPIRE_OBJECT_BATTLE_ICON:
                    if (panel_buttons(build_menu_buttons.empire_editor_battle, 2) != 0) {
                        return;
                    }
                    break;
                case EMPIRE_OBJECT_TEXT:
                    if (panel_buttons(&build_menu_buttons.empire_editor_city[6], 1) != 0) {
                        return;
                    }
                    break;
                default:
                    break;
            }
            empire_editor_place_object(g_empire_selected_object, 1);
            object = empire_editor_object_at_mouse();
            if (object == 0) {
                return;
            }
            editor_select_object(object);
            return;
        case EMPIRE_EDITOR_MODE_GENERAL:
            panel_buttons(build_menu_buttons.empire_editor_general, 1);
            return;
        default:
            return;
    }
}
