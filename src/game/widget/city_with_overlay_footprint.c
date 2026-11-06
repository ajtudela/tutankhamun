// The footprint pass of the city view under an overlay: the ground of every view tile around the viewport,
// the overlays' own tiles (desirability, fertility, goods, water, cliffs), map images of roads, canals and royal
// tombs, building tiles and walls.

#include "widget/city_with_overlay_footprint.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "figure/action.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "widget/city_building_tiles.h"
#include "widget/city_overlay_custom.h"
#include "widget/city_overlay_goods.h"
#include "widget/city_view.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include <stdint.h>

enum footprint_view_margin {
    FOOTPRINT_ROWS_ABOVE = 8,
    FOOTPRINT_EXTRA_ROWS = 14,
    FOOTPRINT_COLS_LEFT = 4,
    FOOTPRINT_EXTRA_COLS = 6,
};

static int32_t is_royal_tomb_type(int16_t type)
{
    return type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB ||
        type == BUILDING_ABU_SIMBEL;
}

static void draw_map_image(int32_t x, int32_t y, int32_t size)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    int32_t dx;
    int32_t dy;
    if (!city_view_footprint_offset(size, &dx, &dy)) {
        return;
    }
    image_draw(g_last_image_id, x + dx, y - dy);
}

static int32_t draw_own_overlay_tile(int32_t x, int32_t y)
{
    goods_overlay goods;
    if (city_overlay == OVERLAY_DESIRABILITY) {
        city_overlay_desirability(x, y, 0);
        return 1;
    }
    if (city_overlay == OVERLAY_FERTILITY) {
        city_overlay_fertility_footprint(x, y);
        return 1;
    }
    if (city_overlay_goods(city_overlay, &goods)) {
        city_overlay_resource(x, y, goods.resource, goods.inventory_slot);
        return 1;
    }
    return 0;
}

static void draw_terrain_tile(int32_t building_id, int32_t x, int32_t y)
{
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    int32_t size = map_grids.bitfields[city_view.grid_offset] & MAP_BITFIELD_SIZE;
    if ((terrain & TERRAIN_CANAL) != 0) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        if (size == 0) {
            image_draw(g_last_image_id, x, y);
        }
        return;
    }
    if ((terrain & TERRAIN_ROAD) != 0 && (terrain & TERRAIN_BUILDING) == 0) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        if (size <= 1) {
            draw_map_image(x, y, size);
        }
        return;
    }
    if ((terrain & TERRAIN_WALKABLE_BUILDING) != 0) {
        city_building_draw_tiles(building_id, x, y, 0);
        return;
    }
    if (city_overlay != OVERLAY_CITY_DEFENSES && (terrain & TERRAIN_WALL_KINDS) != 0) {
        g_last_image_id = image_group(IMAGE_GROUP_TERRAIN_OVERLAY);
        image_draw(g_last_image_id, x, y);
        return;
    }
    if ((terrain & TERRAIN_BUILDING) != 0) {
        city_building_draw_tiles(building_id, x, y, 0);
        return;
    }
    draw_map_image(x, y, size);
}

static void draw_footprint_tile(int32_t x, int32_t y)
{
    if (city_view.grid_offset >= VIEW_LOOKUP_EMPTY) {
        g_last_image_id = city_view.grid_offset - VIEW_LOOKUP_EMPTY;
        image_draw(g_last_image_id, x, y);
        return;
    }
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    int32_t is_tomb = is_royal_tomb_type(building_get(building_id)->type);
    if (!is_tomb && draw_own_overlay_tile(x, y)) {
        return;
    }
    if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) == 0) {
        return;
    }
    if (is_tomb) {
        draw_map_image(x, y, map_grids.bitfields[city_view.grid_offset] & MAP_BITFIELD_SIZE);
    } else if (city_overlay == OVERLAY_WATER) {
        city_overlay_water_footprint(x, y);
    } else if (city_overlay == OVERLAY_NATIVE) {
        return;
    } else if (city_overlay == OVERLAY_HIDE_CLIFFS || overlay_show_all != 0) {
        city_overlay_tombs_footprint(x, y);
    } else {
        draw_terrain_tile(building_id, x, y);
    }
}

void city_with_overlay_draw_footprint(void)
{
    int32_t pixel_y = city_view.origin_y - (city_view.tile_half_height * (FOOTPRINT_ROWS_ABOVE + 1));
    int32_t shifted_row = 0;
    city_view.row = city_view.camera_row - FOOTPRINT_ROWS_ABOVE;
    for (int32_t row = 0; row < city_view.height_tiles + FOOTPRINT_EXTRA_ROWS; row++, city_view.row++) {
        int32_t pixel_x = shifted_row ? city_view.origin_x - city_view.tile_half_width - 1 : city_view.origin_x;
        shifted_row = !shifted_row;
        pixel_x -= (city_view.tile_width + 2) * FOOTPRINT_COLS_LEFT;
        city_view.col = city_view.camera_col - FOOTPRINT_COLS_LEFT;
        for (int32_t col = 0; col <= city_view.width_tiles + FOOTPRINT_EXTRA_COLS; col++, city_view.col++) {
            if (city_view.col >= 0 && city_view.col < VIEW_COLS && city_view.row >= 0 && city_view.row < VIEW_ROWS) {
                city_view.grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
                if (city_view.grid_offset == construction_start_grid_offset) {
                    city_view.selected_pixel_x = pixel_y;
                    city_view.selected_pixel_y = pixel_x;
                }
                draw_footprint_tile(pixel_x, pixel_y);
            }
            pixel_x += city_view.tile_width + 2;
        }
        pixel_y += city_view.tile_half_height;
    }
}
