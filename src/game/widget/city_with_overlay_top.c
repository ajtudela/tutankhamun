// The top pass of the city view under an overlay, row by row: the figures, then the tops (bridges, the
// overlays' own tiles, the tops of the buildings the overlay shows and the column over the others), then the
// animations of the shown buildings.
//
// The figure loop takes its row and column back from city_view after drawing a figure.

#include "widget/city_with_overlay_top.h"
#include "building/animation_offset.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/city_building_anims.h"
#include "widget/city_building_sprites.h"
#include "widget/city_figure.h"
#include "widget/city_overlay_columns.h"
#include "widget/city_overlay_custom.h"
#include "widget/city_overlay_goods.h"
#include "widget/city_view.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum overlay_top_walk {
    TOP_ROWS_BEFORE = 8,
    TOP_ROWS_EXTRA = 18,
    TOP_COLS_BEFORE = 4,
    TOP_COLS_EXTRA = 6,
    TOP_HALF_TILES_ABOVE = 9,
    TOP_TILE_GAP_X = 2,
    TOP_NO_HIGHLIGHT = 9999,
};

enum overlay_top_tile {
    TOP_SIZE_MASK = 0xF,
    TOMB_GROUND_RAISED_FIRST = 48,
    TOMB_GROUND_RAISED_END = 72,
    TOMB_GROUND_RAISE = 150,
    VENUE_TOP_TERRAIN = TERRAIN_ROAD | TERRAIN_GARDEN,
};

static int32_t row_start_x(int32_t odd_row)
{
    int32_t x = city_view.origin_x;
    if (odd_row) {
        x = (city_view.origin_x - city_view.tile_half_width) - 1;
    }
    return x - ((TOP_TILE_GAP_X + city_view.tile_width) * TOP_COLS_BEFORE);
}

static int32_t is_tomb(int32_t type)
{
    return type == BUILDING_SMALL_ROYAL_TOMB || type == BUILDING_MEDIUM_ROYAL_TOMB || type == BUILDING_LARGE_ROYAL_TOMB || type == BUILDING_GRAND_ROYAL_TOMB ||
        type == BUILDING_ABU_SIMBEL;
}

static int32_t is_venue(int32_t type)
{
    return type == BUILDING_PAVILION || type == BUILDING_BANDSTAND || type == BUILDING_BOOTH;
}

static int32_t draw_raised_tomb_ground(int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    if (g_last_image_id < image_group(IMAGE_GROUP_ROYAL_TOMB_GROUND) + TOMB_GROUND_RAISED_FIRST || g_last_image_id >= image_group(IMAGE_GROUP_ROYAL_TOMB_GROUND) + TOMB_GROUND_RAISED_END) {
        return 0;
    }
    image_draw(g_last_image_id, x, y - TOMB_GROUND_RAISE);
    image_draw_isometric_top(g_last_image_id, x, y - TOMB_GROUND_RAISE);
    return 1;
}

static void draw_top_by_size(int32_t x, int32_t y, int32_t larger_at_tile)
{
    int32_t dx;
    int32_t dy;
    if (city_view_footprint_offset(map_grids.bitfields[city_view.grid_offset] & TOP_SIZE_MASK, &dx, &dy)) {
        image_draw_isometric_top(g_last_image_id, x + dx, y - dy);
    } else if (larger_at_tile) {
        image_draw_isometric_top(g_last_image_id, x, y);
    }
}

static void draw_figures(int32_t odd_row, int32_t y)
{
    int32_t x = row_start_x(odd_row);
    int32_t row = city_view.row;
    int32_t col = city_view.camera_col - TOP_COLS_BEFORE;
    for (int32_t col_number = 0; col_number <= city_view.width_tiles + TOP_COLS_EXTRA; col_number++) {
        if (col >= 0 && col < VIEW_COLS) {
            city_view.grid_offset = city_view.lookup[(row * VIEW_COLS) + col];
            if (city_view.grid_offset < VIEW_LOOKUP_EMPTY) {
                int32_t figure_id = map_grids.figure_ids[city_view.grid_offset];
                city_view.col = col;
                while (figure_id > 0) {
                    const figure *f = figure_get(figure_id);
                    if (f->is_ghost == 0 && f->pyramid_walk_mode == 0) {
                        city_draw_figure(figure_id, x, y, TOP_NO_HIGHLIGHT);
                        col = city_view.col;
                        row = city_view.row;
                    }
                    figure_id = f->next_figure_id_on_same_tile;
                }
            }
        }
        col++;
        x += TOP_TILE_GAP_X + city_view.tile_width;
    }
}

