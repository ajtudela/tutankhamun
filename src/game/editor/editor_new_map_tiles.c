// Scenario editor helpers of a new scenario: the map images inside the map's diamond, the dune images of a region,
// the action of the editor's flag figures (a flag with its point's marker), the empire editor's object tick and the
// climate change.
//
// Tiles and points are read loosely:
// - a dune tile not of the expected set keeps only the dune bit of its terrain;
// - only the low byte of each point coordinate is read, and a flag number past the invasion points reads other
//   scenario fields as a point.

#include "editor/editor_new_map_tiles.h"
#include "city/resource_type.h"
#include "empire/empire.h"
#include "figure/figure.h"
#include "figure/figure_core.h"
#include "graphics/image_collection.h"
#include "io/game_file_internal.h"
#include "map/grid.h"
#include "map/moisture.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"
#include "map/tiles_road.h"
#include "scenario/invasion.h"
#include "scenario/scenario_climate.h"
#include <stdint.h>

enum editor_flag_image_group {
    EDITOR_IMAGE_GROUP_HERD_MARKERS = 396,
    EDITOR_IMAGE_GROUP_FLAG = 408,
    EDITOR_IMAGE_GROUP_POINT_MARKERS = 409,
    EDITOR_IMAGE_GROUP_SEA_INVASION_MARKER = 348,
    EDITOR_IMAGE_GROUP_LANDING_MARKER = 446,
    EDITOR_IMAGE_GROUP_RESOURCE_ICONS = 713,
};

enum editor_point_marker {
    POINT_MARKER_EARTHQUAKE = 0,
    POINT_MARKER_LAND_INVASION = 1,
    POINT_MARKER_ENTRY = 2,
    POINT_MARKER_EXIT = 3,
    POINT_MARKER_RIVER_ENTRY = 4,
    POINT_MARKER_RIVER_EXIT = 5,
    HERD_MARKER_PREDATOR = 1,
    HERD_MARKER_PREY = 5,
};

enum editor_flag_values {
    EDITOR_FLAG_FRAMES = 8,
    FLAG_TILE_PIXELS = 15,
    FLAG_TILE_CENTER = 7,
    DUNE_IMAGE_VARIANTS = 8,
    MAP_IMAGE_INSIDE = 6,
};

void map_images_set_inside(void)
{
    int32_t height = map_info.grid_height;
    for (int32_t y = 0; y < height; y++) {
        int32_t x_start = map_row_x_start[y];
        int32_t x_end = map_row_x_end[y];
        if (x_start < x_end) {
            int32_t *image = &map_grids.images[map_row_start_offset[y]];
            for (int32_t n = x_end - x_start; n != 0; n--) {
                *image++ = MAP_IMAGE_INSIDE;
            }
        }
    }
}

void map_tiles_set_dune_image(int32_t x, int32_t y, int32_t set)
{
    if (!map_grid_is_inside(map_info.diamond_x_min + x, map_info.diamond_y_min + y)) {
        return;
    }
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    if (set != 1) {
        map_grids.terrain[offset] &= TERRAIN_DUNE;
        return;
    }
    int32_t image = (map_grids.random[offset] & (DUNE_IMAGE_VARIANTS - 1)) + image_group(GROUP_TERRAIN_DUNES);
    map_grids.terrain[offset] |= TERRAIN_DUNE;
    map_grids.images[offset] = image;
    map_grids.bitfields[offset] &= (uint8_t) ~MAP_BITFIELD_SIZE;
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
}

static int32_t dune_needs_image(uint32_t terrain)
{
    return (terrain & TERRAIN_DUNE) && !(terrain & (TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP));
}

void map_tiles_update_region_dunes(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 >= map_info.grid_width) {
        x1 = map_info.grid_width - 1;
    }
    if (y1 >= map_info.grid_height) {
        y1 = map_info.grid_height - 1;
    }
    int32_t row_step = map_info.grid_width - x1 + x0 - 1;
    int32_t offset = map_grid_offset(x0, y0);
    for (int32_t y = y0; y <= y1; y++) {
        if (x0 <= x1) {
            const uint32_t *terrain = &map_grids.terrain[offset];
            offset += x1 - x0 + 1;
            for (int32_t x = x0; x <= x1; x++, terrain++) {
                if (dune_needs_image(*terrain)) {
                    map_tiles_set_dune_image(x, y, 1);
                }
            }
        }
        offset += map_info.grid_border_size + row_step;
    }
}

static void place_flag(figure *f, int32_t x, int32_t y)
{
    f->x = (uint8_t) x;
    f->y = (uint8_t) y;
}

