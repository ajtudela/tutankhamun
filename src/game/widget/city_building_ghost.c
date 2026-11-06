// The construction ghost of the city view: the cost under the cursor (red when the treasury does not cover
// it), the dispatcher to the ghost of the building type being placed, and the ghosts drawn here: venues, roads,
// canals, bridges and the regular buildings, whose footprint is checked against the map edge, terrain needs,
// unique buildings, floodplain, blocked tiles and figures, then drawn green or tile by tile red and green.
//
// Two thresholds look off but are what players see:
// - the treasury counts as broke at -4999 or less;
// - a canal ghost over a road takes its image variant from the canal grid 224 rows below the tile.

#include "widget/city_building_ghost.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "building/construction_venue.h"
#include "building/gatehouse_place.h"
#include "building/library.h"
#include "building/pyramid_images.h"
#include "building/royal_tomb.h"
#include "city/city_player.h"
#include "editor/editor_terrain_tiles.h"
#include "editor/editor_terrain_tool.h"
#include "figure/animals.h"
#include "game/game_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_blit_mask.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/text_draw.h"
#include "map/bridge.h"
#include "map/construction_terrain.h"
#include "map/farm.h"
#include "map/grid.h"
#include "map/image_context.h"
#include "map/map_data.h"
#include "map/routing_citizen.h"
#include "map/routing_distance.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles.h"
#include "map/tiles_road.h"
#include "platform/platform_input.h"
#include "widget/city_building_anims.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/construction_ghost_ferry.h"
#include "widget/construction_ghost_fort.h"
#include "widget/construction_ghost_monuments.h"
#include "widget/construction_ghost_small.h"
#include "widget/construction_ghost_sphinx.h"
#include "widget/construction_ghost_statue.h"
#include "widget/construction_ghost_temple_complex.h"
#include "widget/construction_ghost_temple_upgrade.h"
#include "widget/widget_data.h"
#include "window/window_image_group.h"
#include <stdint.h>
#include <string.h>

enum city_building_ghost {
    REGULAR_GHOST_TREASURY_BROKE = -4999,
    GHOST_MAP_EDGE = 3,
    GHOST_EDITOR_POINT_EDGE = 4,
    GHOST_LAND_INVASION_POINTS = 9,
    COST_COLOR_AFFORDABLE = 0xFAC1,
    COST_COLOR_TOO_DEAR = 0xF800,
    SHIP_BRIDGE_MIN_LENGTH = 5,
    FARM_GHOST_SIZE = 3,
    FARM_FERTILITY_PER_IMAGE = 14,
    STORAGE_YARD_GHOST_SIZE = 3,
    STORAGE_YARD_SPACES = 8,
    CANAL_STRAIGHT_IMAGES = 48,
    CANAL_CONTEXT_TILES = 16,
    GHOST_CANAL_ORIENTATION_ROWS = 224,
    PAVILION_STAGE_SIZE = 2,
};

typedef struct {
    int32_t num_tiles;
    int32_t allow_flood;
    const int32_t *grid_offsets;
} ghost_footprint;

enum bridge_draw_record {
    BRIDGE_DRAW_IMAGE,
    BRIDGE_DRAW_X,
    BRIDGE_DRAW_Y,
    BRIDGE_DRAW_RECORD_INTS = 4,
};

static int32_t treasury_broke(void)
{
    return city_player_current()->treasury <= REGULAR_GHOST_TREASURY_BROKE;
}

static int32_t building_occupied(int32_t grid_offset)
{
    const flood_tile *tile = floods_data.tiles[grid_offset];
    return tile != 0 && tile->is_floodplain == 0;
}

static int32_t building_image(int32_t type)
{
    const building_property *property = building_property_get(type);
    return image_group(property->image_group) + property->image_offset;
}

static void draw_green(int32_t image, int32_t x, int32_t y)
{
    city_view.color_mask = GHOST_COLOR_FREE;
    ghost_image_draw(image, x, y);
    ghost_image_draw_top(image, x, y);
    city_view.color_mask = 0;
}

static int32_t near_map_edge(int32_t x, int32_t y, int32_t edge)
{
    return map_grid_is_inside_size(map_info.diamond_x_min + x, map_info.diamond_y_min + y, edge) != 0;
}

static int32_t is_editor_point(int32_t type)
{
    return type == BUILDING_EDITOR_INVASION_POINT || type == BUILDING_EDITOR_FISHING_WATERS || type == BUILDING_EDITOR_KILLER_POINT ||
        type == BUILDING_EDITOR_ENTRY_POINT || type == BUILDING_EDITOR_EXIT_POINT || type == BUILDING_EDITOR_EARTHQUAKE || type == BUILDING_EDITOR_RIVER_IN ||
        type == BUILDING_EDITOR_RIVER_OUT || type == BUILDING_EDITOR_PREY;
}

void city_building_ghost_draw_construction_cost(void)
{
    int32_t px, py;
    if (map_info.hover_offset == 0 || scroll_in_progress != 0 || construction_cost == 0) {
        return;
    }
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    g_clip_x0 = city_view.viewport_x;
    g_clip_x1 = city_view.viewport_x1;
    g_clip_y0 = city_view.viewport_y;
    g_clip_y1 = city_view.viewport_y1;
    py += city_view.tile_half_height;
    int32_t color = construction_cost <= city_player_current()->treasury ? COST_COLOR_AFFORDABLE : COST_COLOR_TOO_DEAR;
    int32_t y = py - city_view.tile_half_height;
    text_draw_number(construction_cost, NUMBER_PREFIX_NONE, postfix_none, city_view.tile_width + 1 + px, y + 1, image_group(IMAGE_GROUP_FONT_SMALL), 0);
    text_draw_number(construction_cost, NUMBER_PREFIX_NONE, postfix_none, city_view.tile_width + px, y, image_group(IMAGE_GROUP_FONT_SMALL), color);
    construction_cost = 0;
    g_clip_x0 = 0;
    g_clip_x1 = g_screen_width;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}

