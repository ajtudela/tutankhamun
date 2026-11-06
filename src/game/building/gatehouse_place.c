// Gatehouses on the build tool: the four parts at their offsets for the orientation, the preview marks, the site
// check and the ghost (tower images by god, or red/green tiles); and the open water check of a dock.

#include "building/gatehouse_place.h"
#include "building/building_industry.h"
#include "city/gods.h"
#include "graphics/image_collection.h"
#include "map/construction_tiles.h"
#include "map/grid.h"
#include "map/routing_water.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "map/tiles_internal.h"
#include "map/water.h"
#include "platform/platform_input.h"
#include "widget/construction_ghost.h"
#include <stdbool.h>
#include <stdint.h>

static inline int32_t gatehouse_part_order(int32_t orientation, int32_t part)
{
    return gatehouse_orientation_parts[(orientation * GATEHOUSE_PART_COUNT) + part];
}

static inline int32_t gatehouse_part_group(int32_t part)
{
    return complex_part_groups[part];
}

static inline int32_t gatehouse_tower_group(int32_t god)
{
    return complex_part_groups[GATEHOUSE_PART_COUNT + god];
}

int32_t building_dock_is_connected_to_open_water(int32_t x, int32_t y)
{
    map_routing_calculate_distances_water_boat(scenario_data.river_entry_x, scenario_data.river_entry_y);
    return map_terrain_is_adjacent_to_open_water(x, y, DOCK_SIZE) != 0;
}

static void part_tile(const gatehouse_part *part, int32_t orientation, int32_t x, int32_t y, int32_t *out_x, int32_t *out_y)
{
    if (orientation == 0) {
        *out_x = part->dx + x;
        *out_y = part->dy + y;
    } else if (orientation == 1) {
        *out_x = part->dy + x;
        *out_y = part->dx + y;
    } else {
        *out_x = x;
        *out_y = y;
    }
}

static void part_view_origin(int32_t size, int32_t *x, int32_t *y)
{
    int32_t view = map_info.orientation;
    if (view == DIRECTION_TOP) {
        return;
    }
    if (view == DIRECTION_RIGHT) {
        *x += size - 1;
    } else if (view == DIRECTION_BOTTOM) {
        *x += size - 1;
        *y += size - 1;
    } else {
        *y += size - 1;
    }
}

void building_gatehouse_mark_construction(int32_t x, int32_t y, int32_t orientation)
{
    for (int32_t i = 0; i < GATEHOUSE_PART_COUNT; i++) {
        int32_t index = gatehouse_part_order(orientation, i);
        const gatehouse_part *part = &gatehouse_parts.parts[index];
        int32_t px, py;
        part_tile(part, orientation, x, y, &px, &py);
        part_view_origin(part->size, &px, &py);
        map_building_tiles_mark_construction(part->size, px, py, ~(uint32_t) TERRAIN_ROAD, 0, 0);
    }
}

static int32_t part_tile_fits(const gatehouse_part *part, int32_t x, int32_t y)
{
    if ((uint8_t) part->on_road != 0 && map_building_tiles_shrub_fits(x, y, 1, 1, TERRAIN_ROAD, 0, 0) == 0) {
        return 1;
    }
    return map_building_tiles_shrub_fits(x, y, 1, 1, 0xFFFFFFFF, 0, 0) != 0;
}

bool building_gatehouse_site_fits(int32_t origin_x, int32_t origin_y, int32_t orientation)
{
    for (int32_t p = 0; p < GATEHOUSE_PART_COUNT; p++) {
        const gatehouse_part *part = &gatehouse_parts.parts[p];
        int32_t x, y;
        part_tile(part, orientation, origin_x, origin_y, &x, &y);
        int32_t size = part->size;
        for (int32_t i = 0; i < size; i++, x++) {
            for (int32_t j = 0; j < size; j++) {
                if (!part_tile_fits(part, x, y + j)) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void building_gatehouse_draw_flat_tiles(int32_t x0, int32_t y0, int32_t orientation)
{
    for (int32_t p = 0; p < GATEHOUSE_PART_COUNT; p++) {
        const gatehouse_part *part = &gatehouse_parts.parts[p];
        int32_t x, y;
        part_tile(part, orientation, x0, y0, &x, &y);
        int32_t size = part->size;
        int32_t px, py;
        city_view_tile_to_pixel(x, y, &px, &py);
        for (int32_t i = 0; i < size; i++, x++) {
            for (int32_t j = 0; j < size; j++) {
                city_view_tile_to_pixel(x, y + j, &px, &py);
                int32_t color = part_tile_fits(part, x, y + j) ? GHOST_COLOR_FREE : (int32_t) 0xFFFFF863;
                ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), px, py, color);
            }
        }
    }
}

void city_building_ghost_draw_gatehouse(void)
{
    int32_t god = city_gods_with_status(GOD_STATUS_PATRON);
    if (god == -1) {
        god = city_gods_with_status(GOD_STATUS_LOCAL);
        if (god == -1) {
            god = 0;
        }
    }
    int32_t tower_group = gatehouse_tower_group(god);
    int32_t x = map_info.hover_x;
    int32_t y = map_info.hover_y;
    if (platform_key_state('R') == (int16_t) 0x8001) {
        construction_orientation = construction_orientation != 1;
    }
    if (!building_gatehouse_site_fits(x, y, construction_orientation)) {
        construction_place_ok = 0;
        building_gatehouse_draw_flat_tiles(x, y, construction_orientation);
        return;
    }
    int32_t orientation = map_info.orientation + construction_orientation;
    if (orientation >= DIRECTION_COUNT) {
        orientation -= DIRECTION_COUNT;
    }
    for (int32_t i = 0; i < GATEHOUSE_PART_COUNT; i++) {
        int32_t index = gatehouse_part_order(orientation, i);
        int32_t image_offset;
        switch (orientation) {
            case 0:
            case 3:
            case 4:
            case 7:
                image_offset = 1;
                break;
            default:
                image_offset = 0;
                break;
        }
        const gatehouse_part *part = &gatehouse_parts.parts[index];
        int32_t px, py;
        part_tile(part, construction_orientation, x, y, &px, &py);
        part_view_origin(part->size, &px, &py);
        int32_t group = gatehouse_part_group(i);
        if (group == GATEHOUSE_TOWER_PLACEHOLDER_GROUP) {
            group = tower_group;
        }
        int32_t image = image_group(group) + image_offset;
        int32_t sx, sy;
        city_view_tile_to_pixel(px, py, &sx, &sy);
        city_view.color_mask = GHOST_COLOR_FREE;
        ghost_image_draw(image, sx, sy);
        ghost_image_draw_top(image, sx, sy);
        city_view.color_mask = 0;
    }
}
