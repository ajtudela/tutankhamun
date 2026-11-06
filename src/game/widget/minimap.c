// The minimap: its bounds around the camera, its draw (one tile per two pixels; figures, overlay footprints,
// walls and terrain, black outside the map and under the fog), the tile under the mouse and a click that moves
// the view there.

#include "widget/minimap.h"
#include "building/building_data.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/graphics_display.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_blit.h"
#include "graphics/image_collection.h"
#include "input/input_data.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "scenario/scenario_climate.h"
#include "widget/city_view.h"
#include "widget/city_with_overlay.h"
#include "widget/minimap_figures.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum minimap_layout {
    MINIMAP_MARGIN = 4,
    MINIMAP_PIXELS_PER_TILE = 2,
    MINIMAP_EDITOR_BORDER_WIDTH = 4,
    MINIMAP_EDITOR_BORDER_HEIGHT = 6,
};

enum minimap_color {
    MINIMAP_VIEWPORT_COLOR = 0xE70B,
    MINIMAP_PREDATOR_COLOR = 0,
};

enum minimap_tile_image {
    MINIMAP_TILE_GARDEN = 50,
    MINIMAP_TILE_MONUMENT = 55,
    MINIMAP_TILE_BUILDING = 60,
    MINIMAP_TILE_WALL_COLUMN = 5,
    MINIMAP_TERRAIN_VARIANTS = 4,
    MINIMAP_EMPTY_LAND_VARIANTS = 8,
    MINIMAP_MARSH_VARIANT_BIT = 4,
};

static const uint32_t figure_color_pair_rgb565 = 0xF000F000u;
static const uint32_t figure_color_pair_rgb555 = 0x78007800u;

static int32_t enemy_color(void)
{
    if (display_mode == DISPLAY_MODE_RGB565) {
        switch (scenario_data.climate_type) {
            case CLIMATE_CENTRAL:
                return 0x181F;
            case CLIMATE_NORTHERN:
                return 0x7800;
            default:
                return 0x080F;
        }
    }
    switch (scenario_data.climate_type) {
        case CLIMATE_CENTRAL:
            return 0x3C00;
        case CLIMATE_NORTHERN:
            return 0x0C1F;
        default:
            return 0x041F;
    }
}

static int32_t clamp_start(int32_t start, int32_t min, int32_t max)
{
    if (start < min) {
        return min;
    }
    if (start > max) {
        return max;
    }
    return start;
}

void widget_minimap_set_bounds(int32_t width, int32_t height)
{
    city_view.minimap_abs_x = (VIEW_COLS - width) / 2;
    if ((city_view.x_max - city_view.x_min) - width + 1 > 0) {
        int32_t start = (city_view.width_tiles / 2) - (width / 2) + city_view.camera_col;
        city_view.minimap_abs_x = clamp_start(start, city_view.x_min, city_view.x_max - width);
    }
    int32_t start_row = (VIEW_ROWS - height) / 2;
    if ((city_view.y_max - city_view.y_min) - height + 1 > 0) {
        start_row = clamp_start((city_view.height_tiles / 2) - (height / 2) + city_view.camera_row, city_view.y_min, city_view.y_max - height);
    }
    city_view.minimap_abs_y = start_row & ~1;
}

static void draw_viewport_rectangle(int32_t x_offset, int32_t y_offset)
{
    int32_t x = x_offset + ((city_view.camera_col - city_view.minimap_abs_x) * MINIMAP_PIXELS_PER_TILE);
    int32_t y = (city_view.camera_row - city_view.minimap_abs_y) + y_offset;
    int32_t width = city_view.width_tiles * MINIMAP_PIXELS_PER_TILE;
    int32_t height = city_view.height_tiles;
    if (editor_wide_bounds != 0 && editor_active == 1) {
        width -= MINIMAP_EDITOR_BORDER_WIDTH;
        height -= MINIMAP_EDITOR_BORDER_HEIGHT;
    }
    if (x < x_offset) {
        x = x_offset;
    }
    graphics_draw_rect(x, y, width, height, MINIMAP_VIEWPORT_COLOR);
}