static const int32_t *venue_tiles(int32_t type, int32_t view)
{
    switch (type) {
        case BUILDING_BANDSTAND:
            return ghost_bandstand_tiles[view];
        case BUILDING_BOOTH:
            return venue_booth_tiles[view];
        case BUILDING_PAVILION:
            return venue_pavilion_tiles[view];
        default:
            return ghost_festival_tiles[view];
    }
}

static int32_t venue_base_image(int32_t type)
{
    switch (type) {
        case BUILDING_BANDSTAND:
            return image_group(IMAGE_GROUP_BANDSTAND_PLAZA);
        case BUILDING_BOOTH:
            return image_group(IMAGE_GROUP_BOOTH);
        case BUILDING_PAVILION:
            return image_group(IMAGE_GROUP_PAVILION_PLAZA);
        default:
            return image_group(IMAGE_GROUP_FESTIVAL_SQUARE);
    }
}

static int32_t venue_image_for_tile(int32_t type, int32_t size, int32_t col, int32_t row)
{
    if (type != BUILDING_BANDSTAND && type != BUILDING_BOOTH && type != BUILDING_PAVILION && type != BUILDING_FESTIVAL_SQUARE) {
        return image_group(IMAGE_GROUP_VENUE_PLAZA);
    }
    return venue_tiles(type, map_info.orientation / VIEW_ORIENTATION_STEP)[(row * size) + col] + venue_base_image(type);
}

static int32_t venue_size(int32_t type)
{
    switch (type) {
        case BUILDING_BOOTH:
            return 2;
        case BUILDING_BANDSTAND:
            return 3;
        case BUILDING_PAVILION:
            return 4;
        case BUILDING_FESTIVAL_SQUARE:
            return 5;
        default:
            return 1;
    }
}

void city_building_ghost_draw_venue(int32_t type)
{
    int32_t size = venue_size(type);
    int32_t origin_x = map_info.hover_x;
    int32_t origin_y = map_info.hover_y;
    int32_t band_x = origin_x, band_y = origin_y;
    int32_t booth_x = origin_x, booth_y = origin_y;
    int32_t pavilion_x = origin_x, pavilion_y = origin_y;
    int32_t band_orient = 0;
    int32_t booth_result = 0;
    uint8_t blocked;
    uint8_t want_booth = 0, want_band = 0, want_pavilion = 0;
    int32_t row, col, ty;
    int32_t px, py, img_a, img_b;

    if (construction_venue_can_place(type) == 0) {
        city_view.color_mask = GHOST_COLOR_BLOCKED;
        construction_place_ok = 0;
        blocked = 1;
    } else {
        city_view.color_mask = GHOST_COLOR_FREE;
        construction_place_ok = 1;
        blocked = 0;
    }
    construction_venue_fill_road_grid(origin_x, origin_y, type);

    if (type == BUILDING_BANDSTAND) {
        want_band = 1;
        want_booth = 1;
    } else if (type == BUILDING_BOOTH) {
        want_booth = 1;
    } else if (type == BUILDING_PAVILION) {
        want_pavilion = 1;
        want_band = 1;
        want_booth = 1;
    }

    if (want_pavilion) {
        booth_result = construction_venue_take_spot(origin_x, origin_y, &pavilion_x, &pavilion_y, type, BUILDING_PAVILION);
        if (booth_result == -1) {
            blocked = 1;
        }
    }
    if (want_band) {
        band_orient = construction_venue_take_spot(origin_x, origin_y, &band_x, &band_y, type, BUILDING_BANDSTAND);
        if (band_orient == -1) {
            blocked = 1;
        }
    }
    if (want_booth) {
        booth_result = construction_venue_take_spot(origin_x, origin_y, &booth_x, &booth_y, type, BUILDING_BOOTH);
    }
    if (booth_result == -1) {
        blocked = 1;
    }

    ty = origin_y;
    for (row = 0; row < size; row++) {
        for (col = 0; col < size; col++) {
            int32_t tx = col + origin_x;
            city_view_tile_to_pixel(tx, ty, &px, &py);
            if (blocked) {
                ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px, py, GHOST_COLOR_BLOCKED);
                continue;
            }
            if (want_booth && tx == booth_x && ty == booth_y) {
                ghost_image_draw(image_group(IMAGE_GROUP_BOOTH), px, py);
                ghost_image_draw_top(image_group(IMAGE_GROUP_BOOTH), px, py);
                continue;
            }
            if (want_band) {
                if (tx == band_x && ty == band_y) {
                    venue_orientation_images(band_orient, &img_a, &img_b);
                    ghost_image_draw(img_a, px, py);
                    ghost_image_draw_top(img_a, px, py);
                    continue;
                }
                if (band_orient != 0) {
                    if (tx == band_x + 1 && ty == band_y) {
                        venue_orientation_images(band_orient, &img_a, &img_b);
                        ghost_image_draw(img_b, px, py);
                        ghost_image_draw_top(img_b, px, py);
                        continue;
                    }
                } else if (tx == band_x && ty == band_y + 1) {
                    venue_orientation_images(0, &img_a, &img_b);
                    ghost_image_draw(img_b, px, py);
                    ghost_image_draw_top(img_b, px, py);
                    continue;
                }
            }
            if (want_pavilion) {
                if (tx == pavilion_x && ty == pavilion_y) {
                    int32_t vx = px, vy = py;
                    if (map_info.orientation == VIEW_ORIENTATION_EAST) {
                        city_view_tile_to_pixel(col + 1 + origin_x, ty, &vx, &vy);
                    } else if (map_info.orientation == VIEW_ORIENTATION_SOUTH) {
                        city_view_tile_to_pixel(col + 1 + origin_x, ty + 1, &vx, &vy);
                    } else if (map_info.orientation == VIEW_ORIENTATION_WEST) {
                        city_view_tile_to_pixel(tx, ty + 1, &vx, &vy);
                    }
                    ghost_image_draw(image_group(IMAGE_GROUP_PAVILION), vx, vy);
                    ghost_image_draw_top(image_group(IMAGE_GROUP_PAVILION), vx, vy);
                    continue;
                }
                if (tx >= pavilion_x && ty >= pavilion_y && tx < pavilion_x + PAVILION_STAGE_SIZE && ty < pavilion_y + PAVILION_STAGE_SIZE) {
                    continue;
                }
            }
            ghost_image_draw(venue_image_for_tile(type, size, col, row), px, py);
        }
        ty++;
    }
    city_view.color_mask = 0;
    construction_place_ok = blocked ? 0 : 1;
}

