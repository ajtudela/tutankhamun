// Water queries of the map: water next to a building, the water tile in front of a building that boats use, the 3x3
// water check of a building, the shore tile where a transport ship lands its legion, the wharf that gets a new fishing
// boat, the open water a shipwreck drifts to, the free water tile nearest a point, and the orientation of a 2x2 or
// 3x3 shore building and of a 2x2 water pattern under the view.

#include "map/water.h"

#include "building/building.h"
#include "building/building_type.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "map/construction_terrain.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/terrain.h"

#include <float.h>
#include <stdint.h>

static uint8_t terrain_low_byte(int32_t offset)
{
    return (uint8_t) map_grids.terrain[offset];
}

static int32_t landing_tile_ok(int32_t offset)
{
    uint8_t terrain = terrain_low_byte(offset);
    if (terrain & TERRAIN_ROAD) {
        return 1;
    }
    if (terrain & TERRAIN_WATER) {
        return 0;
    }
    for (int32_t direction = 0; direction < DIRECTION_COUNT; direction++) {
        if (terrain_low_byte(offset + map_grid_direction_delta[direction]) & TERRAIN_WATER) {
            return 0;
        }
    }
    return 1;
}

int32_t map_water_landing_point(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t *out_x, int32_t *out_y)
{
    int32_t best = 0;
    int32_t sx = 0;
    int32_t sy = 0;
    if (x0 == x1 && y0 == y1) {
        *out_x = x0;
        *out_y = y0;
        return 0;
    }
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;
    int32_t best_x = x0;
    int32_t best_y = y0;
    double slope;
    double inverse;
    if (dx == 0) {
        slope = FLT_MAX;
        inverse = 0.0;
    } else {
        if (dy == 0) {
            slope = 0.0;
            inverse = FLT_MAX;
        } else {
            slope = (double) dy / (double) dx;
            inverse = 1.0 / slope;
        }
        if (dx < 0) {
            sx = -1;
        } else if (dx > 0) {
            sx = 1;
        }
    }
    if (dy < 0) {
        sy = -1;
    } else if (dy > 0) {
        sy = 1;
    }
    int32_t rows = (dy < 0) ? -dy : dy;
    int32_t columns = (dx < 0) ? -dx : dx;
    int32_t y = y0;
    int32_t row_offset = map_info.grid_start + (y0 * GRID_SIZE);
    for (int32_t i = rows; i > 0; i--) {
        if (inverse == (double) FLT_MAX) {
            continue;
        }
        y += sy;
        row_offset += (sy * GRID_SIZE);
        int32_t ry = y - y0;
        int32_t x = crt_ftol_double(((double) ry * inverse) + (double) x0);
        if (landing_tile_ok(x + row_offset)) {
            int32_t d = ((x - x0) * (x - x0)) + (ry * ry);
            if (d >= best) {
                best = d;
                best_x = x;
                best_y = y;
            }
        }
    }
    int32_t x = x0;
    for (int32_t i = columns; i > 0; i--) {
        if (slope == (double) FLT_MAX) {
            continue;
        }
        x += sx;
        int32_t rx = x - x0;
        int32_t ty = crt_ftol_double(((double) rx * slope) + (double) y0);
        if (landing_tile_ok(map_info.grid_start + (ty * GRID_SIZE) + x)) {
            int32_t d = (rx * rx) + ((ty - y0) * (ty - y0));
            if (d >= best) {
                best = d;
                best_x = x;
                best_y = ty;
            }
        }
    }
    *out_x = best_x;
    *out_y = best_y;
    int32_t offset = *out_x + (best_y * GRID_SIZE) + map_info.grid_start;
    if (x0 != *out_x || y0 != best_y) {
        return best;
    }
    if (!(terrain_low_byte(offset - GRID_SIZE) & TERRAIN_WATER)) {
        *out_y = best_y - 1;
    } else if (!(terrain_low_byte(offset - GRID_SIZE + 1) & TERRAIN_WATER)) {
        *out_y = best_y - 1;
        *out_x = *out_x + 1;
    } else if (!(terrain_low_byte(offset + GRID_SIZE) & TERRAIN_WATER)) {
        *out_y = best_y + 1;
    } else if (!(terrain_low_byte(offset - GRID_SIZE - 1) & TERRAIN_WATER)) {
        *out_y = best_y - 1;
        *out_x = *out_x - 1;
    } else if (!(terrain_low_byte(offset + 1) & TERRAIN_WATER)) {
        *out_x = *out_x + 1;
    } else if (!(terrain_low_byte(offset + GRID_SIZE + 1) & TERRAIN_WATER)) {
        *out_y = best_y + 1;
        *out_x = *out_x + 1;
    } else if (!(terrain_low_byte(offset - 1) & TERRAIN_WATER)) {
        *out_x = *out_x - 1;
    } else if (!(terrain_low_byte(offset + GRID_SIZE - 1) & TERRAIN_WATER)) {
        *out_y = best_y + 1;
        *out_x = *out_x - 1;
    }
    return best;
}

