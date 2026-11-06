// The sphinx's ghost: three 6 x 6 parts on open land away from the map edge, drawn green per view, or red when
// the city has a sphinx already or is broke.

#include "widget/construction_ghost_sphinx.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include <stdint.h>

sphinx_ghost_half_size_table sphinx_ghost_half_sizes;

enum sphinx_ghost {
    SPHINX_PART_SIZE = 6,
    SPHINX_PARTS = 3,
    SPHINX_SIDE_PARTS = 2,
    SPHINX_VIEWS = 4,
    SPHINX_MAP_EDGE = 3,
};

static int32_t part_index(int32_t part, int32_t view)
{
    return (part * SPHINX_VIEWS) + view;
}

static int32_t part_x(int32_t part, int32_t view, int32_t px)
{
    int32_t index = part_index(part, view);
    return (sphinx_ghost_half_sizes.part_half_tiles[index].x * city_view.tile_half_width) + (ghost_sphinx_part_tiles[index].x * city_view.tile_width) +
        ghost_sphinx_part_pixels[index] + px;
}

static int32_t part_y(int32_t part, int32_t view, int32_t py_raised, int32_t flat)
{
    int32_t index = part_index(part, view);
    int32_t half_tiles = sphinx_ghost_half_sizes.part_half_tiles[index].y;
    int32_t tiles = ghost_sphinx_part_tiles[index].y;
    if (flat) {
        return (half_tiles * city_view.tile_half_height) + py_raised + (tiles * city_view.tile_height);
    }
    return (tiles * city_view.tile_height) + py_raised + ((half_tiles - 1) * city_view.tile_half_height);
}

static int32_t tile_fits(int32_t offset)
{
    uint32_t terrain = map_grids.terrain[offset];
    return !map_grid_is_inside_size(offset % GRID_SIZE, offset / GRID_SIZE, SPHINX_MAP_EDGE) && (terrain & TERRAIN_NOT_CLEAR) == 0;
}

static void draw_green(int32_t image, int32_t x, int32_t y)
{
    ghost_image_draw(image, x, y);
    ghost_image_draw_top(image, x, y);
}

void city_building_ghost_draw_sphinx(void)
{
    int32_t all_red = 0;
    int32_t blocked = 0;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t py_raised = py + city_view.tile_half_height;
    const city_player *player = city_player_current();
    if (player->sphinx_stage != 0) {
        all_red = 1;
        blocked = 1;
    }
    if (player->treasury <= GHOST_TREASURY_BROKE) {
        all_red = 1;
        blocked = 1;
    }
    int32_t tiles = ghost_tiles_of_size(SPHINX_PART_SIZE);
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    int32_t origins[SPHINX_PARTS];
    origins[0] = map_info.hover_offset;
    origins[1] = ghost_sphinx_part_grid_offsets[part_index(0, view)] + map_info.hover_offset;
    origins[2] = ghost_sphinx_part_grid_offsets[part_index(1, view)] + map_info.hover_offset;
    for (int32_t p = 0; p < SPHINX_PARTS; p++) {
        for (int32_t i = 0; i < tiles; i++) {
            if (!tile_fits(ghost_view_tile_offset(view, i) + origins[p])) {
                blocked = 1;
            }
        }
    }
    if (!blocked) {
        city_view.color_mask = GHOST_COLOR_FREE;
        int32_t y0 = py_raised - city_view.tile_half_height;
        if (view == 0) {
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_A) + 1, px, y0);
            draw_green(image_group(IMAGE_GROUP_SPHINX_MIDDLE) + 1, part_x(0, 0, px), part_y(0, 0, py_raised, 0));
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_B) + 1, part_x(1, 0, px), part_y(1, 0, py_raised, 0));
        } else if (view == 2) {
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_B) + 1, px, y0);
            draw_green(image_group(IMAGE_GROUP_SPHINX_MIDDLE) + 1, part_x(0, 2, px), part_y(0, 2, py_raised, 0));
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_A) + 1, part_x(1, 2, px), part_y(1, 2, py_raised, 0));
        } else if (view == 3) {
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_A), px, y0);
            draw_green(image_group(IMAGE_GROUP_SPHINX_MIDDLE), part_x(0, 3, px) - 1, part_y(0, 3, py_raised, 0));
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_B), part_x(1, 3, px) - 2, part_y(1, 3, py_raised, 0));
        } else {
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_A), part_x(1, view, px), part_y(1, view, py_raised, 0));
            draw_green(image_group(IMAGE_GROUP_SPHINX_MIDDLE), part_x(0, view, px), part_y(0, view, py_raised, 0));
            draw_green(image_group(IMAGE_GROUP_SPHINX_END_B), px, py_raised - city_view.tile_half_height);
        }
        city_view.color_mask = 0;
        return;
    }
    for (int32_t i = 0; i < tiles; i++) {
        int32_t color = (tile_fits(ghost_view_tile_offset(view, i) + origins[0]) && !all_red) ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) - city_view.tile_half_height + py_raised, color);
    }
    for (int32_t p = 0; p < SPHINX_SIDE_PARTS; p++) {
        view = map_info.orientation / VIEW_ORIENTATION_STEP;
        int32_t origin = ghost_sphinx_part_grid_offsets[part_index(p, view)] + map_info.hover_offset;
        int32_t x = part_x(p, view, px);
        int32_t y = part_y(p, view, py_raised, 1);
        for (int32_t i = 0; i < tiles; i++) {
            int32_t color = (tile_fits(origin + ghost_view_tile_offset(view, i)) && !all_red) ? GHOST_COLOR_FREE : GHOST_COLOR_BLOCKED;
            ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + x, ghost_flat_tile_dy(i) - city_view.tile_half_height + y, color);
        }
    }
}
