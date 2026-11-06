// The statues' ghost (sizes 1 to 3): R steps the statue's variant, which picks the image set and image; the
// site must be away from the map edge, off the floodplain, on open land and without figures.

#include "widget/construction_ghost_statue.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/construction_place.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum statue_variant {
    STATUE_VARIANTS = 16,
    STATUE_IMAGE_SETS = 4,
    STATUE_OWN_IMAGE_SETS = 2,
    STATUE_IMAGES_PER_SET = 4,
    STATUE_MAP_EDGE = 3,
};

static int32_t statue_image(int32_t size, int32_t variant)
{
    int32_t set = variant % STATUE_IMAGE_SETS;
    int32_t index = (variant + STATUE_IMAGES_PER_SET) / STATUE_IMAGES_PER_SET;
    if (index >= STATUE_IMAGES_PER_SET) {
        index = 0;
    }
    int32_t image;
    if (set < STATUE_OWN_IMAGE_SETS) {
        image = image_group(building_property_get(construction_building_type)->image_group) + (set * STATUE_IMAGES_PER_SET);
    } else {
        int32_t base = (size == 1) ? image_group(IMAGE_GROUP_SMALL_STATUE_VARIANTS) : (size == 2) ? image_group(IMAGE_GROUP_MEDIUM_STATUE_VARIANTS) : image_group(IMAGE_GROUP_LARGE_STATUE_VARIANTS);
        image = base + ((set - STATUE_OWN_IMAGE_SETS) * STATUE_IMAGES_PER_SET);
    }
    return image + index;
}

static int32_t near_map_edge(int32_t size)
{
    return map_info.hover_x < STATUE_MAP_EDGE || map_info.hover_x + size - 1 >= map_info.grid_width - STATUE_MAP_EDGE ||
        map_info.hover_y < STATUE_MAP_EDGE || map_info.hover_y + size - 1 >= map_info.grid_height - STATUE_MAP_EDGE;
}

void city_building_ghost_draw_statue(void)
{
    int32_t blocked = 0;
    int32_t all_red = 0;
    int32_t px, py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t py_raised = py + city_view.tile_half_height;
    int32_t size = building_property_get(construction_building_type)->size;
    int32_t hover = map_info.hover_offset;
    if (ghost_rotate_key_went_down()) {
        construction_orientation++;
    }
    if (construction_orientation >= STATUE_VARIANTS) {
        construction_orientation = 0;
    }
    int32_t image = statue_image(size, construction_orientation);
    if (editor_active == 0 && near_map_edge(size)) {
        all_red = 1;
        blocked = 1;
    }
    int32_t tiles = ghost_tiles_of_size(size);
    int32_t view = map_info.orientation / 2;
    for (int32_t i = 0; i < tiles; i++) {
        int32_t offset = ghost_view_tile_offset(view, i) + hover;
        uint32_t terrain = map_grids.terrain[offset];
        const flood_tile *node = floods_data.tiles[offset];
        if ((terrain & TERRAIN_FLOODPLAIN) || (node != 0 && node->is_floodplain == 0)) {
            blocked = 1;
            all_red = 1;
        }
        if (terrain & TERRAIN_NOT_CLEAR) {
            blocked = 1;
        }
        if (map_grids.figure_ids[offset] != 0) {
            blocked = 1;
        }
    }
    if (city_player_current()->treasury <= GHOST_TREASURY_BROKE) {
        all_red = 1;
    } else if (!blocked) {
        int32_t y = py_raised - city_view.tile_half_height;
        city_view.color_mask = GHOST_COLOR_FREE;
        ghost_image_draw(image, px, y);
        ghost_image_draw_top(image, px, y);
        city_view.color_mask = 0;
        return;
    }
    for (int32_t i = 0; i < tiles; i++) {
        int32_t offset = hover + ghost_view_tile_offset(view, i);
        uint32_t not_free = map_grids.terrain[offset] & TERRAIN_NOT_CLEAR;
        if (map_grids.figure_ids[offset] != 0) {
            not_free = 1;
        }
        int32_t color = all_red ? GHOST_COLOR_BLOCKED : not_free ? GHOST_COLOR_BLOCKED : GHOST_COLOR_FREE;
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) - city_view.tile_half_height + py_raised, color);
    }
}
