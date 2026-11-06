// Road, wall and canal tile images, for the tile at the tile cursor with its neighbours in map_adjacent_tiles: dirt and
// paved roads, roads on the floodplain, roads over a canal (bridges), walls with the gatehouse context, canals with
// their water images, the road and canal tiles placed while dragging, and the road image over a canal piece.
//
// The canal image's test for the water images can never hold, so only a full update adds water, and the canal region
// update skips tiles whose canal value is 16 or more.

#include "map/tiles_road.h"

#include "building/building.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "graphics/image_collection.h"
#include "map/construction_terrain.h"
#include "map/map_data.h"
#include "map/routing_citizen.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include "map/tiles_internal.h"

#include <stdint.h>

int32_t all_roads_paved;

static const int32_t neighbour_delta[DIRECTION_COUNT] = { -GRID_SIZE, 1 - GRID_SIZE, 1, GRID_SIZE + 1, GRID_SIZE, GRID_SIZE - 1, -1, -GRID_SIZE - 1 };

static int32_t is_gatehouse_building(int32_t type)
{
    return type == BUILDING_MUD_GATEHOUSE || type == BUILDING_BRICK_GATEHOUSE || type == BUILDING_GATEHOUSE || type == BUILDING_MUD_GATEHOUSE_UP ||
        type == BUILDING_BRICK_GATEHOUSE_UP || type == BUILDING_GATEHOUSE_UP;
}

void map_image_context_add_gatehouse_tiles(void)
{
    static const struct {
        int32_t tile;
        int32_t subtype;
    } sides[4] = { { DIRECTION_TOP, 1 }, { DIRECTION_RIGHT, 2 }, { DIRECTION_BOTTOM, 1 }, { DIRECTION_LEFT, 2 } };
    for (int32_t i = 0; i < 4; i++) {
        int32_t offset = map_tile_cursor_offset + neighbour_delta[sides[i].tile];
        if (map_grids.terrain[offset] & TERRAIN_GATEHOUSE) {
            const building *gatehouse = building_get(map_grids.building_ids[offset]);
            if (is_gatehouse_building(gatehouse->type) && gatehouse->subtype == sides[i].subtype) {
                map_adjacent_tiles[sides[i].tile] = 1;
            }
        }
    }
}

int32_t map_tiles_road_is_paved(void)
{
    return map_tiles_road_desirability_paved(map_tile_cursor_offset);
}

typedef struct {
    int32_t neighbour;
    int32_t left;
    int32_t right;
    int32_t neighbour_left;
    int32_t neighbour_right;
} canal_side_offsets;

static const canal_side_offsets canal_side_delta[4] = {
    { -GRID_SIZE, -1, 1, -GRID_SIZE - 1, 1 - GRID_SIZE },
    { 1, -GRID_SIZE, GRID_SIZE, 1 - GRID_SIZE, GRID_SIZE + 1 },
    { GRID_SIZE, -1, 1, GRID_SIZE - 1, GRID_SIZE + 1 },
    { -1, -GRID_SIZE, GRID_SIZE, -GRID_SIZE - 1, GRID_SIZE - 1 },
};

static int32_t canal_connects(int32_t dir)
{
    const canal_side_offsets *d = &canal_side_delta[dir / 2];
    uint32_t neighbour = map_grids.terrain[d->neighbour + map_tile_cursor_offset];
    if (!(neighbour & TERRAIN_CANAL)) {
        return 0;
    }
    int32_t has_road = (neighbour & TERRAIN_ROADS) != 0;
    uint32_t other;
    if (!(map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROAD)) {
        if (!has_road || (map_grids.terrain[d->neighbour_left + map_tile_cursor_offset] & TERRAIN_ROADS)) {
            return 1;
        }
        other = map_grids.terrain[d->neighbour_right + map_tile_cursor_offset];
    } else {
        if (has_road) {
            return 0;
        }
        if (map_grids.terrain[d->left + map_tile_cursor_offset] & TERRAIN_ROADS) {
            return 1;
        }
        other = map_grids.terrain[d->right + map_tile_cursor_offset];
    }
    return (other & TERRAIN_ROADS) != 0;
}

void map_image_context_add_canal_side(int32_t dir, int32_t *count)
{
    if (canal_connects(dir)) {
        map_adjacent_tiles[dir] = 1;
        ++*count;
    }
}

