// Road access: the road a building reaches next to it, within a radius, nearby on the axes, around a temple complex or
// by the routing distance (also for three placements side by side), ranked by the player's largest road networks; and
// the roads a roaming walker may take from a tile (the least visited ones when there is a choice).
//
// The triple search's distance fallback adds the shift to map_result_x and to the returned tile a second time.

#include "map/road_access.h"

#include "building/building.h"
#include "building/building_stocks.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/service_visit.h"
#include "map/terrain.h"
#include "map/tiles_internal.h"

#include <stdint.h>

static int32_t road_network_rank(int32_t offset, int32_t limit)
{
    uint32_t network = map_grids.road_network[offset];
    const road_network_entry *entry = city_player_current()->largest_road_networks;
    for (int32_t i = 0; i < limit; i++, entry++) {
        if ((uint32_t) entry->id == network) {
            return i;
        }
    }
    return ROAD_RANK_UNLISTED_NETWORK;
}

static int32_t building_type_at_offset(int32_t offset)
{
    return building_get(map_grids.building_ids[offset])->type;
}

static int32_t is_open_road_tile(int32_t tile)
{
    uint32_t terrain = map_grids.terrain[tile];
    int32_t type = building_type_at_offset(tile);
    int32_t passable_building = ((terrain & TERRAIN_BUILDING) == 0) || (type != BUILDING_ROADBLOCK && type != BUILDING_FERRY_LANDING);
    return passable_building && (terrain & TERRAIN_ROAD) != 0;
}

static int32_t is_open_dry_road_tile(int32_t tile)
{
    return is_open_road_tile(tile) && (map_grids.terrain[tile] & TERRAIN_WATER) == 0;
}

static int32_t is_blocked_road_building(int32_t type, int32_t roadblock_closed)
{
    return type == BUILDING_GATEHOUSE || type == BUILDING_GATEHOUSE_UP || (type == BUILDING_ROADBLOCK && roadblock_closed) || type == BUILDING_FERRY_LANDING;
}

int32_t map_road_within_radius(int32_t x, int32_t y, int32_t size, int32_t radius, int32_t kind)
{
    int32_t x0 = x - radius;
    int32_t x1 = x + size - 1 + radius;
    int32_t y0 = y - radius;
    int32_t y1 = y + size - 1 + radius;
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
    int32_t best_rank = ROAD_RANK_NONE;
    int32_t best_x = x0;
    int32_t best_y = y0;
    int32_t best_offset = map_grid_offset(x0, y0);
    int32_t offset = best_offset;
    map_result_y = y0;
    map_tile_cursor_offset = offset;
    if (y0 > y1) {
        return 0;
    }
    int32_t ycur = y0;
    do {
        map_result_x = x0;
        if (x0 <= x1) {
            int32_t xcur = x0;
            do {
                if (((map_grids.terrain[offset] & (TERRAIN_WATER | TERRAIN_ROAD)) == TERRAIN_ROAD) && kind == 0) {
                    int32_t rank = road_network_rank(offset, ROAD_RANK_RADIUS_NETWORKS);
                    if (rank < best_rank) {
                        best_rank = rank;
                        best_x = xcur;
                        best_y = ycur;
                        best_offset = offset;
                    }
                }
                xcur++;
                offset++;
                map_result_x = xcur;
            } while (xcur <= x1);
        }
        ycur++;
        offset += x0 - x1 + (GRID_SIZE - 1);
        map_result_y = ycur;
        map_tile_cursor_offset = offset;
    } while (ycur <= y1);
    if (best_rank < ROAD_RANK_NONE) {
        map_tile_cursor_offset = best_offset;
        map_result_x = best_x;
        map_result_y = best_y;
        return 1;
    }
    return 0;
}

int32_t map_closest_road_within_radius(int32_t x, int32_t y, int32_t size, int32_t max_radius)
{
    if (max_radius > 0) {
        for (int32_t radius = 1; radius <= max_radius; radius++) {
            if (map_road_within_radius(x, y, size, radius, 0)) {
                return 1;
            }
        }
    }
    return 0;
}