static void draw_overlay_column(int32_t building_id, int32_t x, int32_t y)
{
    switch (city_overlay) {
        case OVERLAY_FIRE:
            city_overlay_column_fire(building_id, x, y);
            break;
        case OVERLAY_DAMAGE:
            city_overlay_column_damage(building_id, x, y);
            break;
        case OVERLAY_CRIME:
            city_overlay_column_crime(building_id, x, y);
            break;
        case OVERLAY_DISEASE:
            city_overlay_column_disease(building_id, x, y);
            break;
        case OVERLAY_MALARIA:
            city_overlay_column_malaria(building_id, x, y);
            break;
        case OVERLAY_ENTERTAINMENT:
            city_overlay_column_entertainment(building_id, x, y);
            break;
        case OVERLAY_EDUCATION:
            city_overlay_column_education(building_id, x, y);
            break;
        case OVERLAY_JUGGLER:
            city_overlay_column_booth(building_id, x, y);
            break;
        case OVERLAY_MUSICIAN:
            city_overlay_column_bandstand(building_id, x, y);
            break;
        case OVERLAY_DANCER:
            city_overlay_column_pavilion(building_id, x, y);
            break;
        case OVERLAY_SENET:
            city_overlay_column_senet(building_id, x, y);
            break;
        case OVERLAY_ZOOKEEPER:
            city_overlay_column_zoo(building_id, x, y);
            break;
        case OVERLAY_BAZAAR_ACCESS:
            city_overlay_column_bazaar_access(building_id, x, y);
            break;
        case OVERLAY_RELIGION:
            city_overlay_column_religion(building_id, x, y);
            break;
        case OVERLAY_SCRIBAL_SCHOOL:
            city_overlay_column_school(building_id, x, y);
            break;
        case OVERLAY_LIBRARY:
            city_overlay_column_library(building_id, x, y);
            break;
        case OVERLAY_WATER_CROSSINGS:
            break;
        case OVERLAY_DENTIST:
            city_overlay_column_dentist(building_id, x, y);
            break;
        case OVERLAY_WATER:
            city_overlay_column_water(building_id, x, y);
            break;
        case OVERLAY_APOTHECARY:
            city_overlay_column_apothecary(building_id, x, y);
            break;
        case OVERLAY_PHYSICIAN:
            city_overlay_column_physician(building_id, x, y);
            break;
        case OVERLAY_MORTUARY:
            city_overlay_column_mortuary(building_id, x, y);
            break;
        case OVERLAY_TAX_INCOME:
            city_overlay_column_tax_income(building_id, x, y);
            break;
        case OVERLAY_LABOR:
            city_overlay_column_labor(building_id, x, y);
            break;
        case OVERLAY_PROBLEMS:
            city_overlay_column_problems(building_id, x, y);
            break;
        case OVERLAY_INFECTED_HOUSING:
            city_overlay_column_infected_housing(building_id, x, y);
            break;
        case OVERLAY_MAGISTRATE:
            city_overlay_column_courthouse(building_id, x, y);
            break;
        default:
            break;
    }
}

static int32_t top_shown_without_building(uint32_t terrain)
{
    return (terrain & TERRAIN_WALKABLE_BUILDING) == 0 && ((terrain & TERRAIN_GROUNDWATER) == 0 || city_overlay != OVERLAY_WATER) &&
        ((terrain & TERRAIN_WALL_KINDS) == 0 || city_overlay == OVERLAY_CITY_DEFENSES);
}

