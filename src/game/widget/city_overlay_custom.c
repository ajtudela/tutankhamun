// The overlays that draw their own tiles in the city view's footprint and top passes: water, desirability,
// fertility, the goods overlays and the open tombs. Natural terrain, roads and canals keep their own image.
//
// In the water footprint of a walkable building the groundwater test compares the whole terrain word, so it never
// matches.

#include "widget/city_overlay_custom.h"
#include "building/building.h"
#include "building/building_data.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_building_tiles.h"
#include "widget/city_overlay_heights.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include <stdint.h>

enum overlay_terrain_mask {
    TERRAIN_SHOWN_UNDER_GOODS_OVERLAY = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_ELEVATION |
        TERRAIN_ACCESS_RAMP | TERRAIN_RUBBLE | TERRAIN_MARSH | TERRAIN_ORE | TERRAIN_GEM_ROCK | TERRAIN_DUNE | TERRAIN_DEEPWATER | TERRAIN_ROCK_NEAR_MONUMENT,
    TERRAIN_SHOWN_UNDER_OVERLAY = TERRAIN_SHOWN_UNDER_GOODS_OVERLAY | TERRAIN_CANAL,
    TERRAIN_WATER_SHADE = TERRAIN_GROUNDWATER | TERRAIN_WELL_RANGE,
    TERRAIN_OPEN_TOMB = TERRAIN_ROCK | TERRAIN_ROCK_NEAR_MONUMENT,
};

enum water_shade {
    WATER_SHADE_NONE = 0,
    WATER_SHADE_GROUNDWATER = 1,
    WATER_SHADE_WELL_RANGE = 2,
};

enum overlay_level {
    DESIRABILITY_MIN = -10,
    DESIRABILITY_TOP = 70,
    DESIRABILITY_STEP = 10,
    DESIRABILITY_LEVEL_ZERO = 2,
    DESIRABILITY_LEVEL_TOP = 9,
    FERTILITY_TOP = 90,
    FERTILITY_STEP = 10,
    FERTILITY_LEVEL_TOP = 9,
};

enum tile_count {
    TILES_SIZE2 = 4,
    TILES_SIZE3 = 9,
    TILES_SIZE4 = 16,
    TILES_SIZE5 = 25,
    TILES_SIZE6 = 36,
};

static int32_t tile_size(void)
{
    return map_grids.bitfields[city_view.grid_offset] & MAP_BITFIELD_SIZE;
}

static int32_t grass_image(void)
{
    return (map_grids.random[city_view.grid_offset] & (ALTERNATE_GRASS_IMAGES - 1)) + image_group(IMAGE_GROUP_GRASS);
}

static void draw_at_size(overlay_image_draw_fn draw, int32_t image, int32_t x, int32_t y, int32_t size, int32_t others_at_tile)
{
    switch (size) {
        case 0:
            draw(image, x, y);
            return;
        case 1:
            draw(image, city_view.tile_half_width + 1 + x, y - city_view.tile_half_height);
            return;
        case 2:
            draw(image, city_view.tile_width + 2 + x, y - city_view.tile_height);
            return;
        case 3:
            draw(image, city_view.size3_x + 3 + x, y - city_view.size3_y);
            return;
        case 4:
            draw(image, city_view.size4_x + 4 + x, y - city_view.size4_y);
            return;
        case 5:
            draw(image, city_view.size4_x + city_view.tile_half_width + 5 + x, (y - city_view.size4_y) - city_view.tile_half_height);
            return;
        default:
            if (others_at_tile) {
                draw(image, x, y);
            }
            return;
    }
}

static void draw_tile_image(overlay_image_draw_fn draw, int32_t x, int32_t y, int32_t others_at_tile)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    draw_at_size(draw, g_last_image_id, x, y, tile_size(), others_at_tile);
}

static const int32_t *tile_offsets(int32_t size, int32_t *count, int32_t *own)
{
    switch (size) {
        case 1:
            *count = TILES_SIZE2;
            return tile_grid_offsets_base.size2_pixels;
        case 2:
            *count = TILES_SIZE3;
            return tile_grid_offsets_base.size3_pixels;
        case 3:
            *count = TILES_SIZE4;
            return tile_grid_offsets_base.size4_pixels;
        case 4:
            *count = TILES_SIZE5;
            return tile_grid_offsets_base.size5_pixels;
        case 5:
            *count = TILES_SIZE6;
            return tile_grid_offsets_base.size6_pixels;
        default:
            own[0] = 0;
            own[1] = 0;
            *count = 1;
            return own;
    }
}

