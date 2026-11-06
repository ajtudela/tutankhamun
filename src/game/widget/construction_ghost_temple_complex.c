// The temple complex ghost: a 13 x 7 site laid out by the complex's turn and the view (R turns it while the
// city has no complex). A fitting site loads its god's script and draws the decoration tiles in two passes
// around the temple, oracle and altar; otherwise the blocked tiles are red.

#include "widget/construction_ghost_temple_complex.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include "window/hotkeys.h"
#include <stdint.h>

enum temple_complex_ghost {
    COMPLEX_TILE_RECORD_INTS = 5,
    COMPLEX_TILE_GROUP = 0,
    COMPLEX_TILE_NONE = 0,
    COMPLEX_TURNS = 4,
    COMPLEX_TURNED_IMAGES = 3,
    COMPLEX_ORIGINS_PER_VIEW = 4,
    COMPLEX_VIEW_SHIFT = -2,
    COMPLEX_SCRIPT_SLOT = 10,
};

static int32_t turn_in_view(int32_t turn, int32_t view)
{
    return (turn - (view / VIEW_ORIENTATION_STEP)) & (COMPLEX_TURNS - 1);
}

static void site_tile(int32_t view, int32_t turn, int32_t x0, int32_t y0, int32_t row, int32_t column, int32_t *x, int32_t *y)
{
    switch (turn_in_view(turn, view)) {
        case 0:
            *x = x0 + column;
            *y = y0 + row;
            break;
        case 1:
            *x = x0 - row;
            *y = y0 - column;
            break;
        case 2:
            *x = x0 - column;
            *y = y0 - row;
            break;
        default:
            *x = x0 + row;
            *y = y0 + column;
            break;
    }
}

static const int32_t *complex_tile(int32_t row, int32_t column)
{
    return &temple_complex_tiles[(row * TEMPLE_COMPLEX_SITE_COLUMNS + column) * COMPLEX_TILE_RECORD_INTS];
}

static int32_t columns_reversed(int32_t turn)
{
    return turn == 1 || turn == 2;
}

static void draw_green(int32_t image, int32_t x, int32_t y)
{
    ghost_image_draw(image, x, y);
    ghost_image_draw_top(image, x, y);
}

static void draw_rows(int32_t view, int32_t turn, int32_t image_field, int32_t from, int32_t to, int32_t x0, int32_t y0)
{
    int32_t step = (from <= to) ? 1 : -1;
    for (int32_t row = from;; row += step) {
        for (int32_t i = 0; i < TEMPLE_COMPLEX_SITE_COLUMNS; i++) {
            int32_t column = columns_reversed(turn) ? TEMPLE_COMPLEX_SITE_COLUMNS - 1 - i : i;
            const int32_t *tile = complex_tile(row, column);
            if (tile[COMPLEX_TILE_GROUP] == COMPLEX_TILE_NONE) {
                continue;
            }
            int32_t x, y;
            site_tile(view, turn, x0, y0, row, column, &x, &y);
            int32_t px, py;
            city_view_tile_to_pixel(x, y, &px, &py);
            draw_green(image_group(tile[COMPLEX_TILE_GROUP]) + tile[image_field], px, py);
        }
        if (row == to) {
            break;
        }
    }
}

static void draw_decoration(int32_t second, int32_t turn, int32_t x0, int32_t y0)
{
    int32_t view = map_info.orientation;
    const temple_complex_ghost_pass *pass = &temple_complex_ghost_passes[view / VIEW_ORIENTATION_STEP];
    if (second) {
        draw_rows(view, turn, pass->image_field[turn], pass->second_rows_from[turn], pass->second_rows_to[turn], x0, y0);
    } else {
        draw_rows(view, turn, pass->image_field[turn], pass->first_rows_from[turn], pass->first_rows_to[turn], x0, y0);
    }
}

static void draw_main_buildings(int32_t turn, int32_t px, int32_t py_raised)
{
    int32_t turned = (turn == 1 || turn == 3) ? COMPLEX_TURNED_IMAGES : 0;
    int32_t swapped = turn == 1 || turn == 2;
    int32_t own_x = px;
    int32_t own_y = py_raised - city_view.tile_half_height;
    int32_t oracle_x = (gatehouse_parts.oracle_tiles[turn].x * city_view.tile_width) + gatehouse_parts.oracle_pixels[turn] + city_view.tile_half_width + px;
    int32_t oracle_y = (gatehouse_parts.oracle_tiles[turn].y * city_view.tile_height) + py_raised;
    int32_t altar_x = (gatehouse_parts.altar_tiles[turn].x * city_view.tile_width) + gatehouse_parts.altar_pixels[turn] + px;
    int32_t altar_y = (gatehouse_parts.altar_tiles[turn].y * city_view.tile_height) - city_view.tile_half_height + py_raised;
    draw_green(image_group(IMAGE_GROUP_TEMPLE_COMPLEX) + turned, swapped ? altar_x : own_x, swapped ? altar_y : own_y);
    draw_green(image_group(IMAGE_GROUP_TEMPLE_COMPLEX_ORACLE) + turned, oracle_x, oracle_y);
    draw_green(image_group(IMAGE_GROUP_TEMPLE_COMPLEX_ALTAR) + turned, swapped ? own_x : altar_x, swapped ? own_y : altar_y);
}