int32_t map_has_road_access(int32_t x, int32_t y, int32_t size)
{
    int32_t best_rank = ROAD_RANK_NONE;
    int32_t best_index = 0;
    int32_t base = map_grid_offset(x, y);
    const int32_t *delta = building_tile_offsets.rings + (size * ROAD_ACCESS_RING_TILES);
    for (int32_t i = 0; i < ROAD_ACCESS_RING_TILES; i++, delta++) {
        int32_t d = *delta;
        if (d == 0) {
            break;
        }
        int32_t tile = base + d;
        if (is_open_dry_road_tile(tile)) {
            int32_t rank = road_network_rank(tile, PLAYER_LARGEST_ROAD_NETWORKS);
            if (rank < best_rank) {
                best_rank = rank;
                best_index = i;
            }
        }
    }
    if (best_rank >= ROAD_RANK_NONE) {
        return 0;
    }
    int32_t xy_index = best_index + (size * ROAD_ACCESS_RING_TILES);
    map_result_x = building_ring_xy[xy_index * 2] + x;
    map_result_y = building_ring_xy[xy_index * 2 + 1] + y;
    return 1;
}

int32_t map_has_road_access_granary(int32_t x, int32_t y)
{
    return map_has_road_access(x, y, GRANARY_SIZE);
}

int32_t map_has_road_access_nearby(int32_t x, int32_t y)
{
    static const int32_t deltas[ROAD_ACCESS_NEARBY_TILES] = { -GRID_SIZE, 1, GRID_SIZE, -1, -2 * GRID_SIZE, 2, 2 * GRID_SIZE, -2 };
    static const int32_t xy[ROAD_ACCESS_NEARBY_TILES][2] = {
        { 0, -1 },
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -2 },
        { 2, 0 },
        { -2, 0 },
        { 0, 2 },
    };
    int32_t base = map_grid_offset(x, y);
    int32_t best_rank = ROAD_RANK_NONE;
    int32_t best_index = 0;
    for (int32_t i = 0; i < ROAD_ACCESS_NEARBY_TILES; i++) {
        int32_t d = deltas[i];
        if (d == 0) {
            break;
        }
        int32_t tile = base + d;
        if (is_open_road_tile(tile)) {
            int32_t rank = road_network_rank(tile, PLAYER_LARGEST_ROAD_NETWORKS);
            if (rank < best_rank) {
                best_rank = rank;
                best_index = i;
            }
        }
    }
    if (best_rank >= ROAD_RANK_NONE) {
        return 0;
    }
    map_result_x = xy[best_index][0] + x;
    map_result_y = xy[best_index][1] + y;
    return 1;
}

int32_t map_has_road_access_temple_complex(int32_t x, int32_t y, int32_t orientation)
{
    int32_t inner;
    int32_t outer;
    if (orientation == DIRECTION_TOP || orientation == DIRECTION_BOTTOM) {
        inner = ROAD_ACCESS_TEMPLE_RING_SHORT;
        outer = ROAD_ACCESS_TEMPLE_RING_LONG;
        x -= 1;
        y -= 3;
    } else {
        inner = ROAD_ACCESS_TEMPLE_RING_LONG;
        outer = ROAD_ACCESS_TEMPLE_RING_SHORT;
        x -= 3;
        y -= 1;
    }
    int32_t best_rank = ROAD_RANK_NONE;
    int32_t best_offset = 0;
    int32_t base = map_grid_offset(x, y);
    int32_t col0 = base;
    if (outer == 0) {
        return 0;
    }
    for (int32_t col = 0; col < outer; col++) {
        int32_t step = (col == 0 || col == outer - 1) ? 1 : inner - 1;
        int32_t row = 0;
        int32_t offset = col0;
        while (row < inner) {
            if (is_open_dry_road_tile(offset)) {
                int32_t rank = road_network_rank(offset, PLAYER_LARGEST_ROAD_NETWORKS);
                if (rank < best_rank) {
                    best_rank = rank;
                    best_offset = offset;
                }
            }
            offset += step * GRID_SIZE;
            row += step;
        }
        col0++;
    }
    if (best_rank >= ROAD_RANK_NONE) {
        return 0;
    }
    map_result_x = (best_offset - map_info.grid_start) % GRID_SIZE;
    map_result_y = (best_offset - map_info.grid_start) / GRID_SIZE;
    return 1;
}

