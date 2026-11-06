// The figures' route slots (1000 paths of 500 directions): a figure gets a free slot (stale slots are reclaimed on the
// way) and its path from the search its terrain usage chooses, on land, on water or straight for a warship;
// figure_route_add_to_closest_of routes to the closest of a list of targets.
//
// A figure's own slot is reused as it is, also when it is negative, and the animal usage searches the closest target
// with the citizens' any-land search.

#include "figure/route.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/movement_internal.h"
#include "map/grid.h"
#include "map/routing.h"
#include "map/routing_enemy.h"
#include "map/routing_internal.h"
#include "map/routing_land.h"
#include "map/routing_water.h"
#include "window/window_data.h"
#include <string.h>

int32_t figure_route_get_first_available(int32_t figure_id, int32_t *path_id)
{
    figure *f = figure_get(figure_id);
    int16_t own = f->routing_path_id;
    if (own != 0) {
        if (figure_route_figure_ids[own] == figure_id) {
            *path_id = own;
            int16_t slot = f->routing_path_id;
            f->routing_path_id = 0;
            figure_route_figure_ids[slot] = 0;
            return 1;
        }
        f->routing_path_id = 0;
    }
    *path_id = 1;
    for (;;) {
        int32_t slot = *path_id;
        int32_t owner = figure_route_figure_ids[slot];
        if (owner == 0) {
            return 1;
        }
        if (owner < 0 || owner >= MAX_FIGURES) {
            figure_route_figure_ids[*path_id] = 0;
            return 1;
        }
        figure *o = figure_get(owner);
        if (o->routing_path_id != slot) {
            break;
        }
        if (o->state != FIGURE_STATE_ALIVE) {
            o->routing_path_id = 0;
            break;
        }
        *path_id = slot + 1;
        if (slot + 1 >= MAX_PATHS) {
            *path_id = 0;
            return 0;
        }
    }
    figure_route_figure_ids[*path_id] = 0;
    return 1;
}

void figure_route_remove(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int16_t slot = f->routing_path_id;
    if (slot > 0) {
        if (figure_route_figure_ids[slot] == figure_id) {
            figure_route_figure_ids[slot] = 0;
        }
        f->routing_path_id = 0;
    }
}

void figure_route_clear_all(void)
{
    memset(city_figures.route_paths, 0, MAX_PATHS * MAX_PATH_LENGTH);
    memset(figure_route_figure_ids, 0, MAX_PATHS * sizeof(int16_t));
}

void figure_route_clean(void)
{
    for (int32_t i = 1; i < MAX_PATHS; i++) {
        int32_t owner = figure_route_figure_ids[i];
        if (owner > 0 && owner < MAX_FIGURES) {
            figure *f = figure_get(owner);
            if (f->state != FIGURE_STATE_ALIVE) {
                figure_route_figure_ids[i] = 0;
            }
            if (f->routing_path_id != i) {
                figure_route_figure_ids[i] = 0;
            }
        }
    }
}

int32_t figure_route_count_free(void)
{
    int32_t count = 0;
    for (int32_t i = 1; i < MAX_PATHS; i++) {
        if (figure_route_figure_ids[i] == 0) {
            count++;
        }
    }
    return count;
}

static void calculate_water_distances(const figure *f)
{
    if (editor_active == 1) {
        map_routing_calculate_distances_water_flotsam(scenario_data.river_entry_x, scenario_data.river_entry_y);
    } else if (f->is_boat == FIGURE_BOAT_FLOTSAM) {
        map_routing_calculate_distances_water_flotsam(f->x, f->y);
    } else {
        map_routing_calculate_distances_water_boat(f->x, f->y);
    }
}

static int32_t is_straight_warship(const figure *f)
{
    return f->type == FIGURE_WARSHIP && (f->action_state == FIGURE_ACTION_WARSHIP_STRAIGHT || f->warship_secondary_action == FIGURE_ACTION_WARSHIP_STRAIGHT);
}

