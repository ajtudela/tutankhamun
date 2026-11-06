// The forts' ghost: the fort and its parade ground on open land without flooded floodplain or figures, drawn
// green with its flag, or red when the city has too many legions or is broke.

#include "widget/construction_ghost_fort.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/construction_place.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum fort_ghost {
    FORT_LEGIONS_MAX = 6,
    FORT_IMAGE_GROUND = 1,
    FORT_IMAGE_FLAG_ARCHERS = 2,
    FORT_IMAGE_FLAG_CHARIOTEERS = 3,
    FORT_IMAGE_FLAG_INFANTRY = 4,
    FORT_FLAG_DX = 4,
    FORT_FLAG_DY = -5,
};

static int32_t tile_dry(int32_t offset)
{
    const flood_tile *node = floods_data.tiles[offset];
    return node == 0 || node->is_floodplain != 0;
}

static int32_t tile_free(int32_t offset, int32_t check_figures)
{
    if (map_grids.terrain[offset] & TERRAIN_NOT_EMPTY) {
        return 0;
    }
    if (!tile_dry(offset)) {
        return 0;
    }
    return !check_figures || map_grids.figure_ids[offset] == 0;
}

static void draw_flag(int32_t px, int32_t py)
{
    int32_t type = construction_building_type;
    int32_t image;
    if (type == BUILDING_FORT_CHARIOTEERS) {
        image = image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_FLAG_CHARIOTEERS;
    } else if (type == BUILDING_FORT_INFANTRY) {
        image = image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_FLAG_INFANTRY;
    } else if (type == BUILDING_FORT_ARCHERS) {
        image = image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_FLAG_ARCHERS;
    } else {
        return;
    }
    ghost_image_draw(image, city_view.tile_half_width + px + FORT_FLAG_DX, city_view.tile_half_height + py + FORT_FLAG_DY);
}

static void ground_pixel(int32_t view, int32_t px, int32_t py_raised, int32_t half, int32_t *gx, int32_t *gy)
{
    *gx = (ghost_fort_ground_half_tiles[view].x * city_view.tile_half_width) + (ghost_fort_ground_tiles[view].x * city_view.tile_width) + ghost_fort_ground_pixels[view] +
        px;
    *gy = (ghost_fort_ground_tiles[view].y * city_view.tile_height) + py_raised + ((ghost_fort_ground_half_tiles[view].y - 1) * half);
}

void city_building_ghost_draw_fort(void)
{
    int32_t all_red = 0;
    int32_t blocked = 0;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t half = city_view.tile_half_height;
    int32_t py_raised = py + half;
    if (formation_num_legions >= FORT_LEGIONS_MAX) {
        all_red = 1;
        blocked = 1;
    }
    if (city_player_current()->treasury <= GHOST_TREASURY_BROKE) {
        all_red = 1;
        blocked = 1;
    }
    int32_t fort_tiles = ghost_tiles_of_size(building_property_get(BUILDING_FORT)->size);
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    for (int32_t i = 0; i < fort_tiles; i++) {
        if (!tile_free(ghost_view_tile_offset(view, i) + map_info.hover_offset, 1)) {
            blocked = 1;
        }
    }
    int32_t ground = ghost_fort_ground_grid_offsets[view] + map_info.hover_offset;
    int32_t ground_tiles = ghost_tiles_of_size(building_property_get(BUILDING_FORT_GROUND)->size);
    for (int32_t i = 0; i < ground_tiles; i++) {
        if (!tile_free(ghost_view_tile_offset(view, i) + ground, 1)) {
            blocked = 1;
        }
    }
    if (blocked) {
        int32_t hover = map_info.hover_offset;
        for (int32_t i = 0; i < fort_tiles; i++) {
            int32_t color = (tile_free(ghost_view_tile_offset(view, i) + hover, 0) && !all_red) ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
            ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) - city_view.tile_half_height + py_raised, color);
        }
        view = map_info.orientation / VIEW_ORIENTATION_STEP;
        ground = ghost_fort_ground_grid_offsets[view] + map_info.hover_offset;
        int32_t gx = (ghost_fort_ground_half_tiles[view].x * city_view.tile_half_width) + (ghost_fort_ground_tiles[view].x * city_view.tile_width) +
            ghost_fort_ground_pixels[view] + px;
        int32_t gy = (ghost_fort_ground_half_tiles[view].y * city_view.tile_half_height) + py_raised + (ghost_fort_ground_tiles[view].y * city_view.tile_height);
        ground_tiles = ghost_tiles_of_size(building_property_get(BUILDING_FORT_GROUND)->size);
        for (int32_t i = 0; i < ground_tiles; i++) {
            int32_t color = (tile_free(ghost_view_tile_offset(view, i) + ground, 1) && !all_red) ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
            ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + gx, gy - city_view.tile_half_height + ghost_flat_tile_dy(i), color);
        }
        return;
    }
    int32_t gx, gy;
    if (map_info.orientation != VIEW_ORIENTATION_NORTH && map_info.orientation != VIEW_ORIENTATION_WEST) {
        ground_pixel(view, px, py_raised, half, &gx, &gy);
        city_view.color_mask = GHOST_COLOR_FREE;
        ghost_image_draw(image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_GROUND, gx, gy);
        ghost_image_draw_top(image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_GROUND, gx, gy);
        int32_t fy = py_raised - city_view.tile_half_height;
        ghost_image_draw(image_group(IMAGE_GROUP_FORT), px, fy);
        ghost_image_draw_top(image_group(IMAGE_GROUP_FORT), px, fy);
        draw_flag(px, fy);
        city_view.color_mask = 0;
        return;
    }
    int32_t fy = py_raised - half;
    city_view.color_mask = GHOST_COLOR_FREE;
    ghost_image_draw(image_group(IMAGE_GROUP_FORT), px, fy);
    ghost_image_draw_top(image_group(IMAGE_GROUP_FORT), px, fy);
    draw_flag(px, fy);
    view = map_info.orientation / VIEW_ORIENTATION_STEP;
    ground_pixel(view, px, py_raised, city_view.tile_half_height, &gx, &gy);
    ghost_image_draw(image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_GROUND, gx, gy);
    ghost_image_draw_top(image_group(IMAGE_GROUP_FORT) + FORT_IMAGE_GROUND, gx, gy);
    city_view.color_mask = 0;
}