int32_t map_reachable_road_around(int32_t x, int32_t y, int32_t size)
{
    int32_t best_rank = ROAD_RANK_NONE;
    int32_t best_index = 0;
    int32_t base = map_grid_offset(x, y);
    const int32_t *delta = building_tile_offsets.rings + (size * ROAD_ACCESS_RING_TILES);
    for (int32_t i = 0; i < ROAD_ACCESS_RING_TILES; i++, delta++) {
        int32_t d = *delta;
        if (d == 0) {
            break;
        }
        int32_t tile = base + d;
        if ((map_grids.terrain[tile] & TERRAIN_ROAD) != 0 && (map_grids.terrain[tile] & TERRAIN_WATER) == 0 && routing_grids.distance[tile] > 0) {
            int32_t rank = road_network_rank(tile, PLAYER_LARGEST_ROAD_NETWORKS);
            if (rank < best_rank) {
                best_rank = rank;
                best_index = i;
            }
        }
    }
    if (best_rank < ROAD_RANK_NONE) {
        int32_t xy_index = best_index + (size * ROAD_ACCESS_RING_TILES);
        map_result_x = building_ring_xy[xy_index * 2] + x;
        map_result_y = building_ring_xy[xy_index * 2 + 1] + y;
        return building_tile_offsets.rings[xy_index] + 1 + base;
    }
    int32_t best_dist = 100000;
    int32_t best_tile = -1;
    int32_t best_x = x;
    int32_t best_y = y;
    delta = building_tile_offsets.rings + (size * ROAD_ACCESS_RING_TILES);
    const int32_t *xy = building_ring_xy + ((size * ROAD_ACCESS_RING_TILES) * 2);
    for (int32_t i = 0; i < ROAD_ACCESS_RING_TILES; i++, delta++, xy += 2) {
        if (*delta == 0) {
            break;
        }
        int32_t tile = base + *delta;
        int32_t dist = routing_grids.distance[tile];
        if (dist > 0 && dist < best_dist) {
            best_x = xy[0] + x;
            best_y = xy[1] + y;
            best_tile = tile;
            best_dist = dist;
        }
    }
    if (best_tile < 0) {
        return 0;
    }
    map_result_x = best_x;
    map_result_y = best_y;
    return best_tile + 1;
}