static int32_t flag_marker_image(figure *f)
{
    int32_t number = f->editor_flag_number;
    if (number == EDITOR_FLAG_EARTHQUAKE) {
        place_flag(f, scenario_data.earthquake_x, scenario_data.earthquake_y);
        return image_group(EDITOR_IMAGE_GROUP_POINT_MARKERS) + POINT_MARKER_EARTHQUAKE;
    }
    if (number == EDITOR_FLAG_ENTRY) {
        place_flag(f, scenario_data.entry_point_x, scenario_data.entry_point_y);
        return image_group(EDITOR_IMAGE_GROUP_POINT_MARKERS) + POINT_MARKER_ENTRY;
    }
    if (number == EDITOR_FLAG_EXIT) {
        place_flag(f, scenario_data.exit_point_x, scenario_data.exit_point_y);
        return image_group(EDITOR_IMAGE_GROUP_POINT_MARKERS) + POINT_MARKER_EXIT;
    }
    if (number == EDITOR_FLAG_RIVER_ENTRY) {
        place_flag(f, scenario_data.river_entry_x, scenario_data.river_entry_y);
        return image_group(EDITOR_IMAGE_GROUP_POINT_MARKERS) + POINT_MARKER_RIVER_ENTRY;
    }
    if (number == EDITOR_FLAG_RIVER_EXIT) {
        place_flag(f, scenario_data.river_exit_x, scenario_data.river_exit_y);
        return image_group(EDITOR_IMAGE_GROUP_POINT_MARKERS) + POINT_MARKER_RIVER_EXIT;
    }
    if (number >= EDITOR_FLAG_FISHING_FIRST && number <= EDITOR_FLAG_FISHING_LAST) {
        int32_t i = number - EDITOR_FLAG_FISHING_FIRST;
        place_flag(f, scenario_data.fishing_point_x[i], scenario_data.fishing_point_y[i]);
        return image_group(EDITOR_IMAGE_GROUP_RESOURCE_ICONS) + RESOURCE_FISH;
    }
    if (number >= EDITOR_FLAG_PREDATOR_FIRST && number <= EDITOR_FLAG_PREDATOR_LAST) {
        int32_t i = number - EDITOR_FLAG_PREDATOR_FIRST;
        place_flag(f, scenario_data.predator_herd_x[i], scenario_data.predator_herd_y[i]);
        return image_group(EDITOR_IMAGE_GROUP_HERD_MARKERS) + HERD_MARKER_PREDATOR;
    }
    if (number >= EDITOR_FLAG_PREY_FIRST && number <= EDITOR_FLAG_PREY_LAST) {
        int32_t i = number - EDITOR_FLAG_PREY_FIRST;
        place_flag(f, scenario_data.prey_herd_x[i], scenario_data.prey_herd_y[i]);
        return image_group(EDITOR_IMAGE_GROUP_HERD_MARKERS) + HERD_MARKER_PREY;
    }
    if (number >= EDITOR_FLAG_LANDING_FIRST && number <= EDITOR_FLAG_LANDING_LAST) {
        int32_t i = number - EDITOR_FLAG_LANDING_FIRST;
        place_flag(f, scenario_data.invasion_landing_x[i], scenario_data.invasion_landing_y[i]);
        return image_group(EDITOR_IMAGE_GROUP_LANDING_MARKER);
    }
    int32_t point = number - EDITOR_FLAG_INVASION_FIRST;
    place_flag(f, scenario_data.invasion_point_x[point], scenario_data.invasion_point_y[point]);
    if (point < SCENARIO_LAND_INVASION_POINTS) {
        return image_group(EDITOR_IMAGE_GROUP_POINT_MARKERS) + POINT_MARKER_LAND_INVASION;
    }
    return image_group(EDITOR_IMAGE_GROUP_SEA_INVASION_MARKER);
}

void figure_editor_flag_action(void)
{
    int32_t id = figure_current_id;
    figure *f = figure_get(id);
    uint8_t frame = (uint8_t) (f->animation_frame + 1);
    f->animation_frame = frame;
    if (frame >= EDITOR_FLAG_FRAMES) {
        f->animation_frame = 0;
    }
    f->image_id = f->animation_frame + image_group(EDITOR_IMAGE_GROUP_FLAG);
    map_figure_delete(id);
    f->cart_image_id = flag_marker_image(f);
    f->cross_country.x = (int16_t) ((f->x * FLAG_TILE_PIXELS) + FLAG_TILE_CENTER);
    f->cross_country.y = (int16_t) ((f->y * FLAG_TILE_PIXELS) + FLAG_TILE_CENTER);
    f->grid_offset = map_grid_offset(f->x, f->y);
    map_figure_add(id);
}

static uint8_t *empire_object_field(int32_t object, int32_t field)
{
    return (uint8_t *) &empire_map_objects[object] + field;
}

static int32_t empire_object_is_pharaoh_city(int32_t object)
{
    uint8_t city_type = *empire_object_field(object, EMPIRE_OBJECT_FIELD_CITY_TYPE);
    return *empire_object_field(object, EMPIRE_OBJECT_FIELD_KIND) == EMPIRE_OBJECT_KIND_CITY &&
        (city_type == EMPIRE_CITY_PHARAOH_TRADING || city_type == EMPIRE_CITY_PHARAOH);
}

void empire_editor_update_city_kinds(void)
{
    int32_t pharaoh_cities = 0;
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        if (*empire_object_field(i, EMPIRE_OBJECT_FIELD_IN_USE)) {
            uint8_t *countdown = empire_object_field(i, EMPIRE_OBJECT_FIELD_COUNTDOWN);
            if (*countdown) {
                (*countdown)--;
            }
            if (empire_object_is_pharaoh_city(i)) {
                pharaoh_cities++;
            }
        }
    }
    if (pharaoh_cities == 1) {
        return;
    }
    for (int32_t i = 0; i < EMPIRE_OBJECT_COUNT; i++) {
        uint8_t *city_type = empire_object_field(i, EMPIRE_OBJECT_FIELD_CITY_TYPE);
        if (*empire_object_field(i, EMPIRE_OBJECT_FIELD_IN_USE) && empire_object_is_pharaoh_city(i) &&
            *empire_object_field(i, EMPIRE_OBJECT_FIELD_COUNTDOWN) == 0) {
            *city_type = (uint8_t) ((*city_type == EMPIRE_CITY_PHARAOH_TRADING) ? EMPIRE_CITY_EGYPTIAN_TRADING : EMPIRE_CITY_EGYPTIAN);
        }
    }
}

void map_tiles_update_all_canals(void)
{
    map_tiles_update_region_canal_images(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
}

void editor_set_climate(int32_t climate)
{
    scenario_data.climate_type = (uint8_t) climate;
    map_pyramid_redraw();
    map_tiles_update_all();
}