static void bridge_record_after_draw(void)
{
    int32_t i = city_view.bridge_draw_count;
    city_view.bridge_draw_buf[(i * BRIDGE_DRAW_RECORD_INTS) + BRIDGE_DRAW_IMAGE] = image_draw_last_id;
    city_view.bridge_draw_buf[(i * BRIDGE_DRAW_RECORD_INTS) + BRIDGE_DRAW_X] = image_draw_last_x;
    city_view.bridge_draw_buf[(i * BRIDGE_DRAW_RECORD_INTS) + BRIDGE_DRAW_Y] = image_draw_last_y;
    city_view.bridge_draw_count = i + 1;
}

static void ghost_draw_road(void)
{
    int32_t px, py;
    int32_t blocked = 0;
    int32_t grid = map_info.hover_offset;
    uint32_t terrain = map_grids.terrain[grid];
    int32_t image = image_group(IMAGE_GROUP_ROAD);
    int32_t ypix;

    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    py += city_view.tile_half_height;
    ypix = py;

    if ((terrain & TERRAIN_FLOODPLAIN) != 0 || building_occupied(grid)) {
        image = image_group(IMAGE_GROUP_CANAL_ROAD);
        if (map_can_place_road_on_canal_object(grid) == 0) {
            blocked = 1;
        } else {
            image = image_group(IMAGE_GROUP_CANAL_ROAD) + map_canal_with_road_image(map_grids.images[grid] - image_group(IMAGE_GROUP_CANAL_ROAD));
        }
    }
    terrain &= TERRAIN_NOT_CLEAR;
    if ((terrain & TERRAIN_CANAL) != 0) {
        image = image_group(IMAGE_GROUP_CANAL);
        if (map_can_place_road_under_canal(grid) == 0) {
            blocked = 1;
        } else {
            image = image_group(IMAGE_GROUP_CANAL) + map_get_canal_with_road_image(grid);
        }
    } else if (terrain != 0) {
        blocked = 1;
    } else {
        image = image_group(IMAGE_GROUP_ROAD_PAVED);
        if ((map_grids.bitfields[grid] & MAP_BITFIELD_PLAZA_OR_FAULT) == 0) {
            image = image_group(IMAGE_GROUP_ROAD);
        }
        if (map_terrain_has_adjacent_y_with(map_info.hover_offset, TERRAIN_ROAD) == 0 &&
            map_terrain_has_adjacent_x_with(map_info.hover_offset, TERRAIN_ROAD) != 0) {
            image++;
        }
    }
    if (treasury_broke()) {
        blocked = 1;
    }

    if (near_map_edge(map_info.hover_x, map_info.hover_y, GHOST_MAP_EDGE) || blocked) {
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px, ypix - city_view.tile_half_height, GHOST_COLOR_BLOCKED);
        return;
    }
    draw_green(image, px, ypix - city_view.tile_half_height);
}

static void ghost_draw_canal(void)
{
    int32_t px, py;
    int32_t blocked = 0;
    int32_t image_off;
    int32_t ypix;

    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    py += city_view.tile_half_height;

    if (construction_in_progress == 0) {
        map_tile_cursor_offset = map_info.grid_start + (map_info.hover_y * GRID_SIZE) + map_info.hover_x;
        if ((map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROAD) != 0) {
            blocked = map_can_place_canal_on_road(map_tile_cursor_offset) == 0;
        } else if ((map_grids.terrain[map_tile_cursor_offset] & TERRAIN_NOT_CLEAR) == 0 && !building_occupied(map_tile_cursor_offset)) {
            blocked = 0;
        } else {
            blocked = 1;
        }
    } else if (construction_cost == 0) {
        blocked = 1;
    }

    if (treasury_broke()) {
        blocked = 1;
    }
    if (near_map_edge(map_info.hover_x, map_info.hover_y, GHOST_MAP_EDGE)) {
        blocked = 1;
    }
    ypix = py - city_view.tile_half_height;
    if (blocked) {
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px, ypix, GHOST_COLOR_BLOCKED);
        return;
    }
    city_view.color_mask = GHOST_COLOR_FREE;
    map_tile_cursor_offset = map_info.hover_offset;
    map_image_context_set_canal_road_tiles();
    map_image_context_match(context_canal, CANAL_CONTEXT_TILES, -1, 0);
    if ((map_grids.terrain[map_tile_cursor_offset] & TERRAIN_ROAD) == 0) {
        image_off = context_group_offset + CANAL_STRAIGHT_IMAGES;
    } else {
        if (context_canal_offset == 0) {
            context_group_offset = ((~map_grids.canals[(GHOST_CANAL_ORIENTATION_ROWS * GRID_SIZE) + map_tile_cursor_offset]) >> 6) & 1;
        }
        if ((map_grids.bitfields[map_tile_cursor_offset] & 0x80) == 0) {
            map_tiles_road_is_paved();
            image_off = context_group_offset + CANAL_STRAIGHT_IMAGES;
        } else {
            image_off = map_grids.images[map_tile_cursor_offset] - image_group(IMAGE_GROUP_CANAL);
        }
    }
    ghost_image_draw(image_group(IMAGE_GROUP_CANAL) + image_off, px, ypix);
    city_view.color_mask = 0;
}