static int32_t turn_allowed(int32_t turn, int32_t view_turn)
{
    return turn == view_turn || turn == ((view_turn + COMPLEX_TURNS - 1) % COMPLEX_TURNS);
}

static int32_t choose_turn(int32_t view)
{
    int32_t view_turn = view / VIEW_ORIENTATION_STEP;
    if (city_player_current()->has_temple_complex != 0) {
        return view_turn;
    }
    if (ghost_rotate_key_went_down()) {
        construction_orientation = (construction_orientation == view_turn) ? ((view_turn + COMPLEX_TURNS - 1) % COMPLEX_TURNS) : view_turn;
    }
    if (!turn_allowed(construction_orientation, view_turn)) {
        construction_orientation = view_turn;
    }
    return construction_orientation;
}

static void view_shift(int32_t view, int32_t *dx, int32_t *dy)
{
    *dx = (view == VIEW_ORIENTATION_EAST || view == VIEW_ORIENTATION_SOUTH) ? COMPLEX_VIEW_SHIFT : 0;
    *dy = (view == VIEW_ORIENTATION_SOUTH || view == VIEW_ORIENTATION_WEST) ? COMPLEX_VIEW_SHIFT : 0;
}

static int32_t site_blocked(int32_t view, int32_t turn, int32_t x0, int32_t y0)
{
    int32_t blocked = 0;
    for (int32_t row = 0; row < TEMPLE_COMPLEX_SITE_ROWS; row++) {
        for (int32_t column = 0; column < TEMPLE_COMPLEX_SITE_COLUMNS; column++) {
            int32_t x, y;
            site_tile(view, turn, x0, y0, row, column, &x, &y);
            if (map_grids.terrain[map_info.grid_start + (y * GRID_SIZE) + x] & TERRAIN_NOT_EMPTY) {
                blocked = 1;
            }
        }
    }
    return blocked;
}

static void draw_site_tiles(int32_t view, int32_t turn, int32_t x0, int32_t y0, int32_t all_red)
{
    for (int32_t row = 0; row < TEMPLE_COMPLEX_SITE_ROWS; row++) {
        for (int32_t column = 0; column < TEMPLE_COMPLEX_SITE_COLUMNS; column++) {
            int32_t x, y;
            site_tile(view, turn, x0, y0, row, column, &x, &y);
            int32_t px, py;
            city_view_tile_to_pixel(x, y, &px, &py);
            int32_t color;
            if (map_grids.terrain[map_info.grid_start + (y * GRID_SIZE) + x] & TERRAIN_NOT_EMPTY) {
                color = GHOST_COLOR_BLOCKED;
            } else {
                color = all_red ? GHOST_COLOR_BLOCKED : GHOST_COLOR_FREE;
            }
            ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px, py, color);
        }
    }
}

static int32_t last_turn_at_most(int32_t orientation)
{
    return (orientation < COMPLEX_TURNS) ? orientation : COMPLEX_TURNS - 1;
}

static const char *turn_tag(int32_t turn)
{
    return (turn == 0) ? "complex turn 0" : (turn == 1) ? "complex turn 1" : (turn == 2) ? "complex turn 2" : "complex turn 3";
}

void city_building_ghost_draw_temple_complex(int32_t type)
{
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t py_raised = py + city_view.tile_half_height;
    int32_t view = map_info.orientation;
    int32_t turn = choose_turn(view);
    int32_t origin = construction_orientation + ((view / VIEW_ORIENTATION_STEP) * COMPLEX_ORIGINS_PER_VIEW);
    int32_t x0 = temple_complex_origin[origin * 2] + map_info.hover_x;
    int32_t y0 = temple_complex_origin[(origin * 2) + 1] + map_info.hover_y;
    int32_t all_red = city_player_current()->has_temple_complex != 0;
    int32_t blocked = !all_red && site_blocked(view, construction_orientation, x0, y0);
    if (!all_red && !blocked) {
        image_collection_load_file(temple_complex_god_script_names[building_type_to_god(type)], COMPLEX_SCRIPT_SLOT);
        int32_t dx, dy;
        view_shift(view, &dx, &dy);
        city_view.color_mask = GHOST_COLOR_FREE;
        draw_decoration(0, turn, x0 + dx, y0 + dy);
        draw_main_buildings(turn, px, py_raised);
        draw_decoration(1, turn, x0 + dx, y0 + dy);
        city_view.color_mask = 0;
        return;
    }
    draw_site_tiles(view, last_turn_at_most(construction_orientation), x0, y0, all_red);
}