static void draw_building_top(int32_t tomb, int32_t x, int32_t y)
{
    if ((map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) == 0) {
        return;
    }
    if (city_overlay == OVERLAY_WATER && !tomb) {
        city_overlay_water_top(x, y);
    }
    if (draw_raised_tomb_ground(x, y)) {
        return;
    }
    if (city_overlay == OVERLAY_HIDE_CLIFFS || overlay_show_all != 0) {
        city_overlay_tombs_top(x, y);
        return;
    }
    if (city_overlay == OVERLAY_NATIVE) {
        return;
    }
    int32_t building_id = map_grids.building_ids[city_view.grid_offset];
    uint32_t terrain = map_grids.terrain[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (is_tomb(b->type)) {
        draw_top_by_size(x, y, 0);
        return;
    }
    if ((terrain & TERRAIN_BUILDING) == 0) {
        if (top_shown_without_building(terrain)) {
            draw_top_by_size(x, y, 0);
        }
        return;
    }
    if (building_id == 0) {
        return;
    }
    if (city_with_overlay_show_building(building_id) == 0) {
        int32_t type = b->type;
        if (!is_venue(type) || (map_grids.terrain[city_view.grid_offset] & VENUE_TOP_TERRAIN) == 0) {
            if (type != BUILDING_FESTIVAL_SQUARE) {
                draw_overlay_column(building_id, x, y);
            }
            return;
        }
    }
    g_last_image_id = map_grids.images[city_view.grid_offset];
    draw_top_by_size(x, y, 1);
}

static int32_t draw_own_overlay_tile(int32_t x, int32_t y)
{
    goods_overlay goods;
    if (city_overlay == OVERLAY_DESIRABILITY) {
        city_overlay_desirability_top(x, y);
        draw_raised_tomb_ground(x, y);
        return 1;
    }
    if (city_overlay == OVERLAY_FERTILITY) {
        city_overlay_fertility_top(x, y);
        return 1;
    }
    if (city_overlay_goods(city_overlay, &goods)) {
        city_overlay_resource(x, y, goods.resource, goods.inventory_slot);
        return 1;
    }
    return 0;
}

static void draw_tops(int32_t odd_row, int32_t y)
{
    int32_t x = row_start_x(odd_row);
    city_view.col = city_view.camera_col - TOP_COLS_BEFORE;
    for (int32_t col = 0; col <= city_view.width_tiles + TOP_COLS_EXTRA; col++, x += TOP_TILE_GAP_X + city_view.tile_width, city_view.col++) {
        if (city_view.col < 0 || city_view.col >= VIEW_COLS) {
            continue;
        }
        city_view.grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
        if (city_view.grid_offset >= VIEW_LOOKUP_EMPTY) {
            continue;
        }
        int32_t tomb = is_tomb(building_get(map_grids.building_ids[city_view.grid_offset])->type);
        if ((map_grids.terrain[city_view.grid_offset] & (TERRAIN_WATER | TERRAIN_ROAD)) == (TERRAIN_WATER | TERRAIN_ROAD) && map_grids.sprite[city_view.grid_offset] != 0) {
            image_draw((map_grids.sprite[city_view.grid_offset] - 1) + image_group(IMAGE_GROUP_BRIDGE), x, y - city_view.tile_height);
            continue;
        }
        if (!tomb && draw_own_overlay_tile(x, y)) {
            continue;
        }
        draw_building_top(tomb, x, y);
    }
}

static void draw_animation(int32_t building_id, int32_t image_id, int32_t x, int32_t y)
{
    const building *b = building_get(building_id);
    if (b->type == BUILDING_GRANARY || b->type == BUILDING_GRANARY_UP) {
        draw_granary_stores(building_id, city_view.grid_offset, x, y);
        return;
    }
    int32_t frame = building_animation_offset(image_id, city_view.grid_offset);
    if (frame <= 0) {
        return;
    }
    int16_t frames = image_record_animation_frames(image_id < image_collection.image_count ? image_id : 0);
    if (frames < frame) {
        frame = frames;
    }
    int32_t size = map_grids.bitfields[city_view.grid_offset] & TOP_SIZE_MASK;
    if (building_is_floodplain_farm(building_id) == 0) {
        building_draw_type_anim(b, image_id, frame, size, x, y);
    }
}

static void draw_animations(int32_t odd_row, int32_t y)
{
    int32_t x = row_start_x(odd_row);
    city_view.col = city_view.camera_col - TOP_COLS_BEFORE;
    for (int32_t col = 0; col <= city_view.width_tiles + TOP_COLS_EXTRA; col++, x += TOP_TILE_GAP_X + city_view.tile_width, city_view.col++) {
        if (city_view.col < 0 || city_view.col >= VIEW_COLS) {
            continue;
        }
        city_view.grid_offset = city_view.lookup[(city_view.row * VIEW_COLS) + city_view.col];
        if (city_view.grid_offset >= VIEW_LOOKUP_EMPTY) {
            continue;
        }
        int32_t building_id = map_grids.building_ids[city_view.grid_offset];
        int32_t shown = building_id != 0 && city_with_overlay_show_building(building_id) != 0;
        int32_t image_id = map_grids.images[city_view.grid_offset];
        if (image_record_animation_frames(image_id < image_collection.image_count ? image_id : 0) == 0 || !shown) {
            continue;
        }
        uint8_t edge = map_grids.edge[city_view.grid_offset];
        int32_t type = building_get(building_id)->type;
        if (type == BUILDING_STORAGE_YARD || type == BUILDING_STORAGE_YARD_UP) {
            building_draw_storage_yard_roof(image_group(IMAGE_GROUP_STORAGE_YARD) + STORAGE_YARD_ROOF_IMAGE, x, y);
        }
        if ((edge & MAP_EDGE_DRAW_TILE) != 0) {
            draw_animation(building_id, image_id, x, y);
            draw_workshop_raw_material_storage(building_id, x, y);
        }
    }
}

void city_with_overlay_draw_top(void)
{
    int32_t y = city_view.origin_y - (city_view.tile_half_height * TOP_HALF_TILES_ABOVE);
    int32_t odd_row = 0;
    city_view.row = city_view.camera_row - TOP_ROWS_BEFORE;
    for (int32_t row = 0; row < city_view.height_tiles + TOP_ROWS_EXTRA; row++) {
        if (city_view.row >= 0 && city_view.row < VIEW_ROWS) {
            draw_figures(odd_row, y);
            draw_tops(odd_row, y);
            draw_animations(odd_row, y);
        }
        odd_row = !odd_row;
        y += city_view.tile_half_height;
        city_view.row++;
    }
}