static int32_t water_image(int32_t base, uint32_t water, int32_t house_has_water)
{
    if ((water & TERRAIN_WELL_RANGE) == 0 && !house_has_water) {
        return water == TERRAIN_GROUNDWATER ? base + WATER_SHADE_GROUNDWATER : base + WATER_SHADE_NONE;
    }
    return base + WATER_SHADE_WELL_RANGE;
}

static int32_t water_tiles_group(const building *b)
{
    return image_group(b->house_size != 0 ? IMAGE_GROUP_WATER_TILES_HOUSE : IMAGE_GROUP_WATER_TILES);
}

static void draw_water_tiles(overlay_image_draw_fn draw, int32_t image, int32_t x, int32_t y)
{
    int32_t own[2];
    int32_t count;
    const int32_t *offsets = tile_offsets(tile_size(), &count, own);
    for (; count != 0; count--, offsets += 2) {
        draw(image, offsets[0] + x, offsets[1] + y);
    }
}

static int32_t house_has_water(const building *b)
{
    return b->house_size != 0 && b->data.house.water_supply != 0;
}

void city_overlay_water_tiles(int32_t x, int32_t y, int32_t water, int8_t top)
{
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (house_has_water(b)) {
        city_building_draw_tiles(building_id, x, y, 0);
        return;
    }
    int32_t image = water_image(water_tiles_group(b), (uint32_t) water, 0);
    draw_water_tiles(top ? city_overlay_draw_image_top : city_overlay_draw_image, image, x, y);
}

void city_overlay_water_footprint(int32_t x, int32_t y)
{
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    if ((terrain & TERRAIN_SHOWN_UNDER_OVERLAY) != 0) {
        if ((terrain & TERRAIN_BUILDING) != 0) {
            g_last_image_id = image_group(IMAGE_GROUP_TERRAIN_OVERLAY);
            city_building_draw_tiles(map_grids.building_ids[city_view.grid_offset], x, y, 0);
            return;
        }
        draw_tile_image(city_overlay_draw_image, x, y, 0);
        return;
    }
    if ((terrain & TERRAIN_WALKABLE_BUILDING) != 0) {
        int32_t building_id = map_grids.building_ids[city_view.grid_offset];
        const building *b = building_get(building_id);
        if (house_has_water(b)) {
            city_building_draw_tiles(building_id, x, y, 0);
            return;
        }
        int32_t image = water_image(water_tiles_group(b), terrain, 0);
        draw_water_tiles(city_overlay_draw_image, image, x, y);
        return;
    }
    if ((terrain & TERRAIN_WALL) != 0) {
        g_last_image_id = grass_image();
        image_draw(g_last_image_id, x, y);
        return;
    }
    if ((terrain & TERRAIN_BUILDING) == 0) {
        g_last_image_id = water_image(image_group(IMAGE_GROUP_WATER_TILES), terrain & TERRAIN_WATER_SHADE, 0);
        image_draw(g_last_image_id, x, y);
        return;
    }
    if (city_with_overlay_show_building(map_grids.building_ids[city_view.grid_offset]) == 0) {
        city_overlay_water_tiles(x, y, (int32_t) (terrain & TERRAIN_WATER_SHADE), 0);
        return;
    }
    draw_tile_image(city_overlay_draw_image, x, y, 1);
}

void city_overlay_water_top(int32_t x, int32_t y)
{
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    if ((terrain & TERRAIN_SHOWN_UNDER_OVERLAY) != 0) {
        if ((terrain & TERRAIN_BUILDING) != 0) {
            return;
        }
        draw_tile_image(city_overlay_draw_image_top, x, y, 0);
        return;
    }
    if ((terrain & TERRAIN_BUILDING) == 0) {
        g_last_image_id = water_image(image_group(IMAGE_GROUP_WATER_TILES), terrain & TERRAIN_WATER_SHADE, 0);
        image_draw_isometric_top(g_last_image_id, x, y);
        return;
    }
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (house_has_water(b)) {
        city_building_draw_tiles(building_id, x, y, 0);
        return;
    }
    int32_t image = water_image(water_tiles_group(b), terrain & TERRAIN_WATER_SHADE, 0);
    draw_water_tiles(city_overlay_draw_image_top, image, x, y);
}

static int32_t is_open_tomb_tile(void)
{
    return (map_grids.terrain[city_view.grid_offset] & TERRAIN_OPEN_TOMB) == TERRAIN_OPEN_TOMB;
}

void city_overlay_tombs_footprint(int32_t x, int32_t y)
{
    if (!is_open_tomb_tile()) {
        draw_tile_image(city_overlay_draw_image, x, y, 0);
        return;
    }
    g_last_image_id = image_group(IMAGE_GROUP_WATER_TILES);
    int32_t own[2] = { 0, 0 };
    const int32_t *offsets = own;
    int32_t count = 1;
    int32_t size = tile_size();
    if (size == 1) {
        offsets = tile_grid_offsets_base.size2_pixels;
        count = TILES_SIZE2;
    } else if (size == 2) {
        offsets = tile_grid_offsets_base.size3_pixels;
        count = TILES_SIZE3;
    }
    for (; count != 0; count--, offsets += 2) {
        image_draw(g_last_image_id, x + offsets[0], offsets[1] + y);
    }
}

