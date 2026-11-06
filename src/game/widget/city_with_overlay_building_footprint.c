// A building's flattened footprint under an overlay, for the city view and the minimap: a column of the
// overlay's height at the building's top tile, or the building's overlay tile (red while it burns) for overlays
// without a height. Returns 0 when the tile is not the building's drawn tile or there is no overlay tile.

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "game/game_internal.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "widget/city_overlay_goods.h"
#include "widget/city_overlay_heights.h"
#include "widget/city_with_overlay.h"
#include "widget/minimap_figures.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum footprint_column {
    FOOTPRINT_HEIGHT_MAX = 10,
    FOOTPRINT_IMAGES_PER_HEIGHT = 5,
    FOOTPRINT_COLOR_BURNING = 0xF863,
};

static int32_t overlay_height(int32_t building_id, int32_t grid_offset, int32_t *has_height)
{
    goods_overlay goods;
    *has_height = 1;
    if (city_overlay_goods(city_overlay, &goods)) {
        return city_overlay_resource_level(building_id, goods.resource, goods.inventory_slot);
    }
    switch (city_overlay) {
        case OVERLAY_RELIGION:
            return city_overlay_height_religion(building_id);
        case OVERLAY_FIRE:
            return city_overlay_height_fire(building_id);
        case OVERLAY_DAMAGE:
            if ((map_grids.terrain[grid_offset] & (TERRAIN_ROAD | TERRAIN_CANAL | TERRAIN_WALL)) == 0) {
                return city_overlay_height_damage(building_id);
            }
            return city_overlay_height_damage_terrain(grid_offset);
        case OVERLAY_CRIME:
            return city_overlay_height_crime(building_id);
        case OVERLAY_ENTERTAINMENT:
            return city_overlay_height_entertainment(building_id);
        case OVERLAY_JUGGLER:
            return city_overlay_height_booth(building_id);
        case OVERLAY_MUSICIAN:
            return city_overlay_height_bandstand(building_id);
        case OVERLAY_DANCER:
            return city_overlay_height_pavilion(building_id);
        case OVERLAY_SENET:
        case OVERLAY_ZOOKEEPER:
            return city_overlay_height_senet(building_id);
        case OVERLAY_EDUCATION:
            return city_overlay_height_education(building_id);
        case OVERLAY_SCRIBAL_SCHOOL:
            return city_overlay_height_school(building_id);
        case OVERLAY_LIBRARY:
            return city_overlay_height_library(building_id);
        case OVERLAY_WATER_CROSSINGS:
            return 0;
        case OVERLAY_DENTIST:
            return city_overlay_height_dentist(building_id);
        case OVERLAY_PHYSICIAN:
            return city_overlay_height_physician(building_id);
        case OVERLAY_APOTHECARY:
            return city_overlay_height_apothecary(building_id);
        case OVERLAY_MORTUARY:
            return city_overlay_height_mortuary(building_id);
        case OVERLAY_TAX_INCOME:
            return city_overlay_height_tax_income(building_id);
        case OVERLAY_BAZAAR_ACCESS:
            return city_overlay_height_bazaar_access(building_id);
        case OVERLAY_DESIRABILITY:
            return city_overlay_height_desirability(grid_offset);
        case OVERLAY_FERTILITY:
            return city_overlay_height_fertility(grid_offset);
        case OVERLAY_LABOR:
            return city_overlay_height_labor(building_id);
        case OVERLAY_NATIVE:
            return city_overlay_height_native(building_id);
        case OVERLAY_PROBLEMS:
            return city_overlay_height_problems(building_id);
        case OVERLAY_DISEASE:
            return city_overlay_height_disease(building_id);
        case OVERLAY_WATER:
            return city_overlay_height_water(building_id);
        case OVERLAY_MALARIA:
            return city_overlay_height_malaria(building_id);
        case OVERLAY_CITY_DEFENSES:
            return city_overlay_height_city_defenses(building_id);
        case OVERLAY_MAGISTRATE:
            return city_overlay_height_courthouse(building_id);
        default:
            *has_height = 0;
            return 0;
    }
}

static int32_t overlay_tile_image(const building *b)
{
    if (b->house_size != 0) {
        return image_group(IMAGE_GROUP_MINIMAP_HOUSE);
    }
    if (b->type == BUILDING_WATER_LIFT) {
        return image_group(IMAGE_GROUP_MINIMAP_CANAL) - 1;
    }
    int32_t tile = building_type_overlay_tile_index(b->type);
    if (tile < 0) {
        return -1;
    }
    return image_group(IMAGE_GROUP_MINIMAP_BUILDINGS) + (tile * FOOTPRINT_IMAGES_PER_HEIGHT);
}

static int32_t is_venue(int16_t type)
{
    return type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILION || type == BUILDING_FESTIVAL_SQUARE;
}

static void draw_footprint(int32_t image_id, int32_t size, int32_t x, int32_t y)
{
    switch (size) {
        case 0:
            image_draw_minimap_tile(image_id, x, y);
            return;
        case 1:
        case 2:
        case 3:
        case 4:
            image_draw(image_id + size, x, y - size);
            return;
        case 5:
            image_id += 2;
            image_draw(image_id, x + 3, y - 5);
            image_draw(image_id, x + 3, y + 1);
            image_draw(image_id, x, y - 2);
            image_draw(image_id, x + 6, y - 2);
            return;
        default:
            return;
    }
}

int32_t city_with_overlay_draw_building_footprint(int32_t building_id, int32_t x, int32_t y)
{
    int32_t grid_offset = city_view.grid_offset;
    if ((map_grids.edge[grid_offset] & MAP_EDGE_DRAW_TILE) == 0 || (map_grids.terrain[grid_offset] & TERRAIN_BUILDING) == 0) {
        return 0;
    }
    const building *b = building_get(building_id);
    int32_t has_height;
    int32_t image_id = overlay_height(building_id, grid_offset, &has_height);
    if (!has_height) {
        image_id = overlay_tile_image(b);
        if (image_id < 0) {
            return 0;
        }
        city_view.color_mask = b->fire_duration != 0 ? FOOTPRINT_COLOR_BURNING : 0;
    }
    if (city_overlay != OVERLAY_NONE && city_overlay != OVERLAY_HIDE_CLIFFS) {
        if (image_id > FOOTPRINT_HEIGHT_MAX) {
            image_id = FOOTPRINT_HEIGHT_MAX;
        }
        image_id = image_group(IMAGE_GROUP_COLUMN) + (image_id * FOOTPRINT_IMAGES_PER_HEIGHT);
    }
    if (is_venue(b->type) && (map_grids.terrain[grid_offset] & TERRAIN_ROAD) != 0) {
        image_id = image_group(IMAGE_GROUP_MINIMAP_ROAD);
    }
    draw_footprint(image_id, map_grids.bitfields[grid_offset] & MAP_BITFIELD_SIZE, x, y);
    city_view.color_mask = 0;
    return 1;
}
