// The ghost of a temple complex upgrade (altar or oracle) over the city's complex: drawn green at the part
// that marks the upgrade's place, or red when there is no complex, it has too few upgrade levels or the upgrade
// already, or the city is broke.

#include "widget/construction_ghost_temple_upgrade.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/temple_complex.h"
#include "city/city_player.h"
#include "graphics/image_collection.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_view_rotate.h"
#include "widget/construction_ghost.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum temple_upgrade_ghost {
    UPGRADE_FOOTPRINT_SIZE = 3,
    UPGRADE_LEVELS_NEEDED = 2,
    UPGRADE_VIEW_SHIFT = 2,
    UPGRADE_OTHER_PLACE_IMAGE = 3,
    ALTAR_ON_PLACE = 2,
    ALTAR_ON_OTHER = 3,
    ORACLE_ON_PLACE = 0,
    ORACLE_ON_OTHER = 1,
};

static void view_shift(int32_t *dx, int32_t *dy)
{
    *dx = 0;
    *dy = 0;
    if (map_info.orientation == VIEW_ORIENTATION_EAST) {
        *dx = UPGRADE_VIEW_SHIFT;
    } else if (map_info.orientation == VIEW_ORIENTATION_SOUTH) {
        *dx = UPGRADE_VIEW_SHIFT;
        *dy = UPGRADE_VIEW_SHIFT;
    } else if (map_info.orientation == VIEW_ORIENTATION_WEST) {
        *dy = UPGRADE_VIEW_SHIFT;
    }
}

static int32_t upgrade_blocked(int32_t type, int16_t part_type)
{
    const city_player *player = city_player_current();
    int32_t blocked = player->has_temple_complex == 0;
    if (part_type < BUILDING_TEMPLE_COMPLEX_OSIRIS || part_type > BUILDING_TEMPLE_COMPLEX_BAST) {
        blocked = 1;
    }
    if (player->temple_complex_upgrades < UPGRADE_LEVELS_NEEDED) {
        blocked = 1;
    }
    if (building_temple_complex_has_upgrade(
            player->temple_complex_building_id, (type == BUILDING_TEMPLE_COMPLEX_ALTAR) ? TEMPLE_COMPLEX_ALTAR : TEMPLE_COMPLEX_ORACLE)) {
        blocked = 1;
    }
    return blocked;
}

static void draw_upgrade_on_complex(int32_t type, int32_t part)
{
    int32_t dx;
    int32_t dy;
    view_shift(&dx, &dy);
    city_view.color_mask = GHOST_COLOR_FREE;
    int32_t altar = type == BUILDING_TEMPLE_COMPLEX_ALTAR;
    int32_t place = altar ? image_group(IMAGE_GROUP_TEMPLE_COMPLEX_ALTAR) : image_group(IMAGE_GROUP_TEMPLE_COMPLEX_ORACLE);
    int32_t on_place = altar ? ALTAR_ON_PLACE : ORACLE_ON_PLACE;
    int32_t on_other = altar ? ALTAR_ON_OTHER : ORACLE_ON_OTHER;
    while (part != 0) {
        const building *b = building_get(part);
        int32_t image = map_grids.images[map_info.grid_start + b->x + (b->y * GRID_SIZE)];
        int32_t offset;
        if (image == place) {
            offset = on_place;
        } else if (image == place + UPGRADE_OTHER_PLACE_IMAGE) {
            offset = on_other;
        } else {
            part = b->next_part_building_id;
            continue;
        }
        int32_t ux;
        int32_t uy;
        city_view_tile_to_pixel(b->x + dx, b->y + dy, &ux, &uy);
        ghost_image_draw(image_group(IMAGE_GROUP_TEMPLE_COMPLEX_UPGRADE) + offset, ux, uy);
        ghost_image_draw_top(image_group(IMAGE_GROUP_TEMPLE_COMPLEX_UPGRADE) + offset, ux, uy);
        break;
    }
    city_view.color_mask = 0;
}

void city_building_ghost_draw_temple_upgrade(int32_t type)
{
    int32_t tiles = ghost_tiles_of_size(UPGRADE_FOOTPRINT_SIZE);
    int32_t view = map_info.orientation / VIEW_ORIENTATION_STEP;
    int32_t px;
    int32_t py;
    city_view_tile_to_pixel(map_info.hover_x, map_info.hover_y, &px, &py);
    int32_t py_raised = py + city_view.tile_half_height;
    int32_t hover = map_info.hover_offset;
    int32_t part = building_main(map_grids.building_ids[hover]);
    int32_t blocked = upgrade_blocked(type, building_get(part)->type);
    if (city_player_current()->treasury <= GHOST_TREASURY_BROKE) {
        blocked = 1;
    } else if (!blocked) {
        draw_upgrade_on_complex(type, part);
        return;
    }
    for (int32_t i = 0; i < tiles; i++) {
        int32_t offset = hover + ghost_view_tile_offset(view, i);
        int32_t not_free = map_grids.figure_ids[offset] != 0 || (map_grids.terrain[offset] & TERRAIN_NOT_CLEAR) != 0;
        int32_t color = (blocked || not_free) ? GHOST_COLOR_BLOCKED : GHOST_COLOR_FREE;
        ghost_image_draw_masked(image_group(IMAGE_GROUP_FLAT_TILE), ghost_flat_tile_dx(i) + px, ghost_flat_tile_dy(i) - city_view.tile_half_height + py_raised, color);
    }
    city_view.color_mask = 0;
}