static int32_t is_wharf(int32_t type)
{
    return type == BUILDING_FISHING_WHARF || type == BUILDING_TRANSPORT_WHARF || type == BUILDING_WARSHIP_WHARF;
}

int32_t map_water_get_wharf_for_new_fishing_boat(int32_t figure_id)
{
    int32_t id;
    const building *b = 0;
    for (id = 1; id < MAX_BUILDINGS; id++) {
        b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && is_wharf(b->type) &&
            (b->data.industry.worker_figure_id == 0 || b->data.industry.worker_figure_id == figure_id)) {
            break;
        }
    }
    if (id >= MAX_BUILDINGS) {
        return 0;
    }
    switch (b->data.industry.orientation) {
        case 0:
            map_result_x = b->x + 1;
            map_result_y = b->y - 1;
            break;
        case 1:
            map_result_x = b->x + 2;
            map_result_y = b->y + 1;
            break;
        case 2:
            map_result_x = b->x + 1;
            map_result_y = b->y + 2;
            break;
        default:
            map_result_x = b->x - 1;
            map_result_y = b->y + 1;
            break;
    }
    return id;
}

static int32_t is_open_water(int32_t offset)
{
    int32_t row = OPEN_WATER_DISTANCE * GRID_SIZE;
    return (map_grids.terrain[offset - row] & TERRAIN_WATER) && (map_grids.terrain[offset + OPEN_WATER_DISTANCE] & TERRAIN_WATER) &&
        (map_grids.terrain[offset + row] & TERRAIN_WATER) && (map_grids.terrain[offset - OPEN_WATER_DISTANCE] & TERRAIN_WATER);
}

int32_t map_water_find_shipwreck_tile(int32_t figure_id)
{
    const figure *f = figure_get(figure_id);
    if ((map_grids.terrain[f->grid_offset] & TERRAIN_WATER) && map_grids.figure_ids[f->grid_offset] == figure_id) {
        return 0;
    }
    int32_t fx = f->x;
    int32_t fy = f->y;
    for (int32_t radius = 1; radius <= SHIPWRECK_SEARCH_RADIUS; radius++) {
        int32_t x_min = fx - radius;
        int32_t y_min = fy - radius;
        int32_t x_max = fx + radius;
        int32_t y_max = fy + radius;
        if (x_min < 0) {
            x_min = 0;
        }
        if (y_min < 0) {
            y_min = 0;
        }
        if (x_max >= map_info.grid_width) {
            x_max = map_info.grid_width - 1;
        }
        if (y_max >= map_info.grid_height) {
            y_max = map_info.grid_height - 1;
        }
        int32_t offset = map_info.grid_start + (y_min * GRID_SIZE) + x_min;
        for (map_result_y = y_min; map_result_y <= y_max; map_result_y++) {
            map_result_x = x_min;
            for (; map_result_x <= x_max; map_result_x++, offset++) {
                int32_t id = map_grids.figure_ids[offset];
                if ((map_grids.terrain[offset] & TERRAIN_WATER) && (id == 0 || id == figure_id) && is_open_water(offset)) {
                    return 1;
                }
            }
            offset += x_min - x_max + (GRID_SIZE - 1);
        }
    }
    return 0;
}

