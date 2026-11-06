// The scenario editor's terrain brush on one tile and the floodplain fixes around it. Each setter replaces the
// tile's ground with its terrain (keeping what is built or flagged there) and reports whether the tile changed; the
// fixes spread, remove or take over floodplain in the 3x3 square around a changed tile, recursing from each tile they
// change.
//
// Two details of the fixes matter for the maps the editor writes:
// - the squares are clipped at the grid's width and height, not width - 1 and height - 1;
// - the floodplain setter never reports a tile as unchanged.

#include "editor/editor_terrain_tiles.h"
#include "editor/editor_internal.h"
#include "editor/editor_new_map_tiles.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include <stdbool.h>
#include <stdint.h>

int32_t editor_brush_tile_changed;

int32_t editor_brush_tile_unchanged;

enum grid_neighbour_delta {
    DELTA_TOP = -GRID_SIZE,
    DELTA_TOP_RIGHT = 1 - GRID_SIZE,
    DELTA_RIGHT = 1,
    DELTA_BOTTOM_RIGHT = GRID_SIZE + 1,
    DELTA_BOTTOM = GRID_SIZE,
    DELTA_BOTTOM_LEFT = GRID_SIZE - 1,
    DELTA_LEFT = -1,
    DELTA_TOP_LEFT = -GRID_SIZE - 1,
};

typedef struct {
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
} tile_square;

static tile_square square_around(int32_t x, int32_t y, int32_t radius)
{
    tile_square square;
    square.x0 = (x - radius < 0) ? 0 : x - radius;
    square.x1 = (map_info.grid_width < x + radius) ? map_info.grid_width : x + radius;
    square.y0 = (y - radius < 0) ? 0 : y - radius;
    square.y1 = (map_info.grid_height < y + radius) ? map_info.grid_height : y + radius;
    return square;
}

static int32_t square_row_skip(const tile_square *square)
{
    return map_info.grid_border_size + map_info.grid_width - (square->x1 - square->x0 + 1);
}

int32_t editor_tile_clear(int32_t x, int32_t y)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain == TERRAIN_NONE) {
        editor_brush_tile_unchanged = 1;
        return offset;
    }
    map_grids.canals[offset] = 0;
    map_grids.terrain[offset] = terrain & EDITOR_BRUSH_KEPT_TERRAIN;
    map_grids.building_ids[offset] = 0;
    map_grids.bitfields[offset] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
    map_grids.building_damage[offset] = 0;
    map_grids.sprite[offset] = 0;
    map_grids.edge[offset] = (uint8_t) ((map_grids.edge[offset] & ~(MAP_EDGE_COLUMN | MAP_EDGE_ROW)) | MAP_EDGE_DRAW_TILE);
    editor_brush_tile_changed = 1;
    return offset;
}

static int32_t tile_set_terrain(int32_t x, int32_t y, uint32_t terrain_bit, uint32_t kept)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & terrain_bit) {
        editor_brush_tile_unchanged = 1;
        return offset;
    }
    editor_brush_tile_changed = 1;
    map_grids.terrain[offset] = (terrain & kept) | terrain_bit;
    return offset;
}

int32_t editor_tile_set_tree(int32_t x, int32_t y)
{
    return tile_set_terrain(x, y, TERRAIN_TREE, EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_TREE);
}

int32_t editor_tile_set_rock_kind(int32_t x, int32_t y, uint32_t kind)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    uint32_t terrain = map_grids.terrain[offset];
    if ((terrain & TERRAIN_ROCKS) == kind) {
        editor_brush_tile_unchanged = 1;
        return offset;
    }
    editor_brush_tile_changed = 1;
    map_grids.terrain[offset] = ((terrain & ~TERRAIN_ROCKS) | kind) & (EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_ROCKS);
    return offset;
}

int32_t editor_tile_set_dune(int32_t x, int32_t y)
{
    return tile_set_terrain(x, y, TERRAIN_DUNE, EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_FLOODPLAIN | TERRAIN_DUNE);
}

int32_t editor_tile_set_meadow(int32_t x, int32_t y)
{
    return tile_set_terrain(x, y, TERRAIN_MEADOW, EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_MEADOW);
}

int32_t editor_tile_set_marsh(int32_t x, int32_t y)
{
    return tile_set_terrain(x, y, TERRAIN_MARSH, EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_MARSH);
}