static int32_t water_lift_side(int32_t tile, int32_t opposite_tile, int32_t axis_bit)
{
    int32_t offset = map_tile_cursor_offset + neighbour_delta[tile];
    if (!(map_grids.terrain[offset] & TERRAIN_BUILDING)) {
        return 0;
    }
    const building *water_lift = building_get(map_grids.building_ids[offset]);
    if (water_lift->type != BUILDING_WATER_LIFT) {
        return 0;
    }
    if (map_grids.terrain[map_tile_cursor_offset + neighbour_delta[opposite_tile]] & map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROAD) {
        return 0;
    }
    return (water_lift->data.oriented.orientation & 1) == axis_bit;
}

void map_image_context_set_canal_road_tiles(void)
{
    int32_t count = 0;
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] = 0;
    }
    if (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_FLOODPLAIN) {
        map_adjacent_tiles[DIRECTION_TOP] = (map_grids.terrain[map_tile_cursor_offset - GRID_SIZE] & TERRAIN_WATER) != 0;
        map_adjacent_tiles[DIRECTION_RIGHT] = (map_grids.terrain[map_tile_cursor_offset + 1] & TERRAIN_WATER) != 0;
        map_adjacent_tiles[DIRECTION_BOTTOM] = (map_grids.terrain[map_tile_cursor_offset + GRID_SIZE] & TERRAIN_WATER) != 0;
        if (map_grids.terrain[map_tile_cursor_offset - 1] & TERRAIN_WATER) {
            map_adjacent_tiles[DIRECTION_LEFT] = 1;
        }
    }
    for (int32_t dir = DIRECTION_TOP; dir < DIRECTION_LEFT; dir += 2) {
        if (canal_connects(dir)) {
            map_adjacent_tiles[dir] = 1;
            count++;
        }
    }
    map_image_context_add_canal_side(DIRECTION_LEFT, &count);
    if (water_lift_side(DIRECTION_TOP, DIRECTION_BOTTOM, 0)) {
        map_adjacent_tiles[DIRECTION_TOP] = 1;
    }
    if (water_lift_side(DIRECTION_RIGHT, DIRECTION_LEFT, 1)) {
        map_adjacent_tiles[DIRECTION_RIGHT] = 1;
    }
    if (water_lift_side(DIRECTION_BOTTOM, DIRECTION_TOP, 0)) {
        map_adjacent_tiles[DIRECTION_BOTTOM] = 1;
    }
    if (water_lift_side(DIRECTION_LEFT, DIRECTION_RIGHT, 1)) {
        map_adjacent_tiles[DIRECTION_LEFT] = 1;
    }
}