static void ghost_advance_bridge_pixel(int32_t dir, int32_t *x, int32_t *y)
{
    if (dir == DIRECTION_TOP) {
        *x += city_view.tile_half_width;
        *y -= city_view.tile_half_height;
    } else if (dir == DIRECTION_RIGHT) {
        *x += city_view.tile_half_width;
        *y += city_view.tile_half_height;
    } else if (dir == DIRECTION_BOTTOM) {
        *x -= city_view.tile_half_width;
        *y += city_view.tile_half_height;
    } else if (dir == DIRECTION_LEFT) {
        *x -= city_view.tile_half_width;
        *y -= city_view.tile_half_height;
    }
}

static const bridge_ghost_piece_set *low_bridge_pieces(int32_t dir)
{
    int32_t set = dir == DIRECTION_TOP || dir == DIRECTION_RIGHT || dir == DIRECTION_BOTTOM ? dir / VIEW_ORIENTATION_STEP : BRIDGE_GHOST_DIRECTIONS - 1;
    return &bridge_ghost_pieces[set];
}

static void draw_low_bridge(int32_t dir, int32_t length, int32_t x, int32_t y)
{
    const bridge_ghost_piece_set *pieces = low_bridge_pieces(dir);
    ghost_image_draw_masked(image_group(IMAGE_GROUP_BRIDGE) + pieces->low_start, x, y, GHOST_COLOR_FREE);
    ghost_image_draw(image_group(IMAGE_GROUP_BRIDGE) + pieces->low_start, x, y);
    bridge_record_after_draw();
    for (int32_t i = 1; i < length; i++) {
        ghost_advance_bridge_pixel(dir, &x, &y);
        ghost_image_draw(image_group(IMAGE_GROUP_BRIDGE) + (i == length - 1 ? pieces->low_end : pieces->low_span), x, y);
        bridge_record_after_draw();
    }
}

static int32_t ship_bridge_middle(int32_t length)
{
    if (length >= 9 && length <= 12) {
        return 4;
    }
    if (length == 13 || length == 14) {
        return 6;
    }
    if (length == 15 || length == 16) {
        return 7;
    }
    return 8;
}

static void draw_ship_piece(int32_t dir, const bridge_ghost_piece *piece, int32_t x, int32_t y)
{
    if (dir % VIEW_ORIENTATION_STEP == 0) {
        ghost_image_draw(image_group(IMAGE_GROUP_BRIDGE) + piece->image, x + piece->dx, y + piece->dy);
    }
}

static void draw_ship_bridge(int32_t dir, int32_t length, int32_t x, int32_t y)
{
    const bridge_ghost_piece_set *pieces = &bridge_ghost_pieces[(dir / VIEW_ORIENTATION_STEP) % BRIDGE_GHOST_DIRECTIONS];
    int32_t middle = ship_bridge_middle(length);
    draw_ship_piece(dir, &pieces->ship_start, x, y);
    bridge_record_after_draw();
    for (int32_t i = 1; i < length; i++) {
        ghost_advance_bridge_pixel(dir, &x, &y);
        if (i != length - 1 && i != 1) {
            if (i == length - 2) {
                draw_ship_piece(dir, &pieces->ship_end, x, y);
            } else if (i == middle) {
                draw_ship_piece(dir, &pieces->ship_middle, x, y);
            } else {
                draw_ship_piece(dir, &pieces->ship_span, x, y);
            }
        }
        bridge_record_after_draw();
    }
}

static void ghost_draw_bridge(void)
{
    int32_t px, py;
    int32_t dir, length, blocked_flag, x, y;
    int32_t type = construction_building_type;

    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    py += city_view.tile_half_height;
    map_bridge_calculate_length_direction(map_info.hover_x, map_info.hover_y);

    dir = bridge_direction - map_info.orientation;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
    }
    length = construction_bridge_length;
    if (type == BUILDING_SHIP_BRIDGE && length < SHIP_BRIDGE_MIN_LENGTH) {
        blocked_flag = 1;
    } else {
        blocked_flag = bridge_end_grid_offset == 0;
    }

    if (treasury_broke() || blocked_flag) {
        x = px;
        y = py - city_view.tile_half_height;
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), x, y, length < 1 ? GHOST_COLOR_BLOCKED : GHOST_COLOR_FREE);
        if (length < 2) {
            return;
        }
        for (int32_t i = 1; i < length; i++) {
            ghost_advance_bridge_pixel(dir, &x, &y);
        }
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), x, y, GHOST_COLOR_BLOCKED);
        return;
    }

    city_view.color_mask = GHOST_COLOR_FREE;
    y = (py - city_view.tile_height) - city_view.tile_half_height;
    x = px;
    city_view.bridge_draw_count = 0;

    if (type == BUILDING_LOW_BRIDGE) {
        draw_low_bridge(dir, length, x, y);
    } else {
        draw_ship_bridge(dir, length, x, y);
    }

    if (dir == DIRECTION_TOP || dir == DIRECTION_LEFT) {
        for (int32_t k = city_view.bridge_draw_count - 1; k >= 0; k--) {
            const int32_t *record = &city_view.bridge_draw_buf[k * BRIDGE_DRAW_RECORD_INTS];
            ghost_image_draw(record[BRIDGE_DRAW_IMAGE], record[BRIDGE_DRAW_X], record[BRIDGE_DRAW_Y]);
        }
    }
    city_view.color_mask = 0;
}