static int32_t terrain_image_id(uint32_t terrain, int32_t random)
{
    int32_t variant = random & (MINIMAP_TERRAIN_VARIANTS - 1);
    if ((terrain & (TERRAIN_WATER | TERRAIN_ROAD | TERRAIN_WALKABLE_BUILDING)) != 0 && map_grids.sprite[city_view.grid_offset] != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_ROAD);
    }
    if ((terrain & TERRAIN_MONUMENT) != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_BUILDINGS) + MINIMAP_TILE_MONUMENT;
    }
    if ((terrain & TERRAIN_GARDEN) != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_BUILDINGS) + MINIMAP_TILE_GARDEN;
    }
    if ((terrain & TERRAIN_WATER) != 0) {
        return variant + image_group(IMAGE_GROUP_MINIMAP_WATER);
    }
    if ((terrain & (TERRAIN_SHRUB | TERRAIN_TREE)) != 0) {
        return variant + image_group(IMAGE_GROUP_MINIMAP_TREES);
    }
    if ((terrain & TERRAIN_ROCK) != 0) {
        return variant + image_group(IMAGE_GROUP_MINIMAP_ROCK);
    }
    if ((terrain & TERRAIN_DUNE) != 0) {
        return variant + image_group(IMAGE_GROUP_MINIMAP_DUNE);
    }
    if ((terrain & TERRAIN_ELEVATION) != 0) {
        return variant + image_group(IMAGE_GROUP_MINIMAP_ROCK);
    }
    if ((terrain & (TERRAIN_ROAD | TERRAIN_WALKABLE_BUILDING)) != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_ROAD);
    }
    if ((terrain & TERRAIN_CANAL) != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_CANAL);
    }
    if ((terrain & TERRAIN_WALL) != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_BUILDINGS) + MINIMAP_TILE_BUILDING;
    }
    if ((terrain & (TERRAIN_MEADOW | TERRAIN_FLOODPLAIN)) != 0) {
        return variant + image_group(IMAGE_GROUP_MINIMAP_MEADOW);
    }
    if ((terrain & TERRAIN_MARSH) != 0) {
        return (random & MINIMAP_MARSH_VARIANT_BIT) + image_group(IMAGE_GROUP_MINIMAP_TREES);
    }
    return (random & (MINIMAP_EMPTY_LAND_VARIANTS - 1)) + image_group(IMAGE_GROUP_MINIMAP_EMPTY_LAND);
}

static int32_t is_visible(void)
{
    return g_fog_disabled != 0 ||
        (city_view.col >= g_map_visible_x0 && city_view.row >= g_map_visible_y0 && city_view.col <= g_map_visible_x1 && city_view.row <= ((g_map_visible_y1 - 1) & ~1));
}

static int32_t figure_color(int32_t figure_id, uint32_t figure_color_pair, int32_t enemy_color_pair, int32_t *color)
{
    int32_t military = minimap_figure_military_kind(figure_id);
    int32_t ship = minimap_figure_ship_kind(figure_id);
    *color = (int32_t) figure_color_pair;
    if (military == MINIMAP_MILITARY_ENEMY) {
        *color = enemy_color_pair;
    } else if (military == MINIMAP_MILITARY_PREDATOR) {
        *color = MINIMAP_PREDATOR_COLOR;
    } else if (military == MINIMAP_MILITARY_NONE && ship == MINIMAP_SHIP_ENEMY) {
        *color = enemy_color_pair;
    }
    return minimap_figure_show(figure_id) != 0 || ship != MINIMAP_SHIP_NONE || military != MINIMAP_MILITARY_NONE;
}

static void draw_building_tile(int32_t building_id, uint32_t terrain, int32_t x, int32_t y)
{
    if (city_with_overlay_draw_building_footprint(building_id, x, y) != 0) {
        return;
    }
    image_draw_minimap_tile(image_group(IMAGE_GROUP_MINIMAP_BUILDINGS) + ((terrain & TERRAIN_MONUMENT) != 0 ? MINIMAP_TILE_MONUMENT : MINIMAP_TILE_BUILDING), x, y);
}

static void draw_map_tile(int32_t x, int32_t y, int32_t outside, uint32_t figure_color_pair, int32_t enemy_color_pair)
{
    int32_t grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
    city_view.grid_offset = grid_offset;
    if (grid_offset >= VIEW_LOOKUP_EMPTY || (map_grids.terrain[grid_offset] & TERRAIN_DIKE) != 0 || !is_visible()) {
        if (!outside) {
            image_draw_minimap_tile(image_group(IMAGE_GROUP_MINIMAP_BLACK), x, y);
        }
        return;
    }
    uint32_t terrain = map_grids.terrain[grid_offset];
    int32_t building_id = map_grids.building_ids[grid_offset];
    int32_t color;
    if (figure_color(map_grids.figure_ids[grid_offset], figure_color_pair, enemy_color_pair, &color)) {
        g_draw_y = y;
        g_draw_x = x;
        if (!outside) {
            graphics_set_pixel_pair((uint32_t) color);
        }
    } else if ((terrain & TERRAIN_BUILDING) != 0 && building_id != 0) {
        if ((map_grids.edge[grid_offset] & MAP_EDGE_DRAW_TILE) != 0 && !outside) {
            draw_building_tile(building_id, terrain, x, y);
        }
    } else if ((terrain & TERRAIN_WALL_KINDS) != 0 && city_overlay == OVERLAY_CITY_DEFENSES) {
        image_draw_minimap_tile(image_group(IMAGE_GROUP_COLUMN) + MINIMAP_TILE_WALL_COLUMN, x, y);
    } else if (!outside) {
        image_draw_minimap_tile(terrain_image_id(terrain, map_grids.random[grid_offset]), x, y);
    }
}