void map_tiles_set_road_image(void)
{
    if (map_grids.building_ids[map_tile_cursor_offset] != 0 || (map_grids.bitfields[map_tile_cursor_offset] & MAP_BITFIELD_PLAZA_OR_FAULT)) {
        return;
    }
    int32_t image;
    if (all_roads_paved == 1) {
        map_image_context_match(context_paved_road, 48, -1, 0);
        map_image_context_normalize_tiles();
        image = image_group(GROUP_TERRAIN_ROAD) + context_item_offset + context_group_offset;
    } else if (!(map_grids.terrain[map_tile_cursor_offset] & TERRAIN_CANAL)) {
        if (!(map_grids.terrain[map_tile_cursor_offset] & TERRAIN_FLOODPLAIN) && floodplain_node_dry(map_tile_cursor_offset)) {
            if (!map_tiles_road_is_paved()) {
                map_image_context_match(context_dirt_road, 17, -1, 0);
                map_image_context_normalize_tiles();
                image = image_group(GROUP_TERRAIN_DIRT_ROAD) + context_item_offset + context_group_offset;
            } else {
                map_image_context_match(context_paved_road, 48, -1, 0);
                map_image_context_normalize_tiles();
                image = image_group(GROUP_TERRAIN_ROAD) + context_item_offset + context_group_offset;
            }
        } else {
            int32_t floodplain = image_group(GROUP_TERRAIN_FLOODPLAIN);
            int32_t current = map_grids.images[map_tile_cursor_offset] - floodplain;
            int32_t road = map_canal_with_road_image(current);
            if (current == road && (current < CANAL_IMAGE_ROAD_CROSSING_A || current > CANAL_IMAGE_ROAD_CROSSING_D)) {
                map_image_context_match(context_dirt_road, 17, -1, 0);
                map_image_context_normalize_tiles();
                image = context_item_offset + image_group(GROUP_TERRAIN_FLOODPLAIN_ROAD) + context_group_offset;
            } else {
                image = road + floodplain;
            }
        }
    } else {
        int32_t canal = image_group(GROUP_TERRAIN_CANAL);
        int32_t full = map_grids.images[map_tile_cursor_offset] - canal < CANAL_WATER_IMAGES ? 0 : CANAL_WATER_IMAGES;
        map_image_context_set_canal_road_tiles();
        map_image_context_match(context_canal, 16, -1, 0);
        if (context_canal_offset == 0) {
            if ((map_grids.terrain[map_tile_cursor_offset - 1] & TERRAIN_CANAL) || (map_grids.terrain[map_tile_cursor_offset + 1] & TERRAIN_CANAL)) {
                context_group_offset = 0;
            } else {
                context_group_offset = 1;
            }
        }
        if (map_tiles_road_is_paved()) {
            image = canal + context_group_offset + CANAL_ROAD_IMAGE_PAVED + full;
        } else {
            image = canal + context_group_offset + CANAL_ROAD_IMAGE_DIRT + full;
            if (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_FLOODPLAIN) {
                image = map_canal_floodplain_image(image);
            }
        }
    }
    map_grids.images[map_tile_cursor_offset] = image;
    map_grids.bitfields[map_tile_cursor_offset] &= ~MAP_BITFIELD_SIZE;
    map_grids.edge[map_tile_cursor_offset] |= MAP_EDGE_DRAW_TILE;
}

void map_tiles_update_region_roads(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to)
{
    int32_t x0 = x_from < 0 ? 0 : x_from;
    int32_t y0 = y_from < 0 ? 0 : y_from;
    int32_t x1 = x_to >= map_info.grid_width ? map_info.grid_width - 1 : x_to;
    int32_t y1 = y_to >= map_info.grid_height ? map_info.grid_height - 1 : y_to;
    map_tile_cursor_offset = tile_offset(x0, y0);
    for (map_tile_cursor_y = y0; map_tile_cursor_y <= y1; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x <= x1; map_tile_cursor_x++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if ((terrain & TERRAIN_ROAD) && !(terrain & (TERRAIN_WATER | TERRAIN_BUILDING))) {
                map_image_context_set_tiles(TERRAIN_ROADS);
                map_image_context_add_gatehouse_tiles();
                map_tiles_set_road_image();
            }
            map_tile_cursor_offset++;
        }
        map_tile_cursor_offset += map_info.grid_border_size + map_info.grid_width - x1 - 1 + x0;
    }
}

void map_tiles_update_all_roads(void)
{
    map_tiles_update_region_roads(0, 0, map_info.grid_width - 1, map_info.grid_height - 1);
}

int32_t map_image_context_set_tiles_gatehouse(void)
{
    map_adjacent_tiles[DIRECTION_TOP_RIGHT] = 0;
    map_adjacent_tiles[DIRECTION_BOTTOM_RIGHT] = 0;
    map_adjacent_tiles[DIRECTION_BOTTOM_LEFT] = 0;
    map_adjacent_tiles[DIRECTION_TOP_LEFT] = 0;
    map_adjacent_tiles[DIRECTION_TOP] = map_grids.terrain[map_tile_cursor_offset - GRID_SIZE] & TERRAIN_GATEHOUSE;
    map_adjacent_tiles[DIRECTION_RIGHT] = map_grids.terrain[map_tile_cursor_offset + 1] & TERRAIN_GATEHOUSE;
    map_adjacent_tiles[DIRECTION_BOTTOM] = map_grids.terrain[map_tile_cursor_offset + GRID_SIZE] & TERRAIN_GATEHOUSE;
    map_adjacent_tiles[DIRECTION_LEFT] = map_grids.terrain[map_tile_cursor_offset - 1] & TERRAIN_GATEHOUSE;
    for (int32_t direction = DIRECTION_TOP; direction < DIRECTION_COUNT; direction += 2) {
        if (map_adjacent_tiles[direction]) {
            return 1;
        }
    }
    return 0;
}