int32_t editor_tile_set_floodplain(int32_t x, int32_t y)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    if (map_grids.elevation[offset] == 0) {
        editor_brush_tile_changed = 1;
        map_grids.terrain[offset] = (map_grids.terrain[offset] & (EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_FLOODPLAIN)) | TERRAIN_FLOODPLAIN;
    }
    return offset;
}

int32_t editor_tile_set_water(int32_t x, int32_t y)
{
    int32_t offset = map_grid_offset(x, y);
    map_tile_cursor_offset = offset;
    if (map_grids.elevation[offset] != 0) {
        return offset;
    }
    uint32_t terrain = map_grids.terrain[offset];
    if (terrain & TERRAIN_WATER) {
        editor_brush_tile_unchanged = 1;
        return offset;
    }
    editor_brush_tile_changed = 1;
    map_grids.terrain[offset] = (terrain & (EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_DEEPWATER | TERRAIN_CANAL | TERRAIN_WATER)) | TERRAIN_WATER;
    return offset;
}

int32_t editor_tile_set_road(int32_t x, int32_t y)
{
    return tile_set_terrain(x, y, TERRAIN_ROAD, EDITOR_BRUSH_KEPT_TERRAIN | EDITOR_ROAD_OR_CANAL_KEPT_TERRAIN);
}

int32_t editor_tile_set_canal(int32_t x, int32_t y)
{
    return tile_set_terrain(x, y, TERRAIN_CANAL, EDITOR_BRUSH_KEPT_TERRAIN | EDITOR_ROAD_OR_CANAL_KEPT_TERRAIN);
}

int32_t editor_tile_deep_water_edge(int32_t x, int32_t y)
{
    int32_t offset = map_grid_offset(x, y);
    uint32_t terrain = map_grids.terrain[offset];
    if (!(terrain & TERRAIN_WATER) || !(terrain & TERRAIN_DEEPWATER)) {
        return 0;
    }
    return map_terrain_count_neighbours_with(TERRAIN_DEEPWATER, offset) < DIRECTION_COUNT;
}