static int32_t travel(travel_function fn, const figure *f)
{
    return fn(f->x, f->y, f->destination_x, f->destination_y);
}

static int32_t get_path(int32_t num_directions, int32_t path_id, const figure *f)
{
    return map_routing_get_path(num_directions, path_id, f->x, f->y, f->destination_x, f->destination_y);
}

static int32_t get_path_on_water(int32_t path_id, const figure *f, int32_t is_flotsam)
{
    return map_routing_get_path_on_water(path_id, f->x, f->y, f->destination_x, f->destination_y, is_flotsam);
}

static int32_t searched_with(int32_t found, int32_t directions)
{
    return found ? directions : ROUTE_NOT_FOUND;
}

static int32_t boat_path_length(int32_t path_id, const figure *f)
{
    calculate_water_distances(f);
    if (f->is_boat == FIGURE_BOAT_FLOTSAM) {
        return get_path_on_water(path_id, f, 1);
    }
    if (is_straight_warship(f)) {
        int32_t length = map_routing_get_path_warship(path_id, f->x, f->y, f->destination_x, f->destination_y, f->ship.max_straight_steps);
        if (length != 0) {
            return length;
        }
    }
    return get_path_on_water(path_id, f, 0);
}

static int32_t land_path_length(int32_t path_id, const figure *f, int32_t directions)
{
    if (directions == ROUTE_FOUR_DIRECTIONS_FIRST) {
        int32_t length = get_path(4, path_id, f);
        if (length > 0) {
            return length;
        }
    }
    return get_path(8, path_id, f);
}

static void set_path(figure *f, int32_t figure_id, int32_t path_id, int32_t length)
{
    f->routing_path_id = (int16_t) path_id;
    f->routing_path_length = (int16_t) length;
    figure_route_figure_ids[path_id] = (int16_t) figure_id;
}

