// The empire file (Pharaoh2.emp / Pharaoh.emp): its header of 40 empire maps, the map objects and the trade routes of
// each climate, read into a new scenario or switched by the empire editor and written back; invalid routes are reset
// and valid ones get their path drawn and their end cities found. Also the line drawing of a route path, used when
// saving and by the empire editor.
//
// The objects are always read from the first climate of the file, and the route path is laid out in a mix of float
// and double arithmetic with the coordinates truncated. The routes are read and written with 19600 bytes, more than
// the 50 routes: the trader index and the first traders after them come from the file too.

#include "io/empire_file.h"

#include "core/calc.h"
#include "editor/editor_data.h"
#include "empire/empire.h"
#include "graphics/image_collection.h"
#include "io/file.h"
#include "io/file_write.h"
#include "io/game_file_internal.h"
#include "io/io_data.h"
#include "scenario/scenario_climate.h"

#include <io.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum empire_file_layout {
    EMPIRE_FILE_HEADER_SIZE = sizeof(empire_map_headers),
    EMPIRE_FILE_OBJECTS_SIZE = sizeof(empire_object_records),
    EMPIRE_FILE_OBJECTS_END = EMPIRE_FILE_HEADER_SIZE + (EMPIRE_OBJECT_COUNT * sizeof(empire_object_before_object_flags)),
    EMPIRE_FILE_ROUTES_START = EMPIRE_FILE_HEADER_SIZE + (EMPIRE_MAP_HEADERS * EMPIRE_FILE_OBJECTS_SIZE),
    EMPIRE_FILE_ROUTES_SIZE = EMPIRE_ROUTES * sizeof(empire_route),
    EMPIRE_ROUTES_TRANSFERRED = 19600,
    EMPIRE_FILE_SIZE_PHARAOH = 0x185790,
    EMPIRE_FILE_SIZE_CLEOPATRA = 0x1D2860,
    EMPIRE_FILE_PADDING_ROUTES = 2000,
};

enum empire_route_rule {
    ROUTE_KIND_MAX = 2,
    ROUTE_KIND_RESET = 1,
    ROUTE_STEP_MIN = 1,
    ROUTE_STEP_MAX = 50,
    ROUTE_STEP_RESET = 5,
    ROUTE_PATH_SPARE_POINTS = 50,
};

static int32_t point_inside_image(int32_t x, int32_t y, int32_t left, int32_t top, int32_t width, int32_t height)
{
    return (left <= x) && (x <= width + left) && (top <= y) && (y <= height + top);
}

char empire_route_find_objects(empire_route *r)
{
    r->last_object = -1;
    r->first_object = -1;
    int32_t count = r->num_points;
    if (count == 0) {
        return 0;
    }
    const empire_route_point *first = &r->points[0];
    const empire_route_point *last = &r->points[count - 1];
    int16_t index = 0;
    for (const empire_object *o = empire_map_objects; o < empire_map_objects + EMPIRE_OBJECT_COUNT; o++, index++) {
        if (!o->in_use || (o->type != EMPIRE_OBJECT_CITY)) {
            continue;
        }
        int32_t image = image_known_id(o->image_id);
        int32_t width = image_collection.headers[image].width;
        int32_t height = image_collection.headers[image].height;
        if (point_inside_image(first->x, first->y, o->x, o->y, width, height)) {
            r->first_object = index;
        }
        if (point_inside_image(last->x, last->y, o->x, o->y, width, height)) {
            r->last_object = index;
        }
    }
    return 1;
}

void empire_route_free_paths(void)
{
    for (empire_route *r = empire_map_routes.routes; r < empire_map_routes.routes + EMPIRE_ROUTES; r++) {
        if (empire_route_path(r)) {
            free(empire_route_path(r));
            empire_route_path_set(r, NULL);
            r->path_capacity = 0;
        }
    }
}