void map_image_context_add_tiles_wall(void)
{
    for (int32_t i = 0; i < DIRECTION_COUNT; i++) {
        map_adjacent_tiles[i] += ((map_grids.terrain[map_tile_cursor_offset + neighbour_delta[i]] & TERRAIN_WALL) != 0);
    }
}

static void redraw_wall_tile(void)
{
    map_image_context_set_tiles(TERRAIN_WALL);
    map_image_context_invert_tiles();
    map_image_context_match(context_wall, 48, -1, 0);
    int32_t offset = map_tile_cursor_offset;
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_SIZE;
    map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
    map_grids.images[offset] = image_group(GROUP_TERRAIN_WALL) + context_item_offset + context_group_offset;
    if (map_image_context_set_tiles_gatehouse()) {
        map_image_context_add_tiles_wall();
        if (map_image_context_match(context_wall_gate, 15, -1, 0)) {
            map_grids.images[map_tile_cursor_offset] = image_group(GROUP_TERRAIN_WALL) + context_item_offset + context_group_offset;
        }
    }
}

void map_tiles_update_all_walls(void)
{
    map_tile_cursor_offset = map_info.grid_start;
    for (map_tile_cursor_y = 0; map_tile_cursor_y < map_info.grid_height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = 0; map_tile_cursor_x < map_info.grid_width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if ((terrain & TERRAIN_WALL) && !(terrain & TERRAIN_BUILDING)) {
                redraw_wall_tile();
            }
        }
        map_tile_cursor_offset += map_info.grid_border_size;
    }
}

void map_tiles_update_region_walls(int32_t x, int32_t y, int32_t size)
{
    int32_t x0 = x - 1;
    int32_t width = size;
    int32_t y0 = y - 1;
    int32_t height = size;
    if (x0 < 0) {
        width = x0 + size;
        x0 = 0;
    } else if (map_info.grid_width < x0 + size) {
        width = map_info.grid_width - x0;
    }
    if (y0 < 0) {
        height = y0 + size;
        y0 = 0;
    } else if (map_info.grid_height < y0 + size) {
        height = map_info.grid_height - y0;
    }
    map_tile_cursor_offset = map_grid_offset(x0, y0);
    for (map_tile_cursor_y = y0; map_tile_cursor_y < y0 + height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x < x0 + width; map_tile_cursor_x++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if ((terrain & TERRAIN_WALL) && !(terrain & TERRAIN_BUILDING)) {
                redraw_wall_tile();
                map_tiles_update_empty_land_around(map_tile_cursor_x, map_tile_cursor_y, 1);
            }
            map_tile_cursor_offset++;
        }
        map_tile_cursor_offset += GRID_SIZE - width;
    }
}

static void redraw_canal(int32_t full)
{
    uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
    int32_t plaza_or_fault = map_grids.bitfields[map_tile_cursor_offset] & MAP_BITFIELD_PLAZA_OR_FAULT;
    if ((terrain & TERRAIN_CANAL) && !(terrain & (TERRAIN_WATER | TERRAIN_BUILDING)) && !plaza_or_fault) {
        map_image_context_set_canal_road_tiles();
        map_tiles_set_canal_image(full);
    }
}

void map_tiles_update_region_canal_images(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (map_info.grid_width <= x1) {
        x1 = map_info.grid_width - 1;
    }
    if (map_info.grid_height <= y1) {
        y1 = map_info.grid_height - 1;
    }
    int32_t row_adjust = map_info.grid_width - x1 + map_info.grid_border_size - 1 + x0;
    map_tile_cursor_offset = map_grid_offset(x0, y0);
    for (map_tile_cursor_y = y0; map_tile_cursor_y <= y1; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x <= x1; map_tile_cursor_x++) {
            redraw_canal(0);
            map_tile_cursor_offset++;
        }
        map_tile_cursor_offset += row_adjust;
    }
}