static void water_orient_shift(int32_t *x, int32_t *y, int32_t adjust, int32_t step)
{
    if (adjust == 1 && map_info.orientation != DIRECTION_TOP) {
        if (map_info.orientation == DIRECTION_LEFT) {
            *y -= step;
        } else if (map_info.orientation == DIRECTION_BOTTOM) {
            *x -= step;
            *y -= step;
        } else if (map_info.orientation == DIRECTION_RIGHT) {
            *x -= step;
        }
    }
}

int32_t map_water_determine_orientation_size2(int32_t x, int32_t y, int32_t adjust, int32_t allow_arg)
{
    char allow = (char) allow_arg;
    water_orient_shift(&x, &y, adjust, 1);
    if (!(x >= 0 && y >= 0 && x + 1 < map_info.grid_width && y + 1 < map_info.grid_height)) {
        return WATER_ORIENTATION_NO_MATCH;
    }
    int32_t blocked_kind = 0;
    int32_t base = map_info.grid_start + (y * GRID_SIZE) + x;
    for (int32_t orientation = 0; orientation < 4; orientation++) {
        int32_t road_or_tree = 0;
        int32_t matched = 0;
        const int32_t *delta = water_delta2;
        const int32_t *want = water_pattern2 + orientation * 4;
        for (; matched < 4; matched++, delta++, want++) {
            int32_t tile = *delta + base;
            uint32_t terrain = map_grids.terrain[tile];
            if (*want == 0) {
                const flood_tile *object = floods_data.tiles[tile];
                if ((terrain & WATER_SHORE_WATER_OR_FLOODPLAIN) != 0 && (allow == 0 || object == 0 || object->is_floodplain != 0)) {
                    break;
                }
                if ((terrain & TERRAIN_ROAD) == 0 && (terrain & WATER_SHORE_BLOCKING) == 0) {
                    if ((terrain & TERRAIN_BUILDING) != 0) {
                        road_or_tree++;
                    }
                } else {
                    road_or_tree++;
                }
            } else {
                const flood_tile *object = floods_data.tiles[tile];
                int32_t building_blocks = 0;
                if (object != 0 && object->is_floodplain == 0 && blocked_kind != 2) {
                    blocked_kind = 1;
                    if (allow != 0) {
                        building_blocks = 1;
                    }
                }
                if ((terrain & TERRAIN_WATER) == 0 || blocked_kind == 1) {
                    if (!building_blocks) {
                        break;
                    }
                } else {
                    blocked_kind = 2;
                }
                if ((terrain & TERRAIN_ROAD) != 0) {
                    road_or_tree++;
                }
                if ((terrain & TERRAIN_TREE) != 0) {
                    road_or_tree++;
                }
            }
        }
        uint32_t mask = (allow != 0) ? WATER_SHORE_WATER_OR_FLOODPLAIN : TERRAIN_WATER;
        for (int32_t k = 0; k < 2; k++) {
            int32_t tiles[3] = {
                water_adj2_a[orientation * 2 + k] + base,
                water_adj2_b[orientation * 2 + k] + base,
                water_adj2_c[orientation * 2 + k] + base,
            };
            for (int32_t t = 0; t < 3; t++) {
                if ((map_grids.terrain[tiles[t]] & mask) == 0) {
                    const flood_tile *object = floods_data.tiles[tiles[t]];
                    if (allow == 0 || object == 0 || object->is_floodplain != 0) {
                        matched = 0;
                    }
                }
            }
        }
        if (matched == 4) {
            water_orient_raw = orientation;
            map_water_orientation_relative = orientation - map_info.orientation / 2;
            if (map_water_orientation_relative < 0) {
                map_water_orientation_relative += 4;
            }
            return road_or_tree;
        }
    }
    map_water_orientation_relative = 4;
    return WATER_ORIENTATION_NO_MATCH;
}