void empire_route_reset(empire_route *r)
{
    r->path_length = 0;
    empire_route_path_set(r, NULL);
    r->num_points = 0;
    r->path_capacity = 0;
    r->step = ROUTE_STEP_RESET;
    r->in_use = 0;
    r->first_object = -1;
    r->last_object = -1;
    r->kind = ROUTE_KIND_RESET;
}

int32_t empire_route_is_valid(const empire_route *r)
{
    return !r->path_slot && !r->path_capacity && (r->num_points <= EMPIRE_ROUTE_POINTS) && (r->kind <= ROUTE_KIND_MAX) && (r->step >= ROUTE_STEP_MIN) &&
        (r->step <= ROUTE_STEP_MAX);
}

static int32_t route_path_points(const empire_route *r)
{
    int32_t total = 0;
    for (int32_t p = 1; p < r->num_points; p++) {
        int32_t dx = abs(r->points[p - 1].x - r->points[p].x);
        int32_t dy = abs(r->points[p - 1].y - r->points[p].y);
        total += ((dx > dy) ? dx : dy) + 1;
    }
    return total;
}

static void reserve_route_path(empire_route *r, int32_t points)
{
    if ((points <= r->path_capacity) || (r->path_capacity >= points + ROUTE_PATH_SPARE_POINTS)) {
        return;
    }
    free(empire_route_path(r));
    empire_route_path_set(r, NULL);
    empire_route_path_set(r, malloc((size_t) (points + ROUTE_PATH_SPARE_POINTS) * sizeof(empire_route_point)));
    r->path_capacity = points + ROUTE_PATH_SPARE_POINTS;
}

static void draw_route_path(empire_route *r)
{
    int32_t length = 0;
    for (int32_t p = 1; p < r->num_points; p++) {
        if (length > r->path_length) {
            return;
        }
        length += empire_route_draw_segment(r, p - 1, p, length);
    }
    r->path_length = length;
    empire_route_find_objects(r);
}

void empire_route_prepare_path(empire_route *r)
{
    int32_t points = route_path_points(r);
    reserve_route_path(r, points);
    r->path_length = points;
}

void empire_route_redraw_path(empire_route *r)
{
    empire_route_prepare_path(r);
    draw_route_path(r);
}

static void rebuild_route_paths(void)
{
    empire_map_routes.routes[0].num_points = 0;
    empire_map_routes.routes[0].path_length = 0;
    for (empire_route *r = empire_map_routes.routes; r < empire_map_routes.routes + EMPIRE_ROUTES; r++) {
        if (!empire_route_is_valid(r)) {
            empire_route_reset(r);
            continue;
        }
        empire_route_redraw_path(r);
    }
}

static void read_empire_objects(void)
{
    memset(empire_map_objects, 0, EMPIRE_FILE_OBJECTS_SIZE);
    empire_object *o = empire_map_objects;
    for (int32_t offset = EMPIRE_FILE_HEADER_SIZE; offset < EMPIRE_FILE_OBJECTS_END; offset += (int32_t) sizeof(empire_object_before_object_flags)) {
        io_read_file_part(str_pharaoh2_emp, o, (int32_t) sizeof(empire_object_before_object_flags), offset);
        o++;
    }
    game_file_upgrade_empire_object_flags();
}

static int32_t objects_offset(int32_t climate)
{
    return (climate * EMPIRE_FILE_OBJECTS_SIZE) + EMPIRE_FILE_HEADER_SIZE;
}

static int32_t routes_offset(int32_t climate)
{
    return (climate * EMPIRE_FILE_ROUTES_SIZE) + EMPIRE_FILE_ROUTES_START;
}

static void read_routes(const char *path, int32_t climate)
{
    empire_route_free_paths();
    io_read_file_part(path, &empire_map_routes, EMPIRE_ROUTES_TRANSFERRED, routes_offset(climate));
    rebuild_route_paths();
    empire_object_set_images();
    empire_object_set_city_images();
}