static void ghost_footprint_origin(int32_t size, int32_t *x, int32_t *y)
{
    *x = map_info.hover_x;
    *y = map_info.hover_y;
    if (map_info.orientation == VIEW_ORIENTATION_EAST) {
        *x = map_info.hover_x - size + 1;
    } else if (map_info.orientation == VIEW_ORIENTATION_SOUTH) {
        *x = map_info.hover_x - size + 1;
        *y = map_info.hover_y - size + 1;
    } else if (map_info.orientation != VIEW_ORIENTATION_NORTH) {
        *y = map_info.hover_y - size + 1;
    }
}

static int32_t ghost_outside_map(int32_t size)
{
    return map_info.hover_x < GHOST_MAP_EDGE || map_info.grid_width - GHOST_MAP_EDGE <= map_info.hover_x - 1 + size ||
        map_info.hover_y < GHOST_MAP_EDGE || map_info.grid_height - GHOST_MAP_EDGE <= map_info.hover_y - 1 + size;
}

static int32_t editor_point_site_ok(int32_t type)
{
    int32_t ok = near_map_edge(map_info.hover_x, map_info.hover_y, GHOST_EDITOR_POINT_EDGE);
    if (type == BUILDING_EDITOR_INVASION_POINT) {
        if (editor_invasion_point_number >= GHOST_LAND_INVASION_POINTS && (map_grids.terrain[map_info.hover_offset] & TERRAIN_WATER) == 0) {
            ok = 0;
        }
    } else if (type == BUILDING_EDITOR_RIVER_IN || type == BUILDING_EDITOR_RIVER_OUT) {
        if (map_terrain_all_adjacent_are_water(map_info.hover_offset) == 0) {
            ok = 0;
        }
    }
    return ok;
}

static int32_t ghost_terrain_needs_met(int32_t type, int32_t size, int32_t x, int32_t y)
{
    if (needs_farm_land != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, FARM_GHOST_SIZE, 1, (TERRAIN_MEADOW | TERRAIN_FLOODPLAIN)) != 0;
    }
    if (construction_near_terrain_test != 0) {
        return map_terrain_any_tile_in_radius(x, y, 2, 1, construction_near_terrain_test) != 0;
    }
    if (needs_trees != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, 2, 1, TERRAIN_TREE) != 0;
    }
    if (needs_water_near != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, 2, 3, (TERRAIN_WATER | TERRAIN_FLOODPLAIN)) != 0;
    }
    if (needs_marsh != 0) {
        return map_terrain_exists_tile_in_radius_with_type(x, y, 2, 1, TERRAIN_MARSH) != 0;
    }
    if (needs_wall != 0) {
        return map_terrain_all_tiles_in_radius_are(x, y, 2, 0, TERRAIN_WALL) != 0;
    }
    if (needs_road != 0) {
        return map_terrain_all_tiles_in_radius_are(x, y, 1, 0, TERRAIN_ROAD) != 0;
    }
    if (needs_groundwater != 0) {
        return map_terrain_count_tiles_in_radius_reach(x, y, size, 0, TERRAIN_GROUNDWATER, needs_groundwater) != 0;
    }
    if (type == BUILDING_EDITOR_INVASION_POINT || type == BUILDING_EDITOR_ENTRY_POINT || type == BUILDING_EDITOR_EXIT_POINT ||
        type == BUILDING_EDITOR_RIVER_IN || type == BUILDING_EDITOR_RIVER_OUT) {
        return editor_point_site_ok(type);
    }
    if (type == BUILDING_EDITOR_FISHING_WATERS) {
        return (map_grids.terrain[map_info.hover_offset] & TERRAIN_WATER) != 0;
    }
    if (type == BUILDING_EDITOR_KILLER_POINT || type == BUILDING_EDITOR_PREY) {
        return animal_terrain_allowed_climate(map_grids.terrain[map_info.hover_offset], type == BUILDING_EDITOR_PREY) != 0;
    }
    if (type == BUILDING_DISEMBARK_POINT) {
        return editor_tile_deep_water_edge(map_info.hover_x, map_info.hover_y) != 0;
    }
    return 1;
}

static int32_t ghost_unique_building_placed(int32_t type)
{
    const city_player *player = city_player_current();
    if (type == BUILDING_VILLAGE_PALACE_UP || type == BUILDING_TOWN_PALACE_UP || type == BUILDING_CITY_PALACE) {
        return player->palace_placed != 0;
    }
    if (type == BUILDING_PERSONAL_MANSION || type == BUILDING_FAMILY_MANSION || type == BUILDING_DYNASTY_MANSION) {
        return player->mansion_placed != 0;
    }
    if (type == BUILDING_RECRUITER) {
        return building_count_total[BUILDING_RECRUITER] != 0;
    }
    if (type == BUILDING_DISTRIBUTION_CENTER) {
        return player->distribution_center_placed != 0;
    }
    if (type == BUILDING_MILITARY_ACADEMY) {
        return player->military_academy_placed != 0;
    }
    return 0;
}