int32_t map_water_determine_orientation_size3(int32_t x, int32_t y, int32_t adjust)
{
    water_orient_shift(&x, &y, adjust, 2);
    if (!(x >= 0 && y >= 0 && x + 2 < map_info.grid_width && y + 2 < map_info.grid_height)) {
        return WATER_ORIENTATION_NO_MATCH;
    }
    int32_t base = map_info.grid_start + (y * GRID_SIZE) + x;
    for (int32_t orientation = 0; orientation < 4; orientation++) {
        int32_t cells = 0;
        int32_t road_or_tree = 0;
        const int32_t *delta = water_delta3;
        const int32_t *want = water_pattern3 + orientation * 9;
        for (; cells < 9; cells++, delta++, want++) {
            if (*want == 0) {
                int32_t tile = *delta + base;
                uint32_t terrain = map_grids.terrain[tile];
                const flood_tile *object = floods_data.tiles[tile];
                if ((terrain & WATER_SHORE_WATER_OR_FLOODPLAIN) != 0 || (object != 0 && object->is_floodplain == 0)) {
                    break;
                }
                if ((terrain & TERRAIN_ROAD) == 0 && (terrain & WATER_SHORE_BLOCKING) == 0) {
                    if ((terrain & TERRAIN_BUILDING) != 0) {
                        road_or_tree++;
                    }
                } else {
                    road_or_tree++;
                }
            } else {
                uint32_t terrain = map_grids.terrain[*delta + base];
                if ((terrain & TERRAIN_WATER) == 0) {
                    break;
                }
                if ((terrain & TERRAIN_ROAD) != 0) {
                    road_or_tree++;
                }
                if ((terrain & TERRAIN_TREE) != 0) {
                    road_or_tree++;
                }
            }
        }
        const int32_t *adj = water_adj3 + orientation * 2;
        for (int32_t k = 0; k < 2; k++) {
            if ((map_grids.terrain[adj[k] + base] & TERRAIN_WATER) == 0) {
                cells = 0;
            }
        }
        if (cells == 9) {
            water_orient_raw = orientation;
            map_water_orientation_relative = orientation - map_info.orientation / 2;
            if (map_water_orientation_relative < 0) {
                map_water_orientation_relative += 4;
            }
            return road_or_tree;
        }
    }
    map_water_orientation_relative = 4;
    return WATER_ORIENTATION_NO_MATCH;
}

int32_t map_terrain_is_adjacent_to_water(int32_t x, int32_t y, int32_t size)
{
    int32_t offset = map_info.grid_start + (y * GRID_SIZE) + x;
    const int32_t *delta = &building_tile_offsets.rings[size * BUILDING_RING_OFFSETS_PER_SIZE];
    for (int32_t i = 0; i < BUILDING_RING_OFFSETS_PER_SIZE; i++, delta++) {
        if (*delta == 0) {
            return 0;
        }
        if (map_grids.terrain[offset + *delta] & TERRAIN_WATER) {
            return 1;
        }
    }
    return 0;
}

int32_t map_terrain_is_adjacent_to_open_water(int32_t x, int32_t y, int32_t size)
{
    const int32_t *offsets = &building_tile_offsets.rings[size * BUILDING_RING_OFFSETS_PER_SIZE];
    int32_t base = map_grid_offset(x, y);
    for (int32_t i = 0; offsets[i] != 0 && i < BUILDING_RING_OFFSETS_PER_SIZE; i++) {
        int32_t tile = base + offsets[i];
        if ((map_grids.terrain[tile] & TERRAIN_WATER) && routing_grids.distance[tile] != 0) {
            return 1;
        }
    }
    return 0;
}

int32_t map_water_tile_next_to(int32_t x, int32_t y, int32_t size)
{
    int32_t base = map_grid_offset(x, y);
    const int32_t *offsets = &building_tile_offsets.rings[size * BUILDING_RING_OFFSETS_PER_SIZE];
    for (int32_t i = 0; i < BUILDING_RING_OFFSETS_PER_SIZE; i++) {
        if (offsets[i] == 0) {
            return 0;
        }
        int32_t grid_offset = offsets[i] + base;
        uint32_t terrain = map_grids.terrain[grid_offset];
        if ((terrain & TERRAIN_WATER) && !(terrain & TERRAIN_BUILDING)) {
            map_tile_cursor_offset = grid_offset;
            map_image_context_set_tiles(TERRAIN_WATER);
            if (map_image_context_count_tiles() >= WATER_TILE_MIN_WATER_AROUND) {
                map_result_x = building_ring_xy[(i + (size * BUILDING_RING_OFFSETS_PER_SIZE)) * 2] + x;
                map_result_y = building_ring_xy[((i + (size * BUILDING_RING_OFFSETS_PER_SIZE)) * 2) + 1] + y;
                return 1;
            }
        }
    }
    return 0;
}

