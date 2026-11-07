// The empire editor's trade routes: the route methods (way points, end objects, the point under the mouse), the
// rebuild of a route's path from its way points, the numeric input for a route field and the route buttons of the
// window. A point within 8 pixels of a way point picks it, else a path point picks its segment.

#include "editor/empire_editor_routes.h"
#include "core/crt_compat.h"
#include "editor/editor_data.h"
#include "editor/editor_internal.h"
#include "editor/empire_editor_objects.h"
#include "empire/empire_data.h"
#include "empire/empire_types.h"
#include "graphics/graphics_data.h"
#include "io/empire_file.h"
#include "platform/log.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    ROUTE_PICK_DISTANCE = 8,
    ROUTE_PATH_SPARE_POINTS = 50,
    ROUTE_INPUT_X = 453,
    ROUTE_INPUT_FROM_BOTTOM = 314,
};

int32_t empire_route_last_point(empire_route *self)
{
    return (int32_t) self->num_points - 1;
}

bool empire_route_clear(empire_route *self)
{
    self->num_points = 0;
    self->path_length = 0;
    return true;
}

void empire_route_set_in_use(empire_route *self, int32_t value)
{
    self->in_use = (uint8_t) value;
}

int32_t empire_route_start_object(empire_route *self)
{
    return self->first_object;
}

int32_t empire_route_end_object(empire_route *self)
{
    return self->last_object;
}

int32_t empire_route_point_at(empire_route *self, int32_t x, int32_t y, uint8_t *on_point)
{
    int32_t x0 = x - ROUTE_PICK_DISTANCE;
    int32_t x1 = x + ROUTE_PICK_DISTANCE;
    int32_t y0 = y - ROUTE_PICK_DISTANCE;
    int32_t y1 = y + ROUTE_PICK_DISTANCE;
    int32_t count = self->num_points;
    for (int32_t i = 0; i < count; i++) {
        int32_t px = self->points[i].x;
        if (px >= x0 && px <= x1) {
            int32_t py = self->points[i].y;
            if (py >= y0 && py <= y1) {
                *on_point = 1;
                return i;
            }
        }
    }
    *on_point = 0;
    int32_t path_length = self->path_length;
    const empire_route_point *p = empire_route_path(self);
    for (int32_t i = 0; i < path_length; i++, p++) {
        if (p->x >= x0 && p->x <= x1 && p->y >= y0 && p->y <= y1) {
            return p->segment;
        }
    }
    return -1;
}