static int32_t ghost_site_blocked(int32_t type, int32_t size, int32_t x, int32_t y)
{
    int32_t blocked = 0;
    if (editor_active == 0 && ghost_outside_map(size)) {
        blocked = 1;
    }
    if (map_terrain_exists_tile_in_radius_with_type(x, y, size, 1, TERRAIN_DIKE) != 0) {
        blocked = 1;
    }
    if (!ghost_terrain_needs_met(type, size, x, y)) {
        blocked = 1;
    }
    if (ghost_unique_building_placed(type)) {
        blocked = 1;
    }
    return blocked;
}

static uint32_t ghost_tile_blocking_terrain(int32_t type, uint32_t terrain)
{
    uint32_t mask = terrain & TERRAIN_NOT_CLEAR;
    if (type == BUILDING_MUD_TOWER || type == BUILDING_BRICK_TOWER || type == BUILDING_TOWER) {
        mask = terrain & (TERRAIN_NOT_CLEAR & ~TERRAIN_WALL_KINDS);
    }
    if (type == BUILDING_TRIUMPHAL_ARCH || type == BUILDING_PLAZA) {
        mask &= ~TERRAIN_ROAD;
    } else if (type == BUILDING_ROADBLOCK) {
        mask &= TERRAIN_BUILDING;
    } else if (type == BUILDING_EDITOR_ENTRY_POINT || type == BUILDING_EDITOR_EXIT_POINT || type == BUILDING_EDITOR_EARTHQUAKE) {
        mask &= ~TERRAIN_ROAD;
    }
    if (type == BUILDING_EDITOR_INVASION_POINT) {
        return editor_invasion_point_number < GHOST_LAND_INVASION_POINTS ? mask & ~TERRAIN_ROAD : 0;
    }
    if (type == BUILDING_EDITOR_RIVER_IN || type == BUILDING_EDITOR_RIVER_OUT || type == BUILDING_EDITOR_FISHING_WATERS || type == BUILDING_EDITOR_PREY ||
        type == BUILDING_EDITOR_KILLER_POINT || type == BUILDING_DISEMBARK_POINT) {
        return 0;
    }
    return mask;
}

static int32_t ghost_tiles_blocked(int32_t type, const ghost_footprint *footprint, int32_t *fully_blocked)
{
    int32_t blocked = 0;
    int32_t has_flood = 0;
    int32_t has_clear = 0;
    for (int32_t i = 0; i < footprint->num_tiles; i++) {
        int32_t offset = footprint->grid_offsets[i] + map_info.hover_offset;
        if (building_occupied(offset)) {
            *fully_blocked = 1;
            blocked = 1;
            continue;
        }
        uint32_t terrain = map_grids.terrain[offset];
        if ((terrain & TERRAIN_FLOODPLAIN) != 0) {
            if (has_clear || !footprint->allow_flood) {
                blocked = 1;
            }
            has_flood = 1;
        } else {
            if (has_flood) {
                blocked = 1;
            }
            has_clear = 1;
        }
        if (ghost_tile_blocking_terrain(type, terrain) != 0 || map_grids.figure_ids[offset] != 0) {
            blocked = 1;
        }
    }
    return blocked;
}

static int32_t ghost_road_building_blocked(int32_t type, const ghost_footprint *footprint, int32_t blocked)
{
    int32_t first_tile = footprint->grid_offsets[0] + map_info.hover_offset;
    if (type == BUILDING_PLAZA) {
        if ((map_grids.terrain[first_tile] & TERRAIN_ROAD) == 0) {
            blocked = 1;
        }
        map_tile_cursor_offset = map_info.hover_offset;
        if (map_tiles_road_is_paved() == 0) {
            blocked = 1;
        }
    }
    if (type == BUILDING_ROADBLOCK) {
        uint32_t terrain = map_grids.terrain[first_tile];
        if ((terrain & TERRAIN_ROAD) == 0 || (terrain & TERRAIN_FLOODPLAIN) != 0 || map_grids.sprite[map_info.hover_offset] != 0) {
            blocked = 1;
        }
    }
    return blocked;
}

static int32_t ghost_obelisk_image(void)
{
    if (trade_goods_list.list.beauty_variant == 1) {
        return image_group(IMAGE_GROUP_SPHINX_END_A);
    }
    if (trade_goods_list.list.beauty_variant == 2) {
        return image_group(IMAGE_GROUP_SPHINX_MIDDLE);
    }
    if (trade_goods_list.list.beauty_variant == 3) {
        return image_group(IMAGE_GROUP_SPHINX_END_B);
    }
    return 0;
}

static void ghost_draw_storage_yard(int32_t image_id, int32_t px, int32_t y_base)
{
    int32_t y = y_base - city_view.tile_half_height;
    city_view.color_mask = GHOST_COLOR_FREE;
    ghost_image_draw(image_id, px, y);
    ghost_image_draw_top(image_id, px, y);
    building_draw_storage_yard_roof(image_group(IMAGE_GROUP_STORAGE_YARD) + STORAGE_YARD_ROOF_IMAGE, px, y);
    for (int32_t i = 0; i < STORAGE_YARD_SPACES; i++) {
        int32_t space_x = px + construction_ghost_tile_offsets[i + 1].x;
        int32_t space_y = (construction_ghost_tile_offsets[i + 1].y - city_view.tile_half_height) + y_base;
        ghost_image_draw(image_group(IMAGE_GROUP_STORAGE_YARD_SPACE), space_x, space_y);
        ghost_image_draw_top(image_group(IMAGE_GROUP_STORAGE_YARD_SPACE), space_x, space_y);
    }
    city_view.color_mask = 0;
}