void map_tiles_update_region_canals(int32_t x_from, int32_t y_from, int32_t x_to, int32_t y_to, int32_t full)
{
    int32_t x0 = x_from <= x_to ? x_from : x_to;
    int32_t x1 = x_from <= x_to ? x_to : x_from;
    int32_t y0 = y_to < y_from ? y_to : y_from;
    int32_t y1 = y_to < y_from ? y_from : y_to;
    x0--;
    x1++;
    y0--;
    y1++;
    if (x0 < 0) {
        x0 = 0;
    }
    if (x1 >= map_info.grid_width) {
        x1 = map_info.grid_width - 1;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (y1 >= map_info.grid_height) {
        y1 = map_info.grid_height - 1;
    }
    map_tile_cursor_offset = tile_offset(x0, y0);
    for (map_tile_cursor_y = y0; map_tile_cursor_y <= y1; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x <= x1; map_tile_cursor_x++, map_tile_cursor_offset++) {
            uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
            if (!(terrain & TERRAIN_CANAL) || (terrain & (TERRAIN_WATER | TERRAIN_BUILDING)) || map_grids.canals[map_tile_cursor_offset] >= 16) {
                continue;
            }
            int32_t canal = image_group(GROUP_TERRAIN_CANAL);
            int32_t current = map_grids.images[map_tile_cursor_offset] - canal;
            int32_t water = (current >= CANAL_WATER_IMAGES || current < 0) ? CANAL_WATER_IMAGES : 0;
            if (full) {
                water = CANAL_WATER_IMAGES;
            }
            map_image_context_set_canal_road_tiles();
            map_image_context_match(context_canal, 16, -1, 0);
            if (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROAD) {
                map_grids.bitfields[map_tile_cursor_offset] &= ~MAP_BITFIELD_PLAZA_OR_FAULT;
                if (context_canal_offset == 0) {
                    context_group_offset = !(map_grids.terrain[map_tile_cursor_offset - GRID_SIZE] & TERRAIN_ROAD);
                }
                context_group_offset += map_tiles_road_is_paved() ? CANAL_ROAD_IMAGE_PAVED : CANAL_ROAD_IMAGE_DIRT;
            }
            int32_t image = canal + context_item_offset + context_group_offset + water;
            if (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_FLOODPLAIN) {
                image = map_canal_floodplain_image(image);
            }
            map_grids.images[map_tile_cursor_offset] = image;
            map_grids.bitfields[map_tile_cursor_offset] &= ~MAP_BITFIELD_SIZE;
            map_grids.edge[map_tile_cursor_offset] |= MAP_EDGE_DRAW_TILE;
            map_grids.canals[map_tile_cursor_offset] = (uint8_t) context_canal_offset;
        }
        map_tile_cursor_offset += GRID_SIZE - (x1 - x0 + 1);
    }
    construction_canals_changed = 0;
}

void map_tiles_set_canal_image(int32_t full)
{
    if (map_grids.building_ids[map_tile_cursor_offset] != 0) {
        return;
    }
    int32_t water = full ? CANAL_WATER_IMAGES : 0;
    map_image_context_set_canal_road_tiles();
    map_image_context_match(context_canal, 16, -1, 0);
    if (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROAD) {
        map_grids.bitfields[map_tile_cursor_offset] &= ~MAP_BITFIELD_PLAZA_OR_FAULT;
        if (context_canal_offset == 0) {
            context_group_offset = (map_grids.terrain[map_tile_cursor_offset - GRID_SIZE] & TERRAIN_ROADS) == 0;
        }
        context_group_offset += map_tiles_road_is_paved() ? CANAL_ROAD_IMAGE_PAVED : CANAL_ROAD_IMAGE_DIRT;
    }
    int32_t image = context_item_offset + image_group(GROUP_TERRAIN_CANAL) + context_group_offset + water;
    if (map_grids.terrain[map_tile_cursor_offset] & TERRAIN_FLOODPLAIN) {
        image = map_canal_floodplain_image(image);
    }
    map_grids.images[map_tile_cursor_offset] = image;
    map_grids.bitfields[map_tile_cursor_offset] &= ~MAP_BITFIELD_SIZE;
    map_grids.canals[map_tile_cursor_offset] = (uint8_t) context_canal_offset;
    map_grids.edge[map_tile_cursor_offset] |= MAP_EDGE_DRAW_TILE;
}