static const int32_t water_delta[4] = { 0, 1, GRID_SIZE, GRID_SIZE + 1 };
static const int32_t water_pattern[4][4] = {
    { 1, 1, 0, 0 },
    { 0, 1, 0, 1 },
    { 0, 0, 1, 1 },
    { 1, 0, 1, 0 },
};

int32_t map_water_orientation_match(int32_t x, int32_t y)
{
    water_orient_shift(&x, &y, 1, 1);
    int32_t base = map_info.grid_start + (y * GRID_SIZE) + x;
    for (int32_t pattern = 0; pattern < 4; pattern++) {
        int32_t matched = 0;
        for (int32_t i = 0; i < 4; i++) {
            int32_t water = (map_grids.terrain[base + water_delta[i]] & TERRAIN_WATER) != 0;
            if (water == (water_pattern[pattern][i] != 0)) {
                matched++;
            }
        }
        if (matched == 4) {
            return pattern;
        }
    }
    return WATER_ORIENTATION_NO_MATCH;
}

bool building_near_water(int32_t building_id)
{
    const building *b = building_get(building_id);
    return map_terrain_has_bits(b->x, b->y, BUILDING_NEAR_WATER_SIZE, BUILDING_NEAR_WATER_SIZE, TERRAIN_WATER);
}

static int32_t water_tile_free(int32_t offset)
{
    return routing_grids.water[offset] >= 0 && map_grids.figure_ids[offset] == 0;
}

static int32_t water_free_on_row(int32_t x_start, int32_t x_end, int32_t y, int32_t *found_x, int32_t *found_y)
{
    for (int32_t x = x_start; x <= x_end; x++) {
        if (water_tile_free(x + (y * GRID_SIZE))) {
            *found_x = x;
            *found_y = y;
            return 1;
        }
    }
    return 0;
}

static int32_t water_free_on_column(int32_t x, int32_t y_start, int32_t y_end, int32_t *found_x, int32_t *found_y)
{
    for (int32_t y = y_start; y <= y_end; y++) {
        if (water_tile_free(x + (y * GRID_SIZE))) {
            *found_x = x;
            *found_y = y;
            return 1;
        }
    }
    return 0;
}

bool map_water_find_free_tile(int32_t *x, int32_t *y)
{
    if (water_tile_free(map_grid_offset(*x, *y))) {
        return 1;
    }
    int32_t y_lo = *y - 1 + map_info.diamond_y_min;
    int32_t y_hi = *y + 1 + map_info.diamond_y_min;
    int32_t x_hi = *x + 1 + map_info.diamond_x_min;
    int32_t x_lo = *x - 1 + map_info.diamond_x_min;
    for (int32_t radius = 1; radius <= WATER_FREE_TILE_MAX_RADIUS; radius++) {
        int32_t x_start = (x_lo < map_info.diamond_x_min) ? map_info.diamond_x_min : x_lo;
        int32_t x_end = (x_hi > map_info.diamond_x_max) ? map_info.diamond_x_max : x_hi;
        int32_t y_start = (y_lo < map_info.diamond_y_min) ? map_info.diamond_y_min : y_lo;
        int32_t y_end = (y_hi > map_info.diamond_y_max) ? map_info.diamond_y_max : y_hi;
        int32_t found_x = 0;
        int32_t found_y = 0;
        if ((y_lo >= map_info.diamond_y_min && water_free_on_row(x_start, x_end, y_start, &found_x, &found_y)) ||
            (y_hi <= map_info.diamond_y_max && water_free_on_row(x_start, x_end, y_end, &found_x, &found_y)) ||
            (x_lo >= map_info.diamond_x_min && water_free_on_column(x_start, y_start, y_end, &found_x, &found_y)) ||
            (x_hi <= map_info.diamond_x_max && water_free_on_column(x_end, y_start, y_end, &found_x, &found_y))) {
            *x = found_x - map_info.diamond_x_min;
            *y = found_y - map_info.diamond_y_min;
            return 1;
        }
        x_hi++;
        x_lo--;
        y_lo--;
        y_hi++;
    }
    return 0;
}