static void ghost_draw_fitting(int32_t type, int32_t image_id, int32_t num_tiles, int32_t px, int32_t y_base)
{
    int32_t y = y_base - city_view.tile_half_height;
    if (is_editor_point(type) || type == BUILDING_DISEMBARK_POINT) {
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px, y, GHOST_COLOR_FREE);
    } else if (type == BUILDING_REED_GATHERER) {
        draw_green(image_group(IMAGE_GROUP_REED_GATHERER), px, y);
    } else if (type == BUILDING_CATTLE_RANCH) {
        draw_green(image_group(IMAGE_GROUP_CATTLE_RANCH), px, y);
    } else if (building_type_is_farm(type)) {
        if (map_terrain_has_floodplain(map_info.hover_x, map_info.hover_y, FARM_GHOST_SIZE, FARM_GHOST_SIZE)) {
            image_id = (map_fertility_average(map_info.hover_x, map_info.hover_y, FARM_GHOST_SIZE, FARM_GHOST_SIZE) / FARM_FERTILITY_PER_IMAGE) +
                image_group(IMAGE_GROUP_FLOODPLAIN_FARM);
        } else {
            image_id = image_group(IMAGE_GROUP_MEADOW_FARM);
        }
        draw_green(image_id, px, y);
    } else if (type == BUILDING_SMALL_OBELISK || type == BUILDING_LARGE_OBELISK) {
        draw_green(ghost_obelisk_image(), px, y);
    } else if (type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP) {
        ghost_draw_storage_yard(image_id, px, y_base);
    } else if (type == BUILDING_HOUSE_CRUDE_HUT) {
        draw_green(image_group(IMAGE_GROUP_VACANT_LOT), px, y);
    } else if (type > BUILDING_HOUSE_CRUDE_HUT && type < BUILDING_BANDSTAND && type != BUILDING_GRANARY && type != BUILDING_GRANARY_UP) {
        for (int32_t i = 0; i < num_tiles; i++) {
            ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px + ghost_flat_tile_dx(i), (ghost_flat_tile_dy(i) - city_view.tile_half_height) + y_base, GHOST_COLOR_FREE);
        }
    } else if (type != BUILDING_TRIUMPHAL_ARCH) {
        draw_green(image_id, px, y);
    }
}

static int32_t ghost_tile_color(int32_t type, int32_t offset, int32_t fully_blocked, int32_t allow_flood)
{
    uint32_t terrain = map_grids.terrain[offset];
    uint32_t flood = terrain & TERRAIN_FLOODPLAIN;
    if (type == BUILDING_PLAZA) {
        map_tile_cursor_offset = offset;
        if (map_tiles_road_is_paved() == 0) {
            terrain = 1;
        }
    }
    uint32_t mask = terrain & TERRAIN_NOT_CLEAR;
    if (type == BUILDING_EDITOR_INVASION_POINT || type == BUILDING_EDITOR_ENTRY_POINT || type == BUILDING_EDITOR_EXIT_POINT ||
        type == BUILDING_EDITOR_EARTHQUAKE) {
        mask = terrain & (TERRAIN_NOT_CLEAR & ~TERRAIN_ROAD);
    }
    if (map_grids.figure_ids[offset] != 0) {
        mask = 1;
    }
    if (fully_blocked || mask != 0 || building_occupied(offset) || (flood != 0 && !allow_flood)) {
        return GHOST_COLOR_BLOCKED;
    }
    return GHOST_COLOR_FREE;
}

static void ghost_draw_tiles(int32_t type, const ghost_footprint *footprint, int32_t fully_blocked, int32_t px, int32_t y_base)
{
    for (int32_t i = 0; i < footprint->num_tiles; i++) {
        int32_t x = ghost_flat_tile_dx(i) + px;
        int32_t y = (ghost_flat_tile_dy(i) - city_view.tile_half_height) + y_base;
        int32_t color = ghost_tile_color(type, footprint->grid_offsets[i] + map_info.hover_offset, fully_blocked, footprint->allow_flood);
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), x, y, color);
    }
}

static void ghost_draw_regular(void)
{
    int32_t type = construction_building_type;
    if (construction_orientation > 0 && ghost_rotate_key_went_down()) {
        construction_orientation = construction_orientation == 1 ? 2 : 1;
    }
    int32_t size = type == BUILDING_STORAGE_YARD ? STORAGE_YARD_GHOST_SIZE : building_property_get(type)->size;
    int32_t image_id = building_image(type);
    if (type >= BUILDING_SMALL_STATUE && type <= BUILDING_LARGE_STATUE) {
        image_id += construction_orientation;
    }
    int32_t base_x;
    int32_t base_y;
    ghost_footprint_origin(size, &base_x, &base_y);
    int32_t fully_blocked = ghost_site_blocked(type, size, base_x, base_y);

    int32_t px;
    int32_t py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t y_base = py + city_view.tile_half_height;
    ghost_footprint footprint;
    footprint.num_tiles = ghost_tiles_of_size(size);
    footprint.allow_flood = construction_allowed_on_floodplain(type);
    footprint.grid_offsets = tile_grid_offsets_base.grid_offsets[map_info.orientation / VIEW_ORIENTATION_STEP];

    int32_t blocked = ghost_tiles_blocked(type, &footprint, &fully_blocked) || fully_blocked;
    blocked = ghost_road_building_blocked(type, &footprint, blocked);

    if (treasury_broke()) {
        fully_blocked = 1;
    } else if (blocked == 0) {
        ghost_draw_fitting(type, image_id, footprint.num_tiles, px, y_base);
        return;
    }
    ghost_draw_tiles(type, &footprint, fully_blocked, px, y_base);
}