static void place_routed_tile(int32_t x, int32_t y, int32_t set, uint32_t bit, void (*redraw)(int32_t arg), int32_t arg)
{
    if (!map_grid_is_inside(map_info.diamond_x_min + x, map_info.diamond_y_min + y)) {
        return;
    }
    int32_t offset = tile_offset(x, y);
    uint32_t terrain = map_grids.terrain[offset];
    if (!(terrain & bit)) {
        routed_building_items_placed++;
    }
    map_grids.terrain[offset] = set == 1 ? terrain | bit : terrain & ~bit;
    map_grids.bitfields[offset] &= ~MAP_BITFIELD_OVERLAY;
    int32_t x0 = x - 1;
    int32_t y0 = y - 1;
    int32_t width = 3;
    int32_t height = 3;
    if (x0 < 0) {
        width = x + 2;
        x0 = 0;
    } else if (x + 2 > map_info.grid_width) {
        width = map_info.grid_width - x0;
    }
    if (y0 < 0) {
        height = y + 2;
        y0 = 0;
    } else if (y + 2 > map_info.grid_height) {
        height = map_info.grid_height - y0;
    }
    map_tile_cursor_offset = tile_offset(x0, y0);
    for (map_tile_cursor_y = y0; map_tile_cursor_y < y0 + height; map_tile_cursor_y++) {
        for (map_tile_cursor_x = x0; map_tile_cursor_x < x0 + width; map_tile_cursor_x++, map_tile_cursor_offset++) {
            redraw(arg);
        }
        map_tile_cursor_offset += GRID_SIZE - width;
    }
    map_tiles_update_empty_land_around(x0, y0, width);
}

static void redraw_road(int32_t arg)
{
    (void) arg;
    uint32_t terrain = map_grids.terrain[map_tile_cursor_offset];
    if ((terrain & TERRAIN_ROAD) && !(terrain & (TERRAIN_WATER | TERRAIN_BUILDING))) {
        map_image_context_set_tiles(TERRAIN_ROADS);
        map_image_context_add_gatehouse_tiles();
        map_tiles_set_road_image();
    }
}

void map_tiles_set_road(int32_t x, int32_t y, int32_t set)
{
    place_routed_tile(x, y, set, TERRAIN_ROAD, redraw_road, 0);
}

void map_tiles_set_canal(int32_t x, int32_t y, int32_t full, int32_t set)
{
    place_routed_tile(x, y, set, TERRAIN_CANAL, redraw_canal, full);
}

int32_t map_get_canal_with_road_image(int32_t grid_offset)
{
    static const uint8_t kind[CANAL_WATER_IMAGES] = { CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER,
        CANAL_PIECE_OTHER, CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_STRAIGHT, CANAL_PIECE_OTHER,
        CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_STRAIGHT,
        CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_CROSSING, CANAL_PIECE_CORNER, CANAL_PIECE_OTHER,
        CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_CORNER, CANAL_PIECE_CROSSING, CANAL_PIECE_CORNER, CANAL_PIECE_CROSSING,
        CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_OTHER, CANAL_PIECE_CROSSING, CANAL_PIECE_CORNER,
        CANAL_PIECE_CROSSING, CANAL_PIECE_CORNER, CANAL_PIECE_CROSSING, CANAL_PIECE_CORNER, CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER,
        CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER, CANAL_PIECE_STRAIGHT, CANAL_PIECE_STRAIGHT_OTHER };
    int32_t image = map_grids.images[grid_offset] - image_group(IMAGE_GROUP_CANAL);
    if (image >= CANAL_WATER_IMAGES) {
        image -= CANAL_WATER_IMAGES;
    }
    if ((uint32_t) image >= CANAL_WATER_IMAGES) {
        return 1;
    }
    switch (kind[image]) {
        case CANAL_PIECE_STRAIGHT:
            return map_tiles_road_desirability_paved(grid_offset) ? CANAL_ROAD_IMAGE_PAVED : CANAL_ROAD_IMAGE_DIRT;
        case CANAL_PIECE_STRAIGHT_OTHER:
            return map_tiles_road_desirability_paved(grid_offset) ? CANAL_ROAD_IMAGE_PAVED_OTHER : CANAL_ROAD_IMAGE_DIRT_OTHER;
        case CANAL_PIECE_CROSSING:
            return CANAL_ROAD_IMAGE_CROSSING;
        case CANAL_PIECE_CORNER:
            return CANAL_ROAD_IMAGE_DIRT_OTHER;
        default:
            return 1;
    }
}
