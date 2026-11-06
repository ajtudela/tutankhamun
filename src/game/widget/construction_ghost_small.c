// Small construction ghosts: the view-relative direction, the 2x2 shore buildings (water lift, ferry landing)
// drawn at the water's orientation, the 3x3 shore statues and a one-tile ghost that always fits. A fitting ghost
// draws the building green, else its flat tiles red.

#include "widget/construction_ghost_small.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/construction_place.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/orientation.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/water.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include <stdint.h>

int32_t ghost_image_extra;

enum small_ghost {
    SHORE2_TILES = 4,
    SHORE3_TILES = 9,
    SHORE2_SIZE = 2,
    FERRY_LANDING_IMAGES = 12,
    SHORE3_ORIENTATIONS = 3,
    WATER_FITS = 0,
};

int32_t city_view_relative_direction(int32_t map_direction)
{
    int32_t direction = map_direction - map_info.orientation;
    if (direction < 0) {
        direction += DIRECTION_COUNT;
    }
    return direction / VIEW_ORIENTATION_STEP;
}

static void draw_red_tiles(int32_t count, int32_t px, int32_t py_raised)
{
    for (int32_t i = 0; i < count; i++) {
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) - city_view.tile_half_height + py_raised, GHOST_COLOR_BLOCKED);
    }
}

static void draw_green(int32_t image, int32_t px, int32_t py)
{
    city_view.color_mask = GHOST_COLOR_FREE;
    ghost_image_draw(image, px, py);
    ghost_image_draw_top(image, px, py);
    city_view.color_mask = 0;
}

static int32_t building_image(int32_t type)
{
    const building_property *property = building_property_get(type);
    return image_group(property->image_group) + property->image_offset;
}

void city_building_ghost_draw_shore2(int32_t unused, int32_t allow)
{
    (void) unused;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    py += city_view.tile_half_height;
    int32_t type = construction_building_type;
    int32_t image = building_image(type);
    int32_t result = map_water_determine_orientation_size2(map_info.hover_x, map_info.hover_y, 1, allow);
    if (city_player_current()->treasury > GHOST_TREASURY_BROKE && result == WATER_FITS) {
        if (image >= image_group(IMAGE_GROUP_WATER_LIFT_TURN_0) && image < image_group(IMAGE_GROUP_WATER_LIFT_TURN_0) + FERRY_LANDING_IMAGES) {
            water_lift_flooded = map_terrain_has_bits(map_info.hover_x, map_info.hover_y, SHORE2_SIZE, SHORE2_SIZE, TERRAIN_FLOODPLAIN);
            image = map_water_lift_image(map_water_orientation_relative, 0);
        } else {
            image += map_water_orientation_relative;
        }
        draw_green(image, px, py - city_view.tile_half_height);
        return;
    }
    draw_red_tiles(SHORE2_TILES, px, py);
}

void city_building_ghost_draw_shore3(int32_t group)
{
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    py += city_view.tile_half_height;
    int32_t result = map_water_determine_orientation_size3(map_info.hover_x, map_info.hover_y, 1);
    if (city_player_current()->treasury > GHOST_TREASURY_BROKE && result == WATER_FITS) {
        int32_t orientation = map_water_orientation_relative;
        if (orientation < 0 || orientation >= SHORE3_ORIENTATIONS) {
            orientation = SHORE3_ORIENTATIONS;
        }
        int32_t image = image_group(group) + orientation;
        draw_green(image, px, py - city_view.tile_half_height);
        return;
    }
    draw_red_tiles(SHORE3_TILES, px, py);
}

void city_building_ghost_draw_single(void)
{
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    draw_green(building_image(construction_building_type) + ghost_image_extra, px, py);
}