static int32_t route_directions_to_destination(const figure *f)
{
    switch (f->terrain_usage) {
        case TERRAIN_USAGE_ENEMY:
            if (map_routing_noncitizen_can_travel_over_land(f->x, f->y, f->destination_x, f->destination_y, 0, ROUTE_ENEMY_MAX_TILES)) {
                return ROUTE_EIGHT_DIRECTIONS;
            }
            return searched_with(travel(map_routing_noncitizen_can_travel_through_everything, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_WALLS:
            return searched_with(travel(map_routing_can_travel_over_walls, f), ROUTE_FOUR_DIRECTIONS_FIRST);
        case TERRAIN_USAGE_ANIMAL:
            return searched_with(travel(map_routing_animal_can_travel, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_PREFER_ROADS:
            if (travel(map_routing_citizen_can_travel_over_road_garden, f)) {
                return ROUTE_FOUR_DIRECTIONS_FIRST;
            }
            return searched_with(travel(map_routing_citizen_can_travel_over_land_prefer_roads, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_AMPHIBIA:
            if (travel(map_routing_amphibia_can_travel, f)) {
                return ROUTE_EIGHT_DIRECTIONS;
            }
            return searched_with(travel(map_routing_citizen_can_travel_over_land_any, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_ROADS:
            return searched_with(travel(map_routing_citizen_can_travel_over_road_garden, f), ROUTE_FOUR_DIRECTIONS_FIRST);
        case TERRAIN_USAGE_ANY:
            return searched_with(travel(map_routing_citizen_can_travel_over_land_any, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_NO_FERRY:
            return searched_with(travel(map_routing_citizen_can_travel_no_ferry, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_ANY_OR_FORT:
            return searched_with(travel(map_routing_citizen_can_travel_over_land_fort, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_ROADS_OR_MONUMENT:
            if (travel(map_routing_citizen_can_travel_over_road_garden_ext, f)) {
                return ROUTE_FOUR_DIRECTIONS_FIRST;
            }
            return searched_with(travel(map_routing_citizen_can_travel_over_land_no_unstaffed_ferry, f), ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_ANYWHERE:
            return searched_with(travel(map_routing_can_travel_anywhere, f), ROUTE_EIGHT_DIRECTIONS);
        default:
            return ROUTE_NOT_FOUND;
    }
}

void figure_route_add(int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    f->routing_path_id = 0;
    f->routing_path_current_tile = 0;
    f->routing_path_length = 0;
    int32_t path_id;
    if (!figure_route_get_first_available(figure_id, &path_id)) {
        return;
    }
    int32_t length;
    if (f->is_boat) {
        length = boat_path_length(path_id, f);
    } else {
        int32_t directions = route_directions_to_destination(f);
        if (directions == ROUTE_NOT_FOUND) {
            return;
        }
        length = land_path_length(path_id, f, directions);
    }
    if (length == 0) {
        return;
    }
    set_path(f, figure_id, path_id, length);
}

static int32_t route_directions_to_closest(const figure *f, const int32_t *grid_offsets, int32_t count, int32_t *found)
{
    switch (f->terrain_usage) {
        case TERRAIN_USAGE_ENEMY:
            *found = map_routing_noncitizen_find_first_of(f->x, f->y, grid_offsets, count, 0, ROUTE_ENEMY_MAX_TILES);
            if (*found) {
                return ROUTE_EIGHT_DIRECTIONS;
            }
            *found = map_routing_noncitizen_find_first_of_through_everything(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_WALLS:
            *found = map_routing_walls_find_first_of(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_FOUR_DIRECTIONS_FIRST);
        case TERRAIN_USAGE_ANIMAL:
            *found = map_routing_citizen_find_first_of_any(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_PREFER_ROADS:
            *found = map_routing_citizen_find_first_of_road_garden(f->x, f->y, grid_offsets, count);
            if (*found) {
                return ROUTE_FOUR_DIRECTIONS_FIRST;
            }
            *found = map_routing_citizen_find_first_of_prefer_roads(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_ROADS:
            *found = map_routing_citizen_find_first_of_road_garden(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_FOUR_DIRECTIONS_FIRST);
        case TERRAIN_USAGE_ANY:
            *found = map_routing_citizen_find_first_of_any(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_NO_FERRY:
            *found = map_routing_citizen_find_first_of_no_ferry(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_EIGHT_DIRECTIONS);
        case TERRAIN_USAGE_ANY_OR_FORT:
            *found = map_routing_citizen_find_first_of_fort(f->x, f->y, grid_offsets, count);
            return searched_with(*found, ROUTE_EIGHT_DIRECTIONS);
        default:
            *found = 0;
            return ROUTE_NOT_FOUND;
    }
}

int32_t figure_route_add_to_closest_of(int32_t figure_id, const int32_t *grid_offsets, int32_t count)
{
    figure *f = figure_get(figure_id);
    f->routing_path_id = 0;
    f->routing_path_current_tile = 0;
    f->routing_path_length = 0;
    int32_t path_id;
    if (!figure_route_get_first_available(figure_id, &path_id)) {
        return 0;
    }
    int32_t found;
    int32_t length;
    if (f->is_boat) {
        length = boat_path_length(path_id, f);
        found = length;
    } else {
        int32_t directions = route_directions_to_closest(f, grid_offsets, count, &found);
        if (directions == ROUTE_NOT_FOUND) {
            return 0;
        }
        int32_t offset = grid_offsets[found - 1] - map_info.grid_start;
        f->destination_x = (int16_t) (offset % GRID_SIZE);
        f->destination_y = (int16_t) (offset / GRID_SIZE);
        length = land_path_length(path_id, f, directions);
    }
    if (length == 0) {
        return found;
    }
    figure_route_remove(figure_id);
    set_path(f, figure_id, path_id, length);
    return found;
}