int32_t map_reachable_road_triple(int32_t x, int32_t y, int32_t size)
{
    int32_t best_rank = ROAD_RANK_NONE;
    int32_t best_index = 0;
    int32_t best_shift = 0;
    int32_t base = map_grid_offset(x, y);
    for (int32_t shift = 0; shift < ROAD_ACCESS_TRIPLE_SHIFT_END; shift += ROAD_ACCESS_TRIPLE_STEP) {
        const int32_t *delta = building_tile_offsets.rings + (size * ROAD_ACCESS_RING_TILES);
        for (int32_t i = 0; i < ROAD_ACCESS_RING_TILES; i++, delta++) {
            if (*delta == 0) {
                break;
            }
            int32_t tile = base + shift + *delta;
            if ((map_grids.terrain[tile] & TERRAIN_ROAD) != 0 && routing_grids.distance[tile] > 0) {
                int32_t rank = road_network_rank(tile, PLAYER_LARGEST_ROAD_NETWORKS);
                if (rank < best_rank) {
                    best_rank = rank;
                    best_index = i;
                    best_shift = shift;
                }
            }
        }
    }
    if (best_rank < ROAD_RANK_NONE) {
        int32_t xy_index = best_index + (size * ROAD_ACCESS_RING_TILES);
        map_result_x = building_ring_xy[xy_index * 2] + best_shift + x;
        map_result_y = building_ring_xy[xy_index * 2 + 1] + y;
        return building_tile_offsets.rings[xy_index] + best_shift + 1 + base;
    }
    int32_t best_dist = 100000;
    int32_t best_tile = -1;
    int32_t best_x = x;
    int32_t best_y = y;
    int32_t best_s = 0;
    for (int32_t shift = 0; shift < ROAD_ACCESS_TRIPLE_SHIFT_END; shift += ROAD_ACCESS_TRIPLE_STEP) {
        const int32_t *delta = building_tile_offsets.rings + (size * ROAD_ACCESS_RING_TILES);
        const int32_t *xy = building_ring_xy + ((size * ROAD_ACCESS_RING_TILES) * 2);
        for (int32_t i = 0; i < ROAD_ACCESS_RING_TILES; i++, delta++, xy += 2) {
            if (*delta == 0) {
                break;
            }
            int32_t tile = base + shift + *delta;
            int32_t dist = routing_grids.distance[tile];
            if (dist > 0 && dist < best_dist) {
                best_x = xy[0] + shift + x;
                best_y = xy[1] + y;
                best_tile = tile;
                best_dist = dist;
                best_s = shift;
            }
        }
    }
    if (best_tile < 0) {
        return 0;
    }
    map_result_y = best_y;
    map_result_x = best_x + best_s;
    return best_tile + 1 + best_s;
}

int32_t map_get_adjacent_road_tiles_for_roaming(int32_t offset, int32_t figure_id)
{
    figure *f = figure_get(figure_id);
    int32_t roadblock_closed = f->type != FIGURE_PLAGUED_CITIZEN;
    map_adjacent_tiles[DIRECTION_TOP_RIGHT] = 0;
    map_adjacent_tiles[DIRECTION_BOTTOM_RIGHT] = 0;
    map_adjacent_tiles[DIRECTION_BOTTOM_LEFT] = 0;
    map_adjacent_tiles[DIRECTION_TOP_LEFT] = 0;
    int32_t count = 0;
    for (int32_t direction = DIRECTION_TOP; direction < DIRECTION_COUNT; direction += 2) {
        int32_t tile = offset + map_grid_direction_delta[direction];
        map_adjacent_tiles[direction] = (int32_t) (map_grids.terrain[tile] & (TERRAIN_ROAD | TERRAIN_ACCESS_RAMP));
        if ((map_grids.terrain[tile] & TERRAIN_BUILDING) && is_blocked_road_building(building_type_at_offset(tile), roadblock_closed)) {
            map_adjacent_tiles[direction] = 0;
        }
        count += (map_adjacent_tiles[direction] != 0);
    }
    if (count > 2) {
        int32_t min_visit = 7;
        int32_t max_visit = 0;
        int32_t visits[DIRECTION_COUNT] = { 0 };
        int32_t tag = figure_type_coverage_tag[f->type];
        for (int32_t i = DIRECTION_TOP; i < DIRECTION_COUNT; i += 2) {
            if (map_adjacent_tiles[i] != 0) {
                int32_t v = map_service_visit_get(tag, map_grid_direction_delta[i] + offset);
                visits[i] = v;
                if (v <= min_visit) {
                    min_visit = v;
                }
                if (max_visit <= v) {
                    max_visit = v;
                }
            }
        }
        for (int32_t i = DIRECTION_TOP; i < DIRECTION_COUNT; i += 2) {
            if (map_adjacent_tiles[i] != 0 && min_visit < visits[i]) {
                map_adjacent_tiles[i] = 0;
                count--;
            }
        }
    }
    return count;
}