void city_building_ghost_draw(void)
{
    int32_t type = construction_building_type;

    if (type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB) {
        construction_placing_royal_tomb = 1;
    } else {
        construction_placing_royal_tomb = 0;
    }

    if (map_info.hover_offset == 0) {
        return;
    }
    if (scroll_in_progress != 0 && construction_hover_locked == 0) {
        return;
    }
    if (construction_place_ok != 0) {
        return;
    }
    if (type < 1) {
        return;
    }
    if (editor_active == 1) {
        if (!is_editor_point(type) && type != BUILDING_DISEMBARK_POINT && type != BUILDING_NATIVE_HUT && type != BUILDING_NATIVE_MEETING &&
            type != BUILDING_NATIVE_CROPS && type != BUILDING_EDITOR_ACCESS_RAMP) {
            return;
        }
    }

    if (type == BUILDING_MENU_WATER_LIFT || type == BUILDING_WATER_LIFT) {
        city_building_ghost_draw_shore2(1, 1);
        return;
    }
    if (type == BUILDING_IRRIGATION_DITCH) {
        ghost_draw_canal();
        return;
    }
    if (type == BUILDING_LOW_BRIDGE || type == BUILDING_SHIP_BRIDGE) {
        ghost_draw_bridge();
        return;
    }
    if (type == BUILDING_FORT_CHARIOTEERS || type == BUILDING_FORT_ARCHERS || type == BUILDING_FORT_INFANTRY) {
        city_building_ghost_draw_fort();
        return;
    }
    if (type == BUILDING_SPHINX) {
        city_building_ghost_draw_sphinx();
        return;
    }
    if (type == BUILDING_SUN_TEMPLE) {
        sun_temple_draw_placement();
        return;
    }
    if (type == BUILDING_ALEXANDRIA_LIBRARY) {
        library_draw_placement(&library_object.base);
        return;
    }
    if (type == BUILDING_ABU_SIMBEL) {
        abu_simbel_draw_placement(&abu_simbel_object.base);
        return;
    }
    if (type == BUILDING_CAESAREUM) {
        caesareum_draw_placement(&caesareum_object.base);
        return;
    }
    if (type == BUILDING_SMALL_ROYAL_TOMB) {
        small_royal_tomb_draw_placement(&small_royal_tomb_object.base);
        return;
    }
    if (type == BUILDING_MEDIUM_ROYAL_TOMB) {
        medium_royal_tomb_draw_placement(&medium_royal_tomb_object.base);
        return;
    }
    if (type == BUILDING_LARGE_ROYAL_TOMB) {
        large_royal_tomb_draw_placement(&large_royal_tomb_object.base);
        return;
    }
    if (type == BUILDING_GRAND_ROYAL_TOMB) {
        grand_royal_tomb_draw_placement(&grand_royal_tomb_object.base);
        return;
    }
    if (type == BUILDING_LIGHTHOUSE) {
        lighthouse_draw_placement(&lighthouse_object.base);
        return;
    }
    if (type == BUILDING_MAUSOLEUM) {
        mausoleum_draw_placement();
        return;
    }
    if (type == BUILDING_SHIPWRIGHT) {
        city_building_ghost_draw_shore3(IMAGE_GROUP_SHIPWRIGHT);
        return;
    }
    if (type == BUILDING_PYRAMID) {
        pyramid_draw_construction_preview();
        return;
    }
    if (type == BUILDING_FISHING_WHARF || type == BUILDING_TRANSPORT_WHARF) {
        city_building_ghost_draw_shore2(1, 0);
        return;
    }
    if (type == BUILDING_WARSHIP_WHARF) {
        city_building_ghost_draw_shore3(IMAGE_GROUP_WARSHIP_WHARF);
        return;
    }
    if (type == BUILDING_DOCK) {
        city_building_ghost_draw_shore3(IMAGE_GROUP_DOCK);
        return;
    }
    if (type == BUILDING_DOCK_UP) {
        city_building_ghost_draw_shore3(IMAGE_GROUP_DOCK_UP);
        return;
    }
    if (type == BUILDING_ROAD) {
        ghost_draw_road();
        return;
    }
    if (type == BUILDING_EDITOR_ACCESS_RAMP) {
        city_building_ghost_draw_single();
        return;
    }
    if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        city_building_ghost_draw_temple_complex(type);
        return;
    }
    if (type == BUILDING_TEMPLE_COMPLEX_ALTAR || type == BUILDING_TEMPLE_COMPLEX_ORACLE) {
        city_building_ghost_draw_temple_upgrade(type);
        return;
    }
    if (type == BUILDING_GATEHOUSE || type == BUILDING_GATEHOUSE_UP) {
        city_building_ghost_draw_gatehouse();
        return;
    }
    if (type >= BUILDING_SMALL_STATUE && type <= BUILDING_LARGE_STATUE) {
        city_building_ghost_draw_statue();
        return;
    }
    if (type == BUILDING_FERRY_LANDING) {
        city_building_ghost_draw_ferry();
        return;
    }
    if (type == BUILDING_BOOTH || type == BUILDING_PAVILION || type == BUILDING_BANDSTAND || type == BUILDING_FESTIVAL_SQUARE) {
        city_building_ghost_draw_venue(type);
        return;
    }
    ghost_draw_regular();
}