void editor_update_tiles_around(int32_t x, int32_t y, int32_t radius)
{
    tile_square square = square_around(x, y, radius);
    int32_t x0 = square.x0;
    int32_t y0 = square.y0;
    int32_t x1 = square.x1;
    int32_t y1 = square.y1;
    map_tiles_update_region_water(x0, y0, x1, y1, 1);
    map_tiles_update_region_marshland(x0, y0, x1, y1);
    map_tiles_update_region_faults(x0, y0, x1, y1);
    map_tiles_update_all_rocks();
    map_tiles_update_region_dunes(x0, y0, x1, y1);
    map_tiles_update_region_empty_land(x0, y0, x1, y1, 0, 1);
    map_tiles_update_region_floodplain(x0, y0, x1, y1);
    map_tiles_update_region_roads(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    map_tiles_update_region_canal_images(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
    map_tiles_update_region_meadow(x0, y0, x1, y1);
}

static int32_t corner_lacks(int32_t offset, uint32_t mask, int32_t diagonal, int32_t side_a, int32_t side_b)
{
    return !(mask & (map_grids.terrain[offset + diagonal] | map_grids.terrain[offset + side_a] | map_grids.terrain[offset + side_b]));
}

static int32_t diagonal_has(int32_t offset, uint32_t mask, int32_t diagonal)
{
    return (mask & (map_grids.terrain[offset + diagonal] & map_grids.terrain[offset - diagonal])) != 0;
}

bool editor_terrain_corner_open(int32_t offset, uint32_t mask, bool check_diagonals)
{
    if (check_diagonals) {
        if (diagonal_has(offset, mask, DELTA_TOP_LEFT) || diagonal_has(offset, mask, DELTA_TOP_RIGHT)) {
            return false;
        }
    }
    return corner_lacks(offset, mask, DELTA_TOP_LEFT, DELTA_LEFT, DELTA_TOP) || corner_lacks(offset, mask, DELTA_TOP_RIGHT, DELTA_RIGHT, DELTA_TOP) ||
        corner_lacks(offset, mask, DELTA_BOTTOM_LEFT, DELTA_BOTTOM, DELTA_LEFT) || corner_lacks(offset, mask, DELTA_BOTTOM_RIGHT, DELTA_BOTTOM, DELTA_RIGHT);
}

bool editor_floodplain_spread(int32_t x, int32_t y)
{
    bool changed = false;
    tile_square square = square_around(x, y, 1);
    int32_t row_skip = square_row_skip(&square);
    int32_t offset = map_grid_offset(square.x0, square.y0);
    for (int32_t yy = square.y0; yy <= square.y1; yy++) {
        for (int32_t xx = square.x0; xx <= square.x1; xx++, offset++) {
            uint32_t *terrain = &map_grids.terrain[offset];
            if (*terrain & (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) {
                continue;
            }
            if (editor_terrain_corner_open(offset, TERRAIN_FLOODPLAIN | TERRAIN_WATER, 1)) {
                continue;
            }
            *terrain = (*terrain & (EDITOR_BRUSH_KEPT_TERRAIN | TERRAIN_WALKABLE_BUILDING | TERRAIN_FLOODPLAIN)) | TERRAIN_FLOODPLAIN;
            changed = true;
            editor_update_tiles_around(xx, yy, 2);
            editor_floodplain_spread(xx, yy);
        }
        offset += row_skip;
    }
    return changed;
}

bool editor_floodplain_shrink(int32_t x, int32_t y)
{
    bool changed = false;
    tile_square square = square_around(x, y, 1);
    int32_t row_skip = square_row_skip(&square);
    int32_t offset = map_grid_offset(square.x0, square.y0);
    for (int32_t yy = square.y0; yy <= square.y1; yy++) {
        for (int32_t xx = square.x0; xx <= square.x1; xx++, offset++) {
            uint32_t *terrain = &map_grids.terrain[offset];
            if (!(*terrain & TERRAIN_FLOODPLAIN)) {
                continue;
            }
            *terrain &= ~TERRAIN_FLOODPLAIN;
            changed = true;
            editor_update_tiles_around(xx, yy, 1);
            if (*terrain & EDITOR_FLOODPLAIN_BLOCKING_TERRAIN) {
                editor_floodplain_shrink(xx, yy);
            }
        }
        offset += row_skip;
    }
    return changed;
}

int32_t editor_find_best_neighbour(int32_t offset, uint32_t mask)
{
    static const int32_t deltas[DIRECTION_COUNT] = { DELTA_TOP, DELTA_RIGHT, DELTA_BOTTOM, DELTA_LEFT, DELTA_TOP_RIGHT, DELTA_BOTTOM_RIGHT, DELTA_BOTTOM_LEFT,
        DELTA_TOP_LEFT };
    if (editor_terrain_corner_open(offset, mask, 1)) {
        return 0;
    }
    int32_t best = 0;
    int32_t result = 0;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        int32_t neighbour = deltas[i] + offset;
        int32_t free_sides = DIRECTION_COUNT - map_terrain_count_neighbours_with(mask, neighbour);
        if ((mask & map_grids.terrain[neighbour]) && free_sides > best) {
            best = free_sides;
            result = deltas[i];
        }
    }
    return result;
}

bool editor_floodplain_fill(int32_t x, int32_t y)
{
    bool taken = false;
    int32_t offset = map_grid_offset(x, y);
    if (map_grids.terrain[offset] & TERRAIN_FLOODPLAIN) {
        return false;
    }
    int32_t delta = editor_find_best_neighbour(offset, TERRAIN_FLOODPLAIN);
    while (delta != 0) {
        taken = true;
        int32_t neighbour = delta + offset;
        map_grids.terrain[neighbour] = (map_grids.terrain[neighbour] & ~TERRAIN_FLOODPLAIN) | TERRAIN_GROUNDWATER;
        int32_t relative = neighbour - map_info.grid_start;
        int32_t nx = relative % GRID_SIZE;
        int32_t ny = relative / GRID_SIZE;
        editor_floodplain_fill(nx, ny);
        editor_update_tiles_around(nx, ny, 1);
        delta = editor_find_best_neighbour(offset, TERRAIN_FLOODPLAIN);
    }
    return taken;
}

void editor_fix_region(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
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
    tile_square square = { x0, y0, x1, y1 };
    int32_t row_skip = square_row_skip(&square);
    int32_t offset = map_grid_offset(x0, y0);
    for (int32_t y = y0; y <= y1; y++) {
        for (int32_t x = x0; x <= x1; x++, offset++) {
            uint32_t terrain = map_grids.terrain[offset];
            if (terrain & TERRAIN_WATER) {
                continue;
            }
            if (terrain & TERRAIN_FLOODPLAIN) {
                editor_floodplain_spread(x, y);
            } else if (terrain & EDITOR_FLOODPLAIN_BLOCKING_TERRAIN) {
                editor_floodplain_shrink(x, y);
            } else {
                editor_floodplain_fill(x, y);
            }
        }
        offset += row_skip;
    }
}

void editor_fix_all(void)
{
    editor_fix_region(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
}