static int32_t larger(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

static int32_t route_path_room(const empire_route *route)
{
    int32_t total = 0;
    for (int32_t i = 1; i < route->num_points; i++) {
        int32_t x0 = route->points[i - 1].x;
        int32_t x1 = route->points[i].x;
        int32_t y0 = route->points[i - 1].y;
        int32_t y1 = route->points[i].y;
        int32_t ax = larger(x0 - x1, x1 - x0);
        int32_t ay = larger(y0 - y1, y1 - y0);
        total += (ax > ay ? ax : ay) + 1;
    }
    return total;
}

bool empire_route_rebuild(empire_route *route)
{
    int32_t total = route_path_room(route);
    if (total > route->path_capacity) {
        int32_t capacity = total + ROUTE_PATH_SPARE_POINTS;
        if (route->path_capacity < capacity) {
            free(empire_route_path(route));
            empire_route_path_set(route, NULL);
            empire_route_path_set(route, (empire_route_point *) malloc((uint32_t) (capacity * 6)));
            route->path_capacity = capacity;
        }
    }
    route->path_length = total;

    int32_t running = 0;
    for (int32_t i = 1; i < route->num_points; i++) {
        int32_t n = 0;
        if (running > route->path_length) {
            return false;
        }
        int32_t x0 = route->points[i - 1].x;
        int32_t y0 = route->points[i - 1].y;
        int32_t dx = route->points[i].x - x0;
        int32_t dy = route->points[i].y - y0;
        float len = (float) sqrt(((double) dx * (double) dx) + ((double) dy * (double) dy));
        if (len != 0.0f) {
            if (route->step != 0) {
                float uy = (float) ((double) dy / (double) len);
                float ux = (float) ((double) dx / (double) len);
                if (0.0f <= len) {
                    float yf = (float) y0;
                    int32_t d = 0;
                    do {
                        empire_route_point *p = &empire_route_path(route)[running + n];
                        p->x = (int16_t) crt_ftol_double(((double) d * (double) ux) + (double) x0);
                        p->y = (int16_t) crt_ftol_double(((double) d * (double) uy) + (double) yf);
                        p->segment = (uint8_t) (i - 1);
                        d += route->step;
                        n++;
                    } while ((double) d <= (double) len);
                }
            } else {
                n = 0;
            }
        }
        running += n;
    }
    route->path_length = running;
    empire_route_find_objects(route);
    return true;
}

bool empire_route_delete_point(empire_route *self, int32_t index)
{
    if (index < 0) {
        return false;
    }
    if (index >= self->num_points) {
        return false;
    }
    self->num_points--;
    for (int32_t i = index; i < self->num_points; i++) {
        self->points[i] = self->points[i + 1];
    }
    return empire_route_rebuild(self);
}

void route_step_changed(void)
{
    empire_route_rebuild(&empire_map_routes.routes[route_current]);
    empire_editor_route_dragging = 0;
}

static void numeric_input_return_here(input_dialog_callback on_ok)
{
    numeric_input_on_ok = on_ok;
    numeric_input_return_window.word = window_current_id;
    numeric_input_value = 0;
    numeric_input_digits = 0;
    window_current_id = WINDOW_NUMERIC_INPUT;
}

void numeric_input_show_int32(int32_t x, int32_t y, int32_t *target, input_dialog_callback on_ok)
{
    numeric_input_target32 = target;
    numeric_input_x = x;
    numeric_input_y = y;
    numeric_input_width = 4;
    numeric_input_editor_use = 0;
    numeric_input_target8 = 0;
    numeric_input_target16 = 0;
    numeric_input_return_here(on_ok);
}

void numeric_input_show_int8(int32_t x, int32_t y, uint8_t *target, input_dialog_callback on_ok)
{
    numeric_input_target8 = (int8_t *) target;
    numeric_input_x = x;
    numeric_input_y = y;
    numeric_input_width = 1;
    numeric_input_editor_use = 0;
    numeric_input_target16 = 0;
    numeric_input_target32 = 0;
    numeric_input_return_here(on_ok);
}

static int32_t route_is_free(empire_route *route)
{
    if (route->num_points <= 1) {
        route->in_use = 0;
    }
    return route->in_use == 0;
}

int32_t empire_route_first_free(void)
{
    for (int32_t i = 1; i < EMPIRE_TRADE_ROUTES; i++) {
        if (route_is_free(&empire_map_routes.routes[i])) {
            return i;
        }
    }
    return -1;
}

void route_step_button(void)
{
    numeric_input_show_int8(
        ROUTE_INPUT_X, g_screen_height - ROUTE_INPUT_FROM_BOTTOM, &empire_map_routes.routes[route_current].step, route_step_changed);
}

void route_delete_button(void)
{
    empire_route_clear(&empire_map_routes.routes[route_current]);
    if (route_current > 1) {
        route_current--;
    }
    window_redraw_request = 1;
}

void route_number_changed(void)
{
    int32_t current = route_current;
    if (current < 1 || current >= EMPIRE_TRADE_ROUTES) {
        route_current = 1;
        return;
    }
    if (route_is_free(&empire_map_routes.routes[current])) {
        empire_route_clear(&empire_map_routes.routes[route_current]);
        empire_ui_mode = EMPIRE_EDITOR_MODE_ADD_ROUTE;
    }
}

void route_number_button(void)
{
    numeric_input_show_int32(ROUTE_INPUT_X, g_screen_height - ROUTE_INPUT_FROM_BOTTOM, &route_current, route_number_changed);
}

void route_kind_button(void)
{
    empire_route *route = &empire_map_routes.routes[route_current];
    route->kind = (uint8_t) ((route->kind == EMPIRE_ROUTE_KIND_LAND) ? EMPIRE_ROUTE_KIND_SEA : EMPIRE_ROUTE_KIND_LAND);
    empire_editor_route_dragging = 0;
    window_redraw_request = 1;
}