static int32_t in_view_lookup(void)
{
    return city_view.col >= 0 && city_view.col < VIEW_COLS && city_view.row >= 0 && city_view.row < VIEW_ROWS;
}

void widget_minimap_draw(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height)
{
    uint32_t figure_color_pair = (display_mode == DISPLAY_MODE_RGB565) ? figure_color_pair_rgb565 : figure_color_pair_rgb555;
    int32_t enemy = enemy_color();
    int32_t enemy_color_pair = (enemy << 16) + enemy;

    g_clip_x0 = x_offset;
    g_clip_x1 = x_offset + (width * MINIMAP_PIXELS_PER_TILE);
    g_clip_y0 = y_offset;
    g_clip_y1 = y_offset + height;

    widget_minimap_set_bounds(width, height);

    city_view.minimap_y1 = y_offset + height;
    city_view.minimap_x1 = x_offset + (width * MINIMAP_PIXELS_PER_TILE);
    city_view.minimap_x0 = x_offset;
    city_view.minimap_y0 = y_offset;

    int32_t shifted_row = 0;
    int32_t y = y_offset - MINIMAP_MARGIN;
    city_view.row = city_view.minimap_abs_y - MINIMAP_MARGIN;
    for (int32_t row = -MINIMAP_MARGIN; row < height + MINIMAP_MARGIN; row++, y++, city_view.row++) {
        int32_t x = x_offset - (MINIMAP_MARGIN * MINIMAP_PIXELS_PER_TILE) - (shifted_row ? 1 : 0);
        shifted_row = !shifted_row;
        city_view.col = city_view.minimap_abs_x - MINIMAP_MARGIN;
        for (int32_t col = -MINIMAP_MARGIN; col < width; col++, x += MINIMAP_PIXELS_PER_TILE, city_view.col++) {
            if (in_view_lookup()) {
                int32_t outside = row < 0 || row >= height || col < 0 || col >= width;
                draw_map_tile(x, y, outside, figure_color_pair, enemy_color_pair);
            }
        }
    }

    draw_viewport_rectangle(x_offset, y_offset);

    g_clip_x1 = g_screen_width;
    g_clip_x0 = 0;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}

int32_t widget_minimap_get_mouse_grid_offset(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height)
{
    int32_t shifted_row = 0;
    widget_minimap_set_bounds(width, height);
    city_view.minimap_y0 = y_offset;
    city_view.minimap_x0 = x_offset;
    city_view.row = city_view.minimap_abs_y - MINIMAP_MARGIN;
    int32_t y = y_offset - MINIMAP_MARGIN;
    city_view.minimap_x1 = x_offset + (width * MINIMAP_PIXELS_PER_TILE);
    city_view.minimap_y1 = y_offset + height;
    for (int32_t row = -MINIMAP_MARGIN; row < height + MINIMAP_MARGIN; row++, y++, city_view.row++) {
        int32_t x = x_offset - (MINIMAP_MARGIN * MINIMAP_PIXELS_PER_TILE) - (shifted_row ? 1 : 0);
        shifted_row = !shifted_row;
        city_view.col = city_view.minimap_abs_x - MINIMAP_MARGIN;
        for (int32_t col = -MINIMAP_MARGIN; col < width; col++, x += MINIMAP_PIXELS_PER_TILE, city_view.col++) {
            if (in_view_lookup() && mouse_y == y && (mouse_x == x || mouse_x == x + 1)) {
                int32_t grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
                city_view.grid_offset = grid_offset;
                return (grid_offset >= VIEW_LOOKUP_EMPTY) ? 0 : grid_offset;
            }
        }
    }
    return 0;
}

int32_t minimap_sidebar_click(void)
{
    if (minimap_hovered != 1 || (!mouse_left_went_down && !mouse_right_went_down)) {
        return 0;
    }
    int32_t y = editor_active ? MINIMAP_EDITOR_CLICK_Y : MINIMAP_SIDEBAR_Y;
    int32_t offset = widget_minimap_get_mouse_grid_offset(city_view_width_expanded + MINIMAP_SIDEBAR_CLICK_X, y, MINIMAP_WIDTH, MINIMAP_HEIGHT);
    if (offset < 1) {
        return 0;
    }
    city_view_go_to_grid_offset((uint32_t) offset);
    return 1;
}