static int32_t empire_file_needs_padding(int32_t size)
{
    return (size != EMPIRE_FILE_SIZE_PHARAOH) && (size != EMPIRE_FILE_SIZE_CLEOPATRA) && (size < EMPIRE_FILE_SIZE_PHARAOH);
}

static void pad_empire_file(const char *path, int32_t size)
{
    int32_t objects_left = EMPIRE_MAP_HEADERS - ((size - EMPIRE_FILE_HEADER_SIZE) / EMPIRE_FILE_OBJECTS_SIZE);
    int32_t partial = (size - EMPIRE_FILE_HEADER_SIZE) % EMPIRE_FILE_OBJECTS_SIZE;
    memset(empire_map_objects, 0, EMPIRE_FILE_OBJECTS_SIZE);
    if (partial) {
        io_append_file(path, empire_map_objects, EMPIRE_FILE_OBJECTS_SIZE - partial);
        objects_left--;
    }
    for (; objects_left > 0; objects_left--) {
        io_append_file(path, empire_map_objects, EMPIRE_FILE_OBJECTS_SIZE);
    }
    empire_route empty;
    memset(&empty, 0, sizeof(empty));
    empire_route_reset(&empty);
    for (int32_t i = 0; i < EMPIRE_FILE_PADDING_ROUTES; i++) {
        io_append_file(path, &empty, sizeof(empty));
    }
}

void empire_file_load(int32_t cleopatra)
{
    const char *path = cleopatra ? str_pharaoh2_emp : str_pharaoh_emp;
    FILE *file = fopen(path, file_mode_read_update_binary);
    if (file) {
        int32_t size = _filelength(_fileno(file));
        fclose(file);
        if (empire_file_needs_padding(size)) {
            pad_empire_file(path, size);
        }
    }
    io_read_file_part(path, empire_header, EMPIRE_FILE_HEADER_SIZE, 0);
    read_empire_objects();
    read_routes(path, scenario_climate_current);
}

void empire_file_select(int32_t index)
{
    if ((index >= EMPIRE_MAP_HEADERS) || (index < 0)) {
        return;
    }
    memmove(&empire_header[scenario_climate_current], &empire_header[index], sizeof(empire_map_header));
    read_empire_objects();
    read_routes(str_pharaoh2_emp, index);
    empire_file_save();
}

void empire_file_save(void)
{
    io_write_file_part(str_pharaoh2_emp, empire_header, EMPIRE_FILE_HEADER_SIZE, 0);
    io_write_file_part(str_pharaoh2_emp, empire_map_objects, EMPIRE_FILE_OBJECTS_SIZE, objects_offset(scenario_climate_current));
    empire_map_routes.routes[0].num_points = 0;
    empire_map_routes.routes[0].path_length = 0;
    empire_route_free_paths();
    io_write_file_part(str_pharaoh2_emp, &empire_map_routes, EMPIRE_ROUTES_TRANSFERRED, routes_offset(scenario_climate_current));
    rebuild_route_paths();
}

int32_t empire_route_draw_segment(empire_route *r, int32_t from, int32_t to, int32_t at)
{
    int32_t x0 = r->points[from].x;
    int32_t y0 = r->points[from].y;
    int32_t dx = r->points[to].x - x0;
    int32_t dy = r->points[to].y - y0;
    float distance = (float) sqrt(((double) dx * dx) + ((double) dy * dy));
    if ((distance == 0.0f) || !r->step) {
        return 0;
    }
    float step_y = (float) ((double) dy / distance);
    float step_x = (float) ((double) dx / distance);
    float y0f = (float) y0;
    int32_t count = 0;
    int32_t t = 0;
    empire_route_point *p = &empire_route_path(r)[at];
    do {
        p->x = (int16_t) (int32_t) (((double) t * step_x) + (double) x0);
        p->y = (int16_t) (int32_t) (((double) t * step_y) + (double) y0f);
        p->segment = (uint8_t) from;
        p++;
        t += r->step;
        count++;
    } while ((double) t <= (double) distance);
    return count;
}