void city_overlay_tombs_top(int32_t x, int32_t y)
{
    if (!is_open_tomb_tile()) {
        draw_tile_image(city_overlay_draw_image_top, x, y, 1);
        return;
    }
    g_last_image_id = image_group(IMAGE_GROUP_WATER_TILES);
    image_draw_isometric_top(g_last_image_id, x, y);
}

int32_t city_overlay_draw_image(int32_t image, int32_t x, int32_t y)
{
    return image_draw(image, x, y);
}

int32_t city_overlay_draw_image_top(int32_t image, int32_t x, int32_t y)
{
    return image_draw_isometric_top(image, x, y);
}

static int32_t desirability_level(int32_t value)
{
    if (value < DESIRABILITY_MIN) {
        return 0;
    }
    return value < DESIRABILITY_TOP ? value / DESIRABILITY_STEP + DESIRABILITY_LEVEL_ZERO : DESIRABILITY_LEVEL_TOP;
}

void city_overlay_desirability(int32_t x, int32_t y, int8_t top)
{
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    overlay_image_draw_fn draw = top ? city_overlay_draw_image_top : city_overlay_draw_image;
    int32_t value = map_grids.desirability[city_view.grid_offset];
    if ((terrain & TERRAIN_SHOWN_UNDER_OVERLAY) != 0 && (terrain & TERRAIN_BUILDING) == 0) {
        if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) == 0) {
            return;
        }
        draw_tile_image(draw, x, y, 0);
        return;
    }
    if ((terrain & TERRAIN_WALL) != 0) {
        g_last_image_id = grass_image();
        draw(g_last_image_id, x, y);
        return;
    }
    if ((terrain & (TERRAIN_BUILDING | TERRAIN_GARDEN)) == 0 && value == 0) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        draw(g_last_image_id, x, y);
        return;
    }
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    int32_t group = IMAGE_GROUP_DESIRABILITY;
    if (building_id != 0 && building_get(building_id)->house_size != 0) {
        group = IMAGE_GROUP_DESIRABILITY_HOUSE;
    }
    draw(image_group(group) + desirability_level(value), x, y);
}

void city_overlay_desirability_top(int32_t x, int32_t y)
{
    city_overlay_desirability(x, y, 1);
}

void city_overlay_resource(int32_t x, int32_t y, int32_t resource, int32_t slot)
{
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    if ((terrain & TERRAIN_SHOWN_UNDER_GOODS_OVERLAY) != 0 && (terrain & TERRAIN_BUILDING) == 0) {
        if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) == 0) {
            return;
        }
        draw_tile_image(city_overlay_draw_image, x, y, 0);
        return;
    }
    if ((terrain & (TERRAIN_CANAL | TERRAIN_WALL)) != 0) {
        g_last_image_id = grass_image();
        image_draw(g_last_image_id, x, y);
        return;
    }
    if ((terrain & TERRAIN_BUILDING) == 0) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        image_draw(g_last_image_id, x, y);
        return;
    }
    g_last_image_id = image_group(IMAGE_GROUP_OVERLAY_LEVELS) + city_overlay_resource_level(building_id, resource, slot);
    image_draw(g_last_image_id, x, y);
}

static int32_t fertility_level(uint8_t fertility)
{
    return fertility < FERTILITY_TOP ? fertility / FERTILITY_STEP + 1 : FERTILITY_LEVEL_TOP;
}

static void fertility(int32_t x, int32_t y, overlay_image_draw_fn draw)
{
    uint8_t fertility = map_grids.soil_fertility[city_view.grid_offset];
    if (fertility != 0) {
        g_last_image_id = image_group(IMAGE_GROUP_OVERLAY_LEVELS) + fertility_level(fertility);
        draw(g_last_image_id, x, y);
        return;
    }
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    if ((terrain & TERRAIN_SHOWN_UNDER_GOODS_OVERLAY) == 0 || (terrain & TERRAIN_BUILDING) != 0) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        draw(g_last_image_id, x, y);
        return;
    }
    if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) != 0) {
        draw_tile_image(draw, x, y, 0);
    }
}

void city_overlay_fertility_footprint(int32_t x, int32_t y)
{
    fertility(x, y, city_overlay_draw_image);
}

void city_overlay_fertility_top(int32_t x, int32_t y)
{
    fertility(x, y, city_overlay_draw_image_top);
}
